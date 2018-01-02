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

using Point = vector2<double>;

struct Segment
{
    Point a, b;

    Point key; // compare point

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

    Point just_below(const Point &p) const
    {
        auto v = normal(lower_endpoint() - upper_endpoint());
        return p + v;
    }

    vector2<double> direction() const
    {
        return b - a;
    }
};

shared_ptr<Point> intersection(const Segment &s1, const Segment &s2)
{
    auto left = [](const Point &p, const Segment &s) {
        return cross(s.direction(), s.a - p) < 0;
    };

    auto ptr = shared_ptr<Point>(nullptr);

    if (left(s1.a, s2) != left(s1.b, s2) && left(s2.a, s1) != left(s2.b, s1))
    {
        auto intersection = [](const Point &a, const Point &b, const Point &c, const Point &d) {
            auto x_diff = vector2<double>(a.x - b.x, c.x - d.x);
            auto y_diff = vector2<double>(a.y - b.y, c.y - d.y);
            double det = cross(x_diff, y_diff);
            auto cr = Point(cross(a, b), cross(c, d));
            auto e = Point(cross(cr, x_diff) / det, cross(cr, y_diff) / det);
            return e;
        };
        ptr = make_shared<Point>(intersection(s1.a, s1.b, s2.a, s2.b));
    }

    return ptr;
}

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
    out << new_pt(point.x, point.y);
    return out;
}

Plotter &operator<<(Plotter &out, const Segment &segment)
{
    out << beg_ln << segment.a << segment.b << end_ln;
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

        for (auto segment : segments)
        {
            segment.key = segment.upper_endpoint();

            events.insert({
                segment.upper_endpoint(),
                Event::Type::upper,
                segment,
            });

            events.insert({
                segment.lower_endpoint(),
                Event::Type::lower,
            });
        }

        return events;
    }(); // multiset

    auto status = []() {
        auto cmp = [](const Segment &s1, const Segment &s2) {
            if (s1.key.x == s2.key.x)
            {
                auto p = s1.key;
                return s1.just_below(p).x < s2.just_below(p).x;
            }
            return s1.key.x < s2.key.x;
        };
        return set<Segment, decltype(cmp)>(cmp);
    }(); // set

    while (events.size())
    {
        const auto events_at_next_point = [&events]() {
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

        // report_intersections (events_at_next_point)
        {
            pout << events_at_next_point.front().point << show;
        }

        const auto event_filter = [&events_at_next_point](Event::Type type) {
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
        const auto upper_events = event_filter(Event::Type::upper);
        const auto intersection_events = event_filter(Event::Type::intersection);
        const auto lower_events = event_filter(Event::Type::lower);

        // delete and insert/re-insert the segments into status
        {
            const auto remove_event_segment_from_status = [&status](const vector<Event> &events) {
                for (const auto &event : events)
                {
                    status.erase(event.segment);
                }
            };

            remove_event_segment_from_status(lower_events);
            remove_event_segment_from_status(intersection_events);

            const auto insert_event_segment_to_status = [&status](const vector<Event> &events) {
                for (auto event : events)
                {
                    event.segment.key = event.point;
                    status.insert(event.segment);
                }
            };

            insert_event_segment_to_status(intersection_events);
            insert_event_segment_to_status(upper_events);
        }

        // update intersection in the new status
        {
            const auto append_new_event = [&events](const Segment &l, const Segment &r, const Point &pt) {
                auto ptr = intersection(l, r);
                if (ptr != nullptr)
                {
                    auto int_pt = *ptr;
                    if (vertically_less(int_pt, pt))
                    {
                        events.insert(Event{
                            int_pt,
                            Event::Type::intersection,
                            l});

                        events.insert(Event{
                            int_pt,
                            Event::Type::intersection,
                            r});
                    }
                }
            };
            const auto key_segment = Segment{
                Point(),
                Point(),
                events_at_next_point.front().point};
            const auto lower_it = status.lower_bound(key_segment);
            const auto upper_it = status.upper_bound(key_segment);
            if (lower_it != status.begin() && upper_it != status.end())
            {
                if (upper_events.size() + intersection_events.size() == 0)
                {
                    auto sl = *prev(lower_it);
                    auto sr = *upper_it;
                    append_new_event(sl, sr, key_segment.key);
                }
                else
                {
                    auto sll = *prev(lower_it);
                    auto sl = *lower_it;
                    append_new_event(sll, sl, key_segment.key);

                    if (next(upper_it) != status.end())
                    {
                        auto sr = *upper_it;
                        auto srr = *next(upper_it);
                        append_new_event(sr, srr, key_segment.key);
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    size_t num_segments = argc < 2 ? 5 : stoi(argv[1]);
    vector<Segment> segments;
    segments.reserve(num_segments);
    for (int i = 0; i < num_segments; ++i)
    {
        segments.push_back(random_segment());
    }
    sweep_line(segments);
    return 0;
}
