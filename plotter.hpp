#ifndef PLOTTER_HPP
#define PLOTTER_HPP

#include <sstream>
#include <memory>
#include <fstream>
#include <functional>
#include <array>
#include <string>
#include <iostream>
#include <vector>
#include <map>

#include "json.hpp"

namespace plt
{
class Plotter
{
  public:
    Plotter()
    {
    }

    void flush()
    {
        end_prev_point();
        nlohmann::json dumper = {
            {"points", points},
        };
        call(dumper.dump());
    }

    void end_prev_point()
    {
        if (point != nlohmann::json{})
        {
            points.push_back(point);
            point = {};
        }
    }

    friend Plotter &show(Plotter &out);
    friend Plotter &clean(Plotter &out);
    friend Plotter &beg_ln(Plotter &out);
    friend Plotter &end_ln(Plotter &out);
    friend auto new_pt(double x, double y);
    friend auto pt_size(double radius);
    friend auto pt_color(std::string color);
    friend auto ln_color(std::string color);

  protected:
    void virtual call(const std::string &s) = 0;

    std::vector<std::function<Plotter &(Plotter &)>> applied;

    nlohmann::json point;
    nlohmann::json points;
};

Plotter &show(Plotter &out)
{
    out.flush();
    return out;
}

Plotter &clean(Plotter &out)
{
    out.applied.clear();
    return out;
}

Plotter &beg_ln(Plotter &out)
{
    static int lnk_id_generator = 0;
    int lnk_id = lnk_id_generator++;
    out.applied.push_back([lnk_id](Plotter &out) -> Plotter & {
        out.point["lnk_id"] = lnk_id;
        return out;
    });
    return out;
}

Plotter &end_ln(Plotter &out)
{
    out.applied.push_back([](Plotter &out) -> Plotter & {
        out.point["lnk_id"] = -1;
        return out;
    });
    return out;
}

auto new_pt(double x, double y)
{
    return [x, y](Plotter &out) -> Plotter & {
        out.end_prev_point();
        out.point = {
            {"xy", {x, y}}};
        for (auto f : out.applied)
            f(out);
        return out;
    };
}

auto pt_size(double radius)
{
    return [&radius](Plotter &out) {
        out.applied.push_back([&radius](Plotter &out) -> Plotter & {
            out.point["radius"] = radius;
            return out;
        });
    };
}

auto pt_color(std::string color)
{
    return [&color](Plotter &out) -> Plotter & {
        out.applied.push_back([&color](Plotter &out) -> Plotter & {
            out.point["pt_color"] = color;
            return out;
        });
        return out;
    };
}

auto ln_color(std::string color)
{
    return [&color](Plotter &out) -> Plotter & {
        out.applied.push_back([&color](Plotter &out) -> Plotter & {
            out.point["ln_color"] = color;
            return out;
        });
        return out;
    };
}

Plotter &operator<<(Plotter &pout, std::function<Plotter &(Plotter &)> op)
{
    return op(pout);
}

class PyPlotter : public Plotter
{
  public:
    void call(const std::string &dumped)
    {
        {
            std::ofstream ofs("tmp.txt");
            ofs << dumped;
        }
        std::cout << exec("python3 plot.py tmp.txt");
        exec("rm tmp.txt");
    }

  private:
    std::string exec(const std::string &cmd) const
    {
        std::array<char, 128> buffer;
        std::string result;
        std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe)
            throw std::runtime_error("popen() failed!");
        while (!feof(pipe.get()))
        {
            if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
                result += buffer.data();
        }
        return result;
    }
};

extern PyPlotter pout;
PyPlotter pout;
}

#endif