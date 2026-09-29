#pragma once
// Port of ArrayV (Java, MIT) sorting template base classes, https://github.com/Gaming32/ArrayV
// VisualSort - shared sorting algorithm helpers (template base class ports).
//
// ASCII ONLY: do not put non-ASCII characters in this file. MSVC parses sources in the
// system code page (GBK); use \uXXXX escapes if a wide string is ever needed.
//
// Layout:
//   shared utilities: CompareValues, MarkArray, reverseRange, SortRandomEngine
//   SECTION 1: basic sorts & merge tools
//   SECTION 2: block / bit / shatter / pdq / quad
//   SECTION 3: grail / tim / wiki / kota
//
// This header is #included from Sort.h at file scope (before the namespaces of Sort.h),
// so it opens its own namespace and closes it again at the end of the file.
// All helpers are self-contained: they may call other NSortHelpers functions and the
// standard library only, never a category sort.
#include "Strip.h"
#include "Counter.h"
#include "ConfigManager.h"
#include "WideError.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <queue>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

namespace NVisualSort::NSortAlgorithms::NSortHelpers {

    // Three-way comparison, single logical compare (mirrors Java compareValues).
    template<class T>
    int CompareValues(const T& a, const T& b) {
        if (a < b) return -1;
        if (b < a) return 1;
        return 0;
    }

    // ArrayV highlight marks (1..4) are intentionally removed: the ported
    // algorithms paint nothing. The user's own sorts in Sort.h do their own
    // SetColor highlighting directly, so this must stay a no-op for them to
    // remain the only colored highlights. Kept as a stub so the ArrayV mark
    // call sites stay readable.
    template<class T>
    void MarkArray(int mark, std::vector<T>& data_, ptrdiff_t i) {
        (void)mark;
        (void)data_;
        (void)i;
    }

    // Reverse data_[start .. end).
    template<class T>
    void reverseRange(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        using std::swap;
        while (start < --end) swap(data_[start++], data_[end]);
    }

    // All three phases (int / Counter / Strip) of one visualization run must see
    // the same pseudo-random sequence, so one seed is shared until all three types
    // have consumed it (same pattern as BogoSortRandomEngine in Sort.h).
    class SortRandomEngine {
    private:
        inline static std::atomic<bool> s_isIntUsed = false;
        inline static std::atomic<bool> s_isCounterUsed = false;
        inline static std::atomic<bool> s_isStripUsed = false;
        inline static std::atomic<int> s_randomNumber = GetConfigManager().GenerateRandom();

    public:
        template<class T>
        static int GetSortRandom() {
            if (s_isIntUsed.load() && s_isCounterUsed.load() && s_isStripUsed.load()) {
                s_isIntUsed.store(false);
                s_isCounterUsed.store(false);
                s_isStripUsed.store(false);
                s_randomNumber.store(GetConfigManager().GenerateRandom());
            }
            if constexpr (std::is_same_v<T, int>) s_isIntUsed.store(true);
            else if constexpr (std::is_same_v<T, Counter>) s_isCounterUsed.store(true);
            else if constexpr (std::is_same_v<T, Strip>) s_isStripUsed.store(true);
            return s_randomNumber.load();
        }
    };

    // === SECTION 1: basic sorts & merge tools ===

    // InsertionSorting.insertionSort (Java's sleep and auxwrite arguments are dropped).
    template<class T>
    void insertionSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        if (end - start < 2) return;

