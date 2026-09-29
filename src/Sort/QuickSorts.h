#pragma once
// Ports of ArrayV (Java, MIT) "quick" category sorting classes,
// https://github.com/Gaming32/ArrayV
// VisualSort - ternary (Dutch-flag) quicksort variants:
//   TernaryLLQuickSort - "Quick Sort (ternary, LL ptrs)" by Timo Bingmann, ArrayV port by Gaming32
//   TernaryLRQuickSort - "Quick Sort (ternary, LR ptrs)" by Timo Bingmann, ArrayV port by Gaming32
//
// ASCII ONLY: do not put non-ASCII characters in this file. MSVC parses sources in the
// system code page (GBK); use \uXXXX escapes if a wide string is ever needed.
#include "SortHelpers.h"
#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

namespace NVisualSort::NSortAlgorithms {

    // Quicksort (ternary, LL ptrs): port of TernaryLLQuickSort.runSort
    // (quickSortTernaryLL -> partitionTernaryLL -> selectPivot; median-of-3 pivot,
    // three-way partition, "LL" = both scan pointers start on the left).
    // Expected recursion depth is O(log n), so recursion is kept.
    template<class T = int>
    void TernaryLLQuickSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;

        const ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        // TernaryLLQuickSort.compare (Reads.compareIndices).
        auto compare = [&data_](ptrdiff_t lo, ptrdiff_t hi) -> int {
            return NSortHelpers::CompareValues(data_[lo], data_[hi]);
        };

        // TernaryLLQuickSort.selectPivot (median-of-3; hi is the exclusive end).
        auto selectPivot = [&compare](ptrdiff_t lo, ptrdiff_t hi) -> ptrdiff_t {
            ptrdiff_t mid = (lo + hi) / 2;

            if (compare(lo, mid) == 0)
                return lo;
            if (compare(lo, hi - 1) == 0 || compare(mid, hi - 1) == 0)
                return hi - 1;

            if (compare(lo, mid) < 0) {
                if (compare(mid, hi - 1) < 0) return mid;
                if (compare(lo, hi - 1) < 0) return hi - 1;
                return lo;
            }
            else {
                if (compare(mid, hi - 1) > 0) return mid;
                if (compare(lo, hi - 1) < 0) return lo;
                return hi - 1;
            }
        };

        // TernaryLLQuickSort.partitionTernaryLL; returns PivotPair (first, second).
        auto partitionTernaryLL = [&data_, &selectPivot](ptrdiff_t lo, ptrdiff_t hi) -> std::pair<ptrdiff_t, ptrdiff_t> {
            ptrdiff_t p = selectPivot(lo, hi);

            T pivot = data_[p];
            {
                using std::swap;
                swap(data_[p], data_[hi - 1]);
            }

            ptrdiff_t i = lo;
            ptrdiff_t k = hi - 1;

            for (ptrdiff_t j = lo; j < k; ++j) {
                int cmp = NSortHelpers::CompareValues(data_[j], pivot);
                if (cmp == 0) {
                    --k;
                    using std::swap;
                    swap(data_[k], data_[j]);
                    --j;
                }
                else if (cmp < 0) {
                    using std::swap;
                    swap(data_[i], data_[j]);
                    ++i;
                }
            }

            ptrdiff_t j = i + (hi - k);

            for (ptrdiff_t s = 0; s < hi - k; ++s) {
                using std::swap;
                swap(data_[i + s], data_[hi - 1 - s]);
            }

            return std::pair<ptrdiff_t, ptrdiff_t>(i, j);
        };

        // TernaryLLQuickSort.quickSortTernaryLL.
        auto quickSortTernaryLL = [&partitionTernaryLL](auto&& self, ptrdiff_t lo, ptrdiff_t hi) -> void {
            if (lo + 1 < hi) {
                std::pair<ptrdiff_t, ptrdiff_t> mid = partitionTernaryLL(lo, hi);

                self(self, lo, mid.first);
                self(self, mid.second, hi);
            }
        };

