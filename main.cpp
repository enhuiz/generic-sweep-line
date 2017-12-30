#include <iostream>
#include <set>
#include <vector>
#include <memory>
#include <random>
#include <sstream>

#include "vector2.hpp"
#include "plotter.hpp"

using namespace std;
using namespace plt;

template <class T, template <class = T, class = allocator<T>> class Container>
ostream &operator<<(ostream &out, const Container<T, allocator<T>> &collection)
{
    out << "[";
    for (int i = 0; i < collection.size(); ++i)
    {
        if (i)
            out << ", ";
        out << collection[i];
    }
    out << "]";
    return out;
}

using Point = vector2<double>;

template <class T>
bool vertically_less(const T &a, const T &b)
{
    //  y
    //   ^
    //   |  .5
    //   |    .3 .4
    //   |    .2
    //   | .1
    //   ------> x
    return a.y < b.y || a.y == b.y && a.x < b.x;
}

struct Segment
{
    Point a, b;

    Point &operator[](size_t index)
    {
        assert(index < 2 && "index out of range");
        return index == 0 ? a : b;
    }

    const Point &operator[](size_t index) const
    {
        assert(index < 2 && "index out of range");
        return index == 0 ? a : b;
    }

    Point &upper_endpoint()
    {
        return vertically_less(a, b) ? b : a;
    }

    const Point &upper_endpoint() const
    {
        return vertically_less(a, b) ? b : a;
    }

    Point &lower_endpoint()
    {
        return vertically_less(a, b) ? a : b;
    }

    const Point &lower_endpoint() const
    {
        return vertically_less(a, b) ? a : b;
    }
};

Point random_point()
{
    auto generate = []() {
        static random_device rd;
        static default_random_engine g(rd());
        static normal_distribution<double> dist(0, 1);
        return dist(g);
    };
    return {generate(), generate()};
}

Segment random_segment()
{
    return {random_point(), random_point()};
}

Plotter &operator<<(Plotter &out, const Point &point)
{
    out.append_point(point.x, point.y);
    return out;
}

Plotter &operator<<(Plotter &out, const Segment &segment)
{
    out << ln << segment.a << segment.b << endu;
    return out;
}

Plotter &operator<<(Plotter &out, const vector<Segment> &segments)
{
    for (const auto &segment : segments)
    {
        out << segment;
    }
    return out;
}

void sweep_line(const vector<Segment> &segments)
{
    pout << segments << show;
    // plot(segments);

    struct Event
    {
        enum class Type
        {
            upper,
            lower,
            intersection,
        };
        Point point;
        Type type;
        Segment segment;
    };

    auto events = [&segments]() {
        // initialize Q
        auto v_less = [](const Event &a,
                         const Event &b) {
            return vertically_less(a.point, b.point);
        };

        auto events = multiset<Event,
                               decltype(v_less)>(v_less);

        for (const auto &segment : segments)
        {
            events.insert({
                segment.lower_endpoint(),
                Event::Type::lower,
            });

            events.insert({
                segment.upper_endpoint(),
                Event::Type::upper,
                segment,
            });
        }

        return events;
    }();

    auto status = []() {
        auto cmp = [](const Segment &s1, const Segment &s2) {
            return s1.upper_endpoint().x < s2.upper_endpoint().x;
        };
        return multiset<Segment, decltype(cmp)>(cmp);
    }();

    while (events.size())
    {
        auto events_at_next_point = [&events]() {
            auto result_events = vector<Event>();
            auto point = events.rbegin()->point;
            // there may be more than 1 event at this point
            while (events.size() > 0 &&
                   events.rbegin()->point == point)
            {
                auto event = *events.rbegin();
                result_events.push_back(event);
                events.erase(prev(events.end())); // i.e. erase rbegin
            }
            return result_events;
        }();

        // report_intersections(events_at_next_point);

        auto event_filter = [&events_at_next_point](Event::Type type) {
            auto result_events = vector<Event>();
            for (const auto &event : events_at_next_point)
            {
                if (event.type == type)
                {
                    result_events.push_back(event);
                }
            }
            return result_events;
        };

        auto upper_events = event_filter(Event::Type::upper);
        auto lower_events = event_filter(Event::Type::lower);
        auto intersection_events = event_filter(Event::Type::intersection);

        auto remove_event_segment_from_status = [&status](const vector<Event> &events) {
            for (const auto &event : events)
            {
                status.erase(event.segment);
            }
        };

        remove_event_segment_from_status(lower_events);
        remove_event_segment_from_status(intersection_events);
    }
}

int main(int argc, char *argv[])
{
    size_t num_segments = argc < 2 ? 10 : stoi(argv[1]);
    vector<Segment> segments;
    segments.reserve(num_segments);
    for (int i = 0; i < num_segments; ++i)
    {
        segments.push_back(random_segment());
    }
    sweep_line(segments);
    return 0;
}