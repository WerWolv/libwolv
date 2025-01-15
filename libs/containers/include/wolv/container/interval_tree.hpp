#pragma once

#include <wolv/types.hpp>

#include <map>
#include <vector>
#include <algorithm>
#include <limits>

namespace wolv::container {

    /**
     * @brief A tree structure that allows for fast searching of intervals.
     * @tparam Type The value type to be stored
     * @tparam Scalar The scalar type to be used for the interval start and end values
     * @tparam SearchRange The maximum range to search backwards to look for intervals that encompass other intervals
     */
    template<typename Type, std::integral Scalar = u64, i64 SearchRange = std::numeric_limits<i64>::max()>
    class IntervalTree {
    private:
        static_assert(SearchRange > 0, "SearchRange must be greater than 0");

        constexpr static bool TriviallyCopyable = std::is_trivially_copyable_v<Type>;
        constexpr static bool HandleEncompassedIntervals = SearchRange != std::numeric_limits<i64>::max();

        using FindType = std::remove_cvref_t<typename std::conditional<TriviallyCopyable, Type, const Type*>::type>;

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

        struct Data {
            Interval interval;
            FindType value;
        };

        constexpr IntervalTree() = default;

        /**
         * @brief Construct an interval tree from a list of interval/value pairs
         * @param init List of interval/value pairs
         */
        constexpr IntervalTree(std::initializer_list<InitType> &&init) {
            for (auto &item : init)
                this->insert(item.interval, std::move(item.value));
        }

        /**
         * @brief Inserts a new interval/value pair into the tree, copying the value into the tree
         * @param interval Interval of where to insert the value
         * @param value Value to insert
         */
        constexpr void insert(const Interval &interval, const Type &value) {
            this->m_intervals.insert({ interval.start, { interval.end, value } });
        }

        /**
         * @brief Inserts a new interval/value pair into the tree, moving the value into the tree
         * @param interval Interval of where to insert the value
         * @param value Value to insert
         */
        constexpr void emplace(const Interval &interval, Type &&value) {
            this->m_intervals.insert({ interval.start, { interval.end, std::move(value) } });
        }

        /**
         * @brief Clears the tree
         */
        constexpr void clear() {
            this->m_intervals.clear();
        }

        /**
         * @brief Returns the begin iterator of the tree
         * @return
         */
        constexpr auto begin() {
            return this->m_intervals.begin();
        }

        /**
         * @brief Returns the end iterator of the tree
         * @return
         */
        constexpr auto end() {
            return this->m_intervals.end();
        }

        /**
         * @brief Returns the begin iterator of the tree
         * @return
         */
        constexpr auto begin() const  {
            return this->m_intervals.begin();
        }

        /**
         * @brief Returns the end iterator of the tree
         * @return
         */
        constexpr auto end() const {
            return this->m_intervals.end();
        }

        /**
         * @brief Finds all intervals that overlap with the given interval
         * @note If T is not trivially copyable, the returned vector will contain pointers to the values in the tree
         * @param interval Interval to search for
         * @return Vector of all overlapping intervals and their values
         */
        constexpr std::vector<Data> overlapping(const Interval &interval) const {
            if (this->m_intervals.empty())
                return {};

            std::vector<Data> result;

            // Find the first interval that starts after the given interval
            auto it = this->m_intervals.upper_bound(interval.end);
            if (it == this->m_intervals.begin())
                return {};

            it--;

            // Iterate through all intervals that overlap with the given interval
            for (i64 i = 0; i < SearchRange; i++) {
                const auto &[start, content] = *it;
                const auto &[end, value] = content;

                if constexpr (!HandleEncompassedIntervals) {
                    // If we don't care about intervals that encompass smaller intervals, we can stop
                    if (end < interval.start)
                        break;
                }

                // If the interval overlaps with the given interval, add it to the result
                // If we don't care about encompassing intervals, we can skip this check
                if (!HandleEncompassedIntervals || interval.overlaps({ start, end })) {
                    if constexpr (TriviallyCopyable)
                        result.push_back({ { start, end }, value });
                    else
                        result.push_back({ { start, end }, std::addressof(value) });

                    i -= 1;
                }

                // If we've reached the start of the tree, we can stop
                if (it == this->m_intervals.begin())
                    break;

                // Move to the previous interval
                --it;
            }

            return result;
        }

        size_t size() const {
            return m_intervals.size();
        }

    private:
        std::multimap<Scalar, std::pair<Scalar, Type>> m_intervals;
    };

}