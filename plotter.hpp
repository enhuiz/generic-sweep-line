#ifndef PLOTTER_HPP
#define PLOTTER_HPP

#include <sstream>
#include <memory>
#include <fstream>
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

    void append_point(double x, double y)
    {
        points.push_back({x, y});
        if (draw_line)
            line.push_back({x, y});
    }

    void flush() 
    {
        nlohmann::json dumper = {
            {"points", points},
            {"lines", lines},
        };
        points.clear();
        lines.clear();
        call(dumper.dump());
    }

    friend Plotter &show(Plotter &out);
    friend Plotter &endln(Plotter &out);
    friend Plotter &begln(Plotter &out);

protected:
    void virtual call(const std::string& s) = 0;

    nlohmann::json points;
    nlohmann::json line;
    nlohmann::json lines;

    bool draw_line = false;
};

Plotter &show(Plotter &out)
{
    out.flush();
    return out;
}

Plotter &endln(Plotter &out)
{
    out.draw_line = false;
    out.lines.push_back(std::move(out.line));
    assert(out.line.size() == 0);
    return out;
}

Plotter &begln(Plotter &out)
{
    out.draw_line = true;
    out.line.clear();
    return out;
}

Plotter &operator<<(Plotter &pout, decltype(show) op)
{
    return op(pout);
}

class PyPlotter : public Plotter
{
public:
    void call(const std::string& dumped)
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