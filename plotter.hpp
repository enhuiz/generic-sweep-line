#ifndef PLOTTER_HPP
#define PLOTTER_HPP

#include <sstream>
#include <memory>
#include <fstream>
#include <array>
#include <string>
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
        reset_unit();
    }

    void append_point(double x, double y)
    {
        unit["points"].push_back({x, y});
    }

    void flush() 
    {
        auto dumped =  units.dump();
        units.clear();
        call(dumped);
    }

    friend Plotter &show(Plotter &out);
    friend Plotter &endu(Plotter &out);
    friend Plotter &ln(Plotter &out);

protected:
    void virtual call(const std::string& s) = 0;

    void reset_unit()
    {
        static std::vector<std::string> items = {
            "points",
            "color",
            "type",
        };
        unit.clear();
        for (const auto &item : items)
        {
            unit[item] = {};
        }
        unit["type"] = "pt";
    }

    nlohmann::json unit;
    nlohmann::json units;
};

Plotter &show(Plotter &out)
{
    out.flush();
    return out;
}

Plotter &endu(Plotter &out)
{
    if (out.unit["points"].size() > 0)
    out.units.push_back(out.unit);
    out.reset_unit();
    return out;
}

Plotter &ln(Plotter &out)
{
    out.unit["type"] = "ln";
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