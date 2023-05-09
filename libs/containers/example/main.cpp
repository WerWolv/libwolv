#include <wolv/container/interval_tree.hpp>

#include <cstdio>

int main() {

    wolv::container::IntervalTree<int> tree = {
            { { 0, 5 }, 69   },
            { { 1, 3 }, 420  },
            { { 2, 4 }, 1337 },
            { { 3, 6 }, 9001 },
            { { 6, 8 }, 8008 },
    };

    auto result = tree.overlapping({ 4, 5 });
    for (const auto &value : result)
        std::printf("%d\n", value);

}