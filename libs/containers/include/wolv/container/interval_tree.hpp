#pragma once

#include <wolv/types.hpp>

#include <map>
#include <vector>
#include <algorithm>
#include <limits>
#include <numeric>
#include <optional>
#include <type_traits>
#include <concepts>


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
            this->m_intervals.push_back({ interval, value });
            this->m_indexValid = false;
        }

        /**
         * @brief Inserts a new interval/value pair into the tree, moving the value into the tree
         * @param interval Interval of where to insert the value
         * @param value Value to insert
         */
        constexpr void emplace(const Interval &interval, Type &&value) {
            this->m_intervals.push_back({ interval, std::move(value) });
            this->m_indexValid = false;
        }

        /**
         * @brief Returns the nearest interval located after the searchIndex, if it exists
         * @param searchIndex Index from which to begin looking for the next interval
         */
        constexpr std::optional<Data> nextInterval(const Scalar searchIndex) const {
            this->ensureIndex();

            auto iter = std::upper_bound(this->m_sortedIntervals.begin(), this->m_sortedIntervals.end(), searchIndex, [this](const Scalar value, const size_t index) {
                return value < this->m_intervals[index].interval.start;
            });

            if (iter == this->m_sortedIntervals.end())
                return std::nullopt;

            // Can't simply return it in one line due to std::optional
            return this->toData(this->m_intervals[*iter]);
        }

        /**
         * @brief Returns the nearest interval located before the searchIndex, if it exists
         * @param searchIndex Index from which to begin looking for the previous interval
         */
        constexpr std::optional<Data> prevInterval(const Scalar searchIndex) const {
            this->ensureIndex();

            auto iter = std::lower_bound(this->m_sortedIntervals.begin(), this->m_sortedIntervals.end(), searchIndex, [this](const size_t index, const Scalar value) {
                return this->m_intervals[index].interval.start < value;
            });

            if (iter == this->m_sortedIntervals.begin())
                return std::nullopt;

            --iter; // We need to go back one to get the previous iterator

            // Can't simply return it in one line due to std::optional
            return this->toData(this->m_intervals[*iter]);
        }

        /**
         * @brief Clears the tree
         */
        constexpr void clear() {
            this->m_intervals.clear();
            this->m_sortedIntervals.clear();
            this->m_nodes.clear();
            this->m_indexValid = true;
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

            this->ensureIndex();

            std::vector<Data> result;
            this->overlapping(0, interval, result);

            std::ranges::sort(result, [](const Data &left, const Data &right) {
                if (left.interval.start != right.interval.start)
                    return left.interval.start > right.interval.start;

                return left.interval.end < right.interval.end;
            });

            return result;
        }

        size_t size() const {
            return m_intervals.size();
        }

        bool empty() const {
            return m_intervals.empty();
        }

    private:
        struct StoredInterval {
            Interval interval;
            Type value;
        };

        constexpr static size_t InvalidNode = std::numeric_limits<size_t>::max();

        struct Node {
            size_t begin = 0;
            size_t end = 0;
            size_t left = InvalidNode;
            size_t right = InvalidNode;
            Scalar maxEnd = std::numeric_limits<Scalar>::lowest();
        };

        constexpr Data toData(const StoredInterval &storedInterval) const {
            if constexpr (TriviallyCopyable)
                return { storedInterval.interval, storedInterval.value };
            else
                return { storedInterval.interval, std::addressof(storedInterval.value) };
        }

        constexpr void ensureIndex() const {
            if (this->m_indexValid)
                return;

            this->m_sortedIntervals.resize(this->m_intervals.size());
            std::iota(this->m_sortedIntervals.begin(), this->m_sortedIntervals.end(), size_t(0));
            std::ranges::stable_sort(this->m_sortedIntervals, [this](size_t left, size_t right) {
                const auto &leftInterval = this->m_intervals[left].interval;
                const auto &rightInterval = this->m_intervals[right].interval;

                if (leftInterval.start != rightInterval.start)
                    return leftInterval.start < rightInterval.start;

                return leftInterval.end < rightInterval.end;
            });

            this->m_nodes.clear();
            if (!this->m_sortedIntervals.empty())
                this->buildNode(0, this->m_sortedIntervals.size());

            this->m_indexValid = true;
        }

        constexpr size_t buildNode(size_t begin, size_t end) const {
            const auto nodeIndex = this->m_nodes.size();
            this->m_nodes.push_back({
                .begin = begin,
                .end = end,
            });

            if (end - begin == 1) {
                this->m_nodes[nodeIndex].maxEnd = this->m_intervals[this->m_sortedIntervals[begin]].interval.end;
            } else {
                const auto middle = begin + (end - begin) / 2;
                const auto left = this->buildNode(begin, middle);
                const auto right = this->buildNode(middle, end);

                this->m_nodes[nodeIndex].left = left;
                this->m_nodes[nodeIndex].right = right;
                this->m_nodes[nodeIndex].maxEnd = std::max(this->m_nodes[left].maxEnd, this->m_nodes[right].maxEnd);
            }

            return nodeIndex;
        }

        constexpr void overlapping(size_t nodeIndex, const Interval &interval, std::vector<Data> &result) const {
            const auto &node = this->m_nodes[nodeIndex];
            const auto minStart = this->m_intervals[this->m_sortedIntervals[node.begin]].interval.start;

            if (node.maxEnd < interval.start || minStart > interval.end)
                return;

            if (node.end - node.begin == 1) {
                const auto &storedInterval = this->m_intervals[this->m_sortedIntervals[node.begin]];
                if (interval.overlaps(storedInterval.interval))
                    result.push_back(this->toData(storedInterval));
                return;
            }

            this->overlapping(node.left, interval, result);
            this->overlapping(node.right, interval, result);
        }

        std::vector<StoredInterval> m_intervals;
        mutable std::vector<size_t> m_sortedIntervals;
        mutable std::vector<Node> m_nodes;
        mutable bool m_indexValid = true;
    };

}
