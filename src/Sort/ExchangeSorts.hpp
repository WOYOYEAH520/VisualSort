#pragma once
// Ports of the ArrayV (Java, MIT) "Exchange Sorts" category classes
// (https://github.com/Gaming32/ArrayV, D:\Temp\sorts\exchange) into VisualSort.
//
// One function template per Java sort class, in NVisualSort::NSortAlgorithms.
// Randomness comes from NSortHelpers::SortRandomEngine (one engine per run);
// the parallel variants use std::thread + join.
//
// ASCII ONLY: this file contains no non-ASCII characters. If a wide message is
// ever needed it must use \uXXXX escapes (this category needs none).
//
// Included from Sort.h after SortHelpers.h; opens/closes its own namespaces so
// it also compiles stand-alone (see D:\Temp\check_exchange\main.cpp).
#include <algorithm>
#include <cstddef>
#include <random>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace NVisualSort::NSortAlgorithms {

    // Port of BinaryGnomeSort.runSort (optimized gnome sort + binary search).
    template<class T = int>
    void BinaryGnomeSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        for (ptrdiff_t i = 1; i < length; ++i) {
            T num = data_[i];

            ptrdiff_t lo = 0, hi = i;
            while (lo < hi) {
                ptrdiff_t mid = lo + ((hi - lo) / 2);


                if (num < data_[mid]) { // equal elements are not shifted past each other (stability)
                    hi = mid;
                }
                else {
                    lo = mid + 1;
                }
            }

            // item has to go into position lo
            ptrdiff_t j = i;
            while (j > lo) {
                swap(data_[j], data_[j - 1]);
                --j;
            }
        }
    }

    // Port of BubbleBogoSort.runSort (randomly swap adjacent out-of-order pairs).
    template<class T = int>
    void BubbleBogoSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());

        while (!NSortHelpers::isRangeSorted(data_, 0, length)) {
            ptrdiff_t index = NSortHelpers::bogoRandInt(engine, 0, length - 1);

            if (data_[index] > data_[index + 1])
                swap(data_[index], data_[index + 1]);
        }
    }

    // Port of BubbleSort.runSort. (Name collision with the existing C++ BubbleSort.)
    template<class T = int>
    void BubbleSortJava(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        for (ptrdiff_t i = length - 1; i > 0; --i) {
            bool sorted = true;
            for (ptrdiff_t j = 0; j < i; ++j) {
                if (data_[j] > data_[j + 1]) {
                    swap(data_[j], data_[j + 1]);
                    sorted = false;
                }

            }
            if (sorted) break;
        }
    }

    // Port of CircleSortIterative.runSort (repeats one iterative circle-sort pass
    // until a pass makes no swaps; the helper returns the range instead).
    template<class T = int>
    void CircleSortIterative(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        ptrdiff_t n = 1;
        for (; n < length; n *= 2);

        do {
            NSortHelpers::circleSortIterative(data_, n, length);
        } while (!NSortHelpers::isRangeSorted(data_, 0, length));
    }

    // Port of CircleSortRecursive.runSort.
    template<class T = int>
    void CircleSortRecursive(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        ptrdiff_t n = 1;
        for (; n < length; n *= 2);

        ptrdiff_t numberOfSwaps = 0;
        do {
            numberOfSwaps = NSortHelpers::circleSortRoutine(data_, 0, n - 1, length, 0);
        } while (numberOfSwaps != 0);
    }

    // Port of CircloidSort.runSort (circle pass over the whole range, repeated
    // until a full divide-and-conquer pass reports no swap).
    template<class T = int>
    void CircloidSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        auto circle = [&](ptrdiff_t left, ptrdiff_t right) -> bool {
            ptrdiff_t a = left;
            ptrdiff_t b = right;
            bool swapped = false;
            while (a < b) {
                if (data_[a] > data_[b]) {
                    swap(data_[a], data_[b]);
                    swapped = true;
                }
                ++a;
                --b;
                if (a == b) {
                    ++b;
                }
            }
            return swapped;
        };

        auto circlePass = [&](auto&& self, ptrdiff_t left, ptrdiff_t right) -> bool {
            if (left >= right) return false;
            ptrdiff_t mid = (left + right) / 2;
            bool l = self(self, left, mid);
            bool r = self(self, mid + 1, right);
            return circle(left, right) || l || r;
        };

        while (circlePass(circlePass, 0, length - 1));
    }

    // Port of ClassicThreeSmoothCombSort.runSort (descending 3-smooth gaps).
    template<class T = int>
    void ClassicThreeSmoothCombSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        auto is3Smooth = [&](ptrdiff_t n) -> bool {
            while (n % 6 == 0) n /= 6;
            while (n % 3 == 0) n /= 3;
            while (n % 2 == 0) n /= 2;

            return n == 1;
        };

        for (ptrdiff_t g = length - 1; g > 0; --g)
            if (is3Smooth(g))
                for (ptrdiff_t i = g; i < length; ++i)
                    if (data_[i - g] > data_[i])
                        swap(data_[i - g], data_[i]);
    }

    // Port of CocktailShakerSort.runSort (smartCocktailShaker, sleep 0.1).
    template<class T = int>
    void CocktailShakerSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        ptrdiff_t start = 0, end = length;
        ptrdiff_t i = start;
        while (i < ((end / 2) + start)) {
            bool sorted = true;
            for (ptrdiff_t j = i; j < end + start - i - 1; ++j) {
                if (data_[j] > data_[j + 1]) {
                    swap(data_[j], data_[j + 1]);
                    sorted = false;
                }

            }
            for (ptrdiff_t j = end + start - i - 1; j > i; --j) {
                if (data_[j] < data_[j - 1]) {
                    swap(data_[j], data_[j - 1]);
                    sorted = false;
                }

            }
            if (sorted) break;
            else ++i;
        }
    }

    // Port of CombSort.runSort (the Java bucketCount prompt defaults to 130 -> 1.3).
    // (Name collision with the existing C++ CombSort.)
    template<class T = int>
    void CombSortJava(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        NSortHelpers::combSort(data_, length, 1.3, false);
    }

    // Port of CompleteGraphSort.runSort (complete-graph sorting network).
    template<class T = int>
    void CompleteGraphSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        auto compSwap = [&](ptrdiff_t a, ptrdiff_t b) {
            if (data_[a] > data_[b])
                swap(data_[a], data_[b]);
        };

        auto split = [&](auto&& self, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) -> void {
            if (b - a < 2) return;

            ptrdiff_t c = 0, len1 = (b - a) / 2;
            bool odd = (b - a) % 2 == 1;

            if (odd) {
                if (m - a > b - m) { c = a; ++a; }
                else { --b; c = b; }
            }
            for (ptrdiff_t s = 0; s < len1; ++s) {
                ptrdiff_t i = a;

                for (ptrdiff_t j = s; j < len1; ++j) {
                    compSwap(i, m + j);
                    ++i;
                }
                for (ptrdiff_t j = 0; j < s; ++j) {
                    compSwap(i, m + j);
                    ++i;
                }
            }
            if (odd) {
                if (c < m)
                    for (ptrdiff_t j = 0; j < len1; ++j)
                        compSwap(c, m + j);
                else
                    for (ptrdiff_t j = 0; j < len1; ++j)
                        compSwap(a + j, c);
            }
        };

        ptrdiff_t n = length;
        ptrdiff_t d = 2;
        ptrdiff_t end = static_cast<ptrdiff_t>(1) << static_cast<int>(std::log(static_cast<double>(n - 1)) / std::log(2.0) + 1);

        while (d <= end) {
            ptrdiff_t i = 0, dec = 0;

            while (i < n) {
                ptrdiff_t j = i;
                dec += n;

                while (dec >= d) {
                    dec -= d;
                    ++j;
                }
                ptrdiff_t k = j;
                dec += n;

                while (dec >= d) {
                    dec -= d;
                    ++k;
                }
                split(split, i, j, k);
                i = k;
            }
            d *= 2;
        }
    }

    // Port of DualPivotQuickSort.runSort (Yaroslavskiy-style dual pivot quicksort;
    // short ranges fall back to insertion sort).
    template<class T = int>
    void DualPivotQuickSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        auto dualPivot = [&](auto&& self, ptrdiff_t left, ptrdiff_t right, ptrdiff_t divisor) -> void {
            ptrdiff_t len = right - left;

            // insertion sort for tiny arrays
            if (len < 4) {
                NSortHelpers::insertionSort(data_, left, right + 1);
                return;
            }

            ptrdiff_t third = len / divisor;

            // "medians"
            ptrdiff_t med1 = left + third;
            ptrdiff_t med2 = right - third;

            if (med1 <= left) {
                med1 = left + 1;
            }
            if (med2 >= right) {
                med2 = right - 1;
            }
            if (data_[med1] < data_[med2]) {
                swap(data_[med1], data_[left]);
                swap(data_[med2], data_[right]);
            }
            else {
                swap(data_[med1], data_[right]);
                swap(data_[med2], data_[left]);
            }

            // pivots
            T pivot1 = data_[left];
            T pivot2 = data_[right];

            // pointers
            ptrdiff_t less = left + 1;
            ptrdiff_t great = right - 1;

            // sorting
            for (ptrdiff_t k = less; k <= great; ++k) {
                if (data_[k] < pivot1) {
                    swap(data_[k], data_[less]);
                    ++less;
                }
                else if (data_[k] > pivot2) {
                    while (k < great && data_[great] > pivot2) {
                        --great;
                    }
                    swap(data_[k], data_[great]);
                    --great;

                    if (data_[k] < pivot1) {
                        swap(data_[k], data_[less]);
                        ++less;
                    }
                }
            }

            // swaps
            ptrdiff_t dist = great - less;

            if (dist < 13) {
                ++divisor;
            }
            swap(data_[less - 1], data_[left]);
            swap(data_[great + 1], data_[right]);

            // subarrays
            self(self, left, less - 2, divisor);
            if (pivot1 < pivot2) {
                self(self, less, great, divisor);
            }
            self(self, great + 2, right, divisor);
        };

        dualPivot(dualPivot, 0, length - 1, 3);
    }

    // Port of ExchangeBogoSort.runSort (randomly exchange any two elements).
    template<class T = int>
    void ExchangeBogoSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());

        while (!NSortHelpers::isRangeSorted(data_, 0, length)) {
            ptrdiff_t index1 = NSortHelpers::bogoRandInt(engine, 0, length);
            ptrdiff_t index2 = NSortHelpers::bogoRandInt(engine, 0, length);

            // Java compares once and uses the index order to pick the direction.
            if (index1 < index2) {
                if (data_[index1] > data_[index2])
                    swap(data_[index1], data_[index2]);
            }
            else {
                if (data_[index1] < data_[index2])
                    swap(data_[index1], data_[index2]);
            }
        }
    }

    // Port of ForcedStableQuickSort.runSort (quicksort over an identity key
    // array; swaps move array and key together, ties keep original order).
    template<class T = int>
    void ForcedStableQuickSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        std::vector<ptrdiff_t> key(length);
        for (ptrdiff_t i = 0; i < length; ++i)
            key[i] = i;

        auto stableComp = [&](ptrdiff_t a, ptrdiff_t b) -> bool {
            int comp = NSortHelpers::CompareValues(data_[a], data_[b]);
            return comp > 0 || (comp == 0 && key[a] > key[b]);
        };
        auto stableSwap = [&](ptrdiff_t a, ptrdiff_t b) {
            swap(data_[a], data_[b]);
            swap(key[a], key[b]);
        };
        auto medianOfThree = [&](ptrdiff_t a, ptrdiff_t b) {
            ptrdiff_t m = a + (b - 1 - a) / 2;

            if (stableComp(a, m))
                stableSwap(a, m);

            if (stableComp(m, b - 1)) {
                stableSwap(m, b - 1);

                if (stableComp(a, m))
                    return;
            }

            stableSwap(a, m);
        };
        auto partition = [&](ptrdiff_t a, ptrdiff_t b, ptrdiff_t p) -> ptrdiff_t {
            ptrdiff_t i = a - 1, j = b;

            while (true) {
                do { ++i; } while (i < j && !stableComp(i, p));

                do { --j; } while (j >= i && stableComp(j, p));

                if (i < j) stableSwap(i, j);
                else return j;
            }
        };

        // explicit stack: the poor pivot choice makes the Java recursion O(n) deep
        std::vector<std::pair<ptrdiff_t, ptrdiff_t>> stack;
        stack.push_back({ 0, length });
        while (!stack.empty()) {
            std::pair<ptrdiff_t, ptrdiff_t> range = stack.back();
            stack.pop_back();
            ptrdiff_t a = range.first, b = range.second;

            if (b - a < 3) {
                if (b - a == 2 && stableComp(a, a + 1))
                    stableSwap(a, a + 1);
                continue;
            }

            medianOfThree(a, b);
            ptrdiff_t p = partition(a + 1, b, a);
            stableSwap(a, p);

            stack.push_back({ p + 1, b });
            stack.push_back({ a, p });
        }
    }

    // Port of FunSort.runSort (binary search in an unsorted array, joke sort).
    // NOTE: like the Java original this can spin forever when the binary search
    // returns pos == i + 1 with different values; the behavior is kept verbatim.
    template<class T = int>
    void FunSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        auto binarySearch = [&](ptrdiff_t start, ptrdiff_t end, const T& value) -> ptrdiff_t {
            while (start < end) {
                ptrdiff_t mid = (start + end) >> 1;
                if (data_[mid] < value) {
                    start = mid + 1;
                }
                else {
                    end = mid;
                }
            }
            return start;
        };

        for (ptrdiff_t i = 1; i < length; ++i) {
            bool done = false;
            do {
                done = true;
                T value = data_[i];
                ptrdiff_t pos = binarySearch(0, length - 1, value);
                if (data_[pos] != data_[i]) {
                    if (i < pos - 1) {
                        swap(data_[i], data_[pos - 1]);
                    }
                    else if (i > pos) {
                        swap(data_[i], data_[pos]);
                    }
                    done = false;
                }
            } while (!done);
        }
    }

    // Port of GnomeSort.runSort.
    template<class T = int>
    void GnomeSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        for (ptrdiff_t i = 1; i < length;) {
            if (data_[i] >= data_[i - 1]) {
                ++i;
            }
            else {
                swap(data_[i], data_[i - 1]);

                if (i > 1) {
                    --i;
                }
            }
        }
    }

    // Port of LLQuickSort.runSort ("left/left pointers" Hoare-style partition on
    // the last element; explicit stack for the O(n) worst-case recursion).
    template<class T = int>
    void LLQuickSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        auto partition = [&](ptrdiff_t lo, ptrdiff_t hi) -> ptrdiff_t {
            T pivot = data_[hi];
            ptrdiff_t i = lo;

            for (ptrdiff_t j = lo; j < hi; ++j) {
                if (data_[j] < pivot) {
                    swap(data_[i], data_[j]);
                    ++i;
                }
            }
            swap(data_[i], data_[hi]);
            return i;
        };

        std::vector<std::pair<ptrdiff_t, ptrdiff_t>> stack;
        stack.push_back({ 0, length - 1 });
        while (!stack.empty()) {
            std::pair<ptrdiff_t, ptrdiff_t> range = stack.back();
            stack.pop_back();
            ptrdiff_t lo = range.first, hi = range.second;

            if (lo < hi) {
                ptrdiff_t p = partition(lo, hi);
                stack.push_back({ p + 1, hi });
                stack.push_back({ lo, p - 1 });
            }
        }
    }

    // Port of LRQuickSort.runSort ("left/right pointers" Hoare quicksort with the
    // middle element as pivot; explicit stack for the O(n) worst-case recursion).
    template<class T = int>
    void LRQuickSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        // explicit stack, left-first to match Java's recursion order
        std::vector<std::pair<ptrdiff_t, ptrdiff_t>> stack;
        stack.push_back({ 0, length - 1 });
        while (!stack.empty()) {
            std::pair<ptrdiff_t, ptrdiff_t> range = stack.back();
            stack.pop_back();
            ptrdiff_t p = range.first, r = range.second;
            if (p >= r) continue;

            ptrdiff_t pivot = p + (r - p + 1) / 2;
            T x = data_[pivot];

            ptrdiff_t i = p;
            ptrdiff_t j = r;


            while (i <= j) {
                while (data_[i] < x) {
                    ++i;
                }
                while (data_[j] > x) {
                    --j;
                }

                if (i <= j) {
                    // follow the pivot and highlight it
                    if (i == pivot) {
                    }
                    if (j == pivot) {
                    }

                    swap(data_[i], data_[j]);

                    ++i;
                    --j;
                }
            }

            stack.push_back({ i, r });
            stack.push_back({ p, j });
        }
    }

    // Port of LRQuickSortParallel.runSort (same partition, each half in its own
    // thread). Threads are only spawned for ranges of at least
    // LRQuickSortParallelMinThreadRange elements; smaller ranges run inline, which
    // keeps the Java split logic but avoids Java's unbounded thread creation.
    template<class T = int>
    void LRQuickSortParallel(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        constexpr ptrdiff_t minThreadRange = 1024;

        auto quickSort = [&](auto&& self, ptrdiff_t p, ptrdiff_t r) -> void {
            if (p < r) {
                ptrdiff_t pivot = p + (r - p + 1) / 2;
                T x = data_[pivot];

                ptrdiff_t i = p;
                ptrdiff_t j = r;


                while (i <= j) {
                    while (data_[i] < x) {
                        ++i;
                    }
                    while (data_[j] > x) {
                        --j;
                    }

                    if (i <= j) {
                        if (i == pivot) {
                        }
                        if (j == pivot) {
                        }

                        swap(data_[i], data_[j]);

                        ++i;
                        --j;
                    }
                }

                if (r - p >= minThreadRange) {
                    std::thread left([&]() { self(self, p, j); });
                    std::thread right([&]() { self(self, i, r); });
                    left.join();
                    right.join();
                }
                else {
                    self(self, p, j);
                    self(self, i, r);
                }
            }
        };

        quickSort(quickSort, 0, length - 1);
    }

    // Port of OddEvenSort.runSort. (Name collision with the existing C++ OddEvenSort.)
    template<class T = int>
    void OddEvenSortJava(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        bool sorted = false;

        while (!sorted) {
            sorted = true;

            for (ptrdiff_t i = 1; i < length - 1; i += 2) {
                if (data_[i] > data_[i + 1]) {
                    swap(data_[i], data_[i + 1]);
                    sorted = false;
                }

            }

            for (ptrdiff_t i = 0; i < length - 1; i += 2) {
                if (data_[i] > data_[i + 1]) {
                    swap(data_[i], data_[i + 1]);
                    sorted = false;
                }

            }
        }
    }

    // Port of OptimizedBubbleSort.runSort (skips the tail that just stayed sorted).
    template<class T = int>
    void OptimizedBubbleSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        ptrdiff_t consecSorted = 0;
        for (ptrdiff_t i = length - 1; i > 0; i -= consecSorted) {
            consecSorted = 1;
            for (ptrdiff_t j = 0; j < i; ++j) {
                if (data_[j] > data_[j + 1]) {
                    swap(data_[j], data_[j + 1]);
                    consecSorted = 1;
                }
                else ++consecSorted;
            }
        }
    }

    // Port of OptimizedCocktailShakerSort.runSort.
    template<class T = int>
    void OptimizedCocktailShakerSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        for (ptrdiff_t start = 0, end = length - 1; start < end;) {
            ptrdiff_t consecSorted = 1;
            for (ptrdiff_t i = start; i < end; ++i) {
                if (data_[i] > data_[i + 1]) {
                    swap(data_[i], data_[i + 1]);
                    consecSorted = 1;
                }
                else ++consecSorted;
            }
            end -= consecSorted;

            consecSorted = 1;
            for (ptrdiff_t i = end; i > start; --i) {
                if (data_[i - 1] > data_[i]) {
                    swap(data_[i - 1], data_[i]);
                    consecSorted = 1;
                }
                else ++consecSorted;
            }
            start += consecSorted;
        }
    }

    // Port of OptimizedGnomeSort.runSort (smartGnomeSort; same as the shared
    // NSortHelpers::optimizedGnomeSort on [0, length)).
    template<class T = int>
    void OptimizedGnomeSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        NSortHelpers::optimizedGnomeSort(data_, 0, length);
    }

    // Port of OptimizedStoogeSort.runSort (forward/backward shaker-style passes).
    template<class T = int>
    void OptimizedStoogeSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        auto forward = [&](ptrdiff_t left, ptrdiff_t right) {
            while (left < right) {
                ptrdiff_t index = right;

                while (left < index) {

                    if (data_[left] > data_[index]) {
                        swap(data_[left], data_[index]);
                    }
                    ++left;
                    --index;
                }

                left = 0;
                --right;
            }
        };

        auto backward = [&](ptrdiff_t left, ptrdiff_t right) {
            ptrdiff_t rightStart = right;

            while (left < right) {
                ptrdiff_t index = left;

                while (index < right) {

                    if (data_[index] > data_[right]) {
                        swap(data_[index], data_[right]);
                    }
                    ++index;
                    --right;
                }

                ++left;
                right = rightStart;
            }
        };

        ptrdiff_t left = 0;
        ptrdiff_t right = length - 1;

        while (left < right) {

            if (data_[left] > data_[right]) {
                swap(data_[left], data_[right]);
            }
            ++left;
            --right;
        }

        forward(0, length - 2);
        backward(1, length - 1);
    }

    // Port of OptimizedStoogeSortStudio.runSort (EilrahcF/aphitorite stable
    // stooge sort with range-change flags).
    template<class T = int>
    void OptimizedStoogeSortStudio(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        auto compSwap = [&](ptrdiff_t a, ptrdiff_t b) -> bool {
            if (data_[a] > data_[b]) {
                swap(data_[a], data_[b]);
                return true;
            }
            return false;
        };

        auto stoogeSort = [&](auto&& self, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, bool merge) -> bool {
            if (a >= m)
                return false;
            if (b - a == 2)
                return compSwap(a, m);

            bool lChange = false;
            bool rChange = false;

            ptrdiff_t a2 = (a + a + b) / 3;
            ptrdiff_t b2 = (a + b + b + 2) / 3;

            if (m < b2) {
                lChange = self(self, a, m, b2, merge);

                if (merge) {
                    rChange = self(self, (std::max)(a + b2 - m, a2), b2, b, true);
                    if (rChange) self(self, a + b2 - m, a2, 2 * a2 - a, true);
                }
                else {
                    rChange = self(self, a2, b2, b, false);
                    if (rChange) self(self, a, a2, 2 * a2 - a, true);
                }
            }
            else {
                rChange = self(self, a2, m, b, merge);
                if (rChange) self(self, a, a2, a2 + b - m, true);
            }
            return lChange || rChange;
        };

        stoogeSort(stoogeSort, 0, 1, length, false);
    }

    // Port of QuadStoogeSort.runSort (quad stooge sorting network).
    template<class T = int>
    void QuadStoogeSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        auto quadStooge = [&](auto&& self, ptrdiff_t pos, ptrdiff_t len) -> void {
            if (len >= 2 && data_[pos] > data_[pos + len - 1]) {
                swap(data_[pos], data_[pos + len - 1]);
            }
            if (len <= 2) {
                return;
            }

            ptrdiff_t len1 = len / 2;
            ptrdiff_t len2 = (len + 1) / 2;
            ptrdiff_t len3 = (len1 + 1) / 2 + (len2 + 1) / 2;

            self(self, pos, len1);
            self(self, pos + len1, len2);
            self(self, pos + len1 / 2, len3);
            self(self, pos + len1, len2);
            self(self, pos, len1);
            if (len > 3) {
                self(self, pos + len1 / 2, len3);
            }
        };

        quadStooge(quadStooge, 0, length);
    }

    // Port of ShoveSort.runSort (bubble a disorder to the end, then step back).
    template<class T = int>
    void ShoveSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        ptrdiff_t start = 0, end = length;
        ptrdiff_t i = start;
        while (i < end - 1) {
            if (data_[i] > data_[i + 1]) {
                for (ptrdiff_t f = i; f < end - 1; ++f) {
                    swap(data_[f], data_[f + 1]);
                }
                if (i > start) {
                    --i;
                }
                continue;
            }
            ++i;
        }
    }

    // Port of SillySort.runSort (Tom Duff's silly sort; explicit stack because the
    // Java recursion is O(n) deep).
    template<class T = int>
    void SillySort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        struct Frame {
            ptrdiff_t i, j;
            int stage;
        };

        std::vector<Frame> stack;
        stack.push_back({ 0, length - 1, 0 });
        while (!stack.empty()) {
            Frame& f = stack.back();
            if (f.i >= f.j) {
                stack.pop_back();
                continue;
            }

            ptrdiff_t i = f.i, j = f.j;
            ptrdiff_t m = i + ((j - i) / 2);

            if (f.stage == 0) {
                f.stage = 1;
                stack.push_back({ i, m, 0 });
            }
            else if (f.stage == 1) {
                f.stage = 2;
                stack.push_back({ m + 1, j, 0 });
            }
            else if (f.stage == 2) {
                f.stage = 3;

                // put the smallest of the two half minima in the first position
                if (data_[i] >= data_[m + 1]) {
                    swap(data_[i], data_[m + 1]);
                }

            }
            else {
                stack.pop_back();
                stack.push_back({ i + 1, j, 0 });
            }
        }
    }

    // Port of SlopeSort.runSort (diagonal adjacent comparisons sweeping left).
    template<class T = int>
    void SlopeSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        for (ptrdiff_t i = 1, j = 1; i < length; ++i, ++j) {
            for (ptrdiff_t k = i - 1; k >= 0; --k, --i) {
                if (data_[i] < data_[k]) {
                    swap(data_[i], data_[k]);
                }
            }
            i = j;
        }
    }

    // Port of SlowSort.runSort (explicit stack because the Java recursion is
    // O(n) deep).
    template<class T = int>
    void SlowSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        struct Frame {
            ptrdiff_t i, j;
            int stage;
        };

        std::vector<Frame> stack;
        stack.push_back({ 0, length - 1, 0 });
        while (!stack.empty()) {
            Frame& f = stack.back();
            if (f.i >= f.j) {
                stack.pop_back();
                continue;
            }

            ptrdiff_t i = f.i, j = f.j;
            ptrdiff_t m = i + ((j - i) / 2);

            if (f.stage == 0) {
                f.stage = 1;
                stack.push_back({ i, m, 0 });
            }
            else if (f.stage == 1) {
                f.stage = 2;
                stack.push_back({ m + 1, j, 0 });
            }
            else if (f.stage == 2) {
                f.stage = 3;

                if (data_[m] > data_[j]) {
                    swap(data_[m], data_[j]);
                }

            }
            else {
                stack.pop_back();
                stack.push_back({ i, j - 1, 0 });
            }
        }
    }

    // Port of SnuffleSort.runSort (recursive snuffle; note that the Java repeat
    // count (int) Math.ceil((stop - start + 1) / 2) uses integer division).
    template<class T = int>
    void SnuffleSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        auto snuffleSort = [&](auto&& self, ptrdiff_t start, ptrdiff_t stop) -> void {
            if (stop - start + 1 >= 2) {
                if (data_[start] > data_[stop])
                    swap(data_[start], data_[stop]);
                if (stop - start + 1 >= 3) {
                    ptrdiff_t mid = (stop - start) / 2 + start;
                    ptrdiff_t reps = (stop - start + 1) / 2;
                    for (ptrdiff_t i = 0; i < reps; ++i) {
                        self(self, start, mid);
                        self(self, mid, stop);
                    }
                }
            }
        };

        snuffleSort(snuffleSort, 0, length - 1);
    }

    // Port of StablePermutationSort.runSort (enumerates permutations of an index
    // array and applies them, stopping at the first sorted ordering). The Java
    // recursion is O(n) deep but the registered size (11) keeps it tiny.
    template<class T = int>
    void StablePermutationSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        std::vector<ptrdiff_t> idx(length);
        for (ptrdiff_t i = 0; i < length; ++i)
            idx[i] = i;

        auto permute = [&](auto&& self, ptrdiff_t len) -> bool {
            if (len < 2) return NSortHelpers::isArraySorted(data_, length);

            for (ptrdiff_t i = len - 2; i >= 0; --i) {
                if (self(self, len - 1)) return true;

                swap(data_[idx[i]], data_[idx[len - 1]]);
                swap(idx[i], idx[len - 1]);
            }
            if (self(self, len - 1)) return true;

            ptrdiff_t t = idx[len - 1];

            for (ptrdiff_t i = len - 1; i > 0; --i)
                idx[i] = idx[i - 1];
            idx[0] = t;

            T first = data_[idx[0]];

            for (ptrdiff_t i = 1; i < len; ++i)
                data_[idx[i - 1]] = data_[idx[i]];
            data_[idx[len - 1]] = first;

            return false;
        };

        permute(permute, length);
    }

    // Port of StableQuickSort.runSort (Rodney Shaghoulian's stable quicksort with
    // O(n) auxiliary lists per partition; explicit stack for the O(n) worst case).
    template<class T = int>
    void StableQuickSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        auto stablePartition = [&](ptrdiff_t start, ptrdiff_t end) -> ptrdiff_t {
            T pivotValue = data_[start]; // poor pivot choice

            std::vector<T> leftList, rightList; // Java creates ArrayVList(length) each call
            leftList.reserve(static_cast<size_t>(length));
            rightList.reserve(static_cast<size_t>(length));

            for (ptrdiff_t i = start + 1; i <= end; ++i) {

                if (data_[i] < pivotValue) {
                    leftList.push_back(data_[i]);
                }
                else {
                    rightList.push_back(data_[i]);
                }
            }

            // recreate array
            ptrdiff_t w = start;
            for (size_t n = 0; n < leftList.size(); ++n) {
                data_[w] = leftList[n];
                ++w;
            }

            ptrdiff_t newPivotIndex = start + static_cast<ptrdiff_t>(leftList.size());

            data_[newPivotIndex] = pivotValue;

            w = newPivotIndex + 1;
            for (size_t n = 0; n < rightList.size(); ++n) {
                data_[w] = rightList[n];
                ++w;
            }

            return newPivotIndex;
        };

        std::vector<std::pair<ptrdiff_t, ptrdiff_t>> stack;
        stack.push_back({ 0, length - 1 });
        while (!stack.empty()) {
            std::pair<ptrdiff_t, ptrdiff_t> range = stack.back();
            stack.pop_back();
            ptrdiff_t start = range.first, end = range.second;

            if (start < end) {
                ptrdiff_t pivotIndex = stablePartition(start, end);
                stack.push_back({ pivotIndex + 1, end });
                stack.push_back({ start, pivotIndex - 1 });
            }
        }
    }

    // Port of StableQuickSortParallel.runSort (alternating in-place/external
    // stable quicksort; left halves stay in "Int" mode on data_, right halves are
    // "Ext" mode on tmp). Threads are only spawned for ranges of at least
    // StableQuickSortParallelMinThreadRange elements; smaller ranges run inline.
    template<class T = int>
    void StableQuickSortParallel(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        constexpr ptrdiff_t minThreadRange = 1024;

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());
        std::vector<T> tmp(static_cast<size_t>(length));

        auto partitionInt = [&](ptrdiff_t a, ptrdiff_t b) -> ptrdiff_t {
            ptrdiff_t p = NSortHelpers::bogoRandInt(engine, a, b);

            T piv = data_[p];
            ptrdiff_t j = a, k = b - 1;

            while (j < p && data_[j] <= piv) ++j;
            if (j < p) {
                tmp[k] = data_[j];
                --k;
            }

            for (ptrdiff_t i = j + 1; i < p; ++i) {
                if (data_[i] <= piv) {
                    data_[j] = data_[i];
                    ++j;
                }
                else {
                    tmp[k] = data_[i];
                    --k;
                }
            }
            for (ptrdiff_t i = p + 1; i < b; ++i) {
                if (data_[i] < piv) {
                    data_[j] = data_[i];
                    ++j;
                }
                else {
                    tmp[k] = data_[i];
                    --k;
                }
            }
            data_[j] = piv;

            return j;
        };

        auto partitionExt = [&](ptrdiff_t a, ptrdiff_t b) -> ptrdiff_t {
            ptrdiff_t p = NSortHelpers::bogoRandInt(engine, a, b);

            T piv = tmp[p];
            ptrdiff_t j = b - 1, k = a;

            while (j > p && tmp[j] > piv) --j;
            if (j > p) {
                data_[k] = tmp[j];
                ++k;
            }

            for (ptrdiff_t i = j - 1; i > p; --i) {
                if (tmp[i] > piv) {
                    tmp[j] = tmp[i];
                    --j;
                }
                else {
                    data_[k] = tmp[i];
                    ++k;
                }
            }
            for (ptrdiff_t i = p - 1; i >= a; --i) {
                if (tmp[i] >= piv) {
                    tmp[j] = tmp[i];
                    --j;
                }
                else {
                    data_[k] = tmp[i];
                    ++k;
                }
            }
            data_[k] = piv;

            return k;
        };

        auto quickSort = [&](auto&& self, ptrdiff_t a, ptrdiff_t b, bool ext) -> void {
            ptrdiff_t len = b - a;

            if (len < 2) {
                if (len == 1 && ext) {
                    data_[a] = tmp[a];
                }
                return;
            }

            ptrdiff_t p = ext ? partitionExt(a, b) : partitionInt(a, b);

            if (len >= minThreadRange) {
                std::thread left([&]() { self(self, a, p, false); });
                std::thread right([&]() { self(self, p + 1, b, true); });
                left.join();
                right.join();
            }
            else {
                self(self, a, p, false);
                self(self, p + 1, b, true);
            }
        };

        quickSort(quickSort, 0, length, false);
    }

    // Port of StoogeSort.runSort. (Name collision with the existing C++ StoogeSort.)
    template<class T = int>
    void StoogeSortJava(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        auto stoogeSort = [&](auto&& self, ptrdiff_t i, ptrdiff_t j) -> void {
            if (data_[i] > data_[j]) {
                swap(data_[i], data_[j]);
            }

            if (j - i + 1 >= 3) {
                ptrdiff_t t = (j - i + 1) / 3;

                self(self, i, j - t);
                self(self, i + t, j);
                self(self, i, j - t);
            }
        };

        stoogeSort(stoogeSort, 0, length - 1);
    }

    // Port of SwaplessBubbleSort.runSort (moves a carried element instead of
    // swapping; Java's "pos" optimization is kept exactly).
    template<class T = int>
    void SwaplessBubbleSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        ptrdiff_t last = 0;
        for (ptrdiff_t i = length; i > 0; i = last) {
            last = 0;
            ptrdiff_t pos = 0;
            T comp = data_[0];
            for (ptrdiff_t j = 1; j < i; ++j) {
                if (comp > data_[j]) {
                    data_[j - 1] = data_[j];
                    last = j;
                }
                else {
                    // also handles incrementing pos so this optimization works next time
                    if (pos + 1 < j)
                        data_[j - 1] = comp;
                    pos = j;
                    comp = data_[j];
                }

            }
            data_[i - 1] = comp;
        }
    }

    // Port of TableSort.runSort (quicksort over a permutation table, then applies
    // the cycles to the real array).
    template<class T = int>
    void TableSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        std::vector<ptrdiff_t> table(length);
        for (ptrdiff_t i = 0; i < length; ++i)
            table[i] = i;

        auto stableComp = [&](ptrdiff_t a, ptrdiff_t b) -> bool {
            ptrdiff_t ia = table[a], ib = table[b];
            int comp = NSortHelpers::CompareValues(data_[ia], data_[ib]);
            return comp > 0 || (comp == 0 && ia > ib);
        };
        auto tableSwap = [&](ptrdiff_t a, ptrdiff_t b) {
            swap(table[a], table[b]);
        };
        auto medianOfThree = [&](ptrdiff_t a, ptrdiff_t b) {
            ptrdiff_t m = a + (b - 1 - a) / 2;

            if (stableComp(a, m))
                tableSwap(a, m);

            if (stableComp(m, b - 1)) {
                tableSwap(m, b - 1);

                if (stableComp(a, m))
                    return;
            }

            tableSwap(a, m);
        };
        auto partition = [&](ptrdiff_t a, ptrdiff_t b, ptrdiff_t p) -> ptrdiff_t {
            ptrdiff_t i = a - 1, j = b;

            while (true) {
                do { ++i; } while (i < j && !stableComp(i, p));

                do { --j; } while (j >= i && stableComp(j, p));

                if (i < j) tableSwap(i, j);
                else return j;
            }
        };

        // explicit stack: the poor pivot choice makes the Java recursion O(n) deep
        std::vector<std::pair<ptrdiff_t, ptrdiff_t>> stack;
        stack.push_back({ 0, length });
        while (!stack.empty()) {
            std::pair<ptrdiff_t, ptrdiff_t> range = stack.back();
            stack.pop_back();
            ptrdiff_t a = range.first, b = range.second;

            if (b - a < 3) {
                if (b - a == 2 && stableComp(a, a + 1))
                    tableSwap(a, a + 1);
                continue;
            }

            medianOfThree(a, b);
            ptrdiff_t p = partition(a + 1, b, a);
            tableSwap(a, p);

            stack.push_back({ p + 1, b });
            stack.push_back({ a, p });
        }

        // apply the permutation cycles to the array
        for (ptrdiff_t i = 0; i < length; ++i) {

            if (i != table[i]) {
                T t = data_[i];
                ptrdiff_t j = i, next = table[i];

                do {
                    data_[j] = data_[next];
                    table[j] = j;

                    j = next;
                    next = table[next];
                } while (next != i);

                data_[j] = t;
                table[j] = j;
            }
        }
    }

    // Port of ThreeSmoothCombSortIterative.runSort (powers of two and three).
    template<class T = int>
    void ThreeSmoothCombSortIterative(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        auto compSwap = [&](ptrdiff_t a, ptrdiff_t b) {
            if (data_[a] > data_[b])
                swap(data_[a], data_[b]);
        };

        int pow2 = static_cast<int>(std::log(static_cast<double>(length - 1)) / std::log(2.0));

        for (int k = pow2; k >= 0; --k) {
            int pow3 = static_cast<int>((std::log(static_cast<double>(length)) - k * std::log(2.0)) / std::log(3.0));

            for (int j = pow3; j >= 0; --j) {
                ptrdiff_t gap = static_cast<ptrdiff_t>(std::pow(2.0, k) * std::pow(3.0, j));

                for (ptrdiff_t i = 0; i + gap < length; ++i)
                    compSwap(i, i + gap);
            }
        }
    }

    // Port of ThreeSmoothCombSortParallel.runSort (recursive 3-smooth comb with a
    // thread per recursive branch). Threads are only spawned while the gap subarray
    // still holds at least ThreeSmoothCombSortParallelMinRuns runs; smaller
    // subproblems run inline, which keeps the Java split logic without Java's
    // unbounded thread creation.
    template<class T = int>
    void ThreeSmoothCombSortParallel(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        constexpr ptrdiff_t minRunsPerThread = 128;

        auto compSwap = [&](ptrdiff_t a, ptrdiff_t b) {
            if (data_[a] > data_[b])
                swap(data_[a], data_[b]);
        };

        auto powerOfThree = [&](auto&& self, ptrdiff_t pos, ptrdiff_t gap, ptrdiff_t end) -> void {
            if (pos + gap > end) return;

            if ((end - pos) / gap >= minRunsPerThread) {
                std::thread a([&]() { self(self, pos, gap * 3, end); });
                std::thread b([&]() { self(self, pos + gap, gap * 3, end); });
                std::thread c([&]() { self(self, pos + 2 * gap, gap * 3, end); });
                a.join();
                b.join();
                c.join();
            }
            else {
                self(self, pos, gap * 3, end);
                self(self, pos + gap, gap * 3, end);
                self(self, pos + 2 * gap, gap * 3, end);
            }

            for (ptrdiff_t i = pos; i + gap < end; i += gap)
                compSwap(i, i + gap);
        };

        auto recursiveComb = [&](auto&& self, ptrdiff_t pos, ptrdiff_t gap, ptrdiff_t end) -> void {
            if (pos + gap > end) return;

            if ((end - pos) / gap >= minRunsPerThread) {
                std::thread a([&]() { self(self, pos, gap * 2, end); });
                std::thread b([&]() { self(self, pos + gap, gap * 2, end); });
                a.join();
                b.join();
            }
            else {
                self(self, pos, gap * 2, end);
                self(self, pos + gap, gap * 2, end);
            }

            powerOfThree(powerOfThree, pos, gap, end);
        };

        recursiveComb(recursiveComb, 0, 1, length);
    }

    // Port of ThreeSmoothCombSortRecursive.runSort (single threaded).
    template<class T = int>
    void ThreeSmoothCombSortRecursive(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        auto compSwap = [&](ptrdiff_t a, ptrdiff_t b) {
            if (data_[a] > data_[b])
                swap(data_[a], data_[b]);
        };

        auto powerOfThree = [&](auto&& self, ptrdiff_t pos, ptrdiff_t gap, ptrdiff_t end) -> void {
            if (pos + gap > end) return;

            self(self, pos, gap * 3, end);
            self(self, pos + gap, gap * 3, end);
            self(self, pos + 2 * gap, gap * 3, end);

            for (ptrdiff_t i = pos; i + gap < end; i += gap)
                compSwap(i, i + gap);
        };

        auto recursiveComb = [&](auto&& self, ptrdiff_t pos, ptrdiff_t gap, ptrdiff_t end) -> void {
            if (pos + gap > end) return;

            self(self, pos, gap * 2, end);
            self(self, pos + gap, gap * 2, end);

            powerOfThree(powerOfThree, pos, gap, end);
        };

        recursiveComb(recursiveComb, 0, 1, length);
    }

    // Port of UnoptimizedBubbleSort.runSort (passes until a clean pass happens).
    template<class T = int>
    void UnoptimizedBubbleSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        bool sorted = false;

        while (!sorted) {
            sorted = true;
            for (ptrdiff_t i = 0; i < length - 1; ++i) {
                if (data_[i] > data_[i + 1]) {
                    swap(data_[i], data_[i + 1]);
                    sorted = false;
                }

            }
        }
    }

    // Port of UnoptimizedCocktailShakerSort.runSort (no early exit).
    template<class T = int>
    void UnoptimizedCocktailShakerSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        using std::swap;

        ptrdiff_t start = 0, end = length;
        ptrdiff_t i = start;
        while (i < ((end / 2) + start)) {
            for (ptrdiff_t j = i; j < end + start - i - 1; ++j) {
                if (data_[j] > data_[j + 1]) {
                    swap(data_[j], data_[j + 1]);
                }

            }
            for (ptrdiff_t j = end + start - i - 1; j > i; --j) {
                if (data_[j] < data_[j - 1]) {
                    swap(data_[j], data_[j - 1]);
                }

            }

            ++i;
        }
    }

} // namespace NVisualSort::NSortAlgorithms
