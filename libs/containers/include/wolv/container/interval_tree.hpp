#pragma once

#include <wolv/types.hpp>

#include <map>
#include <vector>
#include <algorithm>

namespace wolv::container {

    template<typename Type, typename Scalar = u64, bool Encompassing = true>
    class IntervalTree {
    private:
        constexpr static bool TriviallyCopyable = std::is_trivially_copyable_v<Type>;
        using FindType = std::remove_cvref_t<typename std::conditional<TriviallyCopyable, Type, Type*>::type>;

    public:
        struct Interval {
            Scalar start, end;

            bool overlaps(const Interval& other) const {
                return end >= other.start && start <= other.end;
            }
            bool operator<(const Interval& other) const {
                return end < other.start;
            }
        };

        struct InitType {
            Interval interval;
            Type value;
        };

        constexpr IntervalTree() = default;
        constexpr IntervalTree(std::initializer_list<InitType> &&init) {
            for (auto &item : init)
                this->insert(item.interval, std::move(item.value));
        }

        constexpr void insert(const Interval &interval, const Type &value) {
            this->m_intervals.insert({ interval.start, { interval.end, value } });
        }

        constexpr void emplace(const Interval &interval, Type &&value) {
            this->m_intervals.insert({ interval.start, { interval.end, std::move(value) } });
        }

        constexpr std::vector<FindType> overlapping(const Interval &interval) const {
            std::vector<FindType> result;

            // Find the first interval that starts after the given interval
            auto it = this->m_intervals.upper_bound(interval.start);

            it--;

            // Iterate through all intervals that overlap with the given interval
            while (true) {
                const auto &[start, content] = *it;
                const auto &[end, value] = content;

                if constexpr (!Encompassing) {
                    // If we don't care about intervals that encompass smaller intervals, we can stop
                    if (end < interval.start)
                        break;
                }

                // If the interval overlaps with the given interval, add it to the result
                // If we don't care about encompassing intervals, we can skip this check
                if (!Encompassing || interval.overlaps({ start, end })) {
                    if constexpr (TriviallyCopyable)
                        result.push_back(value);
                    else
                        result.push_back(std::addressof(value));
                }

                // If we've reached the start of the tree, we can stop
                if (it == this->m_intervals.begin())
                    break;

                --it;
            }

            return result;
        }

    private:
        std::multimap<Scalar, std::pair<Scalar, Type>> m_intervals;
    };

}