        quickSortTernaryLL(quickSortTernaryLL, 0, length);
    }

    // Quicksort (ternary, LR ptrs): port of TernaryLRQuickSort.runSort
    // (quickSortTernaryLR -> selectPivot; median-of-3 pivot, three-way partition,
    // "LR" = the two scan pointers move toward each other and equal keys are
    // collected at both ends). Expected recursion depth is O(log n), so recursion is kept.
    template<class T = int>
    void TernaryLRQuickSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;

        const ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        // TernaryLRQuickSort.compare (Reads.compareIndices).
        auto compare = [&data_](ptrdiff_t lo, ptrdiff_t hi) -> int {
            return NSortHelpers::CompareValues(data_[lo], data_[hi]);
        };

        // TernaryLRQuickSort.selectPivot (median-of-3; hi is the exclusive end).
        auto selectPivot = [&compare](ptrdiff_t lo, ptrdiff_t hi) -> ptrdiff_t {
            ptrdiff_t mid = (lo + hi) / 2;

            if (compare(lo, mid) == 0)
                return lo;
            if (compare(lo, hi - 1) == 0 || compare(mid, hi - 1) == 0)
                return hi - 1;

            if (compare(lo, mid) < 0) {
                if (compare(mid, hi - 1) < 0) return mid;
                if (compare(lo, hi - 1) < 0) return hi - 1;
                return lo;
            }
            else {
                if (compare(mid, hi - 1) > 0) return mid;
                if (compare(lo, hi - 1) < 0) return lo;
                return hi - 1;
            }
        };

        // TernaryLRQuickSort.quickSortTernaryLR (hi is inclusive here).
        auto quickSortTernaryLR = [&data_, &selectPivot](auto&& self, ptrdiff_t lo, ptrdiff_t hi) -> void {
            if (hi <= lo) return;

            int cmp;

            ptrdiff_t piv = selectPivot(lo, hi + 1);
            {
                using std::swap;
                swap(data_[piv], data_[hi]);
            }

            T pivot = data_[hi];

            ptrdiff_t i = lo;
            ptrdiff_t j = hi - 1;
            ptrdiff_t p = lo;
            ptrdiff_t q = hi - 1;


            for (;;) {
                while (i <= j && (cmp = NSortHelpers::CompareValues(data_[i], pivot)) <= 0) {
                    if (cmp == 0) {
                        using std::swap;
                        swap(data_[i], data_[p]);
                        ++p;
                    }
                    ++i;
                }

                while (i <= j && (cmp = NSortHelpers::CompareValues(data_[j], pivot)) >= 0) {
                    if (cmp == 0) {
                        using std::swap;
                        swap(data_[j], data_[q]);
                        --q;
                    }
                    --j;
                }

                if (i > j) break;

                {
                    using std::swap;
                    swap(data_[i], data_[j]);
                }
                ++i;
                --j;
            }

            {
                using std::swap;
                swap(data_[i], data_[hi]);
            }

            ptrdiff_t num_less = i - p;
            ptrdiff_t num_greater = q - j;

            j = i - 1;
            i = i + 1;

            ptrdiff_t pe = lo + (std::min)(p - lo, num_less);
            for (ptrdiff_t k = lo; k < pe; ++k, --j) {
                using std::swap;
                swap(data_[k], data_[j]);
            }

            ptrdiff_t qe = hi - 1 - (std::min)(hi - 1 - q, num_greater - 1);
            for (ptrdiff_t k = hi - 1; k > qe; --k, ++i) {
                using std::swap;
                swap(data_[i], data_[k]);
            }

            self(self, lo, lo + num_less - 1);
            self(self, hi - num_greater + 1, hi);
        };

        quickSortTernaryLR(quickSortTernaryLR, 0, length - 1);
    }

} // namespace NVisualSort::NSortAlgorithms