        for (ptrdiff_t i = start; i < end; ++i) {
            T current = data_[i];
            ptrdiff_t pos = i - 1;

            while (pos >= start && data_[pos] > current) {
                data_[pos + 1] = data_[pos];
                --pos;
            }
            data_[pos + 1] = current;
        }
    }

    // BinaryInsertionSorting.binaryInsertSort
    template<class T>
    void binaryInsertSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        for (ptrdiff_t i = start; i < end; ++i) {
            T num = data_[i];
            ptrdiff_t lo = start;
            ptrdiff_t hi = i;

            while (lo < hi) {
                ptrdiff_t mid = lo + ((hi - lo) / 2); // avoid int overflow!
                MarkArray(1, data_, lo);
                MarkArray(2, data_, mid);
                MarkArray(3, data_, hi);

                if (num < data_[mid]) { // do NOT move equal elements to right of inserted element; this maintains stability!
                    hi = mid;
                }
                else {
                    lo = mid + 1;
                }
            }

            // item has to go into position lo
            ptrdiff_t j = i - 1;

            while (j >= lo) {
                data_[j + 1] = data_[j];
                --j;
            }
            data_[lo] = num;
        }
    }

    // OptimizedGnomeSort.smartGnomeSort + OptimizedGnomeSort.customSort
    template<class T>
    void optimizedGnomeSort(std::vector<T>& data_, ptrdiff_t low, ptrdiff_t high) {
        for (ptrdiff_t i = low + 1; i < high; ++i) {
            ptrdiff_t pos = i;

            while (pos > low && data_[pos - 1] > data_[pos]) {
                using std::swap;
                swap(data_[pos - 1], data_[pos]);
                --pos;
            }
        }
    }

    // HeapSorting.siftDown (1-based heap arithmetic over data_[start + ...]).
    template<class T>
    void heapSiftDown(std::vector<T>& data_, ptrdiff_t root, ptrdiff_t dist, ptrdiff_t start, bool isMax) {
        int compareVal = 0;

        if (isMax) compareVal = -1;
        else compareVal = 1;

        while (root <= dist / 2) {
            ptrdiff_t leaf = 2 * root;
            if (leaf < dist && CompareValues(data_[start + leaf - 1], data_[start + leaf]) == compareVal) {
                ++leaf;
            }
            MarkArray(1, data_, start + root - 1);
            MarkArray(2, data_, start + leaf - 1);
            if (CompareValues(data_[start + root - 1], data_[start + leaf - 1]) == compareVal) {
                using std::swap;
                swap(data_[start + root - 1], data_[start + leaf - 1]);
                root = leaf;
            }
            else break;
        }
    }

    // HeapSorting.heapify
    template<class T>
    void heapify(std::vector<T>& data_, ptrdiff_t low, ptrdiff_t high, bool isMax) {
        ptrdiff_t length = high - low;
        for (ptrdiff_t i = length / 2; i >= 1; --i) {
            heapSiftDown(data_, i, length, low, isMax);
        }
    }

    // HeapSorting.heapSort (Java's length is the absolute end index here).
    template<class T>
    void heapSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, bool isMax) {
        heapify(data_, start, end, isMax);

        for (ptrdiff_t i = end - start; i > 1; --i) {
            using std::swap;
            swap(data_[start], data_[start + i - 1]);
            heapSiftDown(data_, 1, i - 1, start, isMax);
        }

        if (!isMax) {
            reverseRange(data_, start, end);
        }
    }

    // ShellSorting gap arrays.
    inline const std::vector<ptrdiff_t> OriginalGaps         = { 2048, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1 };
    inline const std::vector<ptrdiff_t> PowTwoPlusOneGaps    = { 2049, 1025, 513, 257, 129, 65, 33, 17, 9, 5, 3, 1 };
    inline const std::vector<ptrdiff_t> PowTwoMinusOneGaps   = { 4095, 2047, 1023, 511, 255, 127, 63, 31, 15, 7, 3, 1 };
    inline const std::vector<ptrdiff_t> ThreeSmoothGaps      = { 3888, 3456, 3072, 2916, 2592, 2304, 2187, 2048, 1944, 1728,
                                                                 1536, 1458, 1296, 1152, 1024, 972, 864, 768, 729, 648, 576,
                                                                 512, 486, 432, 384, 324, 288, 256, 243, 216, 192, 162, 144,
                                                                 128, 108, 96, 81, 72, 64, 54, 48, 36, 32, 27, 24, 18, 16, 12,
                                                                 9, 8, 6, 4, 3, 2, 1 };
    inline const std::vector<ptrdiff_t> PowersOfThreeGaps    = { 3280, 1093, 364, 121, 40, 13, 4, 1 };
    inline const std::vector<ptrdiff_t> SedgewickIncerpiGaps = { 1968, 861, 336, 112, 48, 21, 7, 3, 1 };
    inline const std::vector<ptrdiff_t> SedgewickGaps        = { 1073, 281, 77, 23, 8, 1 };
    inline const std::vector<ptrdiff_t> OddEvenSedgewickGaps = { 3905, 2161, 929, 505, 209, 109, 41, 19, 5, 1 };
    inline const std::vector<ptrdiff_t> GonnetBaezaYatesGaps = { 1861, 846, 384, 174, 79, 36, 16, 7, 3, 1 };
    inline const std::vector<ptrdiff_t> TokudaGaps           = { 2660, 1182, 525, 233, 103, 46, 20, 9, 4, 1 };
    inline const std::vector<ptrdiff_t> CiuraGaps            = { 1750, 701, 301, 132, 57, 23, 10, 4, 1 };
    inline const std::vector<ptrdiff_t> ExtendedCiuraGaps    = { 8861, 3938, 1750, 701, 301, 132, 57, 23, 10, 4, 1 };

    // ShellSorting.shellSort, generalized to the range [start, end) and to an explicit
    // gap sequence. Java compares the gap array by reference to pick the "length / 3"
    // variant, which is kept here by comparing the addresses of the inline gap arrays.
    template<class T>
    void shellSortGaps(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, const std::vector<ptrdiff_t>& gaps) {
        ptrdiff_t length = end - start;

        for (size_t k = 0; k < gaps.size(); ++k) {
            if (&gaps == &PowersOfThreeGaps) {
                if (gaps[k] < length / 3) {
                    for (ptrdiff_t h = gaps[k], i = h + start; i < end; ++i) {
                        T v = data_[i];
                        ptrdiff_t j = i;

                        MarkArray(1, data_, j);
                        MarkArray(2, data_, j - h);

                        while (j >= start + h && data_[j - h] > v) {
                            data_[j] = data_[j - h];
                            j -= h;

                            MarkArray(1, data_, j);

                            if (j - h >= 0) {
                                MarkArray(2, data_, j - h);
                            }
                        }
                        data_[j] = v;
                    }
                }
            }
            else {
                if (gaps[k] < length) {
                    for (ptrdiff_t h = gaps[k], i = h + start; i < end; ++i) {
                        T v = data_[i];
                        ptrdiff_t j = i;

                        MarkArray(1, data_, j);
                        MarkArray(2, data_, j - h);

                        while (j >= start + h && data_[j - h] > v) {
                            data_[j] = data_[j - h];
                            j -= h;

                            MarkArray(1, data_, j);

                            if (j - h >= 0) {
                                MarkArray(2, data_, j - h);
                            }
                        }
                        data_[j] = v;
                    }
                }
            }
        }
    }

    // ShellSorting.shellSort with the default ExtendedCiuraGaps sequence.
    template<class T>
    void shellSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        shellSortGaps(data_, start, end, ExtendedCiuraGaps);
    }

    // ShellSorting.quickShellSort (keeps Java's absolute j >= h condition).
    template<class T>
    void quickShellSort(std::vector<T>& data_, ptrdiff_t lo, ptrdiff_t hi) {
        static const std::vector<ptrdiff_t> incs = { 48, 21, 7, 3, 1 };

        for (size_t k = 0; k < incs.size(); ++k) {
            for (ptrdiff_t h = incs[k], i = h + lo; i < hi; ++i) {
                T v = data_[i];
                ptrdiff_t j = i;

                while (j >= h && data_[j - h] > v) {
                    MarkArray(1, data_, j);

                    data_[j] = data_[j - h];
                    j -= h;
                }
                data_[j] = v;
            }
        }
    }

    // CircleSorting.circleSortRoutine (recursive; end is the sort's this.end field).
    template<class T>
    ptrdiff_t circleSortRoutine(std::vector<T>& data_, ptrdiff_t lo, ptrdiff_t hi, ptrdiff_t end, ptrdiff_t numSwaps) {
        if (lo == hi)
            return numSwaps;

        ptrdiff_t high = hi;
        ptrdiff_t low = lo;
        ptrdiff_t mid = (hi - lo) / 2;

        while (lo < hi) {
            if (hi < end && data_[lo] > data_[hi]) {
                using std::swap;
                swap(data_[lo], data_[hi]);
                ++numSwaps;
            }

            ++lo;
            --hi;
        }

        numSwaps = circleSortRoutine(data_, low, low + mid, end, numSwaps);
        if (low + mid + 1 < end)
            numSwaps = circleSortRoutine(data_, low + mid + 1, high, end, numSwaps);

        return numSwaps;
    }

    // IterativeCircleSorting.circleSortRoutine (one full pass over the gaps).
    // NOTE: Java returns the number of swaps; the driver (CircleSortIterative.runSort) repeats
    // this call with length rounded up to a power of two until no swaps happen. This port is
    // void per the porting spec, so a category sort that needs the count must loop until
    // isRangeSorted(data_, 0, length) instead.
    template<class T>
    void circleSortIterative(std::vector<T>& data_, ptrdiff_t length, ptrdiff_t end) {
        for (ptrdiff_t gap = length / 2; gap > 0; gap /= 2) {
            for (ptrdiff_t start = 0; start + gap < end; start += 2 * gap) {
                ptrdiff_t high = start + 2 * gap - 1;
                ptrdiff_t low = start;

                while (low < high) {
                    if (high < end && data_[low] > data_[high]) {
                        using std::swap;
                        swap(data_[low], data_[high]);
                    }

                    ++low;
                    --high;
                }
            }
        }
    }

    // CombSorting.combSort (the hybrid fallback is Java's InsertionSort.customInsertSort).
    template<class T>
    void combSort(std::vector<T>& data_, ptrdiff_t length, double shrink, bool hybrid) {
        bool swapped = false;
        ptrdiff_t gap = length;

        while ((gap > 1) || swapped)
        {
            if (gap > 1) {
                gap = static_cast<ptrdiff_t>(gap / shrink);
            }

            swapped = false;

            for (ptrdiff_t i = 0; (gap + i) < length; ++i)
            {
                if (hybrid && (gap <= ((length * 0.03125 < 8.0) ? (length * 0.03125) : 8.0))) {
                    gap = 0;

                    insertionSort(data_, 0, length);
                    break;
                }
                if (data_[i] > data_[i + gap])
                {
                    using std::swap;
                    swap(data_[i], data_[i + gap]);
                    swapped = true;
                }
                MarkArray(1, data_, i);
                MarkArray(2, data_, i + gap);
            }
        }
    }

    // BogoSorting.randInt (ThreadLocalRandom.nextInt(start, end) -> caller supplied engine).
    inline ptrdiff_t bogoRandInt(std::mt19937& engine, ptrdiff_t start, ptrdiff_t end) {
        return start + static_cast<ptrdiff_t>(engine() % static_cast<uint32_t>(end - start));
    }

    // BogoSorting.randBoolean
    inline bool bogoRandBoolean(std::mt19937& engine) {
        return (engine() & 1u) != 0;
    }

    // BogoSorting.bogoSwap (Fisher-Yates shuffle of [start, end)).
    template<class T>
    void bogoSwap(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, std::mt19937& engine) {
        for (ptrdiff_t i = start; i < end; ++i) {
            ptrdiff_t j = bogoRandInt(engine, i, end);
            using std::swap;
            swap(data_[i], data_[j]);
        }
    }

    // BogoSorting.bogoCombo (writes 0/1 into [start, end), setting size random positions to 1).
    template<class T>
    void bogoCombo(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, ptrdiff_t size, std::mt19937& engine) {
        for (ptrdiff_t i = start; i < end; ++i)
            data_[i] = 0;

        for (ptrdiff_t i = end - size; i < end; ++i) {
            ptrdiff_t j = bogoRandInt(engine, start, i + 1);
            MarkArray(1, data_, j);
            data_[data_[j] == 0 ? j : i] = 1;
        }
    }

    // BogoSorting.isRangeSorted
    template<class T>
    bool isRangeSorted(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        for (ptrdiff_t i = start; i < end - 1; ++i) {
            if (data_[i] > data_[i + 1]) {
                return false;
            }
        }
        return true;
    }

    // BogoSorting.isArraySorted
    template<class T>
    bool isArraySorted(std::vector<T>& data_, ptrdiff_t length) {
        return isRangeSorted(data_, 0, length);
    }

    // BogoSorting.isRangePartitioned (pivot is an index, as in Java).
    template<class T>
    bool isRangePartitioned(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t pivot, ptrdiff_t end) {
        for (ptrdiff_t i = start; i < pivot; ++i) {
            if (data_[i] > data_[pivot])
                return false;
        }
        for (ptrdiff_t i = pivot + 1; i < end; ++i) {
            if (data_[pivot] > data_[i])
                return false;
        }
        return true;
    }

    // BogoSorting.isMinSorted
    template<class T>
    bool isMinSorted(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        return isRangePartitioned(data_, start, start, end);
    }

    // BogoSorting.isMaxSorted
    template<class T>
    bool isMaxSorted(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        return isRangePartitioned(data_, start, end - 1, end);
    }

    // BogoSorting.isRangeSplit (mid belongs to the ending side of the range).
    template<class T>
    bool isRangeSplit(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t mid, ptrdiff_t end) {
        MarkArray(1, data_, start);
        T lowMax = data_[start];
        for (ptrdiff_t i = start + 1; i < mid; ++i) {
            MarkArray(1, data_, i);
            if (lowMax < data_[i])
                lowMax = data_[i];
        }

        for (ptrdiff_t i = mid; i < end; ++i) {
            MarkArray(1, data_, i);
            if (lowMax > data_[i])
                return false;
        }
        return true;
    }

    // MergeSorting.merge (binary insertion fallback for short ranges, as in Java).
    template<class T>
    void mergeSortMerge(std::vector<T>& data_, std::vector<T>& tmp, ptrdiff_t start, ptrdiff_t mid, ptrdiff_t end, bool binary) {
        if (start == mid) return;

        if (end - start < 32 && binary) {
            return;
        }
        else if (end - start < 64 && binary) {
            binaryInsertSort(data_, start, end);
        }
        else {
            mergeSortMerge(data_, tmp, start, (mid + start) / 2, mid, binary);
            mergeSortMerge(data_, tmp, mid, (mid + end) / 2, end, binary);

            ptrdiff_t low = start;
            ptrdiff_t high = mid;

            for (ptrdiff_t nxt = 0; nxt < end - start; ++nxt) {
                if (low >= mid && high >= end) break;

                MarkArray(1, data_, low);
                MarkArray(2, data_, high);

                if (low < mid && high >= end) {
                    tmp[nxt] = data_[low];
                    ++low;
                }
                else if (low >= mid && high < end) {
                    tmp[nxt] = data_[high];
                    ++high;
                }
                else if (data_[low] <= data_[high]) {
                    tmp[nxt] = data_[low];
                    ++low;
                }
                else {
                    tmp[nxt] = data_[high];
                    ++high;
                }
            }

            for (ptrdiff_t i = 0; i < end - start; ++i) {
                data_[start + i] = tmp[i];
            }
        }
    }

    // MergeSorting.mergeSort (sorts [0, length)).
    template<class T>
    void mergeSort(std::vector<T>& data_, ptrdiff_t length, bool binary) {
        if (length < 32 && binary) {
            binaryInsertSort(data_, 0, length);
            return;
        }

        std::vector<T> tmp(length);

        ptrdiff_t start = 0;
        ptrdiff_t end = length;
        ptrdiff_t mid = start + ((end - start) / 2);

        mergeSortMerge(data_, tmp, start, mid, end, binary);
    }

    // TwinSorting.twinSwap (returns 1 when the whole range was found reversed and fixed).
    template<class T>
    int twinSwap(std::vector<T>& data_, ptrdiff_t left, ptrdiff_t nmemb) {
        ptrdiff_t index, start, end;

        index = 0;
        end = nmemb - 2;

        while (index <= end) {
            if (data_[index + left] <= data_[index + 1 + left]) {
                index += 2;
                continue;
            }

            start = index;
            index += 2;

            while (true) {
                if (index > end) {
                    if (start == 0) {
                        if (nmemb % 2 == 0 || data_[index - 1 + left] > data_[index + left]) {
                            // the entire array was reversed
                            end = nmemb - 1;

                            while (start < end) {
                                using std::swap;
                                swap(data_[start + left], data_[end + left]);
                                ++start;
                                --end;
                            }
                            return 1;
                        }
                    }
                    break;
                }

                if (data_[index + left] > data_[index + 1 + left]) {
                    if (data_[index - 1 + left] > data_[index + left]) {
                        index += 2;
                        continue;
                    }

                    using std::swap;
                    swap(data_[index + left], data_[index + 1 + left]);
                }
                break;
            }

            end = index - 1;

            while (start < end) {
                using std::swap;
                swap(data_[start + left], data_[end + left]);
                ++start;
                --end;
            }

            end = nmemb - 2;

            index += 2;
        }
        return 0;
    }

    // TwinSorting.tailMerge (bottom up merge sort; swap_ holds at most nmemb / 2 items).
    template<class T>
    void tailMerge(std::vector<T>& data_, ptrdiff_t left, std::vector<T>& swap_, ptrdiff_t nmemb, ptrdiff_t block) {
        ptrdiff_t offset;
        ptrdiff_t a, s, c, c_max, d, d_max, e;

        s = 0;

        while (block < nmemb) {
            for (offset = 0; offset + block < nmemb; offset += block * 2) {
                a = offset;
                e = a + block - 1;

                if (data_[e + left] <= data_[e + 1 + left])
                    continue;

                if (offset + block * 2 <= nmemb) {
                    c_max = s + block;
                    d_max = a + block * 2;
                }

                else {
                    c_max = s + nmemb - (offset + block);
                    d_max = 0 + nmemb;
                }

                d = d_max - 1;

                while (data_[e + left] <= data_[d + left]) {
                    --d_max;
                    --d;
                    --c_max;
                }

                c = s;
                d = a + block;

                while (c < c_max) {
                    swap_[c] = data_[d + left];
                    MarkArray(1, data_, d + left);
                    ++c;
                    ++d;
                }
                --c;

                d = a + block - 1;
                e = d_max - 1;

                if (data_[a + left] <= data_[a + block + left]) {
                    data_[e + left] = data_[d + left];
                    --e;
                    --d;

                    while (c >= s) {
                        while (data_[d + left] > swap_[c]) {
                            MarkArray(2, data_, c + left + offset);
                            data_[e + left] = data_[d + left];
                            --e;
                            --d;
                        }

                        MarkArray(2, data_, c + left + offset);
                        data_[e + left] = swap_[c];
                        --e;
                        --c;
                    }
                }

                else {
                    data_[e + left] = data_[d + left];
                    --e;
                    --d;

                    while (d >= a)
                    {
                        while (data_[d + left] <= swap_[c]) {
                            MarkArray(2, data_, c + left + offset);
                            data_[e + left] = swap_[c];
                            --e;
                            --c;
                        }

                        data_[e + left] = data_[d + left];
                        --e;
                        --d;
                    }

                    while (c >= s) {
                        MarkArray(2, data_, c + left + offset);
                        data_[e + left] = swap_[c];
                        --e;
                        --c;
                    }

                }
            }
            block *= 2;
        }
    }

    // TwinSorting.twinsortSwap
    template<class T>
    void twinsortSwap(std::vector<T>& data_, ptrdiff_t start, std::vector<T>& swap_, ptrdiff_t nmemb) {
        if (twinSwap(data_, start, nmemb) == 0)
            tailMerge(data_, start, swap_, nmemb, 2);
    }

    // TwinSorting.twinsort
    template<class T>
    void twinsort(std::vector<T>& data_, ptrdiff_t nmemb) {
        if (twinSwap(data_, 0, nmemb) == 0) {
            std::vector<T> swap_(nmemb / 2);

            tailMerge(data_, 0, swap_, nmemb, 2);
        }
    }

    // TwinSorting.tailsort
    template<class T>
    void tailsort(std::vector<T>& data_, ptrdiff_t nmemb) {
        if (nmemb < 2)
            return;

        std::vector<T> swap_(nmemb / 2);
        tailMerge(data_, 0, swap_, nmemb, 1);
    }

    // MultiWayMergeSorting.keyLessThan (ties are broken by the run index, a < b).
    template<class T>
    bool kWayKeyLessThan(std::vector<T>& src, std::vector<ptrdiff_t>& pa, ptrdiff_t a, ptrdiff_t b) {
        int cmp = CompareValues(src[pa[a]], src[pa[b]]);
        return cmp < 0 || (cmp == 0 && a < b);
    }

    // MultiWayMergeSorting.siftDown
    template<class T>
    void kWaySiftDown(std::vector<T>& src, std::vector<ptrdiff_t>& heap, std::vector<ptrdiff_t>& pa, ptrdiff_t t, ptrdiff_t r, ptrdiff_t size) {
        while (2 * r + 2 < size) {
            ptrdiff_t nxt = 2 * r + 1;
            ptrdiff_t min = nxt + (kWayKeyLessThan(src, pa, heap[nxt], heap[nxt + 1]) ? 0 : 1);

            if (kWayKeyLessThan(src, pa, heap[min], t)) {
                heap[r] = heap[min];
                r = min;
            }
            else break;
        }
        ptrdiff_t min = 2 * r + 1;

        if (min < size && kWayKeyLessThan(src, pa, heap[min], t)) {
            heap[r] = heap[min];
            r = min;
        }
        heap[r] = t;
    }

    // MultiWayMergeSorting.kWayMerge
    template<class T>
    void kWayMerge(std::vector<T>& src, std::vector<T>& dest, std::vector<ptrdiff_t>& heap, std::vector<ptrdiff_t>& pa, std::vector<ptrdiff_t>& pb, ptrdiff_t size) {
        for (ptrdiff_t i = 0; i < size; i++)
            heap[i] = i;

        for (ptrdiff_t i = (size - 1) / 2; i >= 0; i--)
            kWaySiftDown(src, heap, pa, heap[i], i, size);

        for (ptrdiff_t i = 0; size > 0; i++) {
            ptrdiff_t min = heap[0];

            MarkArray(2, src, pa[min]);

            dest[i] = src[pa[min]];
            pa[min] = pa[min] + 1;

            if (pa[min] == pb[min]) {
                --size;
                kWaySiftDown(src, heap, pa, heap[size], 0, size);
            }
            else
                kWaySiftDown(src, heap, pa, heap[0], 0, size);
        }
    }

    // === SECTION 1 END ===

    // === SECTION 2: block / bit / shatter / pdq / quad ===

    // BlockMergeSorting.MRUN
    inline constexpr ptrdiff_t BlockMergeMRUN = 16;

    // BlockMergeSorting.shiftFW
    template<class T>
    void blockShiftFW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        while (m < b) {
            using std::swap;
            swap(data_[a], data_[m]);
            ++a;
            ++m;
        }
    }

    // BlockMergeSorting.shiftBW
    template<class T>
    void blockShiftBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        while (m > a) {
            --b;
            --m;
            using std::swap;
            swap(data_[b], data_[m]);
        }
    }

    // BlockMergeSorting.shiftFWExt
    template<class T>
    void blockShiftFWExt(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        while (m < b) {
            data_[a] = data_[m];
            ++a;
            ++m;
        }
    }

    // BlockMergeSorting.shiftBWExt
    template<class T>
    void blockShiftBWExt(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        while (m > a) {
            --b;
            --m;
            data_[b] = data_[m];
        }
    }

    // BlockMergeSorting.insertTo
    template<class T>
    void blockInsertTo(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        T temp = data_[a];
        while (a > b) {
            data_[a] = data_[a - 1];
            --a;
        }
        data_[b] = temp;
    }

    // BlockMergeSorting.insertToBW
    template<class T>
    void blockInsertToBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        T temp = data_[a];
        while (a < b) {
            data_[a] = data_[a + 1];
            ++a;
        }
        data_[a] = temp;
    }

    // BlockMergeSorting.multiSwap
    template<class T>
    void blockMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
        for (ptrdiff_t i = 0; i < len; ++i) {
            using std::swap;
            swap(data_[a + i], data_[b + i]);
        }
    }

    // BlockMergeSorting.rotate (Java delegates to IndexedRotations.cycleReverse,
    // the three reversal rotation: [m, b) is moved in front of [a, m)).
    template<class T>
    void blockRotate(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        reverseRange(data_, a, m);
        reverseRange(data_, m, b);
        reverseRange(data_, a, b);
    }

    // BlockMergeSorting.leftBinSearch
    template<class T>
    ptrdiff_t blockLeftBinSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;
            MarkArray(2, data_, m);

            if (CompareValues(val, data_[m]) <= 0)
                b = m;
            else
                a = m + 1;
        }
        return a;
    }

    // BlockMergeSorting.rightBinSearch
    template<class T>
    ptrdiff_t blockRightBinSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;
            MarkArray(2, data_, m);

            if (CompareValues(val, data_[m]) < 0)
                b = m;
            else
                a = m + 1;
        }
        return a;
    }

    // BlockMergeSorting.buildRuns
    template<class T>
    bool blockBuildRuns(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        bool noSort = true;
        ptrdiff_t i = a + 1, j = a;

        while (i < b) {
            bool descending = data_[i - 1] > data_[i];
            ++i;

            if (descending) {
                while (i < b && data_[i - 1] > data_[i]) ++i;
                reverseRange(data_, j, i);
            }
            else {
                while (i < b && data_[i - 1] <= data_[i]) ++i;
            }

            if (i < b) {
                noSort = false;
                j = i - (i - j - 1) % BlockMergeMRUN - 1; // a%b, if(a%b == 0) -> a = b
            }
            while (i - j < BlockMergeMRUN && i < b) {
                ptrdiff_t loc = blockRightBinSearch(data_, j, i, data_[i]);
                blockInsertTo(data_, i, loc);
                ++i;
            }
            j = i;
            ++i;
        }
        return noSort;
    }

    // BlockMergeSorting.findKeys
    template<class T>
    ptrdiff_t blockFindKeys(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t nKeys, ptrdiff_t n) {
        ptrdiff_t p = a, pEnd = a + nKeys;

        for (ptrdiff_t i = pEnd; i < b && nKeys < n; ++i) {
            MarkArray(1, data_, i);
            ptrdiff_t loc = blockLeftBinSearch(data_, p, pEnd, data_[i]);

            if (pEnd == loc || CompareValues(data_[i], data_[loc]) != 0) {
                blockRotate(data_, p, pEnd, i);
                ptrdiff_t inc = i - pEnd;
                loc += inc;
                p += inc;
                pEnd += inc;

                blockInsertTo(data_, pEnd, loc);
                ++nKeys;
                ++pEnd;
            }
        }
        blockRotate(data_, a, p, pEnd);
        return nKeys;
    }

    // BlockMergeSorting.findKeysBW
    template<class T>
    ptrdiff_t blockFindKeysBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t nKeys, ptrdiff_t n) {
        ptrdiff_t p = b - nKeys, pEnd = b;

        for (ptrdiff_t i = p - 1; i >= a && nKeys < n; --i) {
            MarkArray(1, data_, i);
            ptrdiff_t loc = blockLeftBinSearch(data_, p, pEnd, data_[i]);

            if (pEnd == loc || CompareValues(data_[i], data_[loc]) != 0) {
                blockRotate(data_, i + 1, p, pEnd);
                ptrdiff_t inc = p - (i + 1);
                loc -= inc;
                pEnd -= inc;
                p -= inc + 1;
                ++nKeys;

                blockInsertToBW(data_, i, loc - 1);
            }
        }
        blockRotate(data_, p, pEnd, b);
        return nKeys;
    }

    // BlockMergeSorting.binaryInsertion
    template<class T>
    void blockBinaryInsertion(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        for (ptrdiff_t i = a + 1; i < b; ++i) {
            ptrdiff_t loc = blockRightBinSearch(data_, a, i, data_[i]);
            blockInsertTo(data_, i, loc);
        }
    }

    // BlockMergeSorting.boundCheck
    template<class T>
    bool blockBoundCheck(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        return m >= b || CompareValues(data_[m - 1], data_[m]) <= 0;
    }

    // BlockMergeSorting.mergeBW
    template<class T>
    void blockMergeBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        if (blockBoundCheck(data_, a, m, b)) return;

        ptrdiff_t pLen = b - m;
        blockMultiSwap(data_, m, p, pLen);

        ptrdiff_t i = pLen - 1, j = m - 1, k = b - 1;

        using std::swap;
        while (i >= 0 && j >= a) {
            if (CompareValues(data_[p + i], data_[j]) >= 0) {
                swap(data_[k], data_[p + i]);
                --k;
                --i;
            }
            else {
                swap(data_[k], data_[j]);
                --k;
                --j;
            }
        }
        while (i >= 0) {
            swap(data_[k], data_[p + i]);
            --k;
            --i;
        }
    }

    // BlockMergeSorting.mergeTo
    template<class T>
    void blockMergeTo(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t i = a, j = m;

        using std::swap;
        while (i < m && j < b) {
            if (CompareValues(data_[i], data_[j]) <= 0) {
                swap(data_[p], data_[i]);
                ++p; ++i;
            }
            else {
                swap(data_[p], data_[j]);
                ++p; ++j;
            }
        }
        while (i < m) {
            swap(data_[p], data_[i]);
            ++p; ++i;
        }
        while (j < b) {
            swap(data_[p], data_[j]);
            ++p; ++j;
        }
    }

    // BlockMergeSorting.pingPongMerge
    template<class T>
    void blockPingPongMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m1, ptrdiff_t m2, ptrdiff_t m3, ptrdiff_t b, ptrdiff_t p) {
        if (CompareValues(data_[m1 - 1], data_[m1]) > 0
            || (m3 < b && CompareValues(data_[m3 - 1], data_[m3]) > 0)) {
            ptrdiff_t p1 = p + m2 - a, pEnd = p + b - a;

            blockMergeTo(data_, a, m1, m2, p);
            blockMergeTo(data_, m2, m3, b, p1);
            blockMergeTo(data_, p, p1, pEnd, a);
        }
        else blockMergeBW(data_, a, m2, b, p);
    }

    // BlockMergeSorting.mergeFWExt
    template<class T>
    void blockMergeFWExt(std::vector<T>& data_, std::vector<T>& tmp, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t s = m - a;

        for (ptrdiff_t i = 0; i < s; ++i)
            tmp[i] = data_[a + i];

        ptrdiff_t i = 0, j = m;

        while (i < s && j < b) {
            if (CompareValues(tmp[i], data_[j]) <= 0) {
                data_[a] = tmp[i];
                ++a; ++i;
            }
            else {
                data_[a] = data_[j];
                ++a; ++j;
            }
        }
        while (i < s) {
            data_[a] = tmp[i];
            ++a; ++i;
        }
    }

    // BlockMergeSorting.mergeBWExt
    template<class T>
    void blockMergeBWExt(std::vector<T>& data_, std::vector<T>& tmp, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t s = b - m;

        for (ptrdiff_t i = 0; i < s; ++i)
            tmp[i] = data_[m + i];

        ptrdiff_t i = s - 1, j = m - 1;

        while (i >= 0 && j >= a) {
            if (CompareValues(tmp[i], data_[j]) >= 0) {
                --b;
                data_[b] = tmp[i];
                --i;
            }
            else {
                --b;
                data_[b] = data_[j];
                --j;
            }
        }
        while (i >= 0) {
            --b;
            data_[b] = tmp[i];
            --i;
        }
    }

    // BlockMergeSorting.mergeWithBufFW
    template<class T>
    void blockMergeWithBufFW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t i = m;

        using std::swap;
        while (a < m && i < b) {
            MarkArray(2, data_, i);

            if (CompareValues(data_[a], data_[i]) <= 0) {
                swap(data_[p], data_[a]);
                ++p; ++a;
            }
            else {
                swap(data_[p], data_[i]);
                ++p; ++i;
            }
        }

        if (a > p) blockShiftFW(data_, p, a, m);

        blockShiftFW(data_, p, i, b);
    }

    // BlockMergeSorting.mergeWithBufBW
    template<class T>
    void blockMergeWithBufBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t i = m - 1;
        --b;

        using std::swap;
        while (b >= m && i >= a) {
            MarkArray(2, data_, i);

            if (CompareValues(data_[b], data_[i]) >= 0) {
                --p;
                swap(data_[p], data_[b]);
                --b;
            }
            else {
                --p;
                swap(data_[p], data_[i]);
                --i;
            }
        }

        if (p > b) blockShiftBW(data_, m, b + 1, p);

        blockShiftBW(data_, a, i + 1, p);
    }

    // BlockMergeSorting.mergeWithBufFWExt
    template<class T>
    void blockMergeWithBufFWExt(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t i = m;

        while (a < m && i < b) {
            MarkArray(2, data_, i);

            if (CompareValues(data_[a], data_[i]) <= 0) {
                data_[p] = data_[a];
                ++p; ++a;
            }
            else {
                data_[p] = data_[i];
                ++p; ++i;
            }
        }

        if (a > p) blockShiftFWExt(data_, p, a, m);

        blockShiftFWExt(data_, p, i, b);
    }

    // BlockMergeSorting.mergeWithBufBWExt
    template<class T>
    void blockMergeWithBufBWExt(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t i = m - 1;
        --b;

        while (b >= m && i >= a) {
            MarkArray(2, data_, i);

            if (CompareValues(data_[b], data_[i]) >= 0) {
                --p;
                data_[p] = data_[b];
                --b;
            }
            else {
                --p;
                data_[p] = data_[i];
                --i;
            }
        }

        if (p > b) blockShiftBWExt(data_, m, b + 1, p);

        blockShiftBWExt(data_, a, i + 1, p);
    }

    // BlockMergeSorting.inPlaceMerge
    template<class T>
    void blockInPlaceMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        while (a < m && m < b) {
            a = blockRightBinSearch(data_, a, m, data_[m]);

            if (a == m) return;

            ptrdiff_t i = blockLeftBinSearch(data_, m, b, data_[a]);

            blockRotate(data_, a, m, i);

            ptrdiff_t t = i - m;
            m = i;
            a += t + 1;
        }
    }

    // BlockMergeSorting.inPlaceMergeBW
    template<class T>
    void blockInPlaceMergeBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        while (b > m && m > a) {
            ptrdiff_t i = blockRightBinSearch(data_, a, m, data_[b - 1]);

            blockRotate(data_, i, m, b);

            ptrdiff_t t = m - i;
            m = i;
            b -= t + 1;

            if (m == a) break;

            b = blockLeftBinSearch(data_, m, b, data_[m - 1]);
        }
    }

    // BinaryQuickSorting.Task (bit is a bit index, p is the first index, r the last index).
    struct BinaryQuickTask {
        ptrdiff_t p;
        ptrdiff_t r;
        ptrdiff_t bit;
    };

    // BinaryQuickSorting.getBit: (data_[i] >> bit) & 1 with Java's arithmetic shift.
    template<class T>
    int binaryQuickGetBit(std::vector<T>& data_, ptrdiff_t i, ptrdiff_t bit) {
        return (static_cast<int>(data_[i]) >> bit) & 1;
    }

    // BinaryQuickSorting.partition
    template<class T>
    ptrdiff_t binaryQuickSortPartition(std::vector<T>& data_, ptrdiff_t p, ptrdiff_t r, ptrdiff_t bit) {
        ptrdiff_t i = p - 1;
        ptrdiff_t j = r + 1;

        using std::swap;
        while (true) {
            // Left is not set
            ++i;
            while (i <= r && binaryQuickGetBit(data_, i, bit) == 0) {
                ++i;
                MarkArray(1, data_, i);
            }
            // Right is set
            --j;
            while (j >= p && binaryQuickGetBit(data_, j, bit) == 1) {
                --j;
                MarkArray(2, data_, j);
            }
            // If i is less than j, we swap, otherwise we are done
            if (i < j) {
                swap(data_[i], data_[j]);
            }
            else {
                return j;
            }
        }
    }

    // BinaryQuickSorting.binaryQuickSortRecursive (r is the last index, as in Java).
    template<class T>
    void binaryQuickSortRecursive(std::vector<T>& data_, ptrdiff_t p, ptrdiff_t r, ptrdiff_t bit) {
        if (p < r && bit >= 0) {
            ptrdiff_t q = binaryQuickSortPartition(data_, p, r, bit);
            binaryQuickSortRecursive(data_, p, q, bit - 1);
            binaryQuickSortRecursive(data_, q + 1, r, bit - 1);
        }
    }

    // Java's Reads.analyzeBit: index of the highest set bit of the largest value in
    // [start, end). Needed because the queue driver below takes no bit argument.
    template<class T>
    ptrdiff_t binaryQuickSortHighBit(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        int highestValue = 0;
        for (ptrdiff_t i = start; i < end; ++i) {
            int value = static_cast<int>(data_[i]);
            if (highestValue < value) highestValue = value;
        }

        ptrdiff_t bit = 0;
        while ((highestValue >>= 1) != 0) ++bit;
        return bit;
    }

    // BinaryQuickSorting.binaryQuickSort, with an explicit starting bit (queue driver;
    // r is the last index, as in Java).
    template<class T>
    void binaryQuickSortBits(std::vector<T>& data_, ptrdiff_t p, ptrdiff_t r, ptrdiff_t bit) {
        std::queue<BinaryQuickTask> tasks;
        tasks.push(BinaryQuickTask{ p, r, bit });

        while (!tasks.empty()) {
            BinaryQuickTask task = tasks.front();
            tasks.pop();

            if (task.p < task.r && task.bit >= 0) {
                ptrdiff_t q = binaryQuickSortPartition(data_, task.p, task.r, task.bit);
                tasks.push(BinaryQuickTask{ task.p, q, task.bit - 1 });
                tasks.push(BinaryQuickTask{ q + 1, task.r, task.bit - 1 });
            }
        }
    }

    // BinaryQuickSorting.binaryQuickSort driver: r is the last index and the starting
    // bit is derived from the values in [p, r] (Java's Reads.analyzeBit).
    template<class T>
    void binaryQuickSort(std::vector<T>& data_, ptrdiff_t p, ptrdiff_t r) {
        binaryQuickSortBits(data_, p, r, binaryQuickSortHighBit(data_, p, r + 1));
    }

    // ShatterSorting.shatterPartition. Registers are supplied by the caller (Java allocates
    // them internally) and are resized here. Note array[i] / num assumes values >= 0 (same
    // as Java; the category sort adds the range checks).
    template<class T>
    void shatterPartition(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, ptrdiff_t num,
                          std::vector<std::vector<ptrdiff_t>>& registers) {
        ptrdiff_t length = end - start;
        ptrdiff_t shatters = static_cast<ptrdiff_t>(std::ceil(length / static_cast<double>(num)));

        registers.assign(static_cast<size_t>(shatters), std::vector<ptrdiff_t>());

        for (ptrdiff_t i = 0; i < length; ++i) {
            ptrdiff_t value = static_cast<ptrdiff_t>(data_[start + i]);
            registers[value / num].push_back(value);
            MarkArray(1, data_, start + i);
        }

        // Writes.transcribe(array, registers, 0, ...): the bucket values are written back
        // in bucket order.
        ptrdiff_t w = start;
        for (size_t i = 0; i < registers.size(); ++i) {
            for (size_t j = 0; j < registers[i].size(); ++j) {
                data_[w] = static_cast<int>(registers[i][j]);
                ++w;
            }
        }
    }

    // ShatterSorting.shatterSort
    template<class T>
    void shatterSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, ptrdiff_t num) {
        ptrdiff_t length = end - start;
        ptrdiff_t shatters = static_cast<ptrdiff_t>(std::ceil(length / static_cast<double>(num)));

        std::vector<std::vector<ptrdiff_t>> registers;
        shatterPartition(data_, start, end, num, registers);

        std::vector<ptrdiff_t> tmp(num);
        for (ptrdiff_t i = 0; i < shatters; ++i) {
            for (ptrdiff_t j = 0; j < num; ++j) {
                if (i * num + j >= length)
                    tmp[j] = -1;
                else
                    tmp[j] = static_cast<ptrdiff_t>(data_[start + i * num + j]);

                MarkArray(2, data_, start + i * num + j);
            }

            for (size_t j = 0; j < tmp.size(); ++j) {
                ptrdiff_t tmpj = tmp[j];

                if (i * num + (tmpj % num) >= length || tmpj == -1) {
                    break;
                }

                data_[start + i * num + (tmpj % num)] = static_cast<int>(tmpj);
                MarkArray(1, data_, start + i * num + (tmpj % num));
            }
        }
    }

    // ShatterSorting.simpleShatterSort (length, not start/end, as in Java).
    template<class T>
    void simpleShatterSort(std::vector<T>& data_, ptrdiff_t length, ptrdiff_t num, ptrdiff_t rate) {
        std::vector<std::vector<ptrdiff_t>> registers;

        for (ptrdiff_t i = num; i > 1; i = i / rate) {
            shatterPartition(data_, 0, length, i, registers);
        }
        shatterPartition(data_, 0, length, 1, registers);
    }

    // PDQSorting thresholds.
    inline constexpr ptrdiff_t PdqInsertSortThreshold = 24;
    inline constexpr ptrdiff_t PdqNintherThreshold = 128;
    inline constexpr ptrdiff_t PdqPartialInsertSortLimit = 8;
    inline constexpr ptrdiff_t PdqBlockSize = 64;
    inline constexpr ptrdiff_t PdqCachelineSize = 64;

    // PDQSorting PDQPair.
    struct PdqPair {
        ptrdiff_t pivotPosition;
        bool alreadyPartitioned;
    };

    // PDQSorting.pdqLog: floor(log2(n)), assumes n > 0.
    inline ptrdiff_t pdqLog(ptrdiff_t n) {
        ptrdiff_t log = 0;
        while ((n >>= 1) != 0) ++log;
        return log;
    }

    // PDQSorting.pdqLessThan (Java derives this from Boolean.hashCode; both yield
    // 1 when a < b and 0 otherwise).
    template<class T>
    int pdqLessThan(const T& a, const T& b) {
        return (a < b) ? 1 : 0;
    }

    // PDQSorting.pdqInsertSort
    template<class T>
    void pdqInsertSort(std::vector<T>& data_, ptrdiff_t begin, ptrdiff_t end) {
        if (begin == end) return;

        for (ptrdiff_t cur = begin + 1; cur != end; ++cur) {
            ptrdiff_t sift = cur;
            ptrdiff_t siftMinusOne = cur - 1;

            // Compare first so we can avoid 2 moves for an element already positioned correctly.
            if (CompareValues(data_[sift], data_[siftMinusOne]) < 0) {
                T tmp = data_[sift];
                do {
                    data_[sift] = data_[siftMinusOne];
                    --sift;
                    if (sift == begin) break;
                    --siftMinusOne;
                } while (CompareValues(tmp, data_[siftMinusOne]) < 0);

                data_[sift] = tmp;
            }
        }
    }

    // PDQSorting.pdqUnguardInsertSort (assumes data_[begin - 1] is a lower bound).
    template<class T>
    void pdqUnguardInsertSort(std::vector<T>& data_, ptrdiff_t begin, ptrdiff_t end) {
        if (begin == end) return;

        for (ptrdiff_t cur = begin + 1; cur != end; ++cur) {
            ptrdiff_t sift = cur;
            ptrdiff_t siftMinusOne = cur - 1;

            // Compare first so we can avoid 2 moves for an element already positioned correctly.
            if (CompareValues(data_[sift], data_[siftMinusOne]) < 0) {
                T tmp = data_[sift];

                do {
                    data_[sift] = data_[siftMinusOne];
                    --sift;
                    --siftMinusOne;
                } while (CompareValues(tmp, data_[siftMinusOne]) < 0);

                data_[sift] = tmp;
            }
        }
    }

    // PDQSorting.pdqPartialInsertSort
    template<class T>
    bool pdqPartialInsertSort(std::vector<T>& data_, ptrdiff_t begin, ptrdiff_t end) {
        if (begin == end) return true;

        ptrdiff_t limit = 0;
        for (ptrdiff_t cur = begin + 1; cur != end; ++cur) {
            if (limit > PdqPartialInsertSortLimit) return false;

            ptrdiff_t sift = cur;
            ptrdiff_t siftMinusOne = cur - 1;

            // Compare first so we can avoid 2 moves for an element already positioned correctly.
            if (CompareValues(data_[sift], data_[siftMinusOne]) < 0) {
                T tmp = data_[sift];

                do {
                    data_[sift] = data_[siftMinusOne];
                    --sift;
                    if (sift == begin) break;
                    --siftMinusOne;
                } while (CompareValues(tmp, data_[siftMinusOne]) < 0);

                data_[sift] = tmp;
                limit += cur - sift;
            }
        }
        return true;
    }

    // PDQSorting.pdqSortTwo
    template<class T>
    void pdqSortTwo(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        if (CompareValues(data_[b], data_[a]) < 0) {
            using std::swap;
            swap(data_[a], data_[b]);
        }
    }

    // PDQSorting.pdqSortThree
    template<class T>
    void pdqSortThree(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t c) {
        pdqSortTwo(data_, a, b);
        pdqSortTwo(data_, b, c);
        pdqSortTwo(data_, a, b);
    }

    // PDQSorting.pdqSwapOffsets
    template<class T>
    void pdqSwapOffsets(std::vector<T>& data_, ptrdiff_t first, ptrdiff_t last,
                        std::vector<ptrdiff_t>& leftOffsets, ptrdiff_t leftOffsetsPos,
                        std::vector<ptrdiff_t>& rightOffsets, ptrdiff_t rightOffsetsPos,
                        ptrdiff_t num, bool useSwaps) {
        using std::swap;
        if (useSwaps) {
            // This case is needed for the descending distribution, where we need
            // to have proper swapping for pdqsort to remain O(n).
            for (ptrdiff_t i = 0; i < num; ++i) {
                swap(data_[first + leftOffsets[leftOffsetsPos + i]],
                     data_[last - rightOffsets[rightOffsetsPos + i]]);
            }
        }
        else if (num > 0) {
            ptrdiff_t left = first + leftOffsets[leftOffsetsPos];
            ptrdiff_t right = last - rightOffsets[rightOffsetsPos];
            T tmp = data_[left];
            data_[left] = data_[right];
            for (ptrdiff_t i = 1; i < num; ++i) {
                left = first + leftOffsets[leftOffsetsPos + i];
                data_[right] = data_[left];
                right = last - rightOffsets[rightOffsetsPos + i];
                data_[left] = data_[right];
            }
            data_[right] = tmp;
        }
    }

    // PDQSorting.pdqPartRightBranchless (branchless block partitioning; the offset
    // arrays are Java instance fields created in visualizeAux, they become locals here).
    template<class T>
    PdqPair pdqPartRightBranchless(std::vector<T>& data_, ptrdiff_t begin, ptrdiff_t end) {
        // Move pivot into local for speed.
        T pivot = data_[begin];
        ptrdiff_t first = begin;
        ptrdiff_t last = end;

        ptrdiff_t leftNum = 0, rightNum = 0, leftStart = 0, rightStart = 0;

        // Find the first element greater than or equal than the pivot (the median of 3
        // guarantees this exists).
        while (pdqLessThan(data_[++first], pivot) == 1);

        // Find the first element strictly smaller than the pivot. We have to guard this
        // search if there was no element before *first.
        if (first - 1 == begin)
            while (first < last && pdqLessThan(data_[--last], pivot) == 0);
        else
            while (pdqLessThan(data_[--last], pivot) == 0);

        // If the first pair of elements that should be swapped to partition are the same
        // element, the passed in sequence already was correctly partitioned.
        bool alreadyParted = first >= last;
        if (!alreadyParted) {
            using std::swap;
            swap(data_[first], data_[last]);
            ++first;
        }

        // The following branchless partitioning is derived from "BlockQuicksort: How Branch
        // Mispredictions don't affect Quicksort" by Stefan Edelkamp and Armin Weiss.
        std::vector<ptrdiff_t> leftOffsets(static_cast<size_t>(PdqBlockSize + PdqCachelineSize));
        std::vector<ptrdiff_t> rightOffsets(static_cast<size_t>(PdqBlockSize + PdqCachelineSize));

        while (last - first > 2 * PdqBlockSize) {
            // Fill up offset blocks with elements that are on the wrong side.
            if (leftNum == 0) {
                leftStart = 0;
                ptrdiff_t it = first;
                for (ptrdiff_t i = 0; i < PdqBlockSize;) {
                    leftOffsets[leftNum] = i; ++i; leftNum += 1 - pdqLessThan(data_[it++], pivot);
                    MarkArray(2, data_, it);
                    leftOffsets[leftNum] = i; ++i; leftNum += 1 - pdqLessThan(data_[it++], pivot);
                    MarkArray(2, data_, it);
                    leftOffsets[leftNum] = i; ++i; leftNum += 1 - pdqLessThan(data_[it++], pivot);
                    MarkArray(2, data_, it);
                    leftOffsets[leftNum] = i; ++i; leftNum += 1 - pdqLessThan(data_[it++], pivot);
                    MarkArray(2, data_, it);
                    leftOffsets[leftNum] = i; ++i; leftNum += 1 - pdqLessThan(data_[it++], pivot);
                    MarkArray(2, data_, it);
                    leftOffsets[leftNum] = i; ++i; leftNum += 1 - pdqLessThan(data_[it++], pivot);
                    MarkArray(2, data_, it);
                    leftOffsets[leftNum] = i; ++i; leftNum += 1 - pdqLessThan(data_[it++], pivot);
                    MarkArray(2, data_, it);
                    leftOffsets[leftNum] = i; ++i; leftNum += 1 - pdqLessThan(data_[it++], pivot);
                    MarkArray(2, data_, it);
                }
            }
            if (rightNum == 0) {
                rightStart = 0;
                ptrdiff_t it = last;
                for (ptrdiff_t i = 0; i < PdqBlockSize;) {
                    ++i; rightOffsets[rightNum] = i; --it; rightNum += pdqLessThan(data_[it], pivot);
                    MarkArray(2, data_, it);
                    ++i; rightOffsets[rightNum] = i; --it; rightNum += pdqLessThan(data_[it], pivot);
                    MarkArray(2, data_, it);
                    ++i; rightOffsets[rightNum] = i; --it; rightNum += pdqLessThan(data_[it], pivot);
                    MarkArray(2, data_, it);
                    ++i; rightOffsets[rightNum] = i; --it; rightNum += pdqLessThan(data_[it], pivot);
                    MarkArray(2, data_, it);
                    ++i; rightOffsets[rightNum] = i; --it; rightNum += pdqLessThan(data_[it], pivot);
                    MarkArray(2, data_, it);
                    ++i; rightOffsets[rightNum] = i; --it; rightNum += pdqLessThan(data_[it], pivot);
                    MarkArray(2, data_, it);
                    ++i; rightOffsets[rightNum] = i; --it; rightNum += pdqLessThan(data_[it], pivot);
                    MarkArray(2, data_, it);
                    ++i; rightOffsets[rightNum] = i; --it; rightNum += pdqLessThan(data_[it], pivot);
                    MarkArray(2, data_, it);
                }
            }

            // Swap elements and update block sizes and first/last boundaries.
            ptrdiff_t num = (std::min)(leftNum, rightNum);
            pdqSwapOffsets(data_, first, last, leftOffsets, leftStart, rightOffsets, rightStart, num, leftNum == rightNum);
            leftNum -= num; rightNum -= num;
            leftStart += num; rightStart += num;
            if (leftNum == 0) first += PdqBlockSize;
            if (rightNum == 0) last -= PdqBlockSize;
        }

        ptrdiff_t leftSize = 0, rightSize = 0;
        ptrdiff_t unknownLeft = (last - first) - ((rightNum != 0 || leftNum != 0) ? PdqBlockSize : 0);
        if (rightNum != 0) {
            // Handle leftover block by assigning the unknown elements to the other block.
            leftSize = unknownLeft;
            rightSize = PdqBlockSize;
        }
        else if (leftNum != 0) {
            leftSize = PdqBlockSize;
            rightSize = unknownLeft;
        }
        else {
            // No leftover block, split the unknown elements in two blocks.
            leftSize = unknownLeft / 2;
            rightSize = unknownLeft - leftSize;
        }

        // Fill offset buffers if needed.
        if (unknownLeft != 0 && leftNum == 0) {
            leftStart = 0;
            ptrdiff_t it = first;
            for (ptrdiff_t i = 0; i < leftSize;) {
                leftOffsets[leftNum] = i; ++i; leftNum += 1 - pdqLessThan(data_[it++], pivot);
                MarkArray(2, data_, it);
            }
        }
        if (unknownLeft != 0 && rightNum == 0) {
            rightStart = 0;
            ptrdiff_t it = last;
            for (ptrdiff_t i = 0; i < rightSize;) {
                ++i; rightOffsets[rightNum] = i; --it; rightNum += pdqLessThan(data_[it], pivot);
                MarkArray(2, data_, it);
            }
        }

        ptrdiff_t num = (std::min)(leftNum, rightNum);
        pdqSwapOffsets(data_, first, last, leftOffsets, leftStart, rightOffsets, rightStart, num, leftNum == rightNum);
        leftNum -= num; rightNum -= num;
        leftStart += num; rightStart += num;
        if (leftNum == 0) first += leftSize;
        if (rightNum == 0) last -= rightSize;

        ptrdiff_t leftOffsetsPos = 0;
        ptrdiff_t rightOffsetsPos = 0;

        // We have now fully identified [first, last)'s proper position. Swap the last elements.
        if (leftNum != 0) {
            leftOffsetsPos += leftStart;
            while (leftNum-- != 0) {
                --last;
                using std::swap;
                swap(data_[first + leftOffsets[leftOffsetsPos + leftNum]], data_[last]);
            }
            first = last;
        }
        if (rightNum != 0) {
            rightOffsetsPos += rightStart;
            while (rightNum-- != 0) {
                using std::swap;
                swap(data_[last - rightOffsets[rightOffsetsPos + rightNum]], data_[first]);
                ++first;
            }
            last = first;
        }

        // Put the pivot in the right place.
        ptrdiff_t pivotPos = first - 1;
        data_[begin] = data_[pivotPos];
        data_[pivotPos] = pivot;

        return PdqPair{ pivotPos, alreadyParted };
    }

    // PDQSorting.pdqPartRight
    template<class T>
    PdqPair pdqPartRight(std::vector<T>& data_, ptrdiff_t begin, ptrdiff_t end) {
        // Move pivot into local for speed.
        T pivot = data_[begin];
        ptrdiff_t first = begin;
        ptrdiff_t last = end;

        using std::swap;

        // Find the first element greater than or equal than the pivot (the median of 3
        // guarantees this exists).
        ++first;
        while (CompareValues(data_[first], pivot) < 0) {
            MarkArray(1, data_, first);
            ++first;
        }

        // Find the first element strictly smaller than the pivot. We have to guard this
        // search if there was no element before *first.
        if (first - 1 == begin) {
            while (first < last) {
                --last;
                if (CompareValues(data_[last], pivot) < 0) break;
                MarkArray(2, data_, last);
            }
        }
        else {
            while (true) {
                --last;
                if (CompareValues(data_[last], pivot) < 0) break;
                MarkArray(2, data_, last);
            }
        }

        // If the first pair of elements that should be swapped to partition are the same
        // element, the passed in sequence already was correctly partitioned.
        bool alreadyParted = first >= last;

        // Keep swapping pairs of elements that are on the wrong side of the pivot. Previously
        // swapped pairs guard the searches, which is why the first iteration is special-cased
        // above.
        while (first < last) {
            swap(data_[first], data_[last]);
            ++first;
            while (CompareValues(data_[first], pivot) < 0) {
                MarkArray(1, data_, first);
                ++first;
            }
            while (true) {
                --last;
                if (CompareValues(data_[last], pivot) < 0) break;
                MarkArray(2, data_, last);
            }
        }

        // Put the pivot in the right place.
        ptrdiff_t pivotPos = first - 1;
        data_[begin] = data_[pivotPos];
        data_[pivotPos] = pivot;

        return PdqPair{ pivotPos, alreadyParted };
    }

    // PDQSorting.pdqPartLeft
    template<class T>
    ptrdiff_t pdqPartLeft(std::vector<T>& data_, ptrdiff_t begin, ptrdiff_t end) {
        // Move pivot into local for speed.
        T pivot = data_[begin];
        ptrdiff_t first = begin;
        ptrdiff_t last = end;

        using std::swap;

        while (true) {
            --last;
            if (!(CompareValues(pivot, data_[last]) < 0)) break;
            MarkArray(2, data_, last);
        }

        if (last + 1 == end) {
            while (first < last) {
                ++first;
                if (CompareValues(pivot, data_[first]) < 0) break;
                MarkArray(1, data_, first);
            }
        }
        else {
            while (true) {
                ++first;
                if (CompareValues(pivot, data_[first]) < 0) break;
                MarkArray(1, data_, first);
            }
        }

        while (first < last) {
            swap(data_[first], data_[last]);
            while (true) {
                --last;
                if (!(CompareValues(pivot, data_[last]) < 0)) break;
                MarkArray(2, data_, last);
            }
            while (true) {
                ++first;
                if (CompareValues(pivot, data_[first]) < 0) break;
                MarkArray(1, data_, first);
            }
        }

        ptrdiff_t pivotPos = last;
        data_[begin] = data_[pivotPos];
        data_[pivotPos] = pivot;

        return pivotPos;
    }

    // PDQSorting.pdqLoop (the heap fallback is the section 1 heapSort max variant, as in
    // MaxHeapSort.customHeapSort(array, begin, end, 1)).
    template<class T>
    void pdqLoop(std::vector<T>& data_, ptrdiff_t begin, ptrdiff_t end, bool branchless, ptrdiff_t badAllowed) {
        bool leftmost = true;

        using std::swap;

        // Use a while loop for tail recursion elimination.
        while (true) {
            ptrdiff_t size = end - begin;

            // Insertion sort is faster for small arrays.
            if (size < PdqInsertSortThreshold) {
                if (leftmost) pdqInsertSort(data_, begin, end);
                else pdqUnguardInsertSort(data_, begin, end);
                return;
            }

            // Choose pivot as median of 3 or pseudomedian of 9.
            ptrdiff_t halfSize = size / 2;
            if (size > PdqNintherThreshold) {
                pdqSortThree(data_, begin, begin + halfSize, end - 1);
                pdqSortThree(data_, begin + 1, begin + (halfSize - 1), end - 2);
                pdqSortThree(data_, begin + 2, begin + (halfSize + 1), end - 3);
                pdqSortThree(data_, begin + (halfSize - 1), begin + halfSize, begin + (halfSize + 1));
                swap(data_[begin], data_[begin + halfSize]);
            }
            else pdqSortThree(data_, begin + halfSize, begin, end - 1);

            // If array[begin - 1] is the end of the right partition of a previous partition
            // operation there is no element in [begin, end) that is smaller than array[begin - 1].
            // Then if our pivot compares equal to array[begin - 1] we change strategy, putting
            // equal elements in the left partition, greater elements in the right partition.
            // We do not have to recurse on the left partition, since it's sorted (all equal).
            if (!leftmost && !(CompareValues(data_[begin - 1], data_[begin]) < 0)) {
                begin = pdqPartLeft(data_, begin, end) + 1;
                continue;
            }

            // Partition and get results.
            PdqPair partResult = branchless ? pdqPartRightBranchless(data_, begin, end)
                                            : pdqPartRight(data_, begin, end);

            ptrdiff_t pivotPos = partResult.pivotPosition;
            bool alreadyParted = partResult.alreadyPartitioned;

            // Check for a highly unbalanced partition.
            ptrdiff_t leftSize = pivotPos - begin;
            ptrdiff_t rightSize = end - (pivotPos + 1);
            bool highUnbalance = leftSize < size / 8 || rightSize < size / 8;

            // If we got a highly unbalanced partition, we shuffle elements to break many patterns.
            if (highUnbalance) {
                // If we had too many bad partitions, switch to heapsort to guarantee O(n log n).
                if (--badAllowed == 0) {
                    heapSort(data_, begin, end, true);
                    return;
                }

                if (leftSize >= PdqInsertSortThreshold) {
                    swap(data_[begin], data_[begin + leftSize / 4]);
                    swap(data_[pivotPos - 1], data_[pivotPos - leftSize / 4]);

                    if (leftSize > PdqNintherThreshold) {
                        swap(data_[begin + 1], data_[begin + (leftSize / 4 + 1)]);
                        swap(data_[begin + 2], data_[begin + (leftSize / 4 + 2)]);
                        swap(data_[pivotPos - 2], data_[pivotPos - (leftSize / 4 + 1)]);
                        swap(data_[pivotPos - 3], data_[pivotPos - (leftSize / 4 + 2)]);
                    }
                }

                if (rightSize >= PdqInsertSortThreshold) {
                    swap(data_[pivotPos + 1], data_[pivotPos + (1 + rightSize / 4)]);
                    swap(data_[end - 1], data_[end - rightSize / 4]);

                    if (rightSize > PdqNintherThreshold) {
                        swap(data_[pivotPos + 2], data_[pivotPos + (2 + rightSize / 4)]);
                        swap(data_[pivotPos + 3], data_[pivotPos + (3 + rightSize / 4)]);
                        swap(data_[end - 2], data_[end - (1 + rightSize / 4)]);
                        swap(data_[end - 3], data_[end - (2 + rightSize / 4)]);
                    }
                }
            }
            else {
                // If we were decently balanced and we tried to sort an already partitioned
                // sequence, try to use insertion sort.
                if (alreadyParted && pdqPartialInsertSort(data_, begin, pivotPos)
                                  && pdqPartialInsertSort(data_, pivotPos + 1, end))
                    return;
            }

            // Sort the left partition first using recursion and do tail recursion elimination for
            // the right-hand partition.
            pdqLoop(data_, begin, pivotPos, branchless, badAllowed);
            begin = pivotPos + 1;
            leftmost = false;
        }
    }

    // PDQSorting driver: the Java category sorts call
    // pdqLoop(array, begin, end, branchless, pdqLog(end - begin)).
    template<class T>
    void pdqSort(std::vector<T>& data_, ptrdiff_t begin, ptrdiff_t end, bool branchless) {
        pdqLoop(data_, begin, end, branchless, pdqLog(end - begin));
    }

    // QuadSortBase swapFive / tailSwapEight state. In Java pta/ptt/end are instance
    // fields (class QuadSortBase) and swapSix/Seven/Eight rely on tailSwapEight seeing
    // the "end" value left behind by swapFive.
    struct QuadSortBaseState {
        ptrdiff_t pta = 0;
        ptrdiff_t ptt = 0;
        ptrdiff_t end = 0;
    };

    // QuadSortBase.swapTwo
    template<class T>
    void quadSwapTwo(std::vector<T>& data_, ptrdiff_t start) {
        if (data_[start] > data_[start + 1]) {
            using std::swap;
            swap(data_[start], data_[start + 1]);
        }
    }

    // QuadSortBase.swapThree
    template<class T>
    void quadSwapThree(std::vector<T>& data_, ptrdiff_t start) {
        using std::swap;
        if (data_[start] > data_[start + 1]) {
            if (data_[start] <= data_[start + 2]) {
                swap(data_[start], data_[start + 1]);
            }
            else if (data_[start + 1] > data_[start + 2]) {
                swap(data_[start], data_[start + 2]);
            }
            else {
                T temp = data_[start];
                data_[start] = data_[start + 1];
                data_[start + 1] = data_[start + 2];
                data_[start + 2] = temp;
            }
        }
        else if (data_[start + 1] > data_[start + 2]) {
            if (data_[start] > data_[start + 2]) {
                T temp = data_[start + 2];
                data_[start + 2] = data_[start + 1];
                data_[start + 1] = data_[start];
                data_[start] = temp;
            }
            else {
                swap(data_[start + 2], data_[start + 1]);
            }
        }
    }

    // QuadSortBase.swapFour
    template<class T>
    void quadSwapFour(std::vector<T>& data_, ptrdiff_t start) {
        using std::swap;
        if (data_[start] > data_[start + 1])
            swap(data_[start], data_[start + 1]);

        if (data_[start + 2] > data_[start + 3])
            swap(data_[start + 2], data_[start + 3]);

        if (data_[start + 1] > data_[start + 2]) {
            if (data_[start] <= data_[start + 2]) {
                if (data_[start + 1] <= data_[start + 3]) {
                    swap(data_[start + 1], data_[start + 2]);
                }
                else {
                    T temp = data_[start + 1];
                    data_[start + 1] = data_[start + 2];
                    data_[start + 2] = data_[start + 3];
                    data_[start + 3] = temp;
                }
            }
            else if (data_[start] > data_[start + 3]) {
                swap(data_[start + 1], data_[start + 3]);
                swap(data_[start], data_[start + 2]);
            }
            else if (data_[start + 1] <= data_[start + 3]) {
                T temp = data_[start + 1];
                data_[start + 1] = data_[start];
                data_[start] = data_[start + 2];
                data_[start + 2] = temp;
            }
            else {
                T temp = data_[start + 1];
                data_[start + 1] = data_[start];
                data_[start] = data_[start + 2];
                data_[start + 2] = data_[start + 3];
                data_[start + 3] = temp;
            }
        }
    }

    // QuadSortBase.swapFive
    template<class T>
    void quadSwapFive(std::vector<T>& data_, ptrdiff_t start, QuadSortBaseState& state) {
        state.end = start + 4;
        state.pta = state.end++;
        state.ptt = state.pta--;

        if (data_[state.pta] > data_[state.ptt]) {
            T key = data_[state.ptt];
            data_[state.ptt] = data_[state.pta];
            --state.ptt;
            --state.pta;

            if (state.pta > start && data_[state.pta - 1] > key) {
                data_[state.ptt] = data_[state.pta];
                --state.ptt;
                --state.pta;
                data_[state.ptt] = data_[state.pta];
                --state.ptt;
                --state.pta;
            }

            if (state.pta >= start && data_[state.pta] > key) {
                data_[state.ptt] = data_[state.pta];
                --state.ptt;
                --state.pta;
            }

            data_[state.ptt] = key;
        }
    }

    // QuadSortBase.tailSwapEight
    template<class T>
    void quadTailSwapEight(std::vector<T>& data_, ptrdiff_t start, QuadSortBaseState& state) {
        state.pta = state.end++;
        state.ptt = state.pta--;

        if (data_[state.pta] > data_[state.ptt]) {
            T key = data_[state.ptt];
            data_[state.ptt] = data_[state.pta];
            --state.ptt;
            --state.pta;

            if (data_[state.pta - 2] > key) {
                for (ptrdiff_t i = 0; i < 3; ++i) {
                    data_[state.ptt] = data_[state.pta];
                    --state.ptt;
                    --state.pta;
                }
            }

            if (state.pta > start && data_[state.pta - 1] > key) {
                data_[state.ptt] = data_[state.pta];
                --state.ptt;
                --state.pta;
                data_[state.ptt] = data_[state.pta];
                --state.ptt;
                --state.pta;
            }

            if (state.pta >= start && data_[state.pta] > key) {
                data_[state.ptt] = data_[state.pta];
                --state.ptt;
                --state.pta;
            }

            data_[state.ptt] = key;
        }
    }

    // QuadSortBase.swapSix
    template<class T>
    void quadSwapSix(std::vector<T>& data_, ptrdiff_t start, QuadSortBaseState& state) {
        quadSwapFive(data_, start, state);
        quadTailSwapEight(data_, start, state);
    }

    // QuadSortBase.swapSeven
    template<class T>
    void quadSwapSeven(std::vector<T>& data_, ptrdiff_t start, QuadSortBaseState& state) {
        quadSwapSix(data_, start, state);
        quadTailSwapEight(data_, start, state);
    }

    // QuadSortBase.swapEight
    template<class T>
    void quadSwapEight(std::vector<T>& data_, ptrdiff_t start, QuadSortBaseState& state) {
        quadSwapSeven(data_, start, state);
        quadTailSwapEight(data_, start, state);
    }

    // QuadSorting.tailSwap
    template<class T>
    void quadTailSwap(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t nmemb) {
        ptrdiff_t top, offset;

        QuadSortBaseState qs;

        switch (nmemb) {
        case 0:
        case 1:
            return;

        case 2:
            quadSwapTwo(data_, start);
            return;

        case 3:
            quadSwapThree(data_, start);
            return;

        case 4:
            quadSwapFour(data_, start);
            return;

        case 5:
            quadSwapFour(data_, start);
            quadSwapFive(data_, start, qs);
            return;

        case 6:
            quadSwapFour(data_, start);
            quadSwapSix(data_, start, qs);
            return;

        case 7:
            quadSwapFour(data_, start);
            quadSwapSeven(data_, start, qs);
            return;

        case 8:
            quadSwapFour(data_, start);
            quadSwapEight(data_, start, qs);
            return;
        }

        quadSwapFour(data_, start);
        quadSwapEight(data_, start, qs);

        ptrdiff_t end = start + 8;
        offset = 8;

        while (offset < nmemb) {
            top = offset++;
            ptrdiff_t pta = end++;
            ptrdiff_t ptt = pta--;

            if (data_[pta] <= data_[ptt])
                continue;

            T temp = data_[ptt];

            while (top > 1) {
                ptrdiff_t mid = top / 2;
                if (data_[pta - mid] > temp)
                    pta -= mid;

                top -= mid;
            }

            // memmove(pta+1, pta, (ptt-pta) * sizeof(VAR));
            for (ptrdiff_t i = ptt; i > pta; --i)
                data_[i] = data_[i - 1];

            data_[pta] = temp;
        }
    }

    // QuadSorting.parityMerge4 (merge 4 4 into 8 from "from" to "dest"; auxOffset is 0 or 8)
    template<class T>
    void quadParityMerge4(std::vector<T>& from, ptrdiff_t start, std::vector<T>& dest, ptrdiff_t auxOffset) {
        ptrdiff_t ptl, ptr;
        ptrdiff_t auxP = auxOffset;

        ptl = start;
        ptr = start + 4;

        for (ptrdiff_t i = 0; i < 3; ++i) {
            MarkArray(2, from, ptl);
            MarkArray(3, from, ptr);

            if (CompareValues(from[ptl], from[ptr]) <= 0) {
                dest[auxP] = from[ptl];
                ++auxP; ++ptl;
            }
            else {
                dest[auxP] = from[ptr];
                ++auxP; ++ptr;
            }
        }

        MarkArray(2, from, ptl);
        MarkArray(3, from, ptr);

        if (CompareValues(from[ptl], from[ptr]) <= 0)
            dest[auxP] = from[ptl];
        else
            dest[auxP] = from[ptr];

        ptl = start + 3;
        ptr = start + 7;
        auxP += 4;

        for (ptrdiff_t i = 0; i < 3; ++i) {
            MarkArray(2, from, ptl);
            MarkArray(3, from, ptr);

            if (CompareValues(from[ptl], from[ptr]) > 0) {
                dest[auxP] = from[ptl];
                --auxP; --ptl;
            }
            else {
                dest[auxP] = from[ptr];
                --auxP; --ptr;
            }
        }

        MarkArray(2, from, ptl);
        MarkArray(3, from, ptr);

        if (CompareValues(from[ptl], from[ptr]) > 0)
            dest[auxP] = from[ptl];
        else
            dest[auxP] = from[ptr];
    }

    // QuadSorting.parityMerge8 (merge 8 8 from the aux array back into the main array)
    template<class T>
    void quadParityMerge8(std::vector<T>& from, ptrdiff_t start, std::vector<T>& dest) {
        ptrdiff_t ptl, ptr;
        ptrdiff_t mainP = start;

        ptl = 0;
        ptr = 8;

        for (ptrdiff_t i = 0; i < 7; ++i) {
            MarkArray(2, from, ptl);
            MarkArray(3, from, ptr);

            if (CompareValues(from[ptl], from[ptr]) <= 0) {
                dest[mainP] = from[ptl];
                ++mainP; ++ptl;
            }
            else {
                dest[mainP] = from[ptr];
                ++mainP; ++ptr;
            }
        }

        MarkArray(2, from, ptl);
        MarkArray(3, from, ptr);

        if (CompareValues(from[ptl], from[ptr]) <= 0)
            dest[mainP] = from[ptl];
        else
            dest[mainP] = from[ptr];

        ptl = 7;
        ptr = 15;
        mainP += 8;

        for (ptrdiff_t i = 0; i < 7; ++i) {
            MarkArray(2, from, ptl);
            MarkArray(3, from, ptr);

            if (CompareValues(from[ptl], from[ptr]) > 0) {
                dest[mainP] = from[ptl];
                --mainP; --ptl;
            }
            else {
                dest[mainP] = from[ptr];
                --mainP; --ptr;
            }
        }

        MarkArray(2, from, ptl);
        MarkArray(3, from, ptr);

        if (CompareValues(from[ptl], from[ptr]) > 0)
            dest[mainP] = from[ptl];
        else
            dest[mainP] = from[ptr];
    }

    // QuadSorting.parityMerge16 (merge four 4-blocks into 16, analyzing sorted runs)
    template<class T>
    void quadParityMerge16(std::vector<T>& data_, ptrdiff_t start, std::vector<T>& aux) {
        if (data_[start + 3] <= data_[start + 4] &&
            data_[start + 7] <= data_[start + 8] &&
            data_[start + 11] <= data_[start + 12])
            return;

        quadParityMerge4(data_, start, aux, 0);
        quadParityMerge4(data_, start + 8, aux, 8);

        quadParityMerge8(aux, start, data_);
    }

    // QuadSorting.partialBackwardMerge (partially writes the second block into the aux
    // array and then merges it with the first block)
    template<class T>
    void quadPartialBackwardMerge(std::vector<T>& data_, std::vector<T>& aux, ptrdiff_t start, ptrdiff_t nmemb, ptrdiff_t block) {
        ptrdiff_t r, m, e, s;
        // right, middle, end, swap

        m = start + block;
        e = start + nmemb - 1;
        r = m--;

        if (data_[m] <= data_[r])
            return;

        while (data_[m] <= data_[e])
            --e;

        for (ptrdiff_t i = r; i < r + (e - m); ++i) {
            aux[i - r] = data_[i];
            MarkArray(1, data_, i);
        }

        s = e - r;
        data_[e] = data_[m];
        --e;
        --m;

        if (data_[start] <= aux[0]) {
            do {
                while (data_[m] > aux[s]) {
                    MarkArray(2, data_, m);
                    data_[e] = data_[m];
                    --e;
                    --m;
                }

                MarkArray(2, data_, m);
                data_[e] = aux[s];
                --e;
                --s;
            } while (s >= 0);
        }

        else {
            do {
                while (data_[m] <= aux[s]) {
                    MarkArray(2, data_, m);
                    data_[e] = aux[s];
                    --e;
                    --s;
                }

                MarkArray(2, data_, m);
                data_[e] = data_[m];
                --e;
                --m;
            } while (m >= start);

            do {
                data_[e] = aux[s];
                --e;
                --s;
            } while (s >= 0);
        }
    }

    // QuadSorting.tailMerge (bottom up merge sort on top of the parity merges; this is
    // NOT the TwinSorting tailMerge of section 1)
    template<class T>
    void quadTailMerge(std::vector<T>& data_, std::vector<T>& aux, ptrdiff_t start, ptrdiff_t nmemb, ptrdiff_t block) {
        ptrdiff_t pte = start + nmemb;

        while (block < nmemb) {
            for (ptrdiff_t pta = start; pta + block < pte; pta += block * 2) {
                if (pta + block * 2 < pte) {
                    quadPartialBackwardMerge(data_, aux, pta, block * 2, block);

                    continue;
                }
                quadPartialBackwardMerge(data_, aux, pta, pte - pta, block);

                break;
            }
            block *= 2;
        }
    }

    // QuadSorting.forwardMerge (normal merge with sorted-run analysis; the Java labels
    // leftFirst/rightFirst are the do-while continue targets).
    template<class T>
    void quadForwardMerge(std::vector<T>& dest, std::vector<T>& from, ptrdiff_t start, ptrdiff_t auxStart,
                          ptrdiff_t block, bool toAux) {
        ptrdiff_t l, r, m, e;
        // left, right, middle, end
        ptrdiff_t mergeP = toAux ? auxStart : start;

        l = toAux ? start : auxStart;
        r = toAux ? (start + block) : (auxStart + block);
        m = r;
        e = r + block;

        if (toAux) {
            MarkArray(1, from, r - 1);
            MarkArray(2, from, e - 1);
        }

        if (CompareValues(from[r - 1], from[e - 1]) <= 0) {
            do {
                for (ptrdiff_t i = 0; i < 3; ++i) {
                    if (CompareValues(from[l], from[r]) <= 0) {
                        if (toAux) {
                            MarkArray(1, from, l);
                            MarkArray(2, from, r);
                        }
                        dest[mergeP] = from[l];
                        ++mergeP; ++l;
                        break; // continue leftFirst: re-check "while (l < m)"
                    }

                    if (toAux) {
                        MarkArray(1, from, l);
                        MarkArray(2, from, r);
                    }
                    dest[mergeP] = from[r];
                    ++mergeP; ++r;
                }
            } while (l < m);

            do {
                if (toAux) {
                    MarkArray(1, from, l - 1);
                    MarkArray(2, from, r);
                }
                dest[mergeP] = from[r];
                ++mergeP; ++r;
            } while (r < e);
        }

        else {
            do {
                for (ptrdiff_t i = 0; i < 3; ++i) {
                    if (CompareValues(from[l], from[r]) > 0) {
                        if (toAux) {
                            MarkArray(1, from, l);
                            MarkArray(2, from, r);
                        }
                        dest[mergeP] = from[r];
                        ++mergeP; ++r;
                        break; // continue rightFirst: re-check "while (r < e)"
                    }

                    if (toAux) {
                        MarkArray(1, from, l);
                        MarkArray(2, from, r);
                    }
                    dest[mergeP] = from[l];
                    ++mergeP; ++l;
                }
            } while (r < e);

            do {
                if (toAux) {
                    MarkArray(1, from, l);
                    MarkArray(2, from, r - 1);
                }
                dest[mergeP] = from[l];
                ++mergeP; ++l;
            } while (l < m);
        }
    }

    // QuadSorting.quadMergeBlock (merge 4 blocks into 1: main [A][B][C][D] -> aux [A B]
    // -> aux [A B][C D] -> main [A B C D])
    template<class T>
    void quadMergeBlock(std::vector<T>& data_, ptrdiff_t start, std::vector<T>& aux, ptrdiff_t block) {
        ptrdiff_t pts, c, cMax;
        ptrdiff_t blockX2 = block * 2;

        cMax = start + block;

        // if first 2 blocks are sorted
        if (data_[cMax - 1] <= data_[cMax]) {
            cMax += blockX2;

            // if second 2 blocks are sorted
            if (data_[cMax - 1] <= data_[cMax]) {
                cMax -= block;

                // ...and entire 4 blocks are sorted
                if (data_[cMax - 1] <= data_[cMax]) {
                    return;
                }

                pts = 0;
                c = start;

                do {
                    aux[pts] = data_[c];
                    ++c;
                    MarkArray(1, data_, pts + start);
                    ++pts;
                } while (c < cMax); // step 1

                cMax = c + blockX2;
                do {
                    aux[pts] = data_[c];
                    ++c;
                    MarkArray(1, data_, pts + start);
                    ++pts;
                } while (c < cMax); // step 2

                quadForwardMerge(data_, aux, start, 0, blockX2, false); // step 3
                return;
            }

            pts = 0;
            c = start;
            cMax = start + blockX2;

            do {
                aux[pts] = data_[c];
                ++c;
                MarkArray(1, data_, pts + start);
                ++pts;
            } while (c < cMax); // step 1
        }

        else
            quadForwardMerge(aux, data_, start, 0, block, true); // step 1

        quadForwardMerge(aux, data_, start + blockX2, blockX2, block, true); // step 2
        quadForwardMerge(data_, aux, start, 0, blockX2, false); // step 3
    }

    // QuadSorting.quadMerge (quad merges the entire array; falls back to tail merge when
    // (current block size)*2 is greater than the array size)
    template<class T>
    void quadMerge(std::vector<T>& data_, std::vector<T>& aux, ptrdiff_t start, ptrdiff_t nmemb, ptrdiff_t block) {
        ptrdiff_t pte = start + nmemb;
        block *= 4;

        while (block * 2 <= nmemb) {
            ptrdiff_t pta = start;
            do {
                quadMergeBlock(data_, pta, aux, block / 4);

                pta += block;
            } while (pta + block <= pte);

            quadTailMerge(data_, aux, pta, pte - pta, block / 4);

            block *= 4;
        }
        quadTailMerge(data_, aux, start, nmemb, block / 4);
    }

    // QuadSorting.quadSwap (pre-sorting: 4-item sorting network, detects strictly
    // decreasing runs; returns 1 when the entire array was decreasing and was reversed)
    template<class T>
    int quadSwap(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t nmemb) {
        std::vector<T> aux(16);
        ptrdiff_t count, reverse;
        ptrdiff_t pta, pts = 0, ptt = 0;
        T temp = T();

        using std::swap;

        pta = start;
        count = nmemb / 4;

        // swapper:
        while (count-- > 0) {
            bool restartOuter = false;
            bool stopOuter = false;

            while (true) {
                if (data_[pta] > data_[pta + 1]) {
                    if (data_[pta + 2] > data_[pta + 3]) {
                        if (data_[pta + 1] > data_[pta + 2]) {
                            pts = pta;
                            pta += 4;
                            break;
                        }

                        swap(data_[pta + 2], data_[pta + 3]);
                    }
                    swap(data_[pta], data_[pta + 1]);
                }

                else if (data_[pta + 2] > data_[pta + 3])
                    swap(data_[pta + 2], data_[pta + 3]);

                if (data_[pta + 1] > data_[pta + 2]) {
                    if (data_[pta] <= data_[pta + 2]) {
                        if (data_[pta + 1] <= data_[pta + 3]) {
                            swap(data_[pta + 1], data_[pta + 2]);
                        }
                        else {
                            temp = data_[pta + 1];
                            data_[pta + 1] = data_[pta + 2];
                            data_[pta + 2] = data_[pta + 3];
                            data_[pta + 3] = temp;
                        }
                    }

                    else if (data_[pta] > data_[pta + 3]) {
                        swap(data_[pta + 1], data_[pta + 3]);
                        swap(data_[pta], data_[pta + 2]);
                    }

                    else if (data_[pta + 1] <= data_[pta + 3]) {
                        temp = data_[pta + 1];
                        data_[pta + 1] = data_[pta];
                        data_[pta] = data_[pta + 2];
                        data_[pta + 2] = temp;
                    }

                    else {
                        temp = data_[pta + 1];
                        data_[pta + 1] = data_[pta];
                        data_[pta] = data_[pta + 2];
                        data_[pta + 2] = data_[pta + 3];
                        data_[pta + 3] = temp;
                    }
                }
                pta += 4;
                restartOuter = true;
                break;
            }
            if (restartOuter) continue;

            while (true) {
                if (count-- > 0) {
                    if (data_[pta] > data_[pta + 1]) {
                        if (data_[pta + 2] > data_[pta + 3]) {
                            if (data_[pta + 1] > data_[pta + 2]) {
                                if (data_[pta - 1] > data_[pta]) {
                                    pta += 4;
                                    continue;
                                }
                            }
                            swap(data_[pta + 2], data_[pta + 3]);
                        }
                        swap(data_[pta], data_[pta + 1]);
                    }

                    else if (data_[pta + 2] > data_[pta + 3])
                        swap(data_[pta + 2], data_[pta + 3]);

                    if (data_[pta + 1] > data_[pta + 2]) {
                        if (data_[pta] <= data_[pta + 2]) {
                            if (data_[pta + 1] <= data_[pta + 3]) {
                                swap(data_[pta + 1], data_[pta + 2]);
                            }
                            else {
                                temp = data_[pta + 1];
                                data_[pta + 1] = data_[pta + 2];
                                data_[pta + 2] = data_[pta + 3];
                                data_[pta + 3] = temp;
                            }
                        }

                        else if (data_[pta] > data_[pta + 3]) {
                            swap(data_[pta], data_[pta + 2]);
                            swap(data_[pta + 1], data_[pta + 3]);
                        }

                        else if (data_[pta + 1] <= data_[pta + 3]) {
                            temp = data_[pta];
                            data_[pta] = data_[pta + 2];
                            data_[pta + 2] = data_[pta + 1];
                            data_[pta + 1] = temp;
                        }

                        else {
                            temp = data_[pta];
                            data_[pta] = data_[pta + 2];
                            data_[pta + 2] = data_[pta + 3];
                            data_[pta + 3] = data_[pta + 1];
                            data_[pta + 1] = temp;
                        }
                    }

                    ptt = pta - 1;
                    reverse = (ptt - pts) / 2;

                    do {
                        swap(data_[pts], data_[ptt]);
                        ++pts;
                        --ptt;
                    } while (reverse-- > 0);

                    pta += 4;
                    restartOuter = true;
                    break;
                }

                if (pts == start) {
                    bool reverseAll = false;
                    switch (nmemb % 4) {
                    case 3:
                        if (data_[pta + 1] <= data_[pta + 2])
                            break; // break switch
                        [[fallthrough]];
                    case 2:
                        if (data_[pta] <= data_[pta + 1])
                            break;
                        [[fallthrough]];
                    case 1:
                        if (data_[pta - 1] <= data_[pta])
                            break;
                        [[fallthrough]];
                    case 0:
                        reverseAll = true;
                        break;
                    }

                    if (reverseAll) {
                        ptt = pts + nmemb - 1;
                        reverse = (ptt - pts) / 2;

                        do {
                            swap(data_[pts], data_[ptt]);
                            ++pts;
                            --ptt;
                        } while (reverse-- > 0);

                        return 1;
                    }
                }

                ptt = pta - 1;
                reverse = (ptt - pts) / 2;
                do {
                    swap(data_[pts], data_[ptt]);
                    ++pts;
                    --ptt;
                } while (reverse-- > 0);
                stopOuter = true;
                break;
            }
            if (restartOuter) continue;
            if (stopOuter) break;
        }
        quadTailSwap(data_, pta, nmemb % 4);

        pta = start;
        count = nmemb / 16;
        while (count-- > 0) {
            quadParityMerge16(data_, pta, aux);
            pta += 16;
        }

        if (nmemb % 16 > 4)
            quadTailMerge(data_, aux, pta, nmemb % 16, 4);

        return 0;
    }

    // QuadSorting.quadSort (main sorting method)
    template<class T>
    void quadSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t length) {
        if (length < 16) {
            quadTailSwap(data_, start, length);
        }

        else if (length < 256) {
            if (quadSwap(data_, start, length) == 0) {
                std::vector<T> aux(128);
                quadTailMerge(data_, aux, start, length, 16);
            }
        }

        else {
            if (quadSwap(data_, start, length) == 0) {
                std::vector<T> aux(static_cast<size_t>(length / 2));
                quadMerge(data_, aux, start, length, 16);
            }
        }
    }

    // === SECTION 2 END ===

    // === SECTION 3: grail / tim / wiki / kota ===

    // UnstableGrailSorting.grailSwap (shared with the stable Grail port).
    template<class T>
    void grailSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        using std::swap;
        swap(data_[a], data_[b]);
    }

    // UnstableGrailSorting.grailMultiSwap (shared with the stable Grail port).
    template<class T>
    void grailMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t swapsLeft) {
        while (swapsLeft != 0) {
            grailSwap(data_, a, b);
            ++a;
            ++b;
            --swapsLeft;
        }
    }

    // UnstableGrailSorting.grailRotate / GrailSorting.grailRotate (shared; block-swap based,
    // NOT the three-reversal blockRotate of section 2).
    template<class T>
    void grailRotate(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t lenA, ptrdiff_t lenB) {
        while (lenA != 0 && lenB != 0) {
            if (lenA <= lenB) {
                grailMultiSwap(data_, pos, pos + lenA, lenA);
                pos += lenA;
                lenB -= lenA;
            }
            else {
                grailMultiSwap(data_, pos + (lenA - lenB), pos + lenA, lenB);
                lenA -= lenB;
            }
        }
    }

    // UnstableGrailSorting.grailBinSearch / GrailSorting.grailBinSearch (shared).
    // isLeft picks the direction of the search.
    template<class T>
    ptrdiff_t grailBinSearch(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len, ptrdiff_t keyPos, bool isLeft) {
        ptrdiff_t left = -1, right = len;
        while (left < right - 1) {
            ptrdiff_t mid = left + ((right - left) >> 1);
            if (isLeft) {
                if (CompareValues(data_[pos + mid], data_[keyPos]) >= 0) {
                    right = mid;
                }
                else {
                    left = mid;
                }
            }
            else {
                if (CompareValues(data_[pos + mid], data_[keyPos]) > 0) {
                    right = mid;
                }
                else {
                    left = mid;
                }
            }
            MarkArray(1, data_, pos + mid);
        }
        return right;
    }

    // UnstableGrailSorting.grailMergeWithoutBuffer / GrailSorting.grailMergeWithoutBuffer (shared).
    // cost: min(len1, len2)^2 + max(len1, len2)
    template<class T>
    void grailMergeWithoutBuffer(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len1, ptrdiff_t len2) {
        if (len1 < len2) {
            while (len1 != 0) {
                // Binary Search left
                ptrdiff_t loc = grailBinSearch(data_, pos + len1, len2, pos, true);
                if (loc != 0) {
                    grailRotate(data_, pos, len1, loc);
                    pos += loc;
                    len2 -= loc;
                }
                if (len2 == 0) break;
                do {
                    ++pos;
                    --len1;
                } while (len1 != 0 && CompareValues(data_[pos], data_[pos + len1]) <= 0);
            }
        }
        else {
            while (len2 != 0) {
                // Binary Search right
                ptrdiff_t loc = grailBinSearch(data_, pos, len1, pos + (len1 + len2 - 1), false);
                if (loc != len1) {
                    grailRotate(data_, pos + loc, len1 - loc, len2);
                    len1 = loc;
                }
                if (len1 == 0) break;
                do {
                    --len2;
                } while (len2 != 0 && CompareValues(data_[pos + len1 - 1], data_[pos + len1 + len2 - 1]) <= 0);
            }
        }
    }

    // UnstableGrailSorting.grailMergeLeft / GrailSorting.grailMergeLeft (shared).
    // arr[dist..-1] - buffer, arr[0, leftLen - 1] ++ arr[leftLen, leftLen + rightLen - 1]
    // -> arr[dist, dist + leftLen + rightLen - 1]
    template<class T>
    void grailMergeLeft(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t leftLen, ptrdiff_t rightLen, ptrdiff_t dist) {
        ptrdiff_t left = 0;
        ptrdiff_t right = leftLen;

        rightLen += leftLen;

        while (right < rightLen) {
            if (left == leftLen || CompareValues(data_[pos + left], data_[pos + right]) > 0) {
                grailSwap(data_, pos + dist, pos + right);
                ++dist; ++right;
            }
            else {
                grailSwap(data_, pos + dist, pos + left);
                ++dist; ++left;
            }
            MarkArray(3, data_, pos + left);
            MarkArray(4, data_, pos + right);
        }

        if (dist != left) grailMultiSwap(data_, pos + dist, pos + left, leftLen - left);
    }

    // UnstableGrailSorting.grailMergeRight / GrailSorting.grailMergeRight (shared).
    template<class T>
    void grailMergeRight(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t leftLen, ptrdiff_t rightLen, ptrdiff_t dist) {
        ptrdiff_t mergedPos = leftLen + rightLen + dist - 1;
        ptrdiff_t right = leftLen + rightLen - 1;
        ptrdiff_t left = leftLen - 1;

        while (left >= 0) {
            if (right < leftLen || CompareValues(data_[pos + left], data_[pos + right]) > 0) {
                grailSwap(data_, pos + mergedPos, pos + left);
                --mergedPos; --left;
            }
            else {
                grailSwap(data_, pos + mergedPos, pos + right);
                --mergedPos; --right;
            }
            if (pos + left >= 0) MarkArray(3, data_, pos + left);
            MarkArray(4, data_, pos + right);
        }

        if (right != mergedPos) {
            while (right >= leftLen) {
                grailSwap(data_, pos + mergedPos, pos + right);
                --mergedPos; --right;
            }
        }
    }

    // UnstableGrailSorting.grailSmartMergeWithBuffer (returns just the leftover length).
    template<class T>
    ptrdiff_t unstableGrailSmartMergeWithBuffer(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t leftOverLen, ptrdiff_t blockLen) {
        ptrdiff_t dist = 0 - blockLen, left = 0, right = leftOverLen;
        ptrdiff_t leftEnd = right, rightEnd = right + blockLen;

        while (left < leftEnd && right < rightEnd) {
            if (CompareValues(data_[pos + left], data_[pos + right]) <= 0) {
                grailSwap(data_, pos + dist, pos + left);
                ++dist; ++left;
            }
            else {
                grailSwap(data_, pos + dist, pos + right);
                ++dist; ++right;
            }
            MarkArray(3, data_, pos + left);
            MarkArray(4, data_, pos + right);
        }

        ptrdiff_t length;
        if (left < leftEnd) {
            length = leftEnd - left;
            while (left < leftEnd) {
                --leftEnd;
                --rightEnd;
                grailSwap(data_, pos + leftEnd, pos + rightEnd);
            }
        }
        else {
            length = rightEnd - right;
        }
        return length;
    }

    // UnstableGrailSorting.grailMergeBuffersLeft (unstable variant, no keys).
    template<class T>
    void unstableGrailMergeBuffersLeft(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t blockCount, ptrdiff_t blockLen,
                                       ptrdiff_t aBlockCount, ptrdiff_t lastLen) {
        if (blockCount == 0) {
            ptrdiff_t aBlocksLen = aBlockCount * blockLen;
            grailMergeLeft(data_, pos, aBlocksLen, lastLen, 0 - blockLen);
            return;
        }

        ptrdiff_t leftOverLen = blockLen;
        ptrdiff_t processIndex = blockLen;
        ptrdiff_t restToProcess;

        for (ptrdiff_t keyIndex = 1; keyIndex < blockCount; ++keyIndex, processIndex += blockLen) {
            restToProcess = processIndex - leftOverLen;
            leftOverLen = unstableGrailSmartMergeWithBuffer(data_, pos + restToProcess, leftOverLen, blockLen);
        }
        restToProcess = processIndex - leftOverLen;

        if (lastLen != 0) {
            leftOverLen += blockLen * aBlockCount;
            grailMergeLeft(data_, pos + restToProcess, leftOverLen, lastLen, 0 - blockLen);
        }
        else {
            grailMultiSwap(data_, pos + restToProcess, pos + (restToProcess - blockLen), leftOverLen);
        }
    }

    // UnstableGrailSorting.grailBuildBlocks. The Java insertion fallback is
    // OptimizedGnomeSort.customSort -> section 1 optimizedGnomeSort.
    // build blocks of length buildLen
    // input: [-buildLen, -1] elements are buffer
    // output: first buildLen elements are buffer, blocks 2 * buildLen and last subblock sorted
    template<class T>
    void unstableGrailBuildBlocks(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len, ptrdiff_t buildLen) {
        ptrdiff_t extraDist, part;
        for (ptrdiff_t dist = 1; dist < len; dist += 2) {
            extraDist = 0;
            if (CompareValues(data_[pos + (dist - 1)], data_[pos + dist]) > 0) extraDist = 1;
            grailSwap(data_, pos + (dist - 3), pos + (dist - 1 + extraDist));
            grailSwap(data_, pos + (dist - 2), pos + (dist - extraDist));
        }
        if (len % 2 != 0) grailSwap(data_, pos + (len - 1), pos + (len - 3));
        pos -= 2;
        part = 2;

        for (; part < buildLen; part *= 2) {
            ptrdiff_t left = 0;
            ptrdiff_t right = len - 2 * part;
            while (left <= right) {
                grailMergeLeft(data_, pos + left, part, part, 0 - part);
                left += 2 * part;
            }
            ptrdiff_t rest = len - left;
            if (rest > part) {
                grailMergeLeft(data_, pos + left, part, rest - part, 0 - part);
            }
            else {
                grailRotate(data_, pos + left - part, part, rest);
            }
            pos -= part;
        }
        ptrdiff_t restToBuild = len % (2 * buildLen);
        ptrdiff_t leftOverPos = len - restToBuild;

        if (restToBuild <= buildLen) grailRotate(data_, pos + leftOverPos, restToBuild, buildLen);
        else grailMergeRight(data_, pos + leftOverPos, buildLen, restToBuild - buildLen, buildLen);

        while (leftOverPos > 0) {
            leftOverPos -= 2 * buildLen;
            grailMergeRight(data_, pos + leftOverPos, buildLen, buildLen, buildLen);
        }
    }

    // UnstableGrailSorting.grailCombineBlocks.
    // keys are on the left of arr. Blocks of length buildLen combined. We'll combine them in pairs
    // buildLen and nkeys are powers of 2. (2 * buildLen / regBlockLen) keys are guaranteed
    template<class T>
    void unstableGrailCombineBlocks(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len, ptrdiff_t buildLen, ptrdiff_t regBlockLen) {
        ptrdiff_t combineLen = len / (2 * buildLen);
        ptrdiff_t leftOver = len % (2 * buildLen);
        if (leftOver <= buildLen) {
            len -= leftOver;
            leftOver = 0;
        }

        for (ptrdiff_t i = 0; i <= combineLen; ++i) {
            if (i == combineLen && leftOver == 0) break;

            ptrdiff_t blockPos = pos + i * 2 * buildLen;
            ptrdiff_t blockCount = (i == combineLen ? leftOver : 2 * buildLen) / regBlockLen;

            for (ptrdiff_t index = 1; index < blockCount; ++index) {
                ptrdiff_t leftIndex = index - 1;

                for (ptrdiff_t rightIndex = index; rightIndex < blockCount; ++rightIndex) {
                    int rightComp = CompareValues(data_[blockPos + leftIndex * regBlockLen],
                                                  data_[blockPos + rightIndex * regBlockLen]);
                    if (rightComp > 0 || (rightComp == 0 && CompareValues(data_[blockPos + (leftIndex + 1) * regBlockLen - 1],
                                                                          data_[blockPos + (rightIndex + 1) * regBlockLen - 1]) > 0)) {
                        leftIndex = rightIndex;
                    }
                }
                if (leftIndex != index - 1) {
                    grailMultiSwap(data_, blockPos + (index - 1) * regBlockLen, blockPos + leftIndex * regBlockLen, regBlockLen);
                }
            }

            ptrdiff_t aBlockCount = 0;
            ptrdiff_t lastLen = 0;
            if (i == combineLen) lastLen = leftOver % regBlockLen;

            if (lastLen != 0) {
                while (aBlockCount < blockCount && CompareValues(data_[blockPos + blockCount * regBlockLen],
                                                                 data_[blockPos + (blockCount - aBlockCount - 1) * regBlockLen]) < 0) {
                    ++aBlockCount;
                }
            }
            unstableGrailMergeBuffersLeft(data_, blockPos, blockCount - aBlockCount, regBlockLen, aBlockCount, lastLen);
        }
        while (--len >= 0) {
            grailSwap(data_, pos + len, pos + len - regBlockLen);
        }
    }

    // UnstableGrailSorting.grailCommonSort. len <= 16 uses optimizedGnomeSort(pos, pos + len)
    // (the Java grailInsertSort).
    template<class T>
    void unstableGrailCommonSort(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len) {
        if (len <= 16) {
            optimizedGnomeSort(data_, pos, pos + len);
            return;
        }

        ptrdiff_t blockLen = 1;
        while (blockLen * blockLen < len) blockLen *= 2;
        ptrdiff_t buildLen = blockLen;

        unstableGrailBuildBlocks(data_, pos + blockLen, len - blockLen, buildLen);

        // 2 * buildLen are built
        buildLen *= 2;
        while (len - blockLen > buildLen) {
            unstableGrailCombineBlocks(data_, pos + blockLen, len - blockLen, buildLen, blockLen);
            buildLen *= 2;
        }

        optimizedGnomeSort(data_, pos, pos + blockLen);
        grailMergeWithoutBuffer(data_, pos, blockLen, len - blockLen);
    }

    // GrailSorting.GrailPair (leftover length + leftover fragment).
    struct GrailPair {
        ptrdiff_t leftOverLen;
        ptrdiff_t leftOverFrag;
    };

    // GrailSorting.grailStaticBufferLen
    inline constexpr ptrdiff_t GrailStaticBufferLen = 32;

    // GrailSorting.grailFindKeys
    template<class T>
    ptrdiff_t grailFindKeys(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len, ptrdiff_t numKeys) {
        ptrdiff_t dist = 1, foundKeys = 1, firstKey = 0; // first key is always here

        while (dist < len && foundKeys < numKeys) {
            // Java marks (3, dist + 1) without adding pos; kept as is.
            if (dist < (len - 1)) MarkArray(3, data_, dist + 1);

            // Binary Search left
            ptrdiff_t loc = grailBinSearch(data_, pos + firstKey, foundKeys, pos + dist, true);
            if (loc == foundKeys || CompareValues(data_[pos + dist], data_[pos + (firstKey + loc)]) != 0) {
                grailRotate(data_, pos + firstKey, foundKeys, dist - (firstKey + foundKeys));
                firstKey = dist - foundKeys;
                grailRotate(data_, pos + (firstKey + loc), foundKeys - loc, 1);
                ++foundKeys;
            }

            ++dist;
        }
        grailRotate(data_, pos, firstKey, foundKeys);

        return foundKeys;
    }

    // GrailSorting.grailSmartMergeWithoutBuffer (returns the leftover length, then the leftover fragment).
    template<class T>
    GrailPair grailSmartMergeWithoutBuffer(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t leftOverLen,
                                           ptrdiff_t leftOverFrag, ptrdiff_t regBlockLen) {
        if (regBlockLen == 0) return GrailPair{ leftOverLen, leftOverFrag };

        ptrdiff_t len1 = leftOverLen;
        ptrdiff_t len2 = regBlockLen;
        ptrdiff_t typeFrag = 1 - leftOverFrag; // 1 if inverted

        if (len1 != 0 && CompareValues(data_[pos + (len1 - 1)], data_[pos + len1]) - typeFrag >= 0) {

            while (len1 != 0) {
                ptrdiff_t foundLen;
                if (typeFrag != 0) {
                    // Binary Search left
                    foundLen = grailBinSearch(data_, pos + len1, len2, pos, true);
                }
                else {
                    // Binary Search right
                    foundLen = grailBinSearch(data_, pos + len1, len2, pos, false);
                }
                if (foundLen != 0) {
                    grailRotate(data_, pos, len1, foundLen);
                    pos += foundLen;
                    len2 -= foundLen;
                }
                if (len2 == 0) {
                    return GrailPair{ len1, leftOverFrag };
                }
                do {
                    ++pos;
                    --len1;
                } while (len1 != 0 && CompareValues(data_[pos], data_[pos + len1]) - typeFrag < 0);
            }
        }
        return GrailPair{ len2, typeFrag };
    }

    // GrailSorting.grailSmartMergeWithBuffer (returns the leftover length, then the leftover fragment).
    template<class T>
    GrailPair grailSmartMergeWithBuffer(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t leftOverLen,
                                        ptrdiff_t leftOverFrag, ptrdiff_t blockLen) {
        ptrdiff_t dist = 0 - blockLen, left = 0, right = leftOverLen;
        ptrdiff_t leftEnd = right, rightEnd = right + blockLen;
        ptrdiff_t typeFrag = 1 - leftOverFrag; // 1 if inverted

        while (left < leftEnd && right < rightEnd) {
            if (CompareValues(data_[pos + left], data_[pos + right]) - typeFrag < 0) {
                grailSwap(data_, pos + dist, pos + left);
                ++dist; ++left;
            }
            else {
                grailSwap(data_, pos + dist, pos + right);
                ++dist; ++right;
            }
            MarkArray(3, data_, pos + left);
            MarkArray(4, data_, pos + right);
        }

        ptrdiff_t length, fragment = leftOverFrag;
        if (left < leftEnd) {
            length = leftEnd - left;
            while (left < leftEnd) {
                --rightEnd;
                --leftEnd;
                grailSwap(data_, pos + rightEnd, pos + leftEnd);
            }
        }
        else {
            length = rightEnd - right;
            fragment = typeFrag;
        }
        return GrailPair{ length, fragment };
    }

    // GrailSorting.grailSmartMergeWithXBuf (writes into the free buffer area; the external
    // buffer arguments are unused in Java, they are kept for signature parity).
    template<class T>
    GrailPair grailSmartMergeWithXBuf(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t leftOverLen,
                                      ptrdiff_t leftOverFrag, ptrdiff_t blockLen,
                                      std::vector<T>* /*buffer*/, ptrdiff_t /*bufferPos*/) {
        ptrdiff_t dist = 0 - blockLen, left = 0, right = leftOverLen;
        ptrdiff_t leftEnd = right, rightEnd = right + blockLen;
        ptrdiff_t typeFrag = 1 - leftOverFrag; // 1 if inverted

        while (left < leftEnd && right < rightEnd) {
            if (CompareValues(data_[pos + left], data_[pos + right]) - typeFrag < 0) {
                data_[pos + dist] = data_[pos + left];
                ++dist; ++left;
            }
            else {
                data_[pos + dist] = data_[pos + right];
                ++dist; ++right;
            }
            MarkArray(2, data_, pos + left);
            MarkArray(3, data_, pos + right);
        }

        ptrdiff_t length, fragment = leftOverFrag;
        if (left < leftEnd) {
            length = leftEnd - left;
            while (left < leftEnd) {
                --rightEnd;
                --leftEnd;
                data_[pos + rightEnd] = data_[pos + leftEnd];
            }
        }
        else {
            length = rightEnd - right;
            fragment = typeFrag;
        }
        return GrailPair{ length, fragment };
    }

    // GrailSorting.grailMergeLeftWithXBuf (the external buffer arguments are unused in Java).
    // arr[dist..-1] - free, arr[0, leftEnd - 1] ++ arr[leftEnd, leftEnd + rightEnd - 1]
    // -> arr[dist, dist + leftEnd + rightEnd - 1]
    template<class T>
    void grailMergeLeftWithXBuf(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t leftEnd, ptrdiff_t rightEnd,
                                ptrdiff_t dist, std::vector<T>* /*buffer*/, ptrdiff_t /*bufferPos*/) {
        ptrdiff_t left = 0;
        ptrdiff_t right = leftEnd;
        rightEnd += leftEnd;

        while (right < rightEnd) {
            if (left == leftEnd || CompareValues(data_[pos + left], data_[pos + right]) > 0) {
                data_[pos + dist] = data_[pos + right];
                ++dist; ++right;
            }
            else {
                data_[pos + dist] = data_[pos + left];
                ++dist; ++left;
            }
            MarkArray(2, data_, pos + left);
            MarkArray(3, data_, pos + right);
        }

        if (dist != left) {
            while (left < leftEnd) {
                data_[pos + dist] = data_[pos + left];
                ++dist; ++left;
            }
        }
    }

    // GrailSorting.grailMergeBuffersLeft (stable, with keys; the external buffer arguments
    // belong to the XBuf variant and are unused in Java).
    // aBlockCount are regular blocks from stream A.
    // lastLen is length of last (irregular) block from stream B, that should go before nblock2 blocks.
    // lastLen = 0 requires aBlockCount = 0 (no irregular blocks). lastLen > 0, aBlockCount = 0 is possible.
    template<class T>
    void grailMergeBuffersLeft(std::vector<T>& data_, ptrdiff_t keysPos, ptrdiff_t midkey, ptrdiff_t pos,
                               ptrdiff_t blockCount, ptrdiff_t blockLen, bool havebuf, ptrdiff_t aBlockCount,
                               ptrdiff_t lastLen) {

        if (blockCount == 0) {
            ptrdiff_t aBlocksLen = aBlockCount * blockLen;
            if (havebuf) grailMergeLeft(data_, pos, aBlocksLen, lastLen, 0 - blockLen);
            else grailMergeWithoutBuffer(data_, pos, aBlocksLen, lastLen);
            return;
        }

        ptrdiff_t leftOverLen = blockLen;
        ptrdiff_t leftOverFrag = CompareValues(data_[keysPos], data_[midkey]) < 0 ? 0 : 1;
        ptrdiff_t processIndex = blockLen;
        ptrdiff_t restToProcess;

        for (ptrdiff_t keyIndex = 1; keyIndex < blockCount; ++keyIndex, processIndex += blockLen) {
            restToProcess = processIndex - leftOverLen;
            ptrdiff_t nextFrag = CompareValues(data_[keysPos + keyIndex], data_[midkey]) < 0 ? 0 : 1;

            if (nextFrag == leftOverFrag) {
                if (havebuf) grailMultiSwap(data_, pos + restToProcess - blockLen, pos + restToProcess, leftOverLen);
                restToProcess = processIndex;
                leftOverLen = blockLen;
            }
            else {
                if (havebuf) {
                    GrailPair results = grailSmartMergeWithBuffer(data_, pos + restToProcess, leftOverLen, leftOverFrag, blockLen);
                    leftOverLen = results.leftOverLen;
                    leftOverFrag = results.leftOverFrag;
                }
                else {
                    GrailPair results = grailSmartMergeWithoutBuffer(data_, pos + restToProcess, leftOverLen, leftOverFrag, blockLen);
                    leftOverLen = results.leftOverLen;
                    leftOverFrag = results.leftOverFrag;
                }
            }
        }
        restToProcess = processIndex - leftOverLen;

        if (lastLen != 0) {
            if (leftOverFrag != 0) {
                if (havebuf) {
                    grailMultiSwap(data_, pos + restToProcess - blockLen, pos + restToProcess, leftOverLen);
                }
                restToProcess = processIndex;
                leftOverLen = blockLen * aBlockCount;
                leftOverFrag = 0;
            }
            else {
                leftOverLen += blockLen * aBlockCount;
            }
            if (havebuf) {
                grailMergeLeft(data_, pos + restToProcess, leftOverLen, lastLen, -blockLen);
            }
            else {
                grailMergeWithoutBuffer(data_, pos + restToProcess, leftOverLen, lastLen);
            }
        }
        else {
            if (havebuf) {
                grailMultiSwap(data_, pos + restToProcess, pos + (restToProcess - blockLen), leftOverLen);
            }
        }
    }

    // GrailSorting.grailMergeBuffersLeftWithXBuf (the external buffer arguments are unused in Java).
    template<class T>
    void grailMergeBuffersLeftWithXBuf(std::vector<T>& data_, ptrdiff_t keysPos, ptrdiff_t midkey, ptrdiff_t pos,
                                       ptrdiff_t blockCount, ptrdiff_t regBlockLen, ptrdiff_t aBlockCount,
                                       ptrdiff_t lastLen, std::vector<T>* buffer, ptrdiff_t bufferPos) {

        if (blockCount == 0) {
            ptrdiff_t aBlocksLen = aBlockCount * regBlockLen;
            grailMergeLeftWithXBuf(data_, pos, aBlocksLen, lastLen, 0 - regBlockLen, buffer, bufferPos);
            return;
        }

        ptrdiff_t leftOverLen = regBlockLen;
        ptrdiff_t leftOverFrag = CompareValues(data_[keysPos], data_[midkey]) < 0 ? 0 : 1;
        ptrdiff_t processIndex = regBlockLen;

        ptrdiff_t restToProcess;
        for (ptrdiff_t keyIndex = 1; keyIndex < blockCount; ++keyIndex, processIndex += regBlockLen) {
            restToProcess = processIndex - leftOverLen;
            ptrdiff_t nextFrag = CompareValues(data_[keysPos + keyIndex], data_[midkey]) < 0 ? 0 : 1;

            if (nextFrag == leftOverFrag) {
                // Writes.arraycopy(arr, pos + restToProcess, arr, pos + restToProcess - regBlockLen, leftOverLen)
                for (ptrdiff_t i = 0; i < leftOverLen; ++i) {
                    data_[pos + restToProcess - regBlockLen + i] = data_[pos + restToProcess + i];
                }

                restToProcess = processIndex;
                leftOverLen = regBlockLen;
            }
            else {
                GrailPair results = grailSmartMergeWithXBuf(data_, pos + restToProcess, leftOverLen, leftOverFrag, regBlockLen, buffer, bufferPos);
                leftOverLen = results.leftOverLen;
                leftOverFrag = results.leftOverFrag;
            }
        }
        restToProcess = processIndex - leftOverLen;

        if (lastLen != 0) {
            if (leftOverFrag != 0) {
                for (ptrdiff_t i = 0; i < leftOverLen; ++i) {
                    data_[pos + restToProcess - regBlockLen + i] = data_[pos + restToProcess + i];
                }

                restToProcess = processIndex;
                leftOverLen = regBlockLen * aBlockCount;
                leftOverFrag = 0;
            }
            else {
                leftOverLen += regBlockLen * aBlockCount;
            }
            grailMergeLeftWithXBuf(data_, pos + restToProcess, leftOverLen, lastLen, 0 - regBlockLen, buffer, bufferPos);
        }
        else {
            for (ptrdiff_t i = 0; i < leftOverLen; ++i) {
                data_[pos + restToProcess - regBlockLen + i] = data_[pos + restToProcess + i];
            }
        }
    }

    // GrailSorting.grailBuildBlocks (stable, with an optional external buffer).
    // build blocks of length buildLen
    // input: [-buildLen, -1] elements are buffer
    // output: first buildLen elements are buffer, blocks 2 * buildLen and last subblock sorted
    template<class T>
    void grailBuildBlocks(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len, ptrdiff_t buildLen,
                          std::vector<T>* extbuf, ptrdiff_t bufferPos, ptrdiff_t extBufLen) {

        ptrdiff_t buildBuf = buildLen < extBufLen ? buildLen : extBufLen;
        while ((buildBuf & (buildBuf - 1)) != 0) buildBuf &= buildBuf - 1; // max power or 2 - just in case

        ptrdiff_t extraDist, part;
        if (buildBuf != 0) {
            // Writes.arraycopy(arr, pos - buildBuf, extbuf, bufferPos, buildBuf)
            for (ptrdiff_t i = 0; i < buildBuf; ++i) {
                (*extbuf)[bufferPos + i] = data_[pos - buildBuf + i];
            }

            for (ptrdiff_t dist = 1; dist < len; dist += 2) {
                extraDist = 0;
                if (CompareValues(data_[pos + (dist - 1)], data_[pos + dist]) > 0) extraDist = 1;
                data_[pos + dist - 3] = data_[pos + dist - 1 + extraDist];
                data_[pos + dist - 2] = data_[pos + dist - extraDist];
            }
            if (len % 2 != 0) data_[pos + len - 3] = data_[pos + len - 1];
            pos -= 2;

            for (part = 2; part < buildBuf; part *= 2) {
                ptrdiff_t left = 0;
                ptrdiff_t right = len - 2 * part;
                while (left <= right) {
                    grailMergeLeftWithXBuf(data_, pos + left, part, part, 0 - part, extbuf, bufferPos);
                    left += 2 * part;
                }
                ptrdiff_t rest = len - left;

                if (rest > part) {
                    grailMergeLeftWithXBuf(data_, pos + left, part, rest - part, 0 - part, extbuf, bufferPos);
                }
                else {
                    for (; left < len; ++left) data_[pos + left - part] = data_[pos + left];
                }
                pos -= part;
            }
            // Writes.arraycopy(extbuf, bufferPos, arr, pos + len, buildBuf)
            for (ptrdiff_t i = 0; i < buildBuf; ++i) {
                data_[pos + len + i] = (*extbuf)[bufferPos + i];
            }
        }
        else {
            for (ptrdiff_t dist = 1; dist < len; dist += 2) {
                extraDist = 0;
                if (CompareValues(data_[pos + (dist - 1)], data_[pos + dist]) > 0) extraDist = 1;
                grailSwap(data_, pos + (dist - 3), pos + (dist - 1 + extraDist));
                grailSwap(data_, pos + (dist - 2), pos + (dist - extraDist));
            }
            if (len % 2 != 0) grailSwap(data_, pos + (len - 1), pos + (len - 3));
            pos -= 2;
            part = 2;
        }

        for (; part < buildLen; part *= 2) {
            ptrdiff_t left = 0;
            ptrdiff_t right = len - 2 * part;
            while (left <= right) {
                grailMergeLeft(data_, pos + left, part, part, 0 - part);
                left += 2 * part;
            }
            ptrdiff_t rest = len - left;
            if (rest > part) {
                grailMergeLeft(data_, pos + left, part, rest - part, 0 - part);
            }
            else {
                grailRotate(data_, pos + left - part, part, rest);
            }
            pos -= part;
        }
        ptrdiff_t restToBuild = len % (2 * buildLen);
        ptrdiff_t leftOverPos = len - restToBuild;

        if (restToBuild <= buildLen) grailRotate(data_, pos + leftOverPos, restToBuild, buildLen);
        else grailMergeRight(data_, pos + leftOverPos, buildLen, restToBuild - buildLen, buildLen);

        while (leftOverPos > 0) {
            leftOverPos -= 2 * buildLen;
            grailMergeRight(data_, pos + leftOverPos, buildLen, buildLen, buildLen);
        }
    }

    // GrailSorting.grailCombineBlocks (stable; the Java insertion fallback is optimizedGnomeSort).
    // keys are on the left of arr. Blocks of length buildLen combined. We'll combine them in pairs
    // buildLen and nkeys are powers of 2. (2 * buildLen / regBlockLen) keys are guaranteed
    template<class T>
    void grailCombineBlocks(std::vector<T>& data_, ptrdiff_t keyPos, ptrdiff_t pos, ptrdiff_t len, ptrdiff_t buildLen,
                            ptrdiff_t regBlockLen, bool havebuf, std::vector<T>* buffer, ptrdiff_t bufferPos) {

        ptrdiff_t combineLen = len / (2 * buildLen);
        ptrdiff_t leftOver = len % (2 * buildLen);
        if (leftOver <= buildLen) {
            len -= leftOver;
            leftOver = 0;
        }

        if (buffer != nullptr) {
            for (ptrdiff_t i = 0; i < regBlockLen; ++i) {
                (*buffer)[bufferPos + i] = data_[pos - regBlockLen + i];
            }
        }

        for (ptrdiff_t i = 0; i <= combineLen; ++i) {
            if (i == combineLen && leftOver == 0) break;

            ptrdiff_t blockPos = pos + i * 2 * buildLen;
            ptrdiff_t blockCount = (i == combineLen ? leftOver : 2 * buildLen) / regBlockLen;

            optimizedGnomeSort(data_, keyPos, keyPos + blockCount + (i == combineLen ? 1 : 0));

            ptrdiff_t midkey = buildLen / regBlockLen;

            for (ptrdiff_t index = 1; index < blockCount; ++index) {
                ptrdiff_t leftIndex = index - 1;

                for (ptrdiff_t rightIndex = index; rightIndex < blockCount; ++rightIndex) {
                    int rightComp = CompareValues(data_[blockPos + leftIndex * regBlockLen],
                                                  data_[blockPos + rightIndex * regBlockLen]);
                    if (rightComp > 0 || (rightComp == 0 && CompareValues(data_[keyPos + leftIndex], data_[keyPos + rightIndex]) > 0)) {
                        leftIndex = rightIndex;
                    }
                }
                if (leftIndex != index - 1) {
                    grailMultiSwap(data_, blockPos + (index - 1) * regBlockLen, blockPos + leftIndex * regBlockLen, regBlockLen);
                    grailSwap(data_, keyPos + (index - 1), keyPos + leftIndex);
                    if (midkey == index - 1 || midkey == leftIndex) {
                        midkey ^= (index - 1) ^ leftIndex;
                    }
                }
            }

            ptrdiff_t aBlockCount = 0;
            ptrdiff_t lastLen = 0;
            if (i == combineLen) lastLen = leftOver % regBlockLen;

            if (lastLen != 0) {
                while (aBlockCount < blockCount && CompareValues(data_[blockPos + blockCount * regBlockLen],
                                                                 data_[blockPos + (blockCount - aBlockCount - 1) * regBlockLen]) < 0) {
                    ++aBlockCount;
                }
            }

            if (buffer != nullptr) {
                grailMergeBuffersLeftWithXBuf(data_, keyPos, keyPos + midkey, blockPos,
                                              blockCount - aBlockCount, regBlockLen, aBlockCount, lastLen, buffer, bufferPos);
            }
            else {
                grailMergeBuffersLeft(data_, keyPos, keyPos + midkey, blockPos,
                                      blockCount - aBlockCount, regBlockLen, havebuf, aBlockCount, lastLen);
            }
        }
        if (buffer != nullptr) {
            for (ptrdiff_t i = len; --i >= 0;) data_[pos + i] = data_[pos + i - regBlockLen];
            for (ptrdiff_t i = 0; i < regBlockLen; ++i) {
                data_[pos - regBlockLen + i] = (*buffer)[bufferPos + i];
            }
        }
        else if (havebuf) {
            while (--len >= 0) {
                grailSwap(data_, pos + len, pos + len - regBlockLen);
            }
        }
    }

    // GrailSorting.grailLazyStableSort
    template<class T>
    void grailLazyStableSort(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len) {
        for (ptrdiff_t dist = 1; dist < len; dist += 2) {
            if (CompareValues(data_[pos + dist - 1], data_[pos + dist]) > 0) {
                grailSwap(data_, pos + (dist - 1), pos + dist);
            }
            MarkArray(3, data_, pos + dist - 1);
            MarkArray(4, data_, pos + dist);
        }

        for (ptrdiff_t part = 2; part < len; part *= 2) {
            ptrdiff_t left = 0;
            ptrdiff_t right = len - 2 * part;

            while (left <= right) {
                grailMergeWithoutBuffer(data_, pos + left, part, part);
                left += 2 * part;
            }

            ptrdiff_t rest = len - left;
            if (rest > part) {
                grailMergeWithoutBuffer(data_, pos + left, part, rest - part);
            }
        }
    }

    // GrailSorting.grailCommonSort (stable, with an optional external buffer).
    template<class T>
    void grailCommonSort(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len, std::vector<T>* buffer,
                         ptrdiff_t bufferPos, ptrdiff_t bufferLen) {

        if (len <= 16) {
            optimizedGnomeSort(data_, pos, pos + len);
            return;
        }

        ptrdiff_t blockLen = 1;
        while (blockLen * blockLen < len) blockLen *= 2;

        ptrdiff_t numKeys = (len - 1) / blockLen + 1;

        ptrdiff_t keysFound = grailFindKeys(data_, pos, len, numKeys + blockLen);

        bool bufferEnabled = true;

        if (keysFound < numKeys + blockLen) {
            if (keysFound < 4) {
                grailLazyStableSort(data_, pos, len);
                return;
            }
            numKeys = blockLen;
            while (numKeys > keysFound) numKeys /= 2;
            bufferEnabled = false;
            blockLen = 0;
        }

        ptrdiff_t dist = blockLen + numKeys;
        ptrdiff_t buildLen = bufferEnabled ? blockLen : numKeys;

        if (bufferEnabled) {
            grailBuildBlocks(data_, pos + dist, len - dist, buildLen, buffer, bufferPos, bufferLen);
        }
        else {
            grailBuildBlocks(data_, pos + dist, len - dist, buildLen, static_cast<std::vector<T>*>(nullptr), bufferPos, 0);
        }

        // 2 * buildLen are built
        buildLen *= 2;
        while (len - dist > buildLen) {
            ptrdiff_t regBlockLen = blockLen;
            bool buildBufEnabled = bufferEnabled;

            if (!bufferEnabled) {
                if (numKeys > 4 && numKeys / 8 * numKeys >= buildLen) {
                    regBlockLen = numKeys / 2;
                    buildBufEnabled = true;
                }
                else {
                    ptrdiff_t calcKeys = 1;
                    ptrdiff_t i = buildLen * keysFound / 2;
                    while (calcKeys < numKeys && i != 0) {
                        calcKeys *= 2;
                        i /= 8;
                    }
                    regBlockLen = (2 * buildLen) / calcKeys;
                }
            }
            grailCombineBlocks(data_, pos, pos + dist, len - dist, buildLen, regBlockLen, buildBufEnabled,
                               buildBufEnabled && regBlockLen <= bufferLen ? buffer : nullptr, bufferPos);

            buildLen *= 2;
        }

        optimizedGnomeSort(data_, pos, pos + dist);
        grailMergeWithoutBuffer(data_, pos, dist, len - dist);
    }

    // GrailSorting.grailInPlaceMerge
    template<class T>
    void grailInPlaceMerge(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len1, ptrdiff_t len2) {
        if (len1 < 3 || len2 < 3) {
            grailMergeWithoutBuffer(data_, pos, len1, len2);
            return;
        }

        ptrdiff_t midpoint;
        if (len1 < len2) midpoint = len1 + len2 / 2;
        else midpoint = len1 / 2;

        // Left binary search
        ptrdiff_t len1Left, len1Right;
        len1Left = len1Right = grailBinSearch(data_, pos, len1, pos + midpoint, true);

        // Right binary search
        if (len1Right < len1 && CompareValues(data_[pos + len1Right], data_[pos + midpoint]) == 0) {
            len1Right = grailBinSearch(data_, pos + len1Left, len1 - len1Left, pos + midpoint, false) + len1Left;
        }

        ptrdiff_t len2Left, len2Right;
        len2Left = len2Right = grailBinSearch(data_, pos + len1, len2, pos + midpoint, true);

        if (len2Right < len2 && CompareValues(data_[pos + len1 + len2Right], data_[pos + midpoint]) == 0) {
            len2Right = grailBinSearch(data_, pos + len1 + len2Left, len2 - len2Left, pos + midpoint, false) + len2Left;
        }

        if (len1Left == len1Right) grailRotate(data_, pos + len1Right, len1 - len1Right, len2Right);
        else {
            grailRotate(data_, pos + len1Left, len1 - len1Left, len2Left);

            if (len2Right != len2Left) {
                grailRotate(data_, pos + (len1Right + len2Left), len1 - len1Right, len2Right - len2Left);
            }
        }

        grailInPlaceMerge(data_, pos + (len1Right + len2Right), len1 - len1Right, len2 - len2Right);
        grailInPlaceMerge(data_, pos, len1Left, len2Left);
    }

    // GrailSorting.grailInPlaceMergeSort (len is the absolute end index, as in Java).
    template<class T>
    void grailInPlaceMergeSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t len) {
        for (ptrdiff_t dist = start + 1; dist < len; dist += 2) {
            if (CompareValues(data_[dist - 1], data_[dist]) > 0) grailSwap(data_, dist - 1, dist);
        }
        for (ptrdiff_t part = 2; part < len; part *= 2) {
            ptrdiff_t left = start, right = len - 2 * part;

            while (left <= right) {
                grailInPlaceMerge(data_, left, part, part);
                left += 2 * part;
            }

            ptrdiff_t rest = len - left;
            if (rest > part) grailInPlaceMerge(data_, left, part, rest - part);
        }
    }

    // TimSorting.MIN_MERGE / MIN_GALLOP / INITIAL_TMP_STORAGE_LENGTH.
    inline constexpr ptrdiff_t TimSortMinMerge = 32;
    inline constexpr ptrdiff_t TimSortMinGallop = 7;
    inline constexpr ptrdiff_t TimSortInitialTmpStorageLength = 256;

    // TimSorting instance state (Java fields a / len / minGallop / tmp / runBase / runLen / stackSize).
    template<class T>
    struct TimSortState {
        std::vector<T>& a;
        ptrdiff_t len;
        ptrdiff_t minGallop = TimSortMinGallop;
        std::vector<T> tmp;
        std::vector<ptrdiff_t> runBase;
        std::vector<ptrdiff_t> runLen;
        ptrdiff_t stackSize = 0;
    };

    // TimSorting.binarySort (Java's fallthrough switch on the number of elements to move).
    template<class T>
    void timBinarySort(TimSortState<T>& ts, std::vector<T>& data_, ptrdiff_t lo, ptrdiff_t hi, ptrdiff_t start) {
        if (start == lo) ++start;

        for (; start < hi; ++start) {
            T pivot = data_[start];

            // Set left (and right) to the index where data_[start] (pivot) belongs.
            ptrdiff_t left = lo;
            ptrdiff_t right = start;

            while (left < right) {
                ptrdiff_t mid = (left + right) / 2;

                if (CompareValues(pivot, data_[mid]) < 0) right = mid;
                else left = mid + 1;
            }

            ptrdiff_t n = start - left; // The number of elements to move
            // Switch is just an optimization for arraycopy in default case
            switch (n) {
            case 2: data_[left + 2] = data_[left + 1]; [[fallthrough]];
            case 1: data_[left + 1] = data_[left]; break;
            default:
                for (ptrdiff_t i = n; i > 0; --i) data_[left + i] = data_[left + i - 1];
                break;
            }
            data_[left] = pivot;
        }
    }

    // TimSorting.countRunAndMakeAscending (the descending run is reversed with
    // TimSorting.reverseRange -> section 1 reverseRange).
    template<class T>
    ptrdiff_t timCountRunAndMakeAscending(TimSortState<T>& ts, std::vector<T>& data_, ptrdiff_t lo, ptrdiff_t hi) {
        ptrdiff_t runHi = lo + 1;
        if (runHi == hi) return 1;

        // Find end of run, and reverse range if descending
        int firstComp = CompareValues(data_[runHi], data_[lo]);
        ++runHi;
        if (firstComp < 0) { // Descending
            while (runHi < hi && CompareValues(data_[runHi], data_[runHi - 1]) < 0) {
                MarkArray(1, data_, runHi);
                ++runHi;
            }
            reverseRange(data_, lo, runHi);
        }
        else {               // Ascending
            while (runHi < hi && CompareValues(data_[runHi], data_[runHi - 1]) >= 0) {
                MarkArray(1, data_, runHi);
                ++runHi;
            }
        }
        return runHi - lo;
    }

    // TimSorting.minRunLength
    inline ptrdiff_t timMinRunLength(ptrdiff_t n) {
        ptrdiff_t r = 0; // Becomes 1 if any 1 bits are shifted off
        while (n >= TimSortMinMerge) {
            r |= (n & 1);
            n >>= 1;
        }
        return n + r;
    }

    // TimSorting.pushRun
    template<class T>
    void timPushRun(TimSortState<T>& ts, ptrdiff_t runBase, ptrdiff_t runLen) {
        ts.runBase[ts.stackSize] = runBase;
        ts.runLen[ts.stackSize] = runLen;
        ++ts.stackSize;
    }

    // TimSorting.gallopLeft
    template<class T>
    ptrdiff_t timGallopLeft(TimSortState<T>& ts, const T& key, std::vector<T>& data_, ptrdiff_t base, ptrdiff_t len, ptrdiff_t hint) {
        ptrdiff_t lastOfs = 0;
        ptrdiff_t ofs = 1;

        MarkArray(3, data_, base + hint);

        if (CompareValues(key, data_[base + hint]) > 0) {
            // Gallop right until data_[base+hint+lastOfs] < key <= data_[base+hint+ofs]
            ptrdiff_t maxOfs = len - hint;

            MarkArray(3, data_, base + hint + ofs);

            while (ofs < maxOfs && CompareValues(key, data_[base + hint + ofs]) > 0) {
                lastOfs = ofs;
                ofs = (ofs * 2) + 1;
                if (ofs <= 0) { // int overflow
                    ofs = maxOfs;
                }

                MarkArray(3, data_, base + hint + ofs);
            }
            if (ofs > maxOfs) ofs = maxOfs;

            // Make offsets relative to base
            lastOfs += hint;
            ofs += hint;
        }
        else { // key <= data_[base + hint]
            // Gallop left until data_[base+hint-ofs] < key <= data_[base+hint-lastOfs]
            ptrdiff_t maxOfs = hint + 1;

            MarkArray(3, data_, base + hint - ofs);

            while (ofs < maxOfs && CompareValues(key, data_[base + hint - ofs]) <= 0) {
                lastOfs = ofs;
                ofs = (ofs * 2) + 1;
                if (ofs <= 0) { // int overflow
                    ofs = maxOfs;
                }

                MarkArray(3, data_, base + hint - ofs);
            }
            if (ofs > maxOfs) ofs = maxOfs;

            // Make offsets relative to base
            ptrdiff_t tmp = lastOfs;
            lastOfs = hint - ofs;
            ofs = hint - tmp;
        }

        // Now data_[base+lastOfs] < key <= data_[base+ofs], so key belongs somewhere
        // to the right of lastOfs but no farther right than ofs.
        ++lastOfs;
        while (lastOfs < ofs) {
            ptrdiff_t m = lastOfs + ((ofs - lastOfs) >> 1);

            MarkArray(3, data_, base + m);

            if (CompareValues(key, data_[base + m]) > 0) lastOfs = m + 1; // data_[base + m] < key
            else ofs = m;                                                 // key <= data_[base + m]
        }
        return ofs;
    }

    // TimSorting.gallopRight
    template<class T>
    ptrdiff_t timGallopRight(TimSortState<T>& ts, const T& key, std::vector<T>& data_, ptrdiff_t base, ptrdiff_t len, ptrdiff_t hint) {
        ptrdiff_t ofs = 1;
        ptrdiff_t lastOfs = 0;

        MarkArray(3, data_, base + hint);

        if (CompareValues(key, data_[base + hint]) < 0) {
            // Gallop left until data_[base+hint-ofs] <= key < data_[base+hint-lastOfs]
            ptrdiff_t maxOfs = hint + 1;

            MarkArray(3, data_, base + hint - ofs);

            while (ofs < maxOfs && CompareValues(key, data_[base + hint - ofs]) < 0) {
                lastOfs = ofs;
                ofs = (ofs * 2) + 1;
                if (ofs <= 0) { // int overflow
                    ofs = maxOfs;
                }

                MarkArray(3, data_, base + hint - ofs);
            }
            if (ofs > maxOfs) ofs = maxOfs;

            // Make offsets relative to base
            ptrdiff_t tmp = lastOfs;
            lastOfs = hint - ofs;
            ofs = hint - tmp;
        }
        else { // data_[base + hint] <= key
            // Gallop right until data_[base+hint+lastOfs] <= key < data_[base+hint+ofs]
            ptrdiff_t maxOfs = len - hint;

            MarkArray(3, data_, base + hint + ofs);

            while (ofs < maxOfs && CompareValues(key, data_[base + hint + ofs]) >= 0) {
                lastOfs = ofs;
                ofs = (ofs * 2) + 1;
                if (ofs <= 0) { // int overflow
                    ofs = maxOfs;
                }

                MarkArray(3, data_, base + hint + ofs);
            }
            if (ofs > maxOfs) ofs = maxOfs;

            // Make offsets relative to base
            lastOfs += hint;
            ofs += hint;
        }

        // Now data_[base+lastOfs] <= key < data_[base+ofs], so key belongs somewhere
        // to the right of lastOfs but no farther right than ofs.
        ++lastOfs;
        while (lastOfs < ofs) {
            ptrdiff_t m = lastOfs + ((ofs - lastOfs) >> 1);

            MarkArray(3, data_, base + m);

            if (CompareValues(key, data_[base + m]) < 0) ofs = m;          // key < data_[base + m]
            else lastOfs = m + 1;                                          // data_[base + m] <= key
        }
        return ofs;
    }

    // TimSorting.ensureCapacity
    template<class T>
    void timEnsureCapacity(TimSortState<T>& ts, ptrdiff_t minCapacity) {
        if (static_cast<ptrdiff_t>(ts.tmp.size()) < minCapacity) {
            // Compute smallest power of 2 > minCapacity
            ptrdiff_t newSize = minCapacity;
            newSize |= newSize >> 1;
            newSize |= newSize >> 2;
            newSize |= newSize >> 4;
            newSize |= newSize >> 8;
            newSize |= newSize >> 16;
            ++newSize;
            if (newSize < 0) { // Not bloody likely!
                newSize = minCapacity;
            }
            else {
                newSize = (std::min)(newSize, ts.len >> 1);
            }
            ts.tmp.assign(static_cast<size_t>(newSize), T());
        }
    }

    // TimSorting.mergeLo (Java's "outer" label becomes an early return from this lambda).
    template<class T>
    void timMergeLo(TimSortState<T>& ts, std::vector<T>& data_, ptrdiff_t base1, ptrdiff_t len1, ptrdiff_t base2, ptrdiff_t len2) {
        // Copy first run into temp array
        timEnsureCapacity(ts, len1);
        for (ptrdiff_t i = 0; i < len1; ++i) {
            ts.tmp[i] = data_[base1 + i];
        }
        std::vector<T>& tmp = ts.tmp;

        ptrdiff_t cursor1 = 0;     // Indexes into tmp array
        ptrdiff_t cursor2 = base2; // Indexes into data_
        ptrdiff_t dest = base1;    // Indexes into data_

        // Move first element of second run and deal with degenerate cases
        data_[dest] = data_[cursor2];
        ++dest; ++cursor2;
        MarkArray(1, data_, dest);
        MarkArray(2, data_, cursor2);
        if (--len2 == 0) {
            for (ptrdiff_t i = 0; i < len1; ++i) data_[dest + i] = tmp[cursor1 + i];
            return;
        }
        if (len1 == 1) {
            for (ptrdiff_t i = 0; i < len2; ++i) data_[dest + i] = data_[cursor2 + i];
            data_[dest + len2] = tmp[cursor1]; // Last elt of run 1 to end of merge
            MarkArray(1, data_, dest + len2);
            return;
        }

        ptrdiff_t minGallop = ts.minGallop; // "    "       "     "      "

        auto outerLoop = [&]() {
            while (true) {
                ptrdiff_t count1 = 0; // Number of times in a row that first run won
                ptrdiff_t count2 = 0; // Number of times in a row that second run won

                // Do the straightforward thing until (if ever) one run starts winning consistently.
                do {
                    if (CompareValues(data_[cursor2], tmp[cursor1]) < 0) {
                        data_[dest] = data_[cursor2];
                        ++dest; ++cursor2;
                        MarkArray(1, data_, dest);
                        MarkArray(2, data_, cursor2);
                        ++count2;
                        count1 = 0;
                        if (--len2 == 0) return;
                    }
                    else {
                        data_[dest] = tmp[cursor1];
                        ++dest; ++cursor1;
                        MarkArray(1, data_, dest);
                        ++count1;
                        count2 = 0;
                        if (--len1 == 1) return;
                    }
                } while ((count1 | count2) < minGallop);

                // One run is winning so consistently that galloping may be a huge win. So try that,
                // and continue galloping until (if ever) neither run appears to be winning anymore.
                do {
                    count1 = timGallopRight(ts, data_[cursor2], tmp, cursor1, len1, 0);
                    if (count1 != 0) {
                        for (ptrdiff_t i = 0; i < count1; ++i) data_[dest + i] = tmp[cursor1 + i];
                        dest += count1;
                        cursor1 += count1;
                        len1 -= count1;
                        if (len1 <= 1) return; // len1 == 1 || len1 == 0
                    }
                    data_[dest] = data_[cursor2];
                    ++dest; ++cursor2;
                    MarkArray(1, data_, dest);
                    MarkArray(2, data_, cursor2);
                    if (--len2 == 0) return;

                    count2 = timGallopLeft(ts, tmp[cursor1], data_, cursor2, len2, 0);
                    if (count2 != 0) {
                        for (ptrdiff_t i = 0; i < count2; ++i) data_[dest + i] = data_[cursor2 + i];
                        dest += count2;
                        cursor2 += count2;
                        len2 -= count2;
                        if (len2 == 0) return;
                    }
                    data_[dest] = tmp[cursor1];
                    ++dest; ++cursor1;
                    MarkArray(1, data_, dest);
                    if (--len1 == 1) return;
                    --minGallop;
                } while (count1 >= TimSortMinGallop || count2 >= TimSortMinGallop);

                if (minGallop < 0) minGallop = 0;
                minGallop += 2; // Penalize for leaving gallop mode
            } // End of "outer" loop
        };
        outerLoop();

        ts.minGallop = minGallop < 1 ? 1 : minGallop; // Write back to field

        if (len1 == 1) {
            for (ptrdiff_t i = 0; i < len2; ++i) data_[dest + i] = data_[cursor2 + i];
            data_[dest + len2] = tmp[cursor1]; // Last elt of run 1 to end of merge
            MarkArray(1, data_, dest + len2);
        }
        else if (len1 == 0) {
            throw WideError(L"Comparison method violates its general contract!");
        }
        else {
            for (ptrdiff_t i = 0; i < len1; ++i) data_[dest + i] = tmp[cursor1 + i];
        }
    }

    // TimSorting.mergeHi (Java's "outer" label becomes an early return from this lambda).
    template<class T>
    void timMergeHi(TimSortState<T>& ts, std::vector<T>& data_, ptrdiff_t base1, ptrdiff_t len1, ptrdiff_t base2, ptrdiff_t len2) {
        // Copy second run into temp array
        timEnsureCapacity(ts, len2);
        for (ptrdiff_t i = 0; i < len2; ++i) {
            ts.tmp[i] = data_[base2 + i];
        }
        std::vector<T>& tmp = ts.tmp;

        ptrdiff_t cursor1 = base1 + len1 - 1; // Indexes into data_
        ptrdiff_t cursor2 = len2 - 1;         // Indexes into tmp array
        ptrdiff_t dest = base2 + len2 - 1;    // Indexes into data_

        // Move last element of first run and deal with degenerate cases
        data_[dest] = data_[cursor1];
        --dest; --cursor1;
        MarkArray(1, data_, dest);
        MarkArray(2, data_, cursor1);
        if (--len1 == 0) {
            for (ptrdiff_t i = 0; i < len2; ++i) data_[dest - (len2 - 1) + i] = tmp[i];
            return;
        }
        if (len2 == 1) {
            dest -= len1;
            cursor1 -= len1;
            // Overlapping shift (dest > cursor1): copy backwards (memmove semantics).
            for (ptrdiff_t i = len1 - 1; i >= 0; --i) data_[dest + 1 + i] = data_[cursor1 + 1 + i];
            data_[dest] = tmp[cursor2];
            MarkArray(1, data_, dest);
            return;
        }

        ptrdiff_t minGallop = ts.minGallop; // "    "       "     "      "

        auto outerLoop = [&]() {
            while (true) {
                ptrdiff_t count1 = 0; // Number of times in a row that first run won
                ptrdiff_t count2 = 0; // Number of times in a row that second run won

                // Do the straightforward thing until (if ever) one run appears to win consistently.
                do {
                    if (CompareValues(tmp[cursor2], data_[cursor1]) < 0) {
                        data_[dest] = data_[cursor1];
                        --dest; --cursor1;
                        MarkArray(1, data_, dest);
                        MarkArray(2, data_, cursor1);
                        ++count1;
                        count2 = 0;
                        if (--len1 == 0) return;
                    }
                    else {
                        data_[dest] = tmp[cursor2];
                        --dest; --cursor2;
                        MarkArray(1, data_, dest);
                        ++count2;
                        count1 = 0;
                        if (--len2 == 1) return;
                    }
                } while ((count1 | count2) < minGallop);

                // One run is winning so consistently that galloping may be a huge win. So try that,
                // and continue galloping until (if ever) neither run appears to be winning anymore.
                do {
                    count1 = len1 - timGallopRight(ts, tmp[cursor2], data_, base1, len1, len1 - 1);
                    if (count1 != 0) {
                        dest -= count1;
                        cursor1 -= count1;
                        len1 -= count1;
                        // Overlapping shift (dest > cursor1): copy backwards (memmove semantics).
                        for (ptrdiff_t i = count1 - 1; i >= 0; --i) data_[dest + 1 + i] = data_[cursor1 + 1 + i];
                        if (len1 == 0) return;
                    }
                    data_[dest] = tmp[cursor2];
                    --dest; --cursor2;
                    MarkArray(1, data_, dest);
                    if (--len2 == 1) return;

                    count2 = len2 - timGallopLeft(ts, data_[cursor1], tmp, 0, len2, len2 - 1);
                    if (count2 != 0) {
                        dest -= count2;
                        cursor2 -= count2;
                        len2 -= count2;
                        for (ptrdiff_t i = 0; i < count2; ++i) data_[dest + 1 + i] = tmp[cursor2 + 1 + i];
                        if (len2 <= 1) return; // len2 == 1 || len2 == 0
                    }
                    data_[dest] = data_[cursor1];
                    --dest; --cursor1;
                    MarkArray(1, data_, dest);
                    MarkArray(2, data_, cursor1);
                    if (--len1 == 0) return;
                    --minGallop;
                } while (count1 >= TimSortMinGallop || count2 >= TimSortMinGallop);

                if (minGallop < 0) minGallop = 0;
                minGallop += 2; // Penalize for leaving gallop mode
            } // End of "outer" loop
        };
        outerLoop();

        ts.minGallop = minGallop < 1 ? 1 : minGallop; // Write back to field

        if (len2 == 1) {
            dest -= len1;
            cursor1 -= len1;
            // Overlapping shift (dest > cursor1): copy backwards (memmove semantics).
            for (ptrdiff_t i = len1 - 1; i >= 0; --i) data_[dest + 1 + i] = data_[cursor1 + 1 + i];
            data_[dest] = tmp[cursor2]; // Move first elt of run2 to front of merge
            MarkArray(1, data_, dest);
        }
        else if (len2 == 0) {
            throw WideError(L"Comparison method violates its general contract!");
        }
        else {
            for (ptrdiff_t i = 0; i < len2; ++i) data_[dest - (len2 - 1) + i] = tmp[i];
        }
    }

    // TimSorting.mergeAt
    template<class T>
    void timMergeAt(TimSortState<T>& ts, std::vector<T>& data_, ptrdiff_t i) {
        ptrdiff_t base1 = ts.runBase[i];
        ptrdiff_t len1 = ts.runLen[i];
        ptrdiff_t base2 = ts.runBase[i + 1];
        ptrdiff_t len2 = ts.runLen[i + 1];

        // Record the length of the combined runs; if i is the 3rd-last run now, also slide
        // over the last run (which isn't involved in this merge). The current run (i+1) goes
        // away in any case.
        ts.runLen[i] = len1 + len2;
        if (i == ts.stackSize - 3) {
            ts.runBase[i + 1] = ts.runBase[i + 2];
            ts.runLen[i + 1] = ts.runLen[i + 2];
        }
        --ts.stackSize;

        // Find where the first element of run2 goes in run1. Prior elements in run1 can be
        // ignored (because they're already in place).
        T key1 = data_[base2];
        ptrdiff_t k = timGallopRight(ts, key1, data_, base1, len1, 0);
        base1 += k;
        len1 -= k;
        if (len1 == 0) return;

        // Find where the last element of run1 goes in run2. Subsequent elements in run2 can be
        // ignored (because they're already in place).
        T key2 = data_[base1 + len1 - 1];
        len2 = timGallopLeft(ts, key2, data_, base2, len2, len2 - 1);
        if (len2 == 0) return;

        // Merge remaining runs, using tmp array with min(len1, len2) elements
        if (len1 <= len2) timMergeLo(ts, data_, base1, len1, base2, len2);
        else timMergeHi(ts, data_, base1, len1, base2, len2);
    }

    // TimSorting.mergeCollapse
    template<class T>
    void timMergeCollapse(TimSortState<T>& ts, std::vector<T>& data_) {
        while (ts.stackSize > 1) {
            ptrdiff_t n = ts.stackSize - 2;
            if ((n >= 1 && ts.runLen[n - 1] <= ts.runLen[n] + ts.runLen[n + 1]) ||
                (n >= 2 && ts.runLen[n - 2] <= ts.runLen[n] + ts.runLen[n - 1])) {
                if (ts.runLen[n - 1] < ts.runLen[n + 1]) --n;
            }
            else if (ts.runLen[n] > ts.runLen[n + 1]) {
                break; // Invariant is established
            }
            timMergeAt(ts, data_, n);
        }
    }

    // TimSorting.mergeForceCollapse
    template<class T>
    void timMergeForceCollapse(TimSortState<T>& ts, std::vector<T>& data_) {
        while (ts.stackSize > 1) {
            ptrdiff_t n = ts.stackSize - 2;
            if (n > 0 && ts.runLen[n - 1] < ts.runLen[n + 1]) --n;
            timMergeAt(ts, data_, n);
        }
    }

    // TimSorting constructor + static sort(TimSorting, int[], int lo, int hi), driven by
    // TimSorting.customSort(a, start, length): lo = start, hi = length.
    template<class T>
    void timSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t length) {
        TimSortState<T> ts{ data_, static_cast<ptrdiff_t>(data_.size()) };

        // Allocate temp storage (which may be increased later if necessary).
        ptrdiff_t tmpLen = ts.len < 2 * TimSortInitialTmpStorageLength ? ts.len / 2 : TimSortInitialTmpStorageLength;
        ts.tmp.assign(static_cast<size_t>(tmpLen), T());

        // Allocate the runs-to-be-merged stack (which cannot be expanded).
        ptrdiff_t stackLen = (ts.len < 120 ? 5 :
                              ts.len < 1542 ? 10 :
                              ts.len < 119151 ? 19 : 40);
        ts.runBase.assign(static_cast<size_t>(stackLen), 0);
        ts.runLen.assign(static_cast<size_t>(stackLen), 0);

        ptrdiff_t lo = start;
        ptrdiff_t hi = length;

        ptrdiff_t nRemaining = hi - lo;
        // If array is small, do a "mini-TimSort" with no merges
        if (nRemaining < TimSortMinMerge) {
            ptrdiff_t initRunLen = timCountRunAndMakeAscending(ts, data_, lo, hi);
            timBinarySort(ts, data_, lo, hi, lo + initRunLen);
            return;
        }

        // March over the array once, left to right, finding natural runs, extending short
        // natural runs to minRun elements, and merging runs to maintain stack invariant.
        ptrdiff_t minRun = timMinRunLength(nRemaining);
        do {
            // Identify next run
            ptrdiff_t runLen = timCountRunAndMakeAscending(ts, data_, lo, hi);

            // If run is short, extend to min(minRun, nRemaining)
            if (runLen < minRun) {
                ptrdiff_t force = nRemaining <= minRun ? nRemaining : minRun;
                timBinarySort(ts, data_, lo, lo + force, lo + runLen);
                runLen = force;
            }

            // Push run onto pending-run stack, and maybe merge
            timPushRun(ts, lo, runLen);
            timMergeCollapse(ts, data_);

            // Advance to find next run
            lo += runLen;
            nRemaining -= runLen;
        } while (nRemaining != 0);

        // Merge all remaining runs to complete sort
        timMergeForceCollapse(ts, data_);
    }

    // WikiSorting: structure to represent ranges within the array (Java's top-level class Range).
    struct WikiRange {
        ptrdiff_t start = 0;
        ptrdiff_t end = 0;
    };

    inline ptrdiff_t wikiLength(const WikiRange& range) {
        return range.end - range.start;
    }

    // WikiSorting: Pull.
    struct WikiPull {
        ptrdiff_t from = 0, to = 0, count = 0;
        WikiRange range{};

        void reset() {
            range.start = 0;
            range.end = 0;
            from = 0;
            to = 0;
            count = 0;
        }
    };

    // WikiSorting: Iterator (base is added so nextRange returns absolute indices of the sorted region).
    struct WikiIterator {
        ptrdiff_t base = 0;
        ptrdiff_t size = 0;
        ptrdiff_t power_of_two = 0;
        ptrdiff_t numerator = 0;
        ptrdiff_t decimal = 0;
        ptrdiff_t denominator = 0;
        ptrdiff_t decimal_step = 0;
        ptrdiff_t numerator_step = 0;

        // 63 -> 32, 64 -> 64, etc. (this comes from Hacker's Delight)
        static ptrdiff_t FloorPowerOfTwo(ptrdiff_t value) {
            ptrdiff_t x = value;
            x = x | (x >> 1);
            x = x | (x >> 2);
            x = x | (x >> 4);
            x = x | (x >> 8);
            x = x | (x >> 16);
            return x - (x >> 1);
        }

        WikiIterator(ptrdiff_t base2, ptrdiff_t size2, ptrdiff_t min_level) {
            base = base2;
            size = size2;
            power_of_two = FloorPowerOfTwo(size);
            denominator = power_of_two / min_level;
            numerator_step = size % denominator;
            decimal_step = size / denominator;
            begin();
        }

        void begin() {
            numerator = 0;
            decimal = 0;
        }

        WikiRange nextRange() {
            ptrdiff_t start = decimal;

            decimal += decimal_step;
            numerator += numerator_step;
            if (numerator >= denominator) {
                numerator -= denominator;
                ++decimal;
            }

            return WikiRange{base + start, base + decimal};
        }

        bool finished() const {
            return decimal >= size;
        }

        bool nextLevel() {
            decimal_step += decimal_step;
            numerator_step += numerator_step;
            if (numerator_step >= denominator) {
                numerator_step -= denominator;
                ++decimal_step;
            }

            return decimal_step < size;
        }

        ptrdiff_t length() const {
            return decimal_step;
        }
    };

    // WikiSorting.BinaryFirst: find the index of the first value within the range that is equal to array[index].
    template<class T>
    ptrdiff_t wikiBinaryFirst(std::vector<T>& data_, const T& value, WikiRange range) {
        ptrdiff_t start = range.start, end = range.end - 1;
        while (start < end) {
            ptrdiff_t mid = start + (end - start) / 2;
            if (CompareValues(data_[mid], value) < 0)
                start = mid + 1;
            else
                end = mid;
        }
        if (start == range.end - 1 && CompareValues(data_[start], value) < 0) ++start;
        return start;
    }

    // WikiSorting.BinaryLast: find the index of the last value within the range that is equal to array[index], plus 1.
    template<class T>
    ptrdiff_t wikiBinaryLast(std::vector<T>& data_, const T& value, WikiRange range) {
        ptrdiff_t start = range.start, end = range.end - 1;
        while (start < end) {
            ptrdiff_t mid = start + (end - start) / 2;
            if (CompareValues(value, data_[mid]) >= 0)
                start = mid + 1;
            else
                end = mid;
        }
        if (start == range.end - 1 && CompareValues(value, data_[start]) >= 0) ++start;
        return start;
    }

    // WikiSorting.FindFirstForward
    template<class T>
    ptrdiff_t wikiFindFirstForward(std::vector<T>& data_, const T& value, WikiRange range, ptrdiff_t unique) {
        if (wikiLength(range) == 0) return range.start;
        ptrdiff_t index, skip = (std::max)(wikiLength(range) / unique, static_cast<ptrdiff_t>(1));

        for (index = range.start + skip; CompareValues(data_[index - 1], value) < 0; index += skip)
            if (index >= range.end - skip)
                return wikiBinaryFirst(data_, value, WikiRange{index, range.end});

        return wikiBinaryFirst(data_, value, WikiRange{index - skip, index});
    }

    // WikiSorting.FindLastForward
    template<class T>
    ptrdiff_t wikiFindLastForward(std::vector<T>& data_, const T& value, WikiRange range, ptrdiff_t unique) {
        if (wikiLength(range) == 0) return range.start;
        ptrdiff_t index, skip = (std::max)(wikiLength(range) / unique, static_cast<ptrdiff_t>(1));

        for (index = range.start + skip; CompareValues(value, data_[index - 1]) >= 0; index += skip)
            if (index >= range.end - skip)
                return wikiBinaryLast(data_, value, WikiRange{index, range.end});

        return wikiBinaryLast(data_, value, WikiRange{index - skip, index});
    }

    // WikiSorting.FindFirstBackward
    template<class T>
    ptrdiff_t wikiFindFirstBackward(std::vector<T>& data_, const T& value, WikiRange range, ptrdiff_t unique) {
        if (wikiLength(range) == 0) return range.start;
        ptrdiff_t index, skip = (std::max)(wikiLength(range) / unique, static_cast<ptrdiff_t>(1));

        for (index = range.end - skip; index > range.start && CompareValues(data_[index - 1], value) >= 0; index -= skip)
            if (index < range.start + skip)
                return wikiBinaryFirst(data_, value, WikiRange{range.start, index});

        return wikiBinaryFirst(data_, value, WikiRange{index, index + skip});
    }

    // WikiSorting.FindLastBackward
    template<class T>
    ptrdiff_t wikiFindLastBackward(std::vector<T>& data_, const T& value, WikiRange range, ptrdiff_t unique) {
        if (wikiLength(range) == 0) return range.start;
        ptrdiff_t index, skip = (std::max)(wikiLength(range) / unique, static_cast<ptrdiff_t>(1));

        for (index = range.end - skip; index > range.start && CompareValues(value, data_[index - 1]) < 0; index -= skip)
            if (index < range.start + skip)
                return wikiBinaryLast(data_, value, WikiRange{range.start, index});

        return wikiBinaryLast(data_, value, WikiRange{index, index + skip});
    }

    // WikiSorting.InsertionSort (n^2 sorting algorithm used to sort tiny chunks of the full array;
    // ArrayV's InsertionSort.customInsertSort is a binary insertion sort).
    template<class T>
    void wikiInsertionSort(std::vector<T>& data_, WikiRange range) {
        binaryInsertSort(data_, range.start, range.end);
    }

    // WikiSorting.Reverse: reverse a range of values within the array.
    template<class T>
    void wikiReverse(std::vector<T>& data_, WikiRange range) {
        reverseRange(data_, range.start, range.end);
    }

    // WikiSorting.BlockSwap: swap a series of values in the array.
    template<class T>
    void wikiBlockSwap(std::vector<T>& data_, ptrdiff_t start1, ptrdiff_t start2, ptrdiff_t block_size) {
        using std::swap;
        for (ptrdiff_t index = 0; index < block_size; ++index) {
            swap(data_[start1 + index], data_[start2 + index]);
        }
    }

    // WikiSorting.Rotate: rotate the values in an array ([0 1 2 3] becomes [1 2 3 0] if we rotate by 1).
    // This assumes that 0 <= amount <= range.length().
    template<class T>
    void wikiRotate(std::vector<T>& data_, ptrdiff_t amount, WikiRange range, bool use_cache,
                    std::vector<T>& cache, ptrdiff_t cache_size) {
        if (wikiLength(range) == 0) return;

        ptrdiff_t split;
        if (amount >= 0)
            split = range.start + amount;
        else
            split = range.end + amount;

        WikiRange range1{range.start, split};
        WikiRange range2{split, range.end};

        if (use_cache) {
            // if the smaller of the two ranges fits into the cache, it's *slightly* faster copying it there and shifting the elements over
            if (wikiLength(range1) <= wikiLength(range2)) {
                if (wikiLength(range1) <= cache_size) {
                    if (!cache.empty()) {
                        for (ptrdiff_t i = 0; i < wikiLength(range1); ++i) cache[i] = data_[range1.start + i];
                        for (ptrdiff_t i = 0; i < wikiLength(range2); ++i) data_[range1.start + i] = data_[range2.start + i];
                        for (ptrdiff_t i = 0; i < wikiLength(range1); ++i) data_[range1.start + wikiLength(range2) + i] = cache[i];
                    }
                    return;
                }
            }
            else {
                if (wikiLength(range2) <= cache_size) {
                    if (!cache.empty()) {
                        for (ptrdiff_t i = 0; i < wikiLength(range2); ++i) cache[i] = data_[range2.start + i];
                        for (ptrdiff_t i = 0; i < wikiLength(range1); ++i) data_[range2.end - wikiLength(range1) + i] = data_[range1.start + i];
                        for (ptrdiff_t i = 0; i < wikiLength(range2); ++i) data_[range1.start + i] = cache[i];
                    }
                    return;
                }
            }
        }

        wikiReverse(data_, range1);
        wikiReverse(data_, range2);
        wikiReverse(data_, range);
    }

    // WikiSorting.MergeInto: merge two ranges from one array and save the results into a different array.
    template<class T>
    void wikiMergeInto(std::vector<T>& from, WikiRange A, WikiRange B, std::vector<T>& into, ptrdiff_t at_index, bool tempwrite) {
        ptrdiff_t A_index = A.start;
        ptrdiff_t B_index = B.start;
        ptrdiff_t insert_index = at_index;
        ptrdiff_t A_last = A.end;
        ptrdiff_t B_last = B.end;

        while (true) {
            if (CompareValues(from[B_index], from[A_index]) >= 0) {
                into[insert_index] = from[A_index];

                if (tempwrite) MarkArray(1, from, A_index);
                else MarkArray(1, into, insert_index);

                ++A_index;
                ++insert_index;
                if (A_index == A_last) {
                    // copy the remainder of B into the final array
                    for (ptrdiff_t i = 0; i < B_last - B_index; ++i) into[insert_index + i] = from[B_index + i];
                    break;
                }
            }
            else {
                into[insert_index] = from[B_index];

                if (tempwrite) MarkArray(1, from, B_index);
                else MarkArray(1, into, insert_index);

                ++B_index;
                ++insert_index;
                if (B_index == B_last) {
                    // copy the remainder of A into the final array
                    for (ptrdiff_t i = 0; i < A_last - A_index; ++i) into[insert_index + i] = from[A_index + i];
                    break;
                }
            }
        }
    }

    // WikiSorting.MergeExternal: merge operation using an external buffer.
    template<class T>
    void wikiMergeExternal(std::vector<T>& data_, WikiRange A, WikiRange B, std::vector<T>& cache) {
        // A fits into the cache, so use that instead of the internal buffer
        ptrdiff_t A_index = 0;
        ptrdiff_t B_index = B.start;
        ptrdiff_t insert_index = A.start;
        ptrdiff_t A_last = wikiLength(A);
        ptrdiff_t B_last = B.end;

        if (wikiLength(B) > 0 && wikiLength(A) > 0) {
            while (true) {
                MarkArray(3, data_, A_index);
                MarkArray(4, data_, B_index);
                if (CompareValues(data_[B_index], cache[A_index]) >= 0) {
                    data_[insert_index] = cache[A_index];
                    ++A_index;
                    ++insert_index;
                    if (A_index == A_last) break;
                }
                else {
                    data_[insert_index] = data_[B_index];
                    ++B_index;
                    ++insert_index;
                    if (B_index == B_last) break;
                }
            }
        }

        // copy the remainder of A into the final array
        if (!cache.empty()) {
            for (ptrdiff_t i = 0; i < A_last - A_index; ++i) data_[insert_index + i] = cache[A_index + i];
        }
    }

    // WikiSorting.MergeInternal: merge operation using an internal buffer.
    template<class T>
    void wikiMergeInternal(std::vector<T>& data_, WikiRange A, WikiRange B, WikiRange buffer) {
        // whenever we find a value to add to the final array, swap it with the value that's already in that spot
        // when this algorithm is finished, 'buffer' will contain its original contents, but in a different order
        using std::swap;
        ptrdiff_t A_count = 0, B_count = 0, insert = 0;

        if (wikiLength(B) > 0 && wikiLength(A) > 0) {
            while (true) {
                if (CompareValues(data_[B.start + B_count], data_[buffer.start + A_count]) >= 0) {
                    MarkArray(3, data_, buffer.start + A_count);
                    swap(data_[A.start + insert], data_[buffer.start + A_count]);
                    ++A_count;
                    ++insert;
                    if (A_count >= wikiLength(A)) break;
                }
                else {
                    MarkArray(3, data_, B.start + B_count);
                    swap(data_[A.start + insert], data_[B.start + B_count]);
                    ++B_count;
                    ++insert;
                    if (B_count >= wikiLength(B)) break;
                }
            }
        }

        // swap the remainder of A into the final array
        wikiBlockSwap(data_, buffer.start + A_count, A.start + insert, wikiLength(A) - A_count);
    }

    // WikiSorting.MergeInPlace: merge operation without a buffer.
    template<class T>
    void wikiMergeInPlace(std::vector<T>& data_, WikiRange A, WikiRange B, std::vector<T>& cache, ptrdiff_t cache_size) {
        if (wikiLength(A) == 0 || wikiLength(B) == 0) return;

        while (true) {
            // find the first place in B where the first item in A needs to be inserted
            ptrdiff_t mid = wikiBinaryFirst(data_, data_[A.start], B);

            // rotate A into place
            ptrdiff_t amount = mid - A.end;
            wikiRotate(data_, -amount, WikiRange{A.start, mid}, true, cache, cache_size);
            if (B.end == mid) break;

            // calculate the new A and B ranges
            B.start = mid;
            A.start = A.start + amount;
            A.end = B.start;
            A.start = wikiBinaryLast(data_, data_[A.start], A);
            if (wikiLength(A) == 0) break;
        }
    }

    // WikiSorting.NetSwap (stability via the index compare of order[]).
    template<class T>
    void wikiNetSwap(std::vector<T>& data_, std::vector<ptrdiff_t>& order, WikiRange range, ptrdiff_t x, ptrdiff_t y) {
        using std::swap;
        int compare = CompareValues(data_[range.start + x], data_[range.start + y]);
        if (compare > 0 || (order[x] > order[y] && compare == 0)) {
            swap(data_[range.start + x], data_[range.start + y]);
            swap(order[x], order[y]);
        }
    }

    // WikiSorting.Sort: bottom-up merge sort combined with an in-place merge algorithm for O(1) memory use.
    // Runs on [start, start + length); cache is sized cache_size (0 means no cache).
    template<class T>
    void wikiSortImpl(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t length, std::vector<T>& cache, ptrdiff_t cache_size) {
        using std::swap;
        ptrdiff_t size = length;

        // if the array is of size 0, 1, 2, or 3, just sort them like so:
        if (size < 4) {
            if (size == 3) {
                // hard-coded insertion sort
                if (CompareValues(data_[start + 1], data_[start + 0]) < 0) {
                    swap(data_[start + 0], data_[start + 1]);
                }
                if (CompareValues(data_[start + 2], data_[start + 1]) < 0) {
                    swap(data_[start + 1], data_[start + 2]);
                    if (CompareValues(data_[start + 1], data_[start + 0]) < 0) {
                        swap(data_[start + 0], data_[start + 1]);
                    }
                }
            }
            else if (size == 2) {
                // swap the items if they're out of order
                if (CompareValues(data_[start + 1], data_[start + 0]) < 0) {
                    swap(data_[start + 0], data_[start + 1]);
                }
            }
            return;
        }

        // sort groups of 4-8 items at a time using an unstable sorting network,
        // but keep track of the original item orders to force it to be stable
        WikiIterator iterator(start, size, 4);
        while (!iterator.finished()) {
            std::vector<ptrdiff_t> order{ 0, 1, 2, 3, 4, 5, 6, 7 };
            WikiRange range = iterator.nextRange();

            if (wikiLength(range) == 8) {
                wikiNetSwap(data_, order, range, 0, 1); wikiNetSwap(data_, order, range, 2, 3);
                wikiNetSwap(data_, order, range, 4, 5); wikiNetSwap(data_, order, range, 6, 7);
                wikiNetSwap(data_, order, range, 0, 2); wikiNetSwap(data_, order, range, 1, 3);
                wikiNetSwap(data_, order, range, 4, 6); wikiNetSwap(data_, order, range, 5, 7);
                wikiNetSwap(data_, order, range, 1, 2); wikiNetSwap(data_, order, range, 5, 6);
                wikiNetSwap(data_, order, range, 0, 4); wikiNetSwap(data_, order, range, 3, 7);
                wikiNetSwap(data_, order, range, 1, 5); wikiNetSwap(data_, order, range, 2, 6);
                wikiNetSwap(data_, order, range, 1, 4); wikiNetSwap(data_, order, range, 3, 6);
                wikiNetSwap(data_, order, range, 2, 4); wikiNetSwap(data_, order, range, 3, 5);
                wikiNetSwap(data_, order, range, 3, 4);
            }
            else if (wikiLength(range) == 7) {
                wikiNetSwap(data_, order, range, 1, 2); wikiNetSwap(data_, order, range, 3, 4); wikiNetSwap(data_, order, range, 5, 6);
                wikiNetSwap(data_, order, range, 0, 2); wikiNetSwap(data_, order, range, 3, 5); wikiNetSwap(data_, order, range, 4, 6);
                wikiNetSwap(data_, order, range, 0, 1); wikiNetSwap(data_, order, range, 4, 5); wikiNetSwap(data_, order, range, 2, 6);
                wikiNetSwap(data_, order, range, 0, 4); wikiNetSwap(data_, order, range, 1, 5);
                wikiNetSwap(data_, order, range, 0, 3); wikiNetSwap(data_, order, range, 2, 5);
                wikiNetSwap(data_, order, range, 1, 3); wikiNetSwap(data_, order, range, 2, 4);
                wikiNetSwap(data_, order, range, 2, 3);
            }
            else if (wikiLength(range) == 6) {
                wikiNetSwap(data_, order, range, 1, 2); wikiNetSwap(data_, order, range, 4, 5);
                wikiNetSwap(data_, order, range, 0, 2); wikiNetSwap(data_, order, range, 3, 5);
                wikiNetSwap(data_, order, range, 0, 1); wikiNetSwap(data_, order, range, 3, 4); wikiNetSwap(data_, order, range, 2, 5);
                wikiNetSwap(data_, order, range, 0, 3); wikiNetSwap(data_, order, range, 1, 4);
                wikiNetSwap(data_, order, range, 2, 4); wikiNetSwap(data_, order, range, 1, 3);
                wikiNetSwap(data_, order, range, 2, 3);
            }
            else if (wikiLength(range) == 5) {
                wikiNetSwap(data_, order, range, 0, 1); wikiNetSwap(data_, order, range, 3, 4);
                wikiNetSwap(data_, order, range, 2, 4);
                wikiNetSwap(data_, order, range, 2, 3); wikiNetSwap(data_, order, range, 1, 4);
                wikiNetSwap(data_, order, range, 0, 3);
                wikiNetSwap(data_, order, range, 0, 2); wikiNetSwap(data_, order, range, 1, 3);
                wikiNetSwap(data_, order, range, 1, 2);
            }
            else if (wikiLength(range) == 4) {
                wikiNetSwap(data_, order, range, 0, 1); wikiNetSwap(data_, order, range, 2, 3);
                wikiNetSwap(data_, order, range, 0, 2); wikiNetSwap(data_, order, range, 1, 3);
                wikiNetSwap(data_, order, range, 1, 2);
            }
        }
        if (size < 8) return;

        // we need to keep track of a lot of ranges during this sort!
        WikiRange buffer1{}, buffer2{};
        WikiRange blockA{}, blockB{};
        WikiRange lastA{}, lastB{};
        WikiRange firstA{};
        WikiRange A{}, B{};

        WikiPull pull[2] = {};

        // then merge sort the higher levels, which can be 8-15, 16-31, 32-63, 64-127, etc.
        while (true) {

            // if every A and B block will fit into the cache, use a special branch specifically for merging with the cache
            // (we use < rather than <= since the block size might be one more than iterator.length())
            if (iterator.length() < cache_size) {

                // if four subarrays fit into the cache, it's faster to merge both pairs of subarrays into the cache,
                // then merge the two merged subarrays from the cache back into the original array
                if ((iterator.length() + 1) * 4 <= cache_size && iterator.length() * 4 <= size) {
                    iterator.begin();
                    while (!iterator.finished()) {
                        // merge A1 and B1 into the cache
                        WikiRange A1 = iterator.nextRange();
                        WikiRange B1 = iterator.nextRange();
                        WikiRange A2 = iterator.nextRange();
                        WikiRange B2 = iterator.nextRange();

                        if (CompareValues(data_[B1.end - 1], data_[A1.start]) < 0) {
                            // the two ranges are in reverse order, so copy them in reverse order into the cache
                            for (ptrdiff_t i = 0; i < wikiLength(A1); ++i) cache[wikiLength(B1) + i] = data_[A1.start + i];
                            for (ptrdiff_t i = 0; i < wikiLength(B1); ++i) cache[i] = data_[B1.start + i];
                        }
                        else if (CompareValues(data_[B1.start], data_[A1.end - 1]) < 0) {
                            // these two ranges weren't already in order, so merge them into the cache
                            wikiMergeInto(data_, A1, B1, cache, 0, true);
                        }
                        else {
                            // if A1, B1, A2, and B2 are all in order, skip doing anything else
                            if (CompareValues(data_[B2.start], data_[A2.end - 1]) >= 0 && CompareValues(data_[A2.start], data_[B1.end - 1]) >= 0) continue;

                            // copy A1 and B1 into the cache in the same order
                            for (ptrdiff_t i = 0; i < wikiLength(A1); ++i) cache[i] = data_[A1.start + i];
                            for (ptrdiff_t i = 0; i < wikiLength(B1); ++i) cache[wikiLength(A1) + i] = data_[B1.start + i];
                        }
                        A1.end = B1.end;

                        // merge A2 and B2 into the cache
                        if (CompareValues(data_[B2.end - 1], data_[A2.start]) < 0) {
                            // the two ranges are in reverse order, so copy them in reverse order into the cache
                            for (ptrdiff_t i = 0; i < wikiLength(A2); ++i) cache[wikiLength(A1) + wikiLength(B2) + i] = data_[A2.start + i];
                            for (ptrdiff_t i = 0; i < wikiLength(B2); ++i) cache[wikiLength(A1) + i] = data_[B2.start + i];
                        }
                        else if (CompareValues(data_[B2.start], data_[A2.end - 1]) < 0) {
                            // these two ranges weren't already in order, so merge them into the cache
                            wikiMergeInto(data_, A2, B2, cache, wikiLength(A1), true);
                        }
                        else {
                            // copy A2 and B2 into the cache in the same order
                            for (ptrdiff_t i = 0; i < wikiLength(A2); ++i) cache[wikiLength(A1) + i] = data_[A2.start + i];
                            for (ptrdiff_t i = 0; i < wikiLength(B2); ++i) cache[wikiLength(A1) + wikiLength(A2) + i] = data_[B2.start + i];
                        }
                        A2.end = B2.end;

                        // merge A1 and A2 from the cache into the array
                        WikiRange A3{0, wikiLength(A1)};
                        WikiRange B3{wikiLength(A1), wikiLength(A1) + wikiLength(A2)};

                        if (CompareValues(cache[B3.end - 1], cache[A3.start]) < 0) {
                            // the two ranges are in reverse order, so copy them in reverse order into the cache
                            for (ptrdiff_t i = 0; i < wikiLength(A3); ++i) data_[A1.start + wikiLength(A2) + i] = cache[A3.start + i];
                            for (ptrdiff_t i = 0; i < wikiLength(B3); ++i) data_[A1.start + i] = cache[B3.start + i];
                        }
                        else if (CompareValues(cache[B3.start], cache[A3.end - 1]) < 0) {
                            // these two ranges weren't already in order, so merge them back into the array
                            wikiMergeInto(cache, A3, B3, data_, A1.start, false);
                        }
                        else {
                            // copy A3 and B3 into the array in the same order
                            for (ptrdiff_t i = 0; i < wikiLength(A3); ++i) data_[A1.start + i] = cache[A3.start + i];
                            for (ptrdiff_t i = 0; i < wikiLength(B3); ++i) data_[A1.start + wikiLength(A1) + i] = cache[B3.start + i];
                        }
                    }

                    // we merged two levels at the same time, so we're done with this level already
                    // (iterator.nextLevel() is called again at the bottom of this outer merge loop)
                    iterator.nextLevel();

                }
                else {
                    iterator.begin();
                    while (!iterator.finished()) {
                        A = iterator.nextRange();
                        B = iterator.nextRange();

                        if (CompareValues(data_[B.end - 1], data_[A.start]) < 0) {
                            // the two ranges are in reverse order, so a simple rotation should fix it
                            wikiRotate(data_, wikiLength(A), WikiRange{A.start, B.end}, true, cache, cache_size);
                        }
                        else if (CompareValues(data_[B.start], data_[A.end - 1]) < 0) {
                            // these two ranges weren't already in order, so we'll need to merge them!
                            for (ptrdiff_t i = 0; i < wikiLength(A); ++i) cache[i] = data_[A.start + i];
                            wikiMergeExternal(data_, A, B, cache);
                        }
                    }
                }
            }
            else {
                // this is where the in-place merge logic starts!
                // 1. pull out two internal buffers each containing sqrt(A) unique values
                //     1a. adjust block_size and buffer_size if we couldn't find enough unique values
                // 2. loop over the A and B subarrays within this level of the merge sort
                //     3. break A and B into blocks of size 'block_size'
                //     4. "tag" each of the A blocks with values from the first internal buffer
                //     5. roll the A blocks through the B blocks and drop/rotate them where they belong
                //     6. merge each A block with any B values that follow, using the cache or the second internal buffer
                // 7. sort the second internal buffer if it exists
                // 8. redistribute the two internal buffers back into the array

                ptrdiff_t block_size = static_cast<ptrdiff_t>(std::sqrt(static_cast<double>(iterator.length())));
                ptrdiff_t buffer_size = iterator.length() / block_size + 1;

                // as an optimization, we really only need to pull out the internal buffers once for each level of merges
                // after that we can reuse the same buffers over and over, then redistribute it when we're finished with this level
                ptrdiff_t index = 0, last = 0, count = 0, pull_index = 0;
                buffer1 = WikiRange{0, 0};
                buffer2 = WikiRange{0, 0};

                pull[0].reset();
                pull[1].reset();

                // find two internal buffers of size 'buffer_size' each
                ptrdiff_t find = buffer_size + buffer_size;
                bool find_separately = false;

                if (block_size <= cache_size) {
                    // if every A block fits into the cache then we won't need the second internal buffer,
                    // so we really only need to find 'buffer_size' unique values
                    find = buffer_size;
                }
                else if (find > iterator.length()) {
                    // we can't fit both buffers into the same A or B subarray, so find two buffers separately
                    find = buffer_size;
                    find_separately = true;
                }

                // we need to find either a single contiguous space containing 2 sqrt(A) unique values (which will be split up into two buffers of size sqrt(A) each),
                // or we need to find one buffer of < 2 sqrt(A) unique values, and a second buffer of sqrt(A) unique values,
                // OR if we couldn't find that many unique values, we need the largest possible buffer we can get

                // in the case where it couldn't find a single buffer of at least sqrt(A) unique values,
                // all of the Merge steps must be replaced by a different merge algorithm (MergeInPlace)

                iterator.begin();
                while (!iterator.finished()) {
                    A = iterator.nextRange();
                    B = iterator.nextRange();

                    // check A for the number of unique values we need to fill an internal buffer
                    // these values will be pulled out to the start of A
                    for (last = A.start, count = 1; count < find; last = index, count++) {
                        index = wikiFindLastForward(data_, data_[last], WikiRange{last + 1, A.end}, find - count);
                        if (index == A.end) break;
                    }
                    index = last;

                    if (count >= buffer_size) {
                        // keep track of the range within the array where we'll need to "pull out" these values to create the internal buffer
                        pull[pull_index].range = WikiRange{A.start, B.end};
                        pull[pull_index].count = count;
                        pull[pull_index].from = index;
                        pull[pull_index].to = A.start;
                        pull_index = 1;

                        if (count == buffer_size + buffer_size) {
                            // we were able to find a single contiguous section containing 2 sqrt(A) unique values,
                            // so this section can be used to contain both of the internal buffers we'll need
                            buffer1 = WikiRange{A.start, A.start + buffer_size};
                            buffer2 = WikiRange{A.start + buffer_size, A.start + count};
                            break;
                        }
                        else if (find == buffer_size + buffer_size) {
                            // we found a buffer that contains at least sqrt(A) unique values, but did not contain the full 2 sqrt(A) unique values,
                            // so we still need to find a second separate buffer of at least sqrt(A) unique values
                            buffer1 = WikiRange{A.start, A.start + count};
                            find = buffer_size;
                        }
                        else if (block_size <= cache_size) {
                            // we found the first and only internal buffer that we need, so we're done!
                            buffer1 = WikiRange{A.start, A.start + count};
                            break;
                        }
                        else if (find_separately) {
                            // found one buffer, but now find the other one
                            buffer1 = WikiRange{A.start, A.start + count};
                            find_separately = false;
                        }
                        else {
                            // we found a second buffer in an 'A' subarray containing sqrt(A) unique values, so we're done!
                            buffer2 = WikiRange{A.start, A.start + count};
                            break;
                        }
                    }
                    else if (pull_index == 0 && count > wikiLength(buffer1)) {
                        // keep track of the largest buffer we were able to find
                        buffer1 = WikiRange{A.start, A.start + count};

                        pull[pull_index].range = WikiRange{A.start, B.end};
                        pull[pull_index].count = count;
                        pull[pull_index].from = index;
                        pull[pull_index].to = A.start;
                    }

                    // check B for the number of unique values we need to fill an internal buffer
                    // these values will be pulled out to the end of B
                    for (last = B.end - 1, count = 1; count < find; last = index - 1, count++) {
                        index = wikiFindFirstBackward(data_, data_[last], WikiRange{B.start, last}, find - count);
                        if (index == B.start) break;
                    }
                    index = last;

                    if (count >= buffer_size) {
                        // keep track of the range within the array where we'll need to "pull out" these values to create the internal buffer
                        pull[pull_index].range = WikiRange{A.start, B.end};
                        pull[pull_index].count = count;
                        pull[pull_index].from = index;
                        pull[pull_index].to = B.end;
                        pull_index = 1;

                        if (count == buffer_size + buffer_size) {
                            // we were able to find a single contiguous section containing 2 sqrt(A) unique values,
                            // so this section can be used to contain both of the internal buffers we'll need
                            buffer1 = WikiRange{B.end - count, B.end - buffer_size};
                            buffer2 = WikiRange{B.end - buffer_size, B.end};
                            break;
                        }
                        else if (find == buffer_size + buffer_size) {
                            // we found a buffer that contains at least sqrt(A) unique values, but did not contain the full 2sqrt(A) unique values,
                            // so we still need to find a second separate buffer of at least sqrt(A) unique values
                            buffer1 = WikiRange{B.end - count, B.end};
                            find = buffer_size;
                        }
                        else if (block_size <= cache_size) {
                            // we found the first and only internal buffer that we need, so we're done!
                            buffer1 = WikiRange{B.end - count, B.end};
                            break;
                        }
                        else if (find_separately) {
                            // found one buffer, but now find the other one
                            buffer1 = WikiRange{B.end - count, B.end};
                            find_separately = false;
                        }
                        else {
                            // buffer2 will be pulled out from a 'B' subarray, so if the first buffer was pulled out from the corresponding 'A' subarray,
                            // we need to adjust the end point for that A subarray so it knows to stop redistributing its values before reaching buffer2
                            if (pull[0].range.start == A.start) pull[0].range.end -= pull[1].count;

                            // we found a second buffer in an 'B' subarray containing sqrt(A) unique values, so we're done!
                            buffer2 = WikiRange{B.end - count, B.end};
                            break;
                        }
                    }
                    else if (pull_index == 0 && count > wikiLength(buffer1)) {
                        // keep track of the largest buffer we were able to find
                        buffer1 = WikiRange{B.end - count, B.end};

                        pull[pull_index].range = WikiRange{A.start, B.end};
                        pull[pull_index].count = count;
                        pull[pull_index].from = index;
                        pull[pull_index].to = B.end;
                    }
                }

                // pull out the two ranges so we can use them as internal buffers
                for (pull_index = 0; pull_index < 2; ++pull_index) {
                    ptrdiff_t len = pull[pull_index].count;

                    if (pull[pull_index].to < pull[pull_index].from) {
                        // we're pulling the values out to the left, which means the start of an A subarray
                        index = pull[pull_index].from;
                        for (count = 1; count < len; ++count) {
                            index = wikiFindFirstBackward(data_, data_[index - 1], WikiRange{pull[pull_index].to, pull[pull_index].from - (count - 1)}, len - count);
                            WikiRange range{index + 1, pull[pull_index].from + 1};
                            wikiRotate(data_, wikiLength(range) - count, range, true, cache, cache_size);
                            pull[pull_index].from = index + count;
                        }
                    }
                    else if (pull[pull_index].to > pull[pull_index].from) {
                        // we're pulling values out to the right, which means the end of a B subarray
                        index = pull[pull_index].from + 1;
                        for (count = 1; count < len; ++count) {
                            index = wikiFindLastForward(data_, data_[index], WikiRange{index, pull[pull_index].to}, len - count);
                            WikiRange range{pull[pull_index].from, index - 1};
                            wikiRotate(data_, count, range, true, cache, cache_size);
                            pull[pull_index].from = index - 1 - count;
                        }
                    }
                }

                // adjust block_size and buffer_size based on the values we were able to pull out
                buffer_size = wikiLength(buffer1);
                block_size = iterator.length() / buffer_size + 1;

                // now that the two internal buffers have been created, it's time to merge each A+B combination at this level of the merge sort!
                iterator.begin();
                while (!iterator.finished()) {
                    A = iterator.nextRange();
                    B = iterator.nextRange();

                    // remove any parts of A or B that are being used by the internal buffers
                    ptrdiff_t astart = A.start;
                    if (astart == pull[0].range.start) {
                        if (pull[0].from > pull[0].to) {
                            A.start += pull[0].count;

                            // if the internal buffer takes up the entire A or B subarray, then there's nothing to merge
                            if (wikiLength(A) == 0) continue;
                        }
                        else if (pull[0].from < pull[0].to) {
                            B.end -= pull[0].count;
                            if (wikiLength(B) == 0) continue;
                        }
                    }
                    if (astart == pull[1].range.start) {
                        if (pull[1].from > pull[1].to) {
                            A.start += pull[1].count;
                            if (wikiLength(A) == 0) continue;
                        }
                        else if (pull[1].from < pull[1].to) {
                            B.end -= pull[1].count;
                            if (wikiLength(B) == 0) continue;
                        }
                    }

                    if (CompareValues(data_[B.end - 1], data_[A.start]) < 0) {
                        // the two ranges are in reverse order, so a simple rotation should fix it
                        wikiRotate(data_, wikiLength(A), WikiRange{A.start, B.end}, true, cache, cache_size);
                    }
                    else if (CompareValues(data_[A.end], data_[A.end - 1]) < 0) {
                        // these two ranges weren't already in order, so we'll need to merge them!

                        // break the remainder of A into blocks. firstA is the uneven-sized first A block
                        blockA = WikiRange{A.start, A.end};
                        firstA = WikiRange{A.start, A.start + wikiLength(blockA) % block_size};

                        // swap the first value of each A block with the value in buffer1
                        ptrdiff_t indexA = buffer1.start;
                        for (index = firstA.end; index < blockA.end; index += block_size) {
                            swap(data_[indexA], data_[index]);
                            ++indexA;
                        }

                        // start rolling the A blocks through the B blocks!
                        // whenever we leave an A block behind, we'll need to merge the previous A block with any B blocks that follow it, so track that information as well
                        lastA = WikiRange{firstA.start, firstA.end};
                        lastB = WikiRange{0, 0};
                        blockB = WikiRange{B.start, B.start + (std::min)(block_size, wikiLength(B))};
                        blockA.start += wikiLength(firstA);
                        indexA = buffer1.start;

                        // if the first unevenly sized A block fits into the cache, copy it there for when we go to Merge it
                        // otherwise, if the second buffer is available, block swap the contents into that
                        if (wikiLength(lastA) <= cache_size && !cache.empty()) {
                            for (ptrdiff_t i = 0; i < wikiLength(lastA); ++i) cache[i] = data_[lastA.start + i];
                        }
                        else if (wikiLength(buffer2) > 0) {
                            wikiBlockSwap(data_, lastA.start, buffer2.start, wikiLength(lastA));
                        }

                        if (wikiLength(blockA) > 0) {
                            while (true) {
                                // if there's a previous B block and the first value of the minimum A block is <= the last value of the previous B block,
                                // then drop that minimum A block behind. or if there are no B blocks left then keep dropping the remaining A blocks.
                                if ((wikiLength(lastB) > 0 && CompareValues(data_[lastB.end - 1], data_[indexA]) >= 0) || wikiLength(blockB) == 0) {
                                    // figure out where to split the previous B block, and rotate it at the split
                                    ptrdiff_t B_split = wikiBinaryFirst(data_, data_[indexA], lastB);
                                    ptrdiff_t B_remaining = lastB.end - B_split;

                                    // swap the minimum A block to the beginning of the rolling A blocks
                                    ptrdiff_t minA = blockA.start;
                                    for (ptrdiff_t findA = minA + block_size; findA < blockA.end; findA += block_size)
                                        if (CompareValues(data_[findA], data_[minA]) < 0)
                                            minA = findA;
                                    wikiBlockSwap(data_, blockA.start, minA, block_size);

                                    // swap the first item of the previous A block back with its original value, which is stored in buffer1
                                    swap(data_[blockA.start], data_[indexA]);
                                    ++indexA;

                                    // locally merge the previous A block with the B values that follow it
                                    // if lastA fits into the external cache we'll use that (with MergeExternal),
                                    // or if the second internal buffer exists we'll use that (with MergeInternal),
                                    // or failing that we'll use a strictly in-place merge algorithm (MergeInPlace)
                                    if (wikiLength(lastA) <= cache_size)
                                        wikiMergeExternal(data_, lastA, WikiRange{lastA.end, B_split}, cache);
                                    else if (wikiLength(buffer2) > 0)
                                        wikiMergeInternal(data_, lastA, WikiRange{lastA.end, B_split}, buffer2);
                                    else
                                        wikiMergeInPlace(data_, lastA, WikiRange{lastA.end, B_split}, cache, cache_size);

                                    if (wikiLength(buffer2) > 0 || block_size <= cache_size) {
                                        // copy the previous A block into the cache or buffer2, since that's where we need it to be when we go to merge it anyway
                                        if (block_size <= cache_size) {
                                            for (ptrdiff_t i = 0; i < block_size; ++i) cache[i] = data_[blockA.start + i];
                                        }
                                        else {
                                            wikiBlockSwap(data_, blockA.start, buffer2.start, block_size);
                                        }

                                        // this is equivalent to rotating, but faster
                                        // the area normally taken up by the A block is either the contents of buffer2, or data we don't need anymore since we memcopied it
                                        // either way, we don't need to retain the order of those items, so instead of rotating we can just block swap B to where it belongs
                                        wikiBlockSwap(data_, B_split, blockA.start + block_size - B_remaining, B_remaining);
                                    }
                                    else {
                                        // we are unable to use the 'buffer2' trick to speed up the rotation operation since buffer2 doesn't exist, so perform a normal rotation
                                        wikiRotate(data_, blockA.start - B_split, WikiRange{B_split, blockA.start + block_size}, true, cache, cache_size);
                                    }

                                    // update the range for the remaining A blocks, and the range remaining from the B block after it was split
                                    lastA = WikiRange{blockA.start - B_remaining, blockA.start - B_remaining + block_size};
                                    lastB = WikiRange{lastA.end, lastA.end + B_remaining};

                                    // if there are no more A blocks remaining, this step is finished!
                                    blockA.start += block_size;
                                    if (wikiLength(blockA) == 0)
                                        break;

                                }
                                else if (wikiLength(blockB) < block_size) {
                                    // move the last B block, which is unevenly sized, to before the remaining A blocks, by using a rotation
                                    // the cache is disabled here since it might contain the contents of the previous A block
                                    wikiRotate(data_, -wikiLength(blockB), WikiRange{blockA.start, blockB.end}, false, cache, cache_size);

                                    lastB = WikiRange{blockA.start, blockA.start + wikiLength(blockB)};
                                    blockA.start += wikiLength(blockB);
                                    blockA.end += wikiLength(blockB);
                                    blockB.end = blockB.start;
                                }
                                else {
                                    // roll the leftmost A block to the end by swapping it with the next B block
                                    wikiBlockSwap(data_, blockA.start, blockB.start, block_size);
                                    lastB = WikiRange{blockA.start, blockA.start + block_size};

                                    blockA.start += block_size;
                                    blockA.end += block_size;
                                    blockB.start += block_size;
                                    blockB.end += block_size;

                                    if (blockB.end > B.end)
                                        blockB.end = B.end;
                                }
                            }
                        }

                        // merge the last A block with the remaining B values
                        if (wikiLength(lastA) <= cache_size)
                            wikiMergeExternal(data_, lastA, WikiRange{lastA.end, B.end}, cache);
                        else if (wikiLength(buffer2) > 0)
                            wikiMergeInternal(data_, lastA, WikiRange{lastA.end, B.end}, buffer2);
                        else
                            wikiMergeInPlace(data_, lastA, WikiRange{lastA.end, B.end}, cache, cache_size);
                    }
                }

                // when we're finished with this merge step we should have the one or two internal buffers left over, where the second buffer is all jumbled up
                // insertion sort the second buffer, then redistribute the buffers back into the array using the opposite process used for creating the buffer

                // while an unstable sort like quick sort could be applied here, in benchmarks it was consistently slightly slower than a simple insertion sort,
                // even for tens of millions of items. this may be because insertion sort is quite fast when the data is already somewhat sorted, like it is here
                wikiInsertionSort(data_, buffer2);

                for (pull_index = 0; pull_index < 2; ++pull_index) {
                    ptrdiff_t unique = pull[pull_index].count * 2;
                    if (pull[pull_index].from > pull[pull_index].to) {
                        // the values were pulled out to the left, so redistribute them back to the right
                        WikiRange buffer{pull[pull_index].range.start, pull[pull_index].range.start + pull[pull_index].count};
                        while (wikiLength(buffer) > 0) {
                            index = wikiFindFirstForward(data_, data_[buffer.start], WikiRange{buffer.end, pull[pull_index].range.end}, unique);
                            ptrdiff_t amount = index - buffer.end;
                            wikiRotate(data_, wikiLength(buffer), WikiRange{buffer.start, index}, true, cache, cache_size);
                            buffer.start += (amount + 1);
                            buffer.end += amount;
                            unique -= 2;
                        }
                    }
                    else if (pull[pull_index].from < pull[pull_index].to) {
                        // the values were pulled out to the right, so redistribute them back to the left
                        WikiRange buffer{pull[pull_index].range.end - pull[pull_index].count, pull[pull_index].range.end};
                        while (wikiLength(buffer) > 0) {
                            index = wikiFindLastBackward(data_, data_[buffer.end - 1], WikiRange{pull[pull_index].range.start, buffer.start}, unique);
                            ptrdiff_t amount = buffer.start - index;
                            wikiRotate(data_, amount, WikiRange{index, buffer.end}, true, cache, cache_size);
                            buffer.start -= amount;
                            buffer.end -= (amount + 1);
                            unique -= 2;
                        }
                    }
                }
            }

            // double the size of each A and B subarray that will be merged in the next level
            if (!iterator.nextLevel()) break;
        }
    }

    // WikiSorting constructor + static sort(WikiSorting, int[] array, int currentLen).
    // The cache is a local vector (empty when cacheSize == 0).
    template<class T>
    void wikiSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t length, ptrdiff_t cacheSize) {
        std::vector<T> cache;
        if (cacheSize != 0) cache.assign(static_cast<size_t>(cacheSize), T());

        wikiSortImpl(data_, start, length, cache, cacheSize);
    }

    // KotaSorting.CACHE_SIZE.
    inline constexpr ptrdiff_t KotaCacheSize = 32;

    // KotaSorting instance state (Java fields tags / cache / bufPos / blockLen / tagLen / bufLen / effMem / ext).
    template<class T>
    struct KotaState {
        std::vector<T> tags;
        std::vector<T> cache;
        ptrdiff_t bufPos = 0;
        ptrdiff_t blockLen = 0;
        ptrdiff_t tagLen = 0;
        ptrdiff_t bufLen = 0;
        ptrdiff_t effMem = 0;
        bool ext = false;
    };

    // KotaSorting.kotaSwap
    template<class T>
    void kotaSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, bool aux) {
        if (aux) {
            MarkArray(2, data_, b);
            data_[a] = data_[b];
        }
        else {
            using std::swap;
            swap(data_[a], data_[b]);
        }
    }

    // KotaSorting.shift (shifts values across a range determined by a, m, b).
    template<class T>
    void kotaShift(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, bool left, bool aux) {
        if (left) {
            if (m == b) return;
            while (m > a) {
                --b;
                --m;
                kotaSwap(data_, b, m, aux);
            }
        }
        else {
            if (m == a) return;
            while (m < b) {
                kotaSwap(data_, a, m, aux);
                ++a;
                ++m;
            }
        }
    }

    // KotaSorting.rotate
    template<class T>
    void kotaRotate(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t start, ptrdiff_t split, ptrdiff_t end) {
        ptrdiff_t temp;
        T tempValue;
        while (split < end && split > start) {
            if (end - split < split - start) {
                if (end - split == 1) {
                    tempValue = data_[split];
                    kotaShift(data_, st, start, split, end, true, true);
                    data_[start] = tempValue;
                    return;
                }
                else {
                    kotaShift(data_, st, 2 * split - end, split, end, true, false);
                    temp = end;
                    end = split;
                    split -= temp - split;
                }
            }
            else {
                if (split - start == 1) {
                    tempValue = data_[start];
                    kotaShift(data_, st, start, split, end, false, true);
                    data_[end - 1] = tempValue;
                    return;
                }
                else {
                    kotaShift(data_, st, start, split, 2 * split - start, false, false);
                    temp = start;
                    start = split;
                    split += split - temp;
                }
            }
        }
    }

    // KotaSorting.binarySearch
    template<class T>
    ptrdiff_t kotaBinarySearch(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, const T& value, bool left) {
        ptrdiff_t a = start, b = end;

        while (a < b) {
            ptrdiff_t m = (a + b) / 2;
            bool comp;

            if (left) comp = CompareValues(value, data_[m]) <= 0;
            else      comp = CompareValues(value, data_[m]) < 0;

            if (comp) b = m;
            else      a = m + 1;
        }

        return a;
    }

    // KotaSorting.findKeys
    template<class T>
    ptrdiff_t kotaFindKeys(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t start, ptrdiff_t end, ptrdiff_t num) {
        ptrdiff_t numKeys = 1, pos = start, posEnd = start + 1;

        for (ptrdiff_t i = start + 1; i < end && numKeys < num; ++i) {
            MarkArray(2, data_, i);
            ptrdiff_t loc = kotaBinarySearch(data_, pos, posEnd, data_[i], true);
            if (i == loc || CompareValues(data_[i], data_[loc]) != 0) {
                kotaRotate(data_, st, pos, posEnd, i);
                ptrdiff_t inc = i - posEnd;
                loc += inc;
                pos += inc;
                posEnd += inc;
                kotaRotate(data_, st, loc, posEnd, posEnd + 1);
                ++numKeys;
                ++posEnd;
            }
        }
        kotaRotate(data_, st, start, pos, posEnd);
        return numKeys;
    }

    // KotaSorting.swapToTags
    template<class T>
    void kotaSwapToTags(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t a, ptrdiff_t i, bool aux) {
        if (aux) {
            MarkArray(1, data_, a);
            MarkArray(2, data_, i);
            T temp = st.tags[i];
            st.tags[i] = data_[a];
            data_[a] = temp;
        }
        else {
            kotaSwap(data_, st.bufPos + i, a, false);
        }
    }

    // KotaSorting.multiSwap / multiSwapBW (BW: the starting points are the end points inclusive).
    template<class T>
    void kotaMultiSwap(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len, bool aux) {
        for (ptrdiff_t i = 0; i < len; ++i)
            kotaSwap(data_, a + i, b + i, aux);
    }

    template<class T>
    void kotaMultiSwapBW(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len, bool aux) {
        for (ptrdiff_t i = 0; i < len; ++i)
            kotaSwap(data_, a - i, b - i, aux);
    }

    // KotaSorting.blockSelect (selection sort on the tagged blocks; tag values are unique).
    template<class T>
    void kotaBlockSelect(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t pos, ptrdiff_t count) {
        for (ptrdiff_t j = 0; j < count; ++j) {
            ptrdiff_t start = pos + j * st.blockLen, min = start;

            for (ptrdiff_t i = j + 1; i < count; ++i) {
                ptrdiff_t sel = pos + i * st.blockLen;
                if (CompareValues(data_[sel], data_[min]) == -1)
                    min = sel;
            }
            if (start != min) kotaMultiSwap(data_, st, start, min, st.blockLen, false);
            kotaSwapToTags(data_, st, start, j, false);
        }
    }

    template<class T>
    void kotaBlockSelectBW(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t pos, ptrdiff_t count) {
        for (ptrdiff_t j = 0; j < count; ++j) {
            ptrdiff_t start = pos - j * st.blockLen, min = start;

            for (ptrdiff_t i = j + 1; i < count; ++i) {
                ptrdiff_t sel = pos - i * st.blockLen;
                if (CompareValues(data_[sel], data_[min]) == -1)
                    min = sel;
            }
            if (start != min) kotaMultiSwapBW(data_, st, start, min, st.blockLen, false);
            kotaSwapToTags(data_, st, start, j, false);
        }
    }

    // KotaSorting.blockCycle / blockCycleBW (EctaSort: cycle sort on the tagged blocks).
    template<class T>
    void kotaBlockCycle(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t pos, ptrdiff_t count, ptrdiff_t p) {
        for (ptrdiff_t j = 0; j < count; ++j) {
            ptrdiff_t start = pos + j * st.blockLen;

            if (j != static_cast<ptrdiff_t>(data_[start])) {
                ptrdiff_t first = static_cast<ptrdiff_t>(data_[start]);
                ptrdiff_t val = j;

                kotaMultiSwap(data_, st, p, start, st.blockLen, true);

                while (val != first) {
                    ptrdiff_t valStart = pos + val * st.blockLen;

                    ptrdiff_t k = j + 1, next = pos + k * st.blockLen;
                    while (CompareValues(static_cast<ptrdiff_t>(data_[next]), val) != 0)
                        next = pos + (++k) * st.blockLen;

                    val = k;
                    kotaMultiSwap(data_, st, valStart, next, st.blockLen, true);
                }

                first = pos + first * st.blockLen;
                kotaMultiSwap(data_, st, first, p, st.blockLen, true);
            }

            kotaSwapToTags(data_, st, start, j, true);
        }
    }

    template<class T>
    void kotaBlockCycleBW(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t pos, ptrdiff_t count, ptrdiff_t p) {
        for (ptrdiff_t j = 0; j < count; ++j) {
            ptrdiff_t start = pos - j * st.blockLen;

            if (j != static_cast<ptrdiff_t>(data_[start])) {
                ptrdiff_t first = static_cast<ptrdiff_t>(data_[start]);
                ptrdiff_t val = j;

                kotaMultiSwapBW(data_, st, p, start, st.blockLen, true);

                while (val != first) {
                    ptrdiff_t valStart = pos - val * st.blockLen;

                    ptrdiff_t k = j + 1, next = pos - k * st.blockLen;
                    while (CompareValues(static_cast<ptrdiff_t>(data_[next]), val) != 0)
                        next = pos - (++k) * st.blockLen;

                    val = k;
                    kotaMultiSwapBW(data_, st, valStart, next, st.blockLen, true);
                }

                first = pos - first * st.blockLen;
                kotaMultiSwapBW(data_, st, first, p, st.blockLen, true);
            }

            kotaSwapToTags(data_, st, start, j, true);
        }
    }

    // KotaSorting.inPlaceMerge / inPlaceMergeBW (O(n) worst case in-place merge; [a, m) then [m, b)).
    template<class T>
    void kotaInPlaceMerge(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = a, j = m, k;

        while (i < j && j < b) {
            if (CompareValues(data_[i], data_[j]) == 1) {
                k = kotaBinarySearch(data_, j, b, data_[i], true);
                kotaRotate(data_, st, i, j, k);

                i += k - j;
                j = k;
            }
            else ++i;
        }
    }

    template<class T>
    void kotaInPlaceMergeBW(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = m - 1, j = b - 1, k;

        while (j > i && i >= a) {
            if (CompareValues(data_[i], data_[j]) >= 0) {
                k = kotaBinarySearch(data_, a, i + 1, data_[j], true);
                kotaRotate(data_, st, k, i + 1, j + 1);

                j -= (i + 1) - k;
                i = k - 1;
            }
            else --j;
        }
    }

    // KotaSorting.inPlaceMerge2 (O(n^2) worst case in-place merge for small lists).
    template<class T>
    void kotaInPlaceMerge2(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t start, ptrdiff_t mid, ptrdiff_t end) {
        using std::swap;
        ptrdiff_t i = start, m = mid, k = mid, q;

        while (m < end) {
            if (CompareValues(data_[m - 1], data_[m]) <= 0)
                return;

            while (i < m - 1 && CompareValues(data_[i], data_[m]) <= 0) ++i;
            swap(data_[i], data_[k]);
            ++i;
            ++k;

            while (i < m) {
                MarkArray(3, data_, m);
                while (i < m && k < end && CompareValues(data_[m], data_[k]) == 1) {
                    swap(data_[i], data_[k]);
                    ++i;
                    ++k;
                }

                if (i >= m) break;
                else if (k >= end) {
                    kotaRotate(data_, st, i, m, end);
                    return;
                }
                else if (k - m >= m - i) {
                    kotaRotate(data_, st, i, m, k);
                    break;
                }

                q = m;
                while (i < m && q < k && CompareValues(data_[q], data_[k]) <= 0) {
                    swap(data_[i], data_[q]);
                    ++i;
                    ++q;
                }
                kotaRotate(data_, st, m, q, k);
            }

            m = k;
        }
    }

    // KotaSorting.inPlaceMergeSort2 (O(n^2) worst case in-place stable sort).
    template<class T>
    void kotaInPlaceMergeSort2(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t start, ptrdiff_t end) {
        ptrdiff_t length = end - start, j;

        for (ptrdiff_t i = 1; i < length; i *= 2) {
            for (j = start; j + 2 * i < end; j += 2 * i)
                kotaInPlaceMerge2(data_, st, j, j + i, j + 2 * i);

            if (j + i < end)
                kotaInPlaceMerge2(data_, st, j, j + i, end);
        }
    }

    // KotaSorting.mergeWithBuf
    template<class T>
    void kotaMergeWithBuf(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t l) {
        ptrdiff_t i = a, j = m, k = a - l;

        while (i < m && j < b) {
            if (CompareValues(data_[i], data_[j]) <= 0) {
                kotaSwap(data_, k, i, st.ext);
                ++k;
                ++i;
            }
            else {
                kotaSwap(data_, k, j, st.ext);
                ++k;
                ++j;
            }
        }
        while (j < b) {
            kotaSwap(data_, k, j, st.ext);
            ++k;
            ++j;
        }

        kotaShift(data_, st, k, i, m, false, st.ext);
    }

    // KotaSorting.dualMerge
    template<class T>
    void kotaDualMerge(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t l) {
        if (b - m <= l) {
            kotaMergeWithBuf(data_, st, a, m, b, l);
        }
        else {
            ptrdiff_t i = a, j = m, k = a - l;

            while (k < i && i < m) {
                if (CompareValues(data_[i], data_[j]) <= 0) {
                    kotaSwap(data_, k, i, st.ext);
                    ++k;
                    ++i;
                }
                else {
                    kotaSwap(data_, k, j, st.ext);
                    ++k;
                    ++j;
                }
            }

            if (k < i) {
                kotaShift(data_, st, j - l, j, b, false, st.ext);
            }
            else {
                ptrdiff_t i2 = m - 1, j2 = b - 1;
                k = (m - 1) + (b - j);

                while (i2 >= i && j2 >= j) {
                    if (CompareValues(data_[i2], data_[j2]) == 1) {
                        kotaSwap(data_, k, i2, st.ext);
                        --k;
                        --i2;
                    }
                    else {
                        kotaSwap(data_, k, j2, st.ext);
                        --k;
                        --j2;
                    }
                }
                while (j2 >= j) {
                    kotaSwap(data_, k, j2, st.ext);
                    --k;
                    --j2;
                }
            }
        }
    }

    // KotaSorting.dualMergeBW
    template<class T>
    void kotaDualMergeBW(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t l) {
        ptrdiff_t i = m - 1, j = b - 1, k = b - 1 + l;

        while (k > j && j >= m) {
            if (CompareValues(data_[i], data_[j]) == 1) {
                kotaSwap(data_, k, i, st.ext);
                --k;
                --i;
            }
            else {
                kotaSwap(data_, k, j, st.ext);
                --k;
                --j;
            }
        }

        if (j < m) {
            kotaShift(data_, st, a, i + 1, i + 1 + l, true, st.ext);
        }
        else {
            ptrdiff_t i2 = a, j2 = m;
            ++i;
            ++j;
            k = m - (i - a);

            while (i2 < i && j2 < j) {
                if (CompareValues(data_[i2], data_[j2]) <= 0) {
                    kotaSwap(data_, k, i2, st.ext);
                    ++k;
                    ++i2;
                }
                else {
                    kotaSwap(data_, k, j2, st.ext);
                    ++k;
                    ++j2;
                }
            }
            while (i2 < i) {
                kotaSwap(data_, k, i2, st.ext);
                ++k;
                ++i2;
            }
        }
    }

    // KotaSorting.blockMerge
    template<class T>
    void kotaBlockMerge(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, bool auxTag) {
        if (b - m <= 2 * st.bufLen) {
            kotaDualMerge(data_, st, a, m, b, st.bufLen);
            return;
        }

        ptrdiff_t i = a, j = m, k, first;
        ptrdiff_t leftAD = st.bufLen, rightAD = 0;
        ptrdiff_t left = i - st.bufLen, right = j;
        ptrdiff_t tagCount = 0;

        // merge as many block sized sequences to the left
        // no block tagging happens here
        while (i < m && leftAD >= rightAD) { // j will never be >= b
            k = 0;

            while (i < m && k < st.blockLen) {
                if (CompareValues(data_[i], data_[j]) <= 0) {
                    kotaSwap(data_, left, i, st.ext);
                    ++left;
                    ++i;
                }
                else {
                    kotaSwap(data_, left, j, st.ext);
                    ++left;
                    ++j;
                    ++rightAD;
                    --leftAD;
                }
                ++k;
            }
        }

        ptrdiff_t selStart = left; // where to start selecting the blocks back in place

        while (i < m && j < b) {
            // merge as many block sized sequences to the right
            while (i < m && j < b && rightAD > leftAD) {
                first = right;
                k = 0;

                while (i < m && j < b && k < st.blockLen) {
                    if (CompareValues(data_[i], data_[j]) <= 0) {
                        kotaSwap(data_, right, i, st.ext);
                        ++right;
                        ++i;
                        --rightAD;
                        ++leftAD;
                    }
                    else {
                        kotaSwap(data_, right, j, st.ext);
                        ++right;
                        ++j;
                    }
                    ++k;
                }
                // move as many elements as possible to the block before breaking out
                while (i < m && k < st.blockLen) {
                    kotaSwap(data_, right, i, st.ext);
                    ++right;
                    ++i;
                    --rightAD;
                    ++leftAD;
                    ++k;
                }
                while (j < b && k < st.blockLen) {
                    kotaSwap(data_, right, j, st.ext);
                    ++right;
                    ++j;
                    ++k;
                }

                if (k == st.blockLen) // if block is not complete don't tag
                    kotaSwapToTags(data_, st, first, tagCount++, auxTag);
                else {
                    // if there was a leftover block shift the right buffer back
                    kotaShift(data_, st, first, first + k, b, true, st.ext);
                    j = b - k;
                    right = first;
                }
            }

            // merge as many block sized sequences to the left
            while (i < m && j < b && leftAD >= rightAD) {
                first = left;
                k = 0;

                while (i < m && j < b && k < st.blockLen) {
                    if (CompareValues(data_[i], data_[j]) <= 0) {
                        kotaSwap(data_, left, i, st.ext);
                        ++left;
                        ++i;
                    }
                    else {
                        kotaSwap(data_, left, j, st.ext);
                        ++left;
                        ++j;
                        ++rightAD;
                        --leftAD;
                    }
                    ++k;
                }
                while (i < m && k < st.blockLen) {
                    kotaSwap(data_, left, i, st.ext);
                    ++left;
                    ++i;
                    ++k;
                }
                while (j < b && k < st.blockLen) {
                    kotaSwap(data_, left, j, st.ext);
                    ++left;
                    ++j;
                    ++rightAD;
                    --leftAD;
                    ++k;
                }

                if (k == st.blockLen)
                    kotaSwapToTags(data_, st, first, tagCount++, auxTag);
                else {
                    // rotates leftover block along with buffer to the end
                    kotaRotate(data_, st, first, m, right);
                    left += right - m;
                    leftAD = 0;
                }
            }
        }
        if (i >= m && leftAD == st.blockLen && tagCount > 0) { // if the left buffer is the same size as a block
            kotaMultiSwap(data_, st, left, right - st.blockLen, st.blockLen, st.ext); // it can be swapped with the last tagged block
        }
        else {
            if (i < m) { // if left range wasnt fully merged
                kotaRotate(data_, st, left, m, right); // rotate it to the right buffer along with the left buffer
                left += right - m;
            }
            kotaShift(data_, st, left, left + leftAD, right, false, st.ext); // shift left buffer to connect with right buffer
        }
        if (j < b) {
            kotaShift(data_, st, j - st.bufLen, j, b, false, st.ext); // if right range wasnt fully merged
        }                                                             // shift the full buffer to the end
        if (auxTag) kotaBlockCycle(data_, st, selStart, tagCount, b - st.bufLen);
        else        kotaBlockSelect(data_, st, selStart, tagCount);
    }

    // KotaSorting.blockMergeBW
    template<class T>
    void kotaBlockMergeBW(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, bool auxTag) {
        ptrdiff_t i = m - 1, j = b - 1, k, first;
        ptrdiff_t leftAD = 0, rightAD = st.bufLen;
        ptrdiff_t left = i, right = j + st.bufLen;
        ptrdiff_t tagCount = 0;

        while (j >= m && rightAD >= leftAD) {
            k = 0;

            while (j >= m && k < st.blockLen) {
                if (CompareValues(data_[i], data_[j]) == 1) {
                    kotaSwap(data_, right, i, st.ext);
                    --right;
                    --i;
                    ++leftAD;
                    --rightAD;
                }
                else {
                    kotaSwap(data_, right, j, st.ext);
                    --right;
                    --j;
                }
                ++k;
            }
        }

        ptrdiff_t selStart = right;

        while (j >= m && i >= a) {
            while (j >= m && i >= a && leftAD > rightAD) {
                first = left;
                k = 0;

                while (j >= m && i >= a && k < st.blockLen) {
                    if (CompareValues(data_[i], data_[j]) == 1) {
                        kotaSwap(data_, left, i, st.ext);
                        --left;
                        --i;
                    }
                    else {
                        kotaSwap(data_, left, j, st.ext);
                        --left;
                        --j;
                        ++rightAD;
                        --leftAD;
                    }
                    ++k;
                }
                while (j >= m && k < st.blockLen) {
                    kotaSwap(data_, left, j, st.ext);
                    --left;
                    --j;
                    ++rightAD;
                    --leftAD;
                    ++k;
                }
                while (i >= a && k < st.blockLen) {
                    kotaSwap(data_, left, i, st.ext);
                    --left;
                    --i;
                    ++k;
                }

                if (k == st.blockLen)
                    kotaSwapToTags(data_, st, first, tagCount++, auxTag);
                else {
                    kotaShift(data_, st, a, first + 1 - k, first + 1, false, st.ext);
                    i = a - 1 + k;
                    left = first;
                }
            }

            while (j >= m && i >= a && rightAD >= leftAD) {
                first = right;
                k = 0;

                while (j >= m && i >= a && k < st.blockLen) {
                    if (CompareValues(data_[i], data_[j]) == 1) {
                        kotaSwap(data_, right, i, st.ext);
                        --right;
                        --i;
                        ++leftAD;
                        --rightAD;
                    }
                    else {
                        kotaSwap(data_, right, j, st.ext);
                        --right;
                        --j;
                    }
                    ++k;
                }
                while (j >= m && k < st.blockLen) {
                    kotaSwap(data_, right, j, st.ext);
                    --right;
                    --j;
                    ++k;
                }
                while (i >= a && k < st.blockLen) {
                    kotaSwap(data_, right, i, st.ext);
                    --right;
                    --i;
                    ++leftAD;
                    --rightAD;
                    ++k;
                }

                if (k == st.blockLen)
                    kotaSwapToTags(data_, st, first, tagCount++, auxTag);
                else {
                    kotaRotate(data_, st, left + 1, m, first + 1);
                    right -= m - (left + 1);
                    rightAD = 0;
                }
            }
        }

        if (j < m && rightAD == st.blockLen && tagCount > 0) {
            kotaMultiSwapBW(data_, st, right, left + st.blockLen, st.blockLen, st.ext);
        }
        else {
            if (j >= m) {
                kotaRotate(data_, st, left + 1, m, right + 1);
                right -= m - (left + 1);
            }
            kotaShift(data_, st, left + 1, right + 1 - rightAD, right + 1, true, st.ext);
        }
        if (i >= a) kotaShift(data_, st, a, i + 1, i + 1 + st.bufLen, true, st.ext);

        if (auxTag) kotaBlockCycleBW(data_, st, selStart, tagCount, a - 1 + st.bufLen);
        else        kotaBlockSelectBW(data_, st, selStart, tagCount);
    }

    // KotaSorting.mergeWithBufStatic
    template<class T>
    void kotaMergeWithBufStatic(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p, bool bw) {
        if (m - a < 1 || b - m < 1)
            return;

        using std::swap;
        ptrdiff_t i, j, k, q;
        if (bw) {
            i = (b - m) - 1;
            j = m - 1;
            k = b - 1;
            while (i >= 0 && j >= a) {
                if (CompareValues(data_[j], data_[p + i]) >= 0) {
                    q = kotaBinarySearch(data_, a, j + 1, data_[p + i], true);
                    while (j >= q) {
                        swap(data_[k], data_[j]);
                        --k;
                        --j;
                    }
                }
                swap(data_[k], data_[p + i]);
                --k;
                --i;
            }
            while (i >= 0) {
                swap(data_[k], data_[p + i]);
                --k;
                --i;
            }
        }
        else {
            i = 0;
            j = m;
            k = a;
            while (i < m - a && j < b) {
                if (CompareValues(data_[j], data_[p + i]) == -1) {
                    q = kotaBinarySearch(data_, j, b, data_[p + i], true);
                    while (j < q) {
                        swap(data_[k], data_[j]);
                        ++k;
                        ++j;
                    }
                }
                swap(data_[k], data_[p + i]);
                ++k;
                ++i;
            }
            while (i < m - a) {
                swap(data_[k], data_[p + i]);
                ++k;
                ++i;
            }
        }
    }

    // KotaSorting.mergeExtBuf
    template<class T>
    void kotaMergeExtBuf(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t a, ptrdiff_t b, bool bw) {
        ptrdiff_t i, j, k, m;
        if (bw) {
            i = st.bufLen - 1;
            j = (b - 1) - st.bufLen;
            k = b - 1;
            MarkArray(2, data_, a + i);
            while (i >= 0 && j >= a) {
                if (CompareValues(data_[j], st.cache[i]) >= 0) {
                    m = kotaBinarySearch(data_, a, j + 1, st.cache[i], true);
                    while (j >= m) {
                        data_[k] = data_[j];
                        --k;
                        --j;
                    }
                }
                data_[k] = st.cache[i];
                --k;
                --i;
                if (i > 0) MarkArray(2, data_, a + i);
            }
            while (i >= 0) {
                data_[k] = st.cache[i];
                --k;
                --i;
                if (i > 0) MarkArray(2, data_, a + i);
            }
        }
        else {
            i = 0;
            j = a + st.bufLen;
            k = a;
            MarkArray(2, data_, a + i);
            while (i < st.bufLen && j < b) {
                if (CompareValues(data_[j], st.cache[i]) == -1) {
                    m = kotaBinarySearch(data_, j, b, st.cache[i], true);
                    while (j < m) {
                        data_[k] = data_[j];
                        ++k;
                        ++j;
                    }
                }
                data_[k] = st.cache[i];
                ++k;
                ++i;
                MarkArray(2, data_, a + i);
            }
            while (i < st.bufLen) {
                data_[k] = st.cache[i];
                ++k;
                ++i;
                MarkArray(2, data_, a + i);
            }
        }
    }

    // KotaSorting.kotaIterator (returns true when the backwards merge pass was used).
    template<class T>
    bool kotaIterator(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t start, ptrdiff_t end, bool auxTag) {
        ptrdiff_t i = 1, j, effStart = start + st.bufLen, length = end - effStart;

        if (!st.ext) {
            while (i < 16) {
                for (j = effStart; j + 2 * i < end; j += 2 * i)
                    kotaInPlaceMerge2(data_, st, j, j + i, j + 2 * i);

                if (j + i < end)
                    kotaInPlaceMerge2(data_, st, j, j + i, end);

                i *= 2;
            }
        }

        while (i <= st.bufLen) {
            ptrdiff_t l = i;

            for (j = effStart; j + 2 * i < end; j += 2 * i)
                kotaMergeWithBuf(data_, st, j, j + i, j + 2 * i, l);

            if (j + i < end)
                kotaMergeWithBuf(data_, st, j, j + i, end, l);
            else
                kotaShift(data_, st, j - l, j, end, false, st.ext);

            i *= 2;

            for (j = effStart - l; j + 2 * i < end - l; j += 2 * i);

            if (j + i < end - l)
                kotaDualMergeBW(data_, st, j, j + i, end - l, l);
            else
                kotaShift(data_, st, j, end - l, end, true, st.ext);

            for (j -= 2 * i; j >= effStart - l; j -= 2 * i)
                kotaDualMergeBW(data_, st, j, j + i, j + 2 * i, l);

            i *= 2;

            if (st.ext && st.effMem < (std::min)(i, st.bufLen)) {
                for (ptrdiff_t q = 0; q < st.effMem; ++q) data_[effStart - st.effMem + q] = st.cache[q];
                st.ext = false;
            }
        }

        while (i < length) {
            for (j = effStart; j + 2 * i < end; j += 2 * i)
                kotaBlockMerge(data_, st, j, j + i, j + 2 * i, auxTag);

            if (j + i < end)
                kotaBlockMerge(data_, st, j, j + i, end, auxTag);
            else
                kotaShift(data_, st, j - st.bufLen, j, end, false, st.ext);

            i *= 2;
            if (i >= length) return true;

            for (j = start; j + 2 * i < end - st.bufLen; j += 2 * i);

            if (j + i < end - st.bufLen)
                kotaBlockMergeBW(data_, st, j, j + i, end - st.bufLen, auxTag);
            else
                kotaShift(data_, st, j, end - st.bufLen, end, true, st.ext);

            for (j -= 2 * i; j >= start; j -= 2 * i)
                kotaBlockMergeBW(data_, st, j, j + i, j + 2 * i, auxTag);

            i *= 2;
        }

        return false;
    }

    // KotaSorting.kotaSort
    template<class T>
    void kotaSortImpl(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t start, ptrdiff_t end) {
        ptrdiff_t length = end - start;
        if (length <= 128) {
            kotaInPlaceMergeSort2(data_, st, start, end);
            return;
        }

        st.ext = false;
        st.bufPos = start;
        for (st.blockLen = 1; st.blockLen * st.blockLen < length; st.blockLen *= 2); // ceiling power of 2 sqrt

        ptrdiff_t ideal = st.blockLen * 2;
        st.bufLen = kotaFindKeys(data_, st, start, end, ideal);

        if (st.bufLen < ideal) {
            if (st.bufLen == 1) {
                return;
            }
            else if (st.bufLen <= 16) {
                kotaInPlaceMergeSort2(data_, st, start, end);
                return;
            }
            else {
                // phase 2 (Java prints "phase 2" here before falling back)
                kotaInPlaceMergeSort2(data_, st, start, end);
                return;
            }
        }

        ideal = length / st.blockLen;
        st.tagLen = kotaFindKeys(data_, st, start + st.bufLen, end, ideal);

        if (st.tagLen < ideal) {
            if (st.tagLen <= 16) {
                kotaInPlaceMergeSort2(data_, st, start, end);
                return;
            }
            else {
                // phase 2 (Java prints "phase 2" here before falling back)
                kotaInPlaceMergeSort2(data_, st, start, end);
                return;
            }
        }

        ptrdiff_t bufStart = start + st.tagLen;
        ptrdiff_t effStart = bufStart + st.bufLen;
        ptrdiff_t bufEnd = start + st.bufLen;
        kotaShift(data_, st, start, bufEnd, effStart, false, false);

        bool bw = kotaIterator(data_, st, bufStart, end, false);

        if (bw) {
            ptrdiff_t endStart = end - st.bufLen;
            kotaMultiSwap(data_, st, start, endStart, st.tagLen, false);
            kotaMergeWithBufStatic(data_, st, start, bufStart, endStart, endStart, false);
            kotaInPlaceMergeSort2(data_, st, endStart, end);

            ptrdiff_t mid = endStart + st.blockLen;
            ptrdiff_t pos = kotaBinarySearch(data_, start, endStart, data_[mid - 1], true);
            kotaRotate(data_, st, pos, endStart, mid);
            pos += st.blockLen;

            kotaMultiSwapBW(data_, st, end - 1, pos - 1, st.blockLen, false);
            kotaMergeWithBufStatic(data_, st, start, pos - st.blockLen, pos, mid, true);
            kotaInPlaceMergeSort2(data_, st, mid, end);
            kotaInPlaceMergeBW(data_, st, pos, mid, end);
        }
        else {
            kotaMergeWithBufStatic(data_, st, bufEnd, effStart, end, start, false);
            kotaInPlaceMergeSort2(data_, st, start, bufEnd);

            ptrdiff_t mid = start + st.blockLen;
            ptrdiff_t pos = kotaBinarySearch(data_, bufEnd, end, data_[mid], true);
            kotaRotate(data_, st, mid, bufEnd, pos);
            pos -= st.blockLen;

            kotaMultiSwap(data_, st, start, pos, st.blockLen, false);
            kotaMergeWithBufStatic(data_, st, pos, pos + st.blockLen, end, start, false);
            kotaInPlaceMergeSort2(data_, st, start, mid);
            kotaInPlaceMerge(data_, st, start, mid, pos);
        }
    }

    // KotaSorting.ectaSort
    template<class T>
    void ectaSortImpl(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t start, ptrdiff_t end) {
        ptrdiff_t length = end - start;
        if (length <= 16) {
            kotaInPlaceMergeSort2(data_, st, start, end);
            return;
        }

        st.ext = true;
        st.bufPos = start;
        for (st.blockLen = 1; st.blockLen * st.blockLen < length; st.blockLen *= 2);
        st.bufLen = st.blockLen * 2;
        st.effMem = st.bufLen;
        st.cache.assign(static_cast<size_t>(st.bufLen), T());

        ptrdiff_t effStart = start + st.bufLen;
        kotaInPlaceMergeSort2(data_, st, start, effStart);
        for (ptrdiff_t i = 0; i < st.bufLen; ++i) st.cache[i] = data_[start + i];

        if (st.bufLen < length / 4) {
            st.tagLen = length / st.blockLen;

            st.tags.assign(static_cast<size_t>(st.tagLen), T());
            for (ptrdiff_t i = 0; i < st.tagLen; ++i)
                st.tags[i] = static_cast<T>(static_cast<int>(i));
        }

        bool bw = kotaIterator(data_, st, start, end, true);
        kotaMergeExtBuf(data_, st, start, end, bw);

        st.cache.clear();
        st.tags.clear();
    }

    // KotaSorting.kotaSortDynamicBuf
    template<class T>
    void kotaSortDynamicBufImpl(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t start, ptrdiff_t end) {
        ptrdiff_t length = end - start;
        if (length <= 16) {
            kotaInPlaceMergeSort2(data_, st, start, end);
            return;
        }

        st.ext = true;
        st.bufPos = start;
        for (st.blockLen = 1; st.blockLen * st.blockLen < length; st.blockLen *= 2);
        st.bufLen = st.blockLen * 2;
        st.effMem = st.bufLen;

        if (st.bufLen < length / 4) {
            ptrdiff_t ideal = length / st.blockLen;
            st.tagLen = kotaFindKeys(data_, st, start, end, ideal);

            if (st.tagLen < ideal) {
                if (st.tagLen <= 16) {
                    kotaInPlaceMergeSort2(data_, st, start, end);
                    return;
                }
                else {
                    // phase 2 (Java prints "phase 2" here before falling back)
                    kotaInPlaceMergeSort2(data_, st, start, end);
                    return;
                }
            }
        }
        else st.tagLen = 0;

        st.cache.assign(static_cast<size_t>(st.bufLen), T());

        ptrdiff_t bufStart = start + st.tagLen;
        ptrdiff_t effStart = bufStart + st.bufLen;

        kotaInPlaceMergeSort2(data_, st, bufStart, effStart);
        for (ptrdiff_t i = 0; i < st.bufLen; ++i) st.cache[i] = data_[bufStart + i];

        bool bw = kotaIterator(data_, st, bufStart, end, false);
        kotaMergeExtBuf(data_, st, bufStart, end, bw);
        kotaInPlaceMerge(data_, st, start, bufStart, end);

        st.cache.clear();
    }

    // KotaSorting.kotaSortStaticBuf
    template<class T>
    void kotaSortStaticBufImpl(std::vector<T>& data_, KotaState<T>& st, ptrdiff_t start, ptrdiff_t end) {
        if (KotaCacheSize < 4) {
            kotaSortImpl(data_, st, start, end);
            return;
        }

        ptrdiff_t length = end - start;
        if (length <= 16) {
            kotaInPlaceMergeSort2(data_, st, start, end);
            return;
        }

        st.ext = true;
        st.bufPos = start;
        for (st.blockLen = 1; st.blockLen * st.blockLen < length; st.blockLen *= 2);
        st.bufLen = st.blockLen * 2;
        st.effMem = (std::min)(KotaCacheSize, st.bufLen);

        if (st.effMem == st.bufLen) {
            kotaSortDynamicBufImpl(data_, st, start, end);
            return;
        }

        ptrdiff_t ideal = st.blockLen * 2;
        st.bufLen = kotaFindKeys(data_, st, start, end, ideal);

        if (st.bufLen < ideal) {
            if (st.bufLen == 1) {
                return;
            }
            else if (st.bufLen <= 16) {
                kotaInPlaceMergeSort2(data_, st, start, end);
                return;
            }
            else {
                // phase 2 (Java prints "phase 2" here before falling back)
                kotaInPlaceMergeSort2(data_, st, start, end);
                return;
            }
        }

        if (st.bufLen < length / 4) {
            ideal = length / st.blockLen;
            st.tagLen = kotaFindKeys(data_, st, start + st.bufLen, end, ideal);

            if (st.tagLen < ideal) {
                if (st.tagLen <= 16) {
                    kotaInPlaceMergeSort2(data_, st, start, end);
                    return;
                }
                else {
                    // phase 2 (Java prints "phase 2" here before falling back)
                    kotaInPlaceMergeSort2(data_, st, start, end);
                    return;
                }
            }
        }
        else st.tagLen = 0;

        ptrdiff_t bufStart = start + st.tagLen;
        ptrdiff_t effStart = bufStart + st.bufLen;
        ptrdiff_t bufEnd = start + st.bufLen;
        kotaShift(data_, st, start, bufEnd, effStart, false, false);

        st.cache.assign(static_cast<size_t>(st.effMem), T());
        for (ptrdiff_t i = 0; i < st.effMem; ++i) st.cache[i] = data_[effStart - st.effMem + i];

        bool bw = kotaIterator(data_, st, bufStart, end, false);

        if (bw) {
            ptrdiff_t endStart = end - st.bufLen;
            kotaMultiSwap(data_, st, start, endStart, st.tagLen, false);
            kotaMergeWithBufStatic(data_, st, start, bufStart, endStart, endStart, false);
            kotaInPlaceMergeSort2(data_, st, endStart, end);

            ptrdiff_t mid = endStart + st.blockLen;
            ptrdiff_t pos = kotaBinarySearch(data_, start, endStart, data_[mid - 1], true);
            kotaRotate(data_, st, pos, endStart, mid);
            pos += st.blockLen;

            kotaMultiSwapBW(data_, st, end - 1, pos - 1, st.blockLen, false);
            kotaMergeWithBufStatic(data_, st, start, pos - st.blockLen, pos, mid, true);
            kotaInPlaceMergeSort2(data_, st, mid, end);
            kotaInPlaceMergeBW(data_, st, pos, mid, end);
        }
        else {
            kotaMergeWithBufStatic(data_, st, bufEnd, effStart, end, start, false);
            kotaInPlaceMergeSort2(data_, st, start, bufEnd);

            ptrdiff_t mid = start + st.blockLen;
            ptrdiff_t pos = kotaBinarySearch(data_, bufEnd, end, data_[mid], true);
            kotaRotate(data_, st, mid, bufEnd, pos);
            pos -= st.blockLen;

            kotaMultiSwap(data_, st, start, pos, st.blockLen, false);
            kotaMergeWithBufStatic(data_, st, pos, pos + st.blockLen, end, start, false);
            kotaInPlaceMergeSort2(data_, st, start, mid);
            kotaInPlaceMerge(data_, st, start, mid, pos);
        }

        st.cache.clear();
    }

    // KotaSorting entry points (kotaSort / ectaSort / kotaSortDynamicBuf / kotaSortStaticBuf).
    template<class T>
    void kotaSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        KotaState<T> st;
        kotaSortImpl(data_, st, start, end);
    }

    template<class T>
    void ectaSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        KotaState<T> st;
        ectaSortImpl(data_, st, start, end);
    }

    template<class T>
    void kotaSortDynamicBuf(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        KotaState<T> st;
        kotaSortDynamicBufImpl(data_, st, start, end);
    }

    template<class T>
    void kotaSortStaticBuf(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        KotaState<T> st;
        kotaSortStaticBufImpl(data_, st, start, end);
    }

    // === SECTION 3 END ===

} // namespace NVisualSort::NSortAlgorithms::NSortHelpers
