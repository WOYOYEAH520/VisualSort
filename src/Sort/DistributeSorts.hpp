#pragma once
// Ports of the ArrayV (Java, MIT) "distribute" category sorting classes,
// https://github.com/Gaming32/ArrayV  (sources: src/Sort/DistributeSorts.java counterparts).
// VisualSort - distribution sort family (36 classes of the ArrayV "Distribution Sorts" package).
//
// ASCII ONLY: do not put non-ASCII characters in this file. MSVC parses sources in the
// system code page (GBK); use \uXXXX escapes if a wide string is ever needed.
//
// This header is #included from Sort.h after SortHelpers.h; it opens the shared namespace
// and closes it again at the end of the file.
//
// Naming: the Java classes BogoSort and CountingSort already exist as C++ sorts in
// NSortAlgorithms, so they are ported as BogoSortJava / CountingSortJava.
// Every internal helper starts with the PascalCase name of the algorithm it belongs to.
//
// Randomness: the bogo/guess family draws exactly one std::mt19937 engine at the top of the
// function from NSortHelpers::SortRandomEngine::GetSortRandom<T>() (the Java code uses
// ThreadLocalRandom / BogoSort.randInt) and passes it by reference to the NSortHelpers
// bogo* utilities and to the sortedness predicates that shuffle.
//
// Value domains: the distribution ports follow the Java code, which indexes registers with
// the element values themselves. Sorts whose Java code reads out of bounds for negative or
// too large values carry an int only guard (if constexpr (std::is_same_v<T, int>)) that
// throws WideError with a Chinese message, in the style of the existing BeadSort /
// CountingSort range checks. The int phase always runs first (VisualSort::RunIntSort) and
// aborts the whole run, so the Counter/Strip phases can never see values that would make
// those tables out of bounds.
#include "SortHelpers.hpp"
#include <atomic>
#include <chrono>
#include <cmath>
#include <limits>
#include <mutex>
#include <random>
#include <thread>
#include <type_traits>
#include <vector>

namespace NVisualSort::NSortAlgorithms {

    // AmericanFlagSort.runSort + sort (NUMBER_OF_BUCKETS = 128, the Java class field; ArrayV's
    // run dialog overrides it with the bucket count it asked for).
    // getMaxNumberOfDigits uses log(value), so negative values are not supported.
    template<class T = int>
    void AmericanFlagSort(std::vector<T>& data_) {
        constexpr ptrdiff_t numberOfBuckets = 128;  // Java: private int NUMBER_OF_BUCKETS = 128

        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        if constexpr (std::is_same_v<T, int>) {
            for (ptrdiff_t i = 0; i < length; ++i) {
                if (static_cast<int>(data_[i]) < 0) {
                    // "American Flag Sort does not support negative values!"
                    throw WideError(L"\u7f8e\u5f0f\u56fd\u65d7\u6392\u5e8f\u4e0d\u652f\u6301\u8d1f\u6570\uff01");
                }
            }
        }

        auto getDigit = [](int integer, ptrdiff_t divisor) -> ptrdiff_t {
            // Java: (integer / divisor) % this.NUMBER_OF_BUCKETS
            return static_cast<ptrdiff_t>(integer / static_cast<int>(divisor)) % numberOfBuckets;
        };

        // getMaxNumberOfDigits: maximum of (int) (log(value) / log(buckets)) + 1.
        int numberOfDigits = (std::numeric_limits<int>::min)();
        for (ptrdiff_t i = 0; i < length; ++i) {
            int value = static_cast<int>(data_[i]);
            int temp;
            if (value == 0) {
                // Java: (int) (Math.log(0) / Math.log(buckets)) is -infinity, and
                // (int) (-infinity) == Integer.MIN_VALUE, so temp becomes MIN_VALUE + 1.
                temp = (std::numeric_limits<int>::min)() + 1;
            }
            else {
                temp = static_cast<int>(std::log(static_cast<double>(value))
                                        / std::log(static_cast<double>(numberOfBuckets))) + 1;
            }
            if (temp > numberOfDigits) {
                numberOfDigits = temp;
            }
        }

        ptrdiff_t max = 1;
        for (ptrdiff_t i = 0; i < static_cast<ptrdiff_t>(numberOfDigits) - 1; ++i) {
            max *= numberOfBuckets;
        }

        // sort(array, start, length, divisor): counting pass, cycle-permute into place and
        // recurse into each bucket with divisor / NUMBER_OF_BUCKETS.
        auto sortRange = [&](auto&& self, ptrdiff_t start, ptrdiff_t end, ptrdiff_t divisor) -> void {
            std::vector<ptrdiff_t> count(static_cast<size_t>(numberOfBuckets), 0);
            std::vector<ptrdiff_t> offset(static_cast<size_t>(numberOfBuckets), 0);

            for (ptrdiff_t i = start; i < end; ++i) {
                ++count[getDigit(static_cast<int>(data_[i]), divisor)];
            }

            offset[0] = start;
            for (ptrdiff_t i = 1; i < numberOfBuckets; ++i) {
                offset[i] = count[i - 1] + offset[i - 1];
            }

            for (ptrdiff_t b = 0; b < numberOfBuckets; ++b) {
                while (count[b] > 0) {
                    ptrdiff_t origin = offset[b];
                    ptrdiff_t from = origin;
                    int num = static_cast<int>(data_[from]);
                    data_[from] = -1;  // Writes.visualClear
                    do {
                        ptrdiff_t digit = getDigit(num, divisor);
                        ptrdiff_t to = offset[digit];
                        ++offset[digit];
                        --count[digit];
                        int temp = static_cast<int>(data_[to]);
                        data_[to] = num;
                        num = temp;
                        from = to;
                    } while (from != origin);
                }
            }

            if (divisor > 1) {
                for (ptrdiff_t i = 0; i < numberOfBuckets; ++i) {
                    ptrdiff_t begin = (i > 0) ? offset[i - 1] : start;
                    ptrdiff_t finish = offset[i];
                    if (finish - begin > 1) {
                        self(self, begin, finish, divisor / numberOfBuckets);
                    }
                }
            }
        };

        sortRange(sortRange, 0, length, max);
    }

    // BinaryQuickSortIterative.runSort: Reads.analyzeBit + the queue driven binary quicksort.
    // The bits are read from the values themselves, so negative values are not supported
    // (Java's analyzeBit would not terminate on them).
    template<class T = int>
    void BinaryQuickSortIterative(std::vector<T>& data_) {
        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());
        if (sortLength < 2) {
            return;
        }

        if constexpr (std::is_same_v<T, int>) {
            for (ptrdiff_t i = 0; i < sortLength; ++i) {
                if (static_cast<int>(data_[i]) < 0) {
                    // "Binary quicksort does not support negative values!"
                    throw WideError(L"\u4e8c\u5206\u5feb\u901f\u6392\u5e8f\u4e0d\u652f\u6301\u8d1f\u6570\uff01");
                }
            }
        }

        NSortHelpers::binaryQuickSort(data_, 0, sortLength - 1);
    }

    // BinaryQuickSortRecursive.runSort: Reads.analyzeBit + the recursive binary quicksort.
    template<class T = int>
    void BinaryQuickSortRecursive(std::vector<T>& data_) {
        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());
        if (sortLength < 2) {
            return;
        }

        if constexpr (std::is_same_v<T, int>) {
            for (ptrdiff_t i = 0; i < sortLength; ++i) {
                if (static_cast<int>(data_[i]) < 0) {
                    // "Binary quicksort does not support negative values!"
                    throw WideError(L"\u4e8c\u5206\u5feb\u901f\u6392\u5e8f\u4e0d\u652f\u6301\u8d1f\u6570\uff01");
                }
            }
        }

        ptrdiff_t mostSignificantBit = NSortHelpers::binaryQuickSortHighBit(data_, 0, sortLength);
        NSortHelpers::binaryQuickSortRecursive(data_, 0, sortLength - 1, mostSignificantBit);
    }

    // BogoBogoSort.runSort + bogoBogo / bogoBogoIsSorted. The Java int[][] tmp holds one
    // buffer per length (tmp[i - 2] has length i); the recursion keeps the Java shape.
    template<class T = int>
    void BogoBogoSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());

        std::vector<std::vector<T>> tmp(static_cast<size_t>(length - 1));
        for (ptrdiff_t i = length; i > 1; --i) {
            tmp[static_cast<size_t>(i - 2)].assign(static_cast<size_t>(i), T{});
        }

        struct BogoBogoState {
            std::vector<std::vector<T>>& tmp;
            std::mt19937& engine;

            bool isSorted(std::vector<T>& array, ptrdiff_t len) {
                if (len == 1) {
                    return true;
                }
                ptrdiff_t idx = len - 2;
                for (ptrdiff_t i = 0; i < len; ++i) {
                    tmp[static_cast<size_t>(idx)][static_cast<size_t>(i)] = array[static_cast<size_t>(i)];
                }
                bogoBogo(tmp[static_cast<size_t>(idx)], len - 1);
                while (NSortHelpers::CompareValues(tmp[static_cast<size_t>(idx)][static_cast<size_t>(len - 2)],
                                                    tmp[static_cast<size_t>(idx)][static_cast<size_t>(len - 1)]) > 0) {
                    NSortHelpers::bogoSwap(tmp[static_cast<size_t>(idx)], 0, len, engine);
                    bogoBogo(tmp[static_cast<size_t>(idx)], len - 1);
                }
                for (ptrdiff_t i = 0; i < len; ++i) {
                    if (NSortHelpers::CompareValues(array[static_cast<size_t>(i)],
                                                    tmp[static_cast<size_t>(idx)][static_cast<size_t>(i)]) != 0) {
                        return false;
                    }
                }
                return true;
            }

            void bogoBogo(std::vector<T>& array, ptrdiff_t len) {
                while (!isSorted(array, len)) {
                    NSortHelpers::bogoSwap(array, 0, len, engine);
                }
            }
        };

        BogoBogoState state{ tmp, engine };
        state.bogoBogo(data_, length);
    }

    // BogoSort.runSort (the plain ArrayV bogosort; the classic C++ BogoSort already exists,
    // hence the Java suffix).
    template<class T = int>
    void BogoSortJava(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());
        while (!NSortHelpers::isArraySorted(data_, length)) {
            NSortHelpers::bogoSwap(data_, 0, length, engine);
        }
    }

    // BozoSort.runSort: swap two random positions until the array is sorted.
    template<class T = int>
    void BozoSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());
        using std::swap;
        while (!NSortHelpers::isArraySorted(data_, length)) {
            ptrdiff_t i = NSortHelpers::bogoRandInt(engine, 0, length);
            ptrdiff_t j = NSortHelpers::bogoRandInt(engine, 0, length);
            swap(data_[i], data_[j]);
        }
    }

    // ClassicGravitySort.runSort: Reads.analyzeMax counts beads per column, then rebuilds the
    // array from the right. transpose has max entries, so values must be non negative and the
    // value range has to stay small (same guard as the existing BeadSort).
    template<class T = int>
    void ClassicGravitySort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        int maxValue = static_cast<int>(data_[0]);
        for (ptrdiff_t i = 1; i < length; ++i) {
            if (static_cast<int>(data_[i]) > maxValue) {
                maxValue = static_cast<int>(data_[i]);
            }
        }

        if constexpr (std::is_same_v<T, int>) {
            if (maxValue < 0) {
                // "Classic Gravity Sort does not support negative values!"
                throw WideError(L"\u7ecf\u5178\u91cd\u529b\u6392\u5e8f\u4e0d\u652f\u6301\u8d1f\u6570\uff01");
            }
            if (static_cast<long long>(maxValue) > 10000000) {
                // "data value range is too large for Classic Gravity Sort"
                throw WideError(L"\u8be5\u6570\u636e\u6700\u5c0f\u503c\u4e0e\u6700\u5927\u503c\u5dee\u8ddd\u8fc7\u5927\uff0c\u4e0d\u9002\u5408\u4f7f\u7528\u7ecf\u5178\u91cd\u529b\u6392\u5e8f\uff01");
            }
        }

        std::vector<ptrdiff_t> transpose(static_cast<size_t>(maxValue), 0);

        for (ptrdiff_t i = 0; i < length; ++i) {
            int num = static_cast<int>(data_[i]);
            for (int j = 0; j < num; ++j) {
                ++transpose[static_cast<size_t>(j)];
            }
        }

        for (ptrdiff_t i = 0; i < length; ++i) {
            ptrdiff_t sum = 0;
            for (ptrdiff_t j = 0; j < maxValue; ++j) {
                if (transpose[static_cast<size_t>(j)] > 0) {
                    ++sum;
                }
            }
            data_[length - i - 1] = static_cast<int>(sum);
            for (ptrdiff_t j = 0; j < maxValue; ++j) {
                --transpose[static_cast<size_t>(j)];
            }
        }
    }

    // CocktailBogoSort.runSort: drop the first and last remaining elements whenever they are
    // already in place, otherwise shuffle the remaining range.
    template<class T = int>
    void CocktailBogoSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());

        ptrdiff_t min = 0;
        ptrdiff_t max = length;
        while (min < max - 1) {
            if (NSortHelpers::isMinSorted(data_, min, max)) {
                ++min;
                continue;
            }
            if (NSortHelpers::isMaxSorted(data_, min, max)) {
                --max;
                continue;
            }
            NSortHelpers::bogoSwap(data_, min, max, engine);
        }
    }

    // CountingSort.runSort (the classic C++ CountingSort already exists, hence the Java
    // suffix). counts has max + 1 entries, so negative values do not work in Java either
    // (ArrayIndexOutOfBoundsException) and the value range has to stay small.
    template<class T = int>
    void CountingSortJava(std::vector<T>& data_) {
        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());
        if (sortLength < 2) {
            return;
        }

        // Reads.analyzeMax(array, sortLength, 0, false)
        int maxValue = static_cast<int>(data_[0]);
        for (ptrdiff_t i = 1; i < sortLength; ++i) {
            if (static_cast<int>(data_[i]) > maxValue) {
                maxValue = static_cast<int>(data_[i]);
            }
        }

        if constexpr (std::is_same_v<T, int>) {
            if (maxValue < 0) {
                // "Counting Sort does not support negative values!"
                throw WideError(L"\u8ba1\u6570\u6392\u5e8f\u4e0d\u652f\u6301\u8d1f\u6570\uff01");
            }
            if (static_cast<long long>(maxValue) + 1 > 10000000) {
                // "data value range is too large for Counting Sort"
                throw WideError(L"\u8be5\u6570\u636e\u6700\u5c0f\u503c\u4e0e\u6700\u5927\u503c\u5dee\u8ddd\u8fc7\u5927\uff0c\u4e0d\u9002\u5408\u4f7f\u7528\u8ba1\u6570\u6392\u5e8f\uff01");
            }
        }

        std::vector<T> output(data_);  // Writes.copyOfArray(array, sortLength)
        std::vector<ptrdiff_t> counts(static_cast<size_t>(maxValue) + 1, 0);

        for (ptrdiff_t i = 0; i < sortLength; ++i) {
            ++counts[static_cast<size_t>(static_cast<int>(data_[i]))];
        }
        for (ptrdiff_t i = 1; i < static_cast<ptrdiff_t>(counts.size()); ++i) {
            counts[static_cast<size_t>(i)] += counts[static_cast<size_t>(i - 1)];
        }
        for (ptrdiff_t i = sortLength - 1; i >= 0; --i) {
            output[static_cast<size_t>(counts[static_cast<size_t>(static_cast<int>(data_[i]))] - 1)] = data_[static_cast<size_t>(i)];
            --counts[static_cast<size_t>(static_cast<int>(data_[i]))];
        }
        // Extra loop to simulate the results from the "output" array being written to the
        // visual array.
        for (ptrdiff_t i = sortLength - 1; i >= 0; --i) {
            data_[static_cast<size_t>(i)] = output[static_cast<size_t>(i)];
        }
    }

    // DeterministicBogoSort.permutationSort: walks the permutations of the array (Heap's
    // algorithm shape) until the array is sorted. Recursion depth is the array length.
    template<class T = int>
    void DeterministicBogoSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        using std::swap;
        auto permutationSort = [&](auto&& self, ptrdiff_t depth, ptrdiff_t len) -> bool {
            if (depth >= len - 1) {
                return NSortHelpers::isArraySorted(data_, len);
            }
            for (ptrdiff_t i = len - 1; i > depth; --i) {
                if (self(self, depth + 1, len)) {
                    return true;
                }
                if ((len - depth) % 2 == 0) {
                    swap(data_[depth], data_[i]);
                }
                else {
                    swap(data_[depth], data_[len - 1]);
                }
            }
            return self(self, depth + 1, len);
        };

        permutationSort(permutationSort, 0, length);
    }

    // FlashSort.runSort (Neubert's flashsort, ported from the Java refactoring). The class
    // recursion works on a copy whose result is thrown away (the Java code does the same: the
    // final insertion sort is what produces the sorted array).
    template<class T = int>
    void FlashSort(std::vector<T>& data_) {
        if (data_.empty()) {
            return;
        }

        auto flashRange = [&](auto&& self, std::vector<T>& array) -> void {
            ptrdiff_t sortLength = static_cast<ptrdiff_t>(array.size());
            if (sortLength == 0) {
                return;
            }

            // 20% of the number of elements or 0.2n classes are used (at least 2).
            ptrdiff_t m = static_cast<ptrdiff_t>(0.2 * static_cast<double>(sortLength)) + 2;

            // -------CLASS FORMATION-------
            int minValue = static_cast<int>(array[0]);
            int maxValue = minValue;
            ptrdiff_t maxIndex = 0;
            for (ptrdiff_t i = 1; i < sortLength - 1; i += 2) {
                int smallValue;
                int big;
                ptrdiff_t bigIndex;
                if (NSortHelpers::CompareValues(array[static_cast<size_t>(i)], array[static_cast<size_t>(i + 1)]) == -1) {
                    smallValue = static_cast<int>(array[static_cast<size_t>(i)]);
                    big = static_cast<int>(array[static_cast<size_t>(i + 1)]);
                    bigIndex = i + 1;
                }
                else {
                    big = static_cast<int>(array[static_cast<size_t>(i)]);
                    bigIndex = i;
                    smallValue = static_cast<int>(array[static_cast<size_t>(i + 1)]);
                }
                if (big > maxValue) {
                    maxValue = big;
                    maxIndex = bigIndex;
                }
                if (smallValue < minValue) {
                    minValue = smallValue;
                }
            }
            // do the last element
            int lastValue = static_cast<int>(array[static_cast<size_t>(sortLength - 1)]);
            if (lastValue < minValue) {
                minValue = lastValue;
            }
            else if (lastValue > maxValue) {
                maxValue = lastValue;
                maxIndex = sortLength - 1;
            }

            if (maxValue == minValue) {
                // all the elements are the same
                return;
            }

            std::vector<ptrdiff_t> L(static_cast<size_t>(m) + 1, 0);  // L[0] is unused
            // K(A(i)) = 1 + INT((m-1)(A(i)-Amin)/(Amax-Amin))
            double c = (static_cast<double>(m) - 1.0) / (maxValue - minValue);
            ptrdiff_t K;
            for (ptrdiff_t h = 0; h < sortLength; ++h) {
                K = static_cast<ptrdiff_t>(static_cast<double>(static_cast<int>(array[static_cast<size_t>(h)]) - minValue) * c) + 1;
                ++L[static_cast<size_t>(K)];
            }
            for (K = 2; K <= m; ++K) {
                L[static_cast<size_t>(K)] += L[static_cast<size_t>(K - 1)];
            }

            // -------PERMUTATION-------
            using std::swap;
            swap(array[static_cast<size_t>(maxIndex)], array[0]);
            ptrdiff_t j = 0;
            K = m;
            ptrdiff_t numMoves = 0;
            while (numMoves < sortLength) {
                while (j >= L[static_cast<size_t>(K)]) {
                    ++j;
                    K = static_cast<ptrdiff_t>(static_cast<double>(static_cast<int>(array[static_cast<size_t>(j)]) - minValue) * c) + 1;
                }
                int evicted = static_cast<int>(array[static_cast<size_t>(j)]);
                while (j < L[static_cast<size_t>(K)]) {
                    K = static_cast<ptrdiff_t>(static_cast<double>(evicted - minValue) * c) + 1;
                    ptrdiff_t location = L[static_cast<size_t>(K)] - 1;
                    int temp = static_cast<int>(array[static_cast<size_t>(location)]);
                    array[static_cast<size_t>(location)] = evicted;
                    evicted = temp;
                    --L[static_cast<size_t>(K)];
                    ++numMoves;
                }
            }

            // -------RECURSION or STRAIGHT INSERTION-------
            ptrdiff_t threshold = static_cast<ptrdiff_t>(1.25 * (static_cast<double>(sortLength / m) + 1.0));
            constexpr ptrdiff_t minElements = 30;
            for (K = m - 1; K >= 1; --K) {
                ptrdiff_t classSize = L[static_cast<size_t>(K + 1)] - L[static_cast<size_t>(K)];
                if (classSize > threshold && classSize > minElements) {
                    // Java: runSort(Arrays.copyOfRange(array, L[K], L[K + 1]), classSize, 0)
                    std::vector<T> copiedRange(array.begin() + L[static_cast<size_t>(K)],
                                               array.begin() + L[static_cast<size_t>(K + 1)]);
                    self(self, copiedRange);
                }
            }

            NSortHelpers::insertionSort(array, 0, sortLength);
        };

        flashRange(flashRange, data_);
    }

    // GravitySort.runSort: bead counts per value survive a partial backwards sum, then every
    // element is shifted by one for each value level. Needs max - min + 1 counters.
    template<class T = int>
    void GravitySort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        int minValue = static_cast<int>(data_[0]);
        int maxValue = minValue;
        for (ptrdiff_t i = 1; i < length; ++i) {
            if (static_cast<int>(data_[i]) < minValue) {
                minValue = static_cast<int>(data_[i]);
            }
            if (static_cast<int>(data_[i]) > maxValue) {
                maxValue = static_cast<int>(data_[i]);
            }
        }

        if constexpr (std::is_same_v<T, int>) {
            if (static_cast<long long>(maxValue) - minValue + 1 > 10000000) {
                // "data value range is too large for Gravity Sort"
                throw WideError(L"\u8be5\u6570\u636e\u6700\u5c0f\u503c\u4e0e\u6700\u5927\u503c\u5dee\u8ddd\u8fc7\u5927\uff0c\u4e0d\u9002\u5408\u4f7f\u7528\u91cd\u529b\u6392\u5e8f\uff01");
            }
        }

        std::vector<T> x(static_cast<size_t>(length));
        std::vector<ptrdiff_t> y(static_cast<size_t>(maxValue - minValue + 1), 0);

        // save a copy of array-min in x, increase count of the array-min value in y
        for (ptrdiff_t i = 0; i < length; ++i) {
            x[static_cast<size_t>(i)] = static_cast<int>(data_[i]) - minValue;
            ++y[static_cast<size_t>(static_cast<int>(data_[i]) - minValue)];
        }
        // do a partial sum backwards to determine how many elements are greater than a value
        for (ptrdiff_t i = static_cast<ptrdiff_t>(y.size()) - 1; i > 0; --i) {
            y[static_cast<size_t>(i - 1)] += y[static_cast<size_t>(i)];
        }
        // iterate for every integer value in the array range
        for (ptrdiff_t j = static_cast<ptrdiff_t>(y.size()) - 1; j >= 0; --j) {
            for (ptrdiff_t i = 0; i < length; ++i) {
                int inc = (i >= length - y[static_cast<size_t>(j)] ? 1 : 0)
                        - (static_cast<int>(x[static_cast<size_t>(i)]) >= j ? 1 : 0);
                data_[static_cast<size_t>(i)] = static_cast<int>(data_[static_cast<size_t>(i)]) + inc;
            }
        }
    }

    // GuessSort.runSort: intentionally unoptimized; iterates every n-tuple of indices, keeps
    // the last tuple that is a permutation and sorts the array through it.
    template<class T = int>
    void GuessSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::vector<ptrdiff_t> loops(static_cast<size_t>(length), 0);
        std::vector<ptrdiff_t> indexes(static_cast<size_t>(length), 0);

        while (true) {
            // check to make sure the indexes are unique
            ptrdiff_t total = 0;
            for (ptrdiff_t i = 0; i < length; ++i) {
                for (ptrdiff_t j = 0; j < length; ++j) {
                    if (loops[static_cast<size_t>(i)] == loops[static_cast<size_t>(j)]) {
                        ++total;
                    }
                }
            }
            // check to make sure the resulting array is sorted
            for (ptrdiff_t i = 0; i < length; ++i) {
                for (ptrdiff_t j = 0; j < length; ++j) {
                    if (NSortHelpers::CompareValues(data_[static_cast<size_t>(loops[static_cast<size_t>(i)])],
                                                    data_[static_cast<size_t>(loops[static_cast<size_t>(j)])]) == (i < j ? 1 : -1)) {
                        ++total;
                    }
                }
            }
            if (total == length) {
                for (ptrdiff_t i = 0; i < length; ++i) {
                    indexes[static_cast<size_t>(i)] = loops[static_cast<size_t>(i)];
                }
            }
            // progress the loops
            ptrdiff_t pos = 0;
            while (pos < length) {
                if (loops[static_cast<size_t>(pos)] < length - 1) {
                    ++loops[static_cast<size_t>(pos)];
                    break;
                }
                else {
                    loops[static_cast<size_t>(pos)] = 0;
                    ++pos;
                }
            }
            if (pos == length) {
                break;
            }
        }

        // write the indexes to the array
        std::vector<T> aux(data_);
        for (ptrdiff_t i = 0; i < length; ++i) {
            data_[static_cast<size_t>(i)] = aux[static_cast<size_t>(indexes[static_cast<size_t>(i)])];
        }
    }

    // InPlaceLSDRadixSort.runSort: in-place LSD radix pass per power, using Writes.multiSwap
    // (a rotation that moves the element at pos into its bucket and shifts the elements in
    // between) and the per bucket end pointers vregs.
    // Base 4, like the other radix sorts of this category: Reads.analyzeMaxLog truncates
    // (int) (log(1000) / log(10)) to 2, which would leave "Base 10" data unsorted at n = 1000.
    template<class T = int>
    void InPlaceLSDRadixSort(std::vector<T>& data_) {
        constexpr ptrdiff_t base = 4;

        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());
        if (sortLength < 2) {
            return;
        }

        if constexpr (std::is_same_v<T, int>) {
            for (ptrdiff_t i = 0; i < sortLength; ++i) {
                if (static_cast<int>(data_[i]) < 0) {
                    // "Radix sort does not support negative values!"
                    throw WideError(L"\u57fa\u6570\u6392\u5e8f\u4e0d\u652f\u6301\u8d1f\u6570\uff01");
                }
            }
        }

        auto getDigit = [](int value, ptrdiff_t p) -> ptrdiff_t {
            // Java: Reads.getDigit(value, p, base)
            ptrdiff_t divisor = 1;
            for (ptrdiff_t i = 0; i < p; ++i) {
                divisor *= base;
            }
            return static_cast<ptrdiff_t>(value / static_cast<int>(divisor)) % base;
        };

        // Reads.analyzeMaxLog(array, sortLength, base, 0.5, true)
        int maxValue = static_cast<int>(data_[0]);
        for (ptrdiff_t i = 1; i < sortLength; ++i) {
            if (static_cast<int>(data_[i]) > maxValue) {
                maxValue = static_cast<int>(data_[i]);
            }
        }
        ptrdiff_t maxpower = -1;
        if (maxValue > 0) {
            maxpower = static_cast<ptrdiff_t>(std::log(static_cast<double>(maxValue))
                                              / std::log(static_cast<double>(base)));
        }

        // Writes.multiSwap(array, x, y): move the element at x to y, shifting the elements in
        // between one position towards x (Java implements it as a chain of adjacent swaps).
        auto multiSwap = [&](ptrdiff_t x, ptrdiff_t y) {
            using std::swap;
            if (x == y) {
                return;
            }
            if (x < y) {
                for (ptrdiff_t i = x; i < y; ++i) {
                    swap(data_[static_cast<size_t>(i)], data_[static_cast<size_t>(i + 1)]);
                }
            }
            else {
                for (ptrdiff_t i = x; i > y; --i) {
                    swap(data_[static_cast<size_t>(i)], data_[static_cast<size_t>(i - 1)]);
                }
            }
        };

        std::vector<ptrdiff_t> vregs(static_cast<size_t>(base) - 1, 0);
        for (ptrdiff_t p = 0; p <= maxpower; ++p) {
            for (size_t i = 0; i < vregs.size(); ++i) {
                vregs[i] = sortLength - 1;
            }
            ptrdiff_t pos = 0;
            for (ptrdiff_t i = 0; i < sortLength; ++i) {
                ptrdiff_t digit = getDigit(static_cast<int>(data_[static_cast<size_t>(pos)]), p);
                if (digit == 0) {
                    ++pos;
                }
                else {
                    multiSwap(pos, vregs[static_cast<size_t>(digit - 1)]);
                    for (ptrdiff_t j = digit - 1; j > 0; --j) {
                        --vregs[static_cast<size_t>(j - 1)];
                    }
                }
            }
        }
    }

    // IndexSort.runSort: "Simple Static Sort". Every element is swapped to the position given
    // by its (min-based) value, so the data has to be a run of consecutive integers.
    template<class T = int>
    void IndexSort(std::vector<T>& data_) {
        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());
        if (sortLength < 2) {
            return;
        }

        // Reads.analyzeMin(array, sortLength, 0.5, true)
        int minValue = static_cast<int>(data_[0]);
        for (ptrdiff_t i = 1; i < sortLength; ++i) {
            if (static_cast<int>(data_[i]) < minValue) {
                minValue = static_cast<int>(data_[i]);
            }
        }

        using std::swap;
        for (ptrdiff_t i = 0; i < sortLength; ++i) {
            ptrdiff_t cmpCount = 0;
            ptrdiff_t target = static_cast<ptrdiff_t>(static_cast<int>(data_[static_cast<size_t>(i)]) - minValue);
            while (target != i && cmpCount < sortLength) {
                if (target < 0 || target >= sortLength) {
                    // "Index Sort needs the data to be a run of consecutive integers"
                    throw WideError(L"\u7d22\u5f15\u6392\u5e8f\u8981\u6c42\u6570\u636e\u4e3a\u4ece\u6700\u5c0f\u503c\u5f00\u59cb\u7684\u8fde\u7eed\u6574\u6570\uff01");
                }
                swap(data_[static_cast<size_t>(i)], data_[static_cast<size_t>(target)]);
                ++cmpCount;
                target = static_cast<ptrdiff_t>(static_cast<int>(data_[static_cast<size_t>(i)]) - minValue);
            }
            if (cmpCount >= sortLength - 1) {
                break;
            }
        }
    }

    // LSDRadixSort.runSort: one counting pass per digit (base 4), transcribing the registers
    // back into the array each time (Writes.fancyTranscribe).
    template<class T = int>
    void LSDRadixSort(std::vector<T>& data_) {
        constexpr ptrdiff_t base = 4;  // Java: "Least Significant Digit Radix Sort, Base 4"

        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());
        if (sortLength < 2) {
            return;
        }

        if constexpr (std::is_same_v<T, int>) {
            for (ptrdiff_t i = 0; i < sortLength; ++i) {
                if (static_cast<int>(data_[i]) < 0) {
                    // "Radix sort does not support negative values!"
                    throw WideError(L"\u57fa\u6570\u6392\u5e8f\u4e0d\u652f\u6301\u8d1f\u6570\uff01");
                }
            }
        }

        auto getDigit = [](int value, ptrdiff_t p) -> size_t {
            // Java: Reads.getDigit(value, p, base)
            ptrdiff_t divisor = 1;
            for (ptrdiff_t i = 0; i < p; ++i) {
                divisor *= base;
            }
            return static_cast<size_t>(value / static_cast<int>(divisor)) % static_cast<size_t>(base);
        };

        // Reads.analyzeMaxLog(array, sortLength, bucketCount, 0.5, true)
        int maxValue = static_cast<int>(data_[0]);
        for (ptrdiff_t i = 1; i < sortLength; ++i) {
            if (static_cast<int>(data_[i]) > maxValue) {
                maxValue = static_cast<int>(data_[i]);
            }
        }
        ptrdiff_t highestpower = -1;
        if (maxValue > 0) {
            highestpower = static_cast<ptrdiff_t>(std::log(static_cast<double>(maxValue))
                                                  / std::log(static_cast<double>(base)));
        }

        std::vector<std::vector<ptrdiff_t>> registers(static_cast<size_t>(base));
        for (ptrdiff_t p = 0; p <= highestpower; ++p) {
            for (size_t i = 0; i < registers.size(); ++i) {
                registers[i].clear();
            }
            for (ptrdiff_t i = 0; i < sortLength; ++i) {
                int value = static_cast<int>(data_[i]);
                registers[getDigit(value, p)].push_back(static_cast<ptrdiff_t>(value));
            }
            // Writes.fancyTranscribe(array, sortLength, registers, ...)
            ptrdiff_t w = 0;
            for (size_t i = 0; i < registers.size(); ++i) {
                for (size_t j = 0; j < registers[i].size(); ++j) {
                    data_[static_cast<size_t>(w)] = static_cast<int>(registers[i][j]);
                    ++w;
                }
            }
        }
    }

    // LessBogoSort.runSort: shuffle the remaining range until its first element is the
    // smallest one, then drop it.
    template<class T = int>
    void LessBogoSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());
        for (ptrdiff_t i = 0; i < length; ++i) {
            while (!NSortHelpers::isMinSorted(data_, i, length)) {
                NSortHelpers::bogoSwap(data_, i, length, engine);
            }
        }
    }

    // MSDRadixSort.runSort + radixMSD: most significant digit first, recursing into every
    // register (base 4).
    template<class T = int>
    void MSDRadixSort(std::vector<T>& data_) {
        constexpr ptrdiff_t base = 4;  // Java: "Most Significant Digit Radix Sort, Base 4"

        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());
        if (sortLength < 2) {
            return;
        }

        if constexpr (std::is_same_v<T, int>) {
            for (ptrdiff_t i = 0; i < sortLength; ++i) {
                if (static_cast<int>(data_[i]) < 0) {
                    // "Radix sort does not support negative values!"
                    throw WideError(L"\u57fa\u6570\u6392\u5e8f\u4e0d\u652f\u6301\u8d1f\u6570\uff01");
                }
            }
        }

        auto getDigit = [](int value, ptrdiff_t p) -> size_t {
            // Java: Reads.getDigit(value, p, radix)
            ptrdiff_t divisor = 1;
            for (ptrdiff_t i = 0; i < p; ++i) {
                divisor *= base;
            }
            return static_cast<size_t>(value / static_cast<int>(divisor)) % static_cast<size_t>(base);
        };

        // Reads.analyzeMaxLog(array, sortLength, bucketCount, 0.5, true)
        int maxValue = static_cast<int>(data_[0]);
        for (ptrdiff_t i = 1; i < sortLength; ++i) {
            if (static_cast<int>(data_[i]) > maxValue) {
                maxValue = static_cast<int>(data_[i]);
            }
        }
        ptrdiff_t highestpower = -1;
        if (maxValue > 0) {
            highestpower = static_cast<ptrdiff_t>(std::log(static_cast<double>(maxValue))
                                                  / std::log(static_cast<double>(base)));
        }

        // radixMSD(array, length, min, max, radix, pow)
        auto radixMSD = [&](auto&& self, ptrdiff_t min, ptrdiff_t max, ptrdiff_t pow) -> void {
            if (min >= max || pow < 0) {
                return;
            }

            std::vector<std::vector<ptrdiff_t>> registers(static_cast<size_t>(base));
            for (ptrdiff_t i = min; i < max; ++i) {
                int value = static_cast<int>(data_[i]);
                registers[getDigit(value, pow)].push_back(static_cast<ptrdiff_t>(value));
            }
            // Writes.transcribeMSD(array, registers, 0, min, ...)
            ptrdiff_t w = min;
            for (size_t i = 0; i < registers.size(); ++i) {
                for (size_t j = 0; j < registers[i].size(); ++j) {
                    data_[static_cast<size_t>(w)] = static_cast<int>(registers[i][j]);
                    ++w;
                }
            }

            ptrdiff_t sum = 0;
            for (ptrdiff_t i = 0; i < base; ++i) {
                ptrdiff_t size = static_cast<ptrdiff_t>(registers[static_cast<size_t>(i)].size());
                self(self, sum + min, sum + min + size, pow - 1);
                sum += size;
            }
        };

        radixMSD(radixMSD, 0, sortLength, highestpower);
    }

    // MedianQuickBogoSort.medianQuickBogo: shuffle the range until the middle element splits
    // it, then recurse on both halves.
    template<class T = int>
    void MedianQuickBogoSort(std::vector<T>& data_) {
        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());
        if (sortLength < 2) {
            return;
        }

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());

        auto medianQuickBogo = [&](auto&& self, ptrdiff_t start, ptrdiff_t end) -> void {
            if (start >= end - 1) {
                return;
            }
            ptrdiff_t mid = (start + end) / 2;
            while (!NSortHelpers::isRangeSplit(data_, start, mid, end)) {
                NSortHelpers::bogoSwap(data_, start, end, engine);
            }
            self(self, start, mid);
            self(self, mid, end);
        };

        medianQuickBogo(medianQuickBogo, 0, sortLength);
    }

    // MergeBogoSort.runSort + mergeBogo / bogoWeave: merge two sorted runs by randomly
    // interleaving their elements (bogoCombo picks the picks) until the merged range is
    // sorted.
    template<class T = int>
    void MergeBogoSort(std::vector<T>& data_) {
        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());
        if (sortLength < 2) {
            return;
        }

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());

        auto bogoWeave = [&](std::vector<T>& array, std::vector<T>& tmp, ptrdiff_t start, ptrdiff_t mid, ptrdiff_t end) {
            NSortHelpers::bogoCombo(array, start, end, end - mid, engine);
            ptrdiff_t low = start;
            ptrdiff_t high = mid;
            for (ptrdiff_t i = start; i < end; ++i) {
                if (array[static_cast<size_t>(i)] == 0) {
                    array[static_cast<size_t>(i)] = tmp[static_cast<size_t>(low)];
                    ++low;
                }
                else {
                    array[static_cast<size_t>(i)] = tmp[static_cast<size_t>(high)];
                    ++high;
                }
            }
        };

        auto mergeBogo = [&](auto&& self, std::vector<T>& array, std::vector<T>& tmp, ptrdiff_t start, ptrdiff_t end) -> void {
            if (start >= end - 1) {
                return;
            }
            ptrdiff_t mid = (start + end) / 2;
            self(self, array, tmp, start, mid);
            self(self, array, tmp, mid, end);
            for (ptrdiff_t i = start; i < end; ++i) {
                tmp[static_cast<size_t>(i)] = array[static_cast<size_t>(i)];
            }
            while (!NSortHelpers::isRangeSorted(array, start, end)) {
                bogoWeave(array, tmp, start, mid, end);
            }
        };

        std::vector<T> tmp(static_cast<size_t>(sortLength), T{});  // Writes.createExternalArray
        mergeBogo(mergeBogo, data_, tmp, 0, sortLength);
    }

    // OptimizedGuessSort.runSort: iterates the n-tuples of indices again, but stops as soon as
    // the guesses are a stable sorted ordering.
    template<class T = int>
    void OptimizedGuessSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::vector<ptrdiff_t> loops(static_cast<size_t>(length), 0);

        while (true) {
            // check if the array is stably sorted (doubles as duplicate-detection)
            bool sorted = true;
            for (ptrdiff_t i = 0; i < length - 1; ++i) {
                int comp = NSortHelpers::CompareValues(data_[static_cast<size_t>(loops[static_cast<size_t>(i)])],
                                                       data_[static_cast<size_t>(loops[static_cast<size_t>(i + 1)])]);
                if (comp < 0 || (comp == 0 && loops[static_cast<size_t>(i)] < loops[static_cast<size_t>(i + 1)])) {
                    continue;
                }
                sorted = false;
                break;
            }
            if (sorted) {
                break;
            }
            // progress the loops
            for (ptrdiff_t pos = 0; pos < length; ++pos) {
                if (loops[static_cast<size_t>(pos)] < length - 1) {
                    ++loops[static_cast<size_t>(pos)];
                    break;
                }
                else {
                    loops[static_cast<size_t>(pos)] = 0;
                }
            }
        }

        // write the indexes to the array
        for (ptrdiff_t i = 0; i < length; ++i) {
            loops[static_cast<size_t>(i)] = static_cast<ptrdiff_t>(static_cast<int>(data_[static_cast<size_t>(loops[static_cast<size_t>(i)])]));
        }
        for (ptrdiff_t i = 0; i < length; ++i) {
            data_[static_cast<size_t>(i)] = static_cast<int>(loops[static_cast<size_t>(i)]);
        }
    }

    // PigeonholeSort.runSort: one counter per value of the value range.
    template<class T = int>
    void PigeonholeSort(std::vector<T>& data_) {
        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());
        if (sortLength < 2) {
            return;
        }

        int minValue = static_cast<int>(data_[0]);
        int maxValue = minValue;
        for (ptrdiff_t i = 0; i < sortLength; ++i) {
            if (static_cast<int>(data_[i]) < minValue) {
                minValue = static_cast<int>(data_[i]);
            }
            if (static_cast<int>(data_[i]) > maxValue) {
                maxValue = static_cast<int>(data_[i]);
            }
        }

        ptrdiff_t size = static_cast<ptrdiff_t>(maxValue) - minValue + 1;
        if constexpr (std::is_same_v<T, int>) {
            if (static_cast<long long>(maxValue) - minValue + 1 > 10000000) {
                // "data value range is too large for Pigeonhole Sort"
                throw WideError(L"\u8be5\u6570\u636e\u6700\u5c0f\u503c\u4e0e\u6700\u5927\u503c\u5dee\u8ddd\u8fc7\u5927\uff0c\u4e0d\u9002\u5408\u4f7f\u7528\u9e3d\u5de2\u6392\u5e8f\uff01");
            }
        }

        std::vector<ptrdiff_t> holes(static_cast<size_t>(size), 0);
        for (ptrdiff_t x = 0; x < sortLength; ++x) {
            ++holes[static_cast<size_t>(static_cast<int>(data_[static_cast<size_t>(x)]) - minValue)];
        }

        ptrdiff_t j = 0;
        for (ptrdiff_t count = 0; count < size; ++count) {
            while (holes[static_cast<size_t>(count)] > 0) {
                --holes[static_cast<size_t>(count)];
                data_[static_cast<size_t>(j)] = static_cast<int>(count + minValue);
                ++j;
            }
        }
    }

    // QuickBogoSort.runSort + quickBogo: shuffle the range until it is partitioned around the
    // (moving) pivot index, then recurse on both sides.
    template<class T = int>
    void QuickBogoSort(std::vector<T>& data_) {
        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());
        if (sortLength < 2) {
            return;
        }

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());

        auto quickBogoSwap = [&](ptrdiff_t start, ptrdiff_t pivot, ptrdiff_t end) -> ptrdiff_t {
            using std::swap;
            for (ptrdiff_t i = start; i < end; ++i) {
                ptrdiff_t j = NSortHelpers::bogoRandInt(engine, i, end);
                if (pivot == i) {
                    pivot = j;
                }
                else if (pivot == j) {
                    pivot = i;
                }
                swap(data_[static_cast<size_t>(i)], data_[static_cast<size_t>(j)]);
            }
            return pivot;
        };

        auto quickBogo = [&](auto&& self, ptrdiff_t start, ptrdiff_t end) -> void {
            if (start >= end - 1) {
                return;
            }
            ptrdiff_t pivot = start;
            while (!NSortHelpers::isRangePartitioned(data_, start, pivot, end)) {
                pivot = quickBogoSwap(start, pivot, end);
            }
            self(self, start, pivot);
            self(self, pivot + 1, end);
        };

        quickBogo(quickBogo, 0, sortLength);
    }

    // RandomGuessSort.runSort: guesses random index tuples until they describe a stable sorted
    // ordering.
    template<class T = int>
    void RandomGuessSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());
        std::vector<ptrdiff_t> loops(static_cast<size_t>(length), 0);

        while (true) {
            // check if the array is stably sorted (doubles as duplicate-detection)
            bool sorted = true;
            for (ptrdiff_t i = 0; i < length - 1; ++i) {
                int comp = NSortHelpers::CompareValues(data_[static_cast<size_t>(loops[static_cast<size_t>(i)])],
                                                       data_[static_cast<size_t>(loops[static_cast<size_t>(i + 1)])]);
                if (comp < 0 || (comp == 0 && loops[static_cast<size_t>(i)] < loops[static_cast<size_t>(i + 1)])) {
                    continue;
                }
                sorted = false;
                break;
            }
            if (sorted) {
                break;
            }
            // guess
            for (ptrdiff_t pos = 0; pos < length; ++pos) {
                loops[static_cast<size_t>(pos)] = NSortHelpers::bogoRandInt(engine, 0, length);
            }
        }

        // write the indexes to the array
        for (ptrdiff_t i = 0; i < length; ++i) {
            loops[static_cast<size_t>(i)] = static_cast<ptrdiff_t>(static_cast<int>(data_[static_cast<size_t>(loops[static_cast<size_t>(i)])]));
        }
        for (ptrdiff_t i = 0; i < length; ++i) {
            data_[static_cast<size_t>(i)] = static_cast<int>(loops[static_cast<size_t>(i)]);
        }
    }

    // RotateLSDRadixSort.runSort: a rotate merge sort per digit (base 4), least significant
    // digit first.
    template<class T = int>
    void RotateLSDRadixSort(std::vector<T>& data_) {
        constexpr ptrdiff_t base = 4;  // Java: "Rotate LSD Radix Sort, Base 4"

        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        if constexpr (std::is_same_v<T, int>) {
            for (ptrdiff_t i = 0; i < length; ++i) {
                if (static_cast<int>(data_[i]) < 0) {
                    // "Radix sort does not support negative values!"
                    throw WideError(L"\u57fa\u6570\u6392\u5e8f\u4e0d\u652f\u6301\u8d1f\u6570\uff01");
                }
            }
        }

        auto getDigit = [](int value, ptrdiff_t p) -> ptrdiff_t {
            // Java: Reads.getDigit(value, p, base)
            ptrdiff_t divisor = 1;
            for (ptrdiff_t i = 0; i < p; ++i) {
                divisor *= base;
            }
            return static_cast<ptrdiff_t>(value / static_cast<int>(divisor)) % base;
        };

        auto multiSwap = [&](ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
            using std::swap;
            for (ptrdiff_t i = 0; i < len; ++i) {
                swap(data_[static_cast<size_t>(a + i)], data_[static_cast<size_t>(b + i)]);
            }
        };

        auto rotate = [&](ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
            ptrdiff_t l = m - a;
            ptrdiff_t r = b - m;
            while (l > 0 && r > 0) {
                if (r < l) {
                    multiSwap(m - r, m, r);
                    b -= r;
                    m -= r;
                    l -= r;
                }
                else {
                    multiSwap(a, m, l);
                    a += l;
                    m += l;
                    r -= l;
                }
            }
        };

        auto binSearch = [&](ptrdiff_t a, ptrdiff_t b, ptrdiff_t d, ptrdiff_t p) -> ptrdiff_t {
            while (a < b) {
                ptrdiff_t m = (a + b) / 2;
                if (getDigit(static_cast<int>(data_[static_cast<size_t>(m)]), p) >= d) {
                    b = m;
                }
                else {
                    a = m + 1;
                }
            }
            return a;
        };

        auto merge = [&](auto&& self, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t da, ptrdiff_t db, ptrdiff_t p) -> void {
            if (b - a < 2 || db - da < 2) {
                return;
            }
            ptrdiff_t dm = (da + db) / 2;
            ptrdiff_t m1 = binSearch(a, m, dm, p);
            ptrdiff_t m2 = binSearch(m, b, dm, p);
            rotate(m1, m, m2);
            m = m1 + (m2 - m);
            self(self, m, m2, b, dm, db, p);
            self(self, a, m1, m, da, dm, p);
        };

        auto mergeSort = [&](auto&& self, ptrdiff_t a, ptrdiff_t b, ptrdiff_t p) -> void {
            if (b - a < 2) {
                return;
            }
            ptrdiff_t m = (a + b) / 2;
            self(self, a, m, p);
            self(self, m, b, p);
            merge(merge, a, m, b, 0, base, p);
        };

        // Reads.analyzeMaxLog(array, length, base, 0.5, true)
        int maxValue = static_cast<int>(data_[0]);
        for (ptrdiff_t i = 1; i < length; ++i) {
            if (static_cast<int>(data_[i]) > maxValue) {
                maxValue = static_cast<int>(data_[i]);
            }
        }
        ptrdiff_t max = -1;
        if (maxValue > 0) {
            max = static_cast<ptrdiff_t>(std::log(static_cast<double>(maxValue))
                                         / std::log(static_cast<double>(base)));
        }

        for (ptrdiff_t i = 0; i <= max; ++i) {
            mergeSort(mergeSort, 0, length, i);
        }
    }

    // RotateMSDRadixSort.runSort: the same rotate merge sort, but driven from the most
    // significant digit downwards (dist() merges one digit range and returns the first index
    // of the top digit).
    template<class T = int>
    void RotateMSDRadixSort(std::vector<T>& data_) {
        constexpr ptrdiff_t base = 4;  // Java: "Rotate MSD Radix Sort, Base 4"

        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        if constexpr (std::is_same_v<T, int>) {
            for (ptrdiff_t i = 0; i < length; ++i) {
                if (static_cast<int>(data_[i]) < 0) {
                    // "Radix sort does not support negative values!"
                    throw WideError(L"\u57fa\u6570\u6392\u5e8f\u4e0d\u652f\u6301\u8d1f\u6570\uff01");
                }
            }
        }

        auto getDigit = [](int value, ptrdiff_t p) -> ptrdiff_t {
            // Java: Reads.getDigit(value, p, base)
            ptrdiff_t divisor = 1;
            for (ptrdiff_t i = 0; i < p; ++i) {
                divisor *= base;
            }
            return static_cast<ptrdiff_t>(value / static_cast<int>(divisor)) % base;
        };

        // Java: shift(n, q) = n / base^q (the stability value of an element is its value).
        auto shift = [](ptrdiff_t n, ptrdiff_t q) -> ptrdiff_t {
            while (q > 0) {
                n /= base;
                --q;
            }
            return n;
        };

        auto multiSwap = [&](ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
            using std::swap;
            for (ptrdiff_t i = 0; i < len; ++i) {
                swap(data_[static_cast<size_t>(a + i)], data_[static_cast<size_t>(b + i)]);
            }
        };

        auto rotate = [&](ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
            ptrdiff_t l = m - a;
            ptrdiff_t r = b - m;
            while (l > 0 && r > 0) {
                if (r < l) {
                    multiSwap(m - r, m, r);
                    b -= r;
                    m -= r;
                    l -= r;
                }
                else {
                    multiSwap(a, m, l);
                    a += l;
                    m += l;
                    r -= l;
                }
            }
        };

        auto binSearch = [&](ptrdiff_t a, ptrdiff_t b, ptrdiff_t d, ptrdiff_t p) -> ptrdiff_t {
            while (a < b) {
                ptrdiff_t m = (a + b) / 2;
                if (getDigit(static_cast<int>(data_[static_cast<size_t>(m)]), p) >= d) {
                    b = m;
                }
                else {
                    a = m + 1;
                }
            }
            return a;
        };

        auto merge = [&](auto&& self, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t da, ptrdiff_t db, ptrdiff_t p) -> void {
            if (b - a < 2 || db - da < 2) {
                return;
            }
            ptrdiff_t dm = (da + db) / 2;
            ptrdiff_t m1 = binSearch(a, m, dm, p);
            ptrdiff_t m2 = binSearch(m, b, dm, p);
            rotate(m1, m, m2);
            m = m1 + (m2 - m);
            self(self, m, m2, b, dm, db, p);
            self(self, a, m1, m, da, dm, p);
        };

        auto mergeSort = [&](auto&& self, ptrdiff_t a, ptrdiff_t b, ptrdiff_t p) -> void {
            if (b - a < 2) {
                return;
            }
            ptrdiff_t m = (a + b) / 2;
            self(self, a, m, p);
            self(self, m, b, p);
            merge(merge, a, m, b, 0, base, p);
        };

        auto dist = [&](ptrdiff_t a, ptrdiff_t b, ptrdiff_t p) -> ptrdiff_t {
            mergeSort(mergeSort, a, b, p);
            return binSearch(a, b, 1, p);
        };

        // Reads.analyzeMaxLog(array, length, base, 0.5, true)
        int maxValue = static_cast<int>(data_[0]);
        for (ptrdiff_t i = 1; i < length; ++i) {
            if (static_cast<int>(data_[i]) > maxValue) {
                maxValue = static_cast<int>(data_[i]);
            }
        }
        ptrdiff_t q = -1;
        if (maxValue > 0) {
            q = static_cast<ptrdiff_t>(std::log(static_cast<double>(maxValue))
                                       / std::log(static_cast<double>(base)));
        }

        ptrdiff_t m = 0;
        ptrdiff_t i = 0;
        ptrdiff_t b = length;
        while (i < length) {
            ptrdiff_t p = (b - i < 1) ? i : dist(i, b, q);
            if (q == 0) {
                m += base;
                ptrdiff_t t = m / base;
                while (t % base == 0) {
                    t /= base;
                    ++q;
                }
                i = b;
                while (b < length && shift(static_cast<int>(data_[static_cast<size_t>(b)]), q + 1) == shift(m, q + 1)) {
                    ++b;
                }
            }
            else {
                b = p;
                --q;
            }
        }
    }

    // SelectionBogoSort.runSort: swap a random element of the remaining range to the front
    // until it is the smallest one.
    template<class T = int>
    void SelectionBogoSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());
        using std::swap;
        for (ptrdiff_t i = 0; i < length; ++i) {
            while (!NSortHelpers::isMinSorted(data_, i, length)) {
                ptrdiff_t j = NSortHelpers::bogoRandInt(engine, i, length);
                swap(data_[i], data_[j]);
            }
        }
    }

    // ShatterSort.runSort -> ShatterSorting.shatterSort(array, sortLength, bucketCount).
    // The template indexes its registers with array[i] / num and writes to
    // i * num + array[i] % num, i.e. it needs 0 based values in [0, length). VisualSort data is
    // always the permutation 1..length, so the values are shifted down by their minimum for
    // the sort and shifted back afterwards (the pure Java call would read out of bounds at
    // value == length and otherwise leave the array unsorted).
    template<class T = int>
    void ShatterSort(std::vector<T>& data_) {
        constexpr ptrdiff_t num = 4;  // Java run dialog bucket count (Base 4)

        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        int minValue = static_cast<int>(data_[0]);
        for (ptrdiff_t i = 1; i < length; ++i) {
            if (static_cast<int>(data_[i]) < minValue) {
                minValue = static_cast<int>(data_[i]);
            }
        }

        if constexpr (std::is_same_v<T, int>) {
            for (ptrdiff_t i = 0; i < length; ++i) {
                long long value = static_cast<long long>(data_[i]) - minValue;
                if (value < 0 || value >= static_cast<long long>(length)) {
                    // "The data has to be 0 based and smaller than the data size!"
                    throw WideError(L"\u8be5\u6392\u5e8f\u8981\u6c42\u6570\u636e\u4ece0\u5f00\u59cb\u4e14\u5c0f\u4e8e\u6570\u636e\u91cf\uff01");
                }
            }
        }

        if (minValue != 0) {
            for (ptrdiff_t i = 0; i < length; ++i) {
                data_[static_cast<size_t>(i)] = static_cast<int>(data_[static_cast<size_t>(i)]) - minValue;
            }
            NSortHelpers::shatterSort(data_, 0, length, num);
            for (ptrdiff_t i = 0; i < length; ++i) {
                data_[static_cast<size_t>(i)] = static_cast<int>(data_[static_cast<size_t>(i)]) + minValue;
            }
        }
        else {
            NSortHelpers::shatterSort(data_, 0, length, num);
        }
    }

    // SimpleShatterSort.runSort -> ShatterSorting.simpleShatterSort(array, sortLength,
    // bucketCount, (int) (log(length) / log(2)) / 2). Java divides by the rate, which is zero
    // for length < 4, so the rate is clamped to 1 here.
    template<class T = int>
    void SimpleShatterSort(std::vector<T>& data_) {
        constexpr ptrdiff_t num = 4;  // Java run dialog bucket count (Base 4)

        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        int minValue = static_cast<int>(data_[0]);
        for (ptrdiff_t i = 1; i < length; ++i) {
            if (static_cast<int>(data_[i]) < minValue) {
                minValue = static_cast<int>(data_[i]);
            }
        }

        if constexpr (std::is_same_v<T, int>) {
            for (ptrdiff_t i = 0; i < length; ++i) {
                long long value = static_cast<long long>(data_[i]) - minValue;
                if (value < 0 || value >= static_cast<long long>(length)) {
                    // "The data has to be 0 based and smaller than the data size!"
                    throw WideError(L"\u8be5\u6392\u5e8f\u8981\u6c42\u6570\u636e\u4ece0\u5f00\u59cb\u4e14\u5c0f\u4e8e\u6570\u636e\u91cf\uff01");
                }
            }
        }

        // Java: rate = (int) (Math.log(sortLength) / Math.log(2)) / 2. Its driver divides by
        // the rate, so the rate is 0 for length < 4 (ArithmeticException) and 1 for
        // length < 16 (i / 1 never reaches 1 -> endless loop). The rate is clamped to 2 here
        // so the loop always terminates; every length >= 16 uses the Java value.
        ptrdiff_t rate = static_cast<ptrdiff_t>(std::log(static_cast<double>(length)) / std::log(2.0)) / 2;
        if (rate < 2) {
            rate = 2;
        }

        if (minValue != 0) {
            for (ptrdiff_t i = 0; i < length; ++i) {
                data_[static_cast<size_t>(i)] = static_cast<int>(data_[static_cast<size_t>(i)]) - minValue;
            }
            NSortHelpers::simpleShatterSort(data_, length, num, rate);
            for (ptrdiff_t i = 0; i < length; ++i) {
                data_[static_cast<size_t>(i)] = static_cast<int>(data_[static_cast<size_t>(i)]) + minValue;
            }
        }
        else {
            NSortHelpers::simpleShatterSort(data_, length, num, rate);
        }
    }

    // SimplisticGravitySort.runSort + transferTo / transferFrom: every element is taken apart
    // bead by bead into aux and rebuilt from the last element backwards.
    template<class T = int>
    void SimplisticGravitySort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        int minValue = static_cast<int>(data_[0]);
        int maxValue = minValue;
        for (ptrdiff_t mainPointer = 1; mainPointer < length; ++mainPointer) {
            if (static_cast<int>(data_[mainPointer]) < minValue) {
                minValue = static_cast<int>(data_[mainPointer]);
            }
            if (static_cast<int>(data_[mainPointer]) > maxValue) {
                maxValue = static_cast<int>(data_[mainPointer]);
            }
        }

        if constexpr (std::is_same_v<T, int>) {
            if (static_cast<long long>(maxValue) - minValue > 10000000) {
                // "data value range is too large for Simplistic Gravity Sort"
                throw WideError(L"\u8be5\u6570\u636e\u6700\u5c0f\u503c\u4e0e\u6700\u5927\u503c\u5dee\u8ddd\u8fc7\u5927\uff0c\u4e0d\u9002\u5408\u4f7f\u7528\u7b80\u6613\u91cd\u529b\u6392\u5e8f\uff01");
            }
        }

        std::vector<ptrdiff_t> aux(static_cast<size_t>(maxValue - minValue), 0);

        auto transferFrom = [&](ptrdiff_t arrayLength, ptrdiff_t index) {
            for (ptrdiff_t pointer = 0; pointer < arrayLength && aux[static_cast<size_t>(pointer)] != 0; ++pointer) {
                ++data_[static_cast<size_t>(index)];
                --aux[static_cast<size_t>(pointer)];
            }
        };
        auto transferTo = [&](ptrdiff_t index) {
            for (ptrdiff_t pointer = 0; static_cast<int>(data_[static_cast<size_t>(index)]) > minValue; ++pointer) {
                --data_[static_cast<size_t>(index)];
                ++aux[static_cast<size_t>(pointer)];
            }
        };

        for (ptrdiff_t mainPointer = 0; mainPointer < length; ++mainPointer) {
            transferTo(mainPointer);
        }
        for (ptrdiff_t mainPointer = length - 1; mainPointer >= 0; --mainPointer) {
            transferFrom(static_cast<ptrdiff_t>(aux.size()), mainPointer);
        }
    }

    // SmartBogoBogoSort.runSort + smartBogoBogo: bogobogosort without the compare buffers.
    template<class T = int>
    void SmartBogoBogoSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());

        auto smartBogoBogo = [&](auto&& self, ptrdiff_t len) -> void {
            if (len == 1) {
                return;
            }
            self(self, len - 1);
            while (NSortHelpers::CompareValues(data_[static_cast<size_t>(len - 2)], data_[static_cast<size_t>(len - 1)]) > 0) {
                NSortHelpers::bogoSwap(data_, 0, len, engine);
                self(self, len - 1);
            }
        };

        smartBogoBogo(smartBogoBogo, length);
    }

    // SmartGuessSort.runSort: like OptimizedGuessSort, but the loop counter is advanced from
    // the position where the out of order element was detected.
    template<class T = int>
    void SmartGuessSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::vector<ptrdiff_t> loops(static_cast<size_t>(length), 0);

        while (true) {
            // check if the array is stably sorted (doubles as duplicate-detection)
            bool sorted = true;
            ptrdiff_t i = length - 2;
            for (; i >= 0; --i) {
                int comp = NSortHelpers::CompareValues(data_[static_cast<size_t>(loops[static_cast<size_t>(i)])],
                                                       data_[static_cast<size_t>(loops[static_cast<size_t>(i + 1)])]);
                if (comp < 0 || (comp == 0 && loops[static_cast<size_t>(i)] < loops[static_cast<size_t>(i + 1)])) {
                    continue;
                }
                sorted = false;
                break;
            }
            if (sorted) {
                break;
            }
            // progress the loops (skipping ahead to the index where the out of order was found)
            for (ptrdiff_t pos = 0; pos < length; ++pos) {
                if (pos >= i && loops[static_cast<size_t>(pos)] < length - 1) {
                    ++loops[static_cast<size_t>(pos)];
                    break;
                }
                else {
                    loops[static_cast<size_t>(pos)] = 0;
                }
            }
        }

        // write the indexes to the array
        for (ptrdiff_t i = 0; i < length; ++i) {
            loops[static_cast<size_t>(i)] = static_cast<ptrdiff_t>(static_cast<int>(data_[static_cast<size_t>(loops[static_cast<size_t>(i)])]));
        }
        for (ptrdiff_t i = 0; i < length; ++i) {
            data_[static_cast<size_t>(i)] = static_cast<int>(loops[static_cast<size_t>(i)]);
        }
    }

    // StacklessAmericanFlagSort.runSort + dist / shift: counting pass driven by the counts of
    // the current digit, without an explicit stack.
    template<class T = int>
    void StacklessAmericanFlagSort(std::vector<T>& data_) {
        constexpr ptrdiff_t r = 4;  // Java: bucketCount (run dialog; the class declares no field)

        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        if constexpr (std::is_same_v<T, int>) {
            for (ptrdiff_t i = 0; i < length; ++i) {
                if (static_cast<int>(data_[i]) < 0) {
                    // "Stackless American Flag Sort does not support negative values!"
                    throw WideError(L"\u65e0\u6808\u7f8e\u5f0f\u56fd\u65d7\u6392\u5e8f\u4e0d\u652f\u6301\u8d1f\u6570\uff01");
                }
            }
        }

        auto getDigit = [](int value, ptrdiff_t q) -> size_t {
            // Java: Reads.getDigit(value, q, r)
            ptrdiff_t divisor = 1;
            for (ptrdiff_t i = 0; i < q; ++i) {
                divisor *= r;
            }
            return static_cast<size_t>(value / static_cast<int>(divisor)) % static_cast<size_t>(r);
        };
        auto shift = [](ptrdiff_t n, ptrdiff_t q) -> ptrdiff_t {
            // Java: shift(n, q, r) = n / r^q
            while (q > 0) {
                n /= r;
                --q;
            }
            return n;
        };

        // Reads.analyzeMaxLog(array, length, r, 0.5, true)
        int maxValue = static_cast<int>(data_[0]);
        for (ptrdiff_t i = 1; i < length; ++i) {
            if (static_cast<int>(data_[i]) > maxValue) {
                maxValue = static_cast<int>(data_[i]);
            }
        }
        ptrdiff_t q = -1;
        if (maxValue > 0) {
            q = static_cast<ptrdiff_t>(std::log(static_cast<double>(maxValue)) / std::log(static_cast<double>(r)));
        }

        std::vector<ptrdiff_t> cnts(static_cast<size_t>(r), 0);
        std::vector<ptrdiff_t> offs(static_cast<size_t>(r), 0);

        auto dist = [&](ptrdiff_t a, ptrdiff_t b, ptrdiff_t qq) -> ptrdiff_t {
            for (ptrdiff_t i = 1; i < r; ++i) {
                cnts[static_cast<size_t>(i)] += cnts[static_cast<size_t>(i - 1)];
                offs[static_cast<size_t>(i)] = cnts[static_cast<size_t>(i - 1)];
            }
            for (ptrdiff_t i = 0; i < r - 1; ++i) {
                ptrdiff_t pos = a + offs[static_cast<size_t>(i)];
                if (cnts[static_cast<size_t>(i)] > offs[static_cast<size_t>(i)]) {
                    int t = static_cast<int>(data_[static_cast<size_t>(pos)]);
                    do {
                        size_t digit = getDigit(t, qq);
                        --cnts[digit];
                        int t1 = static_cast<int>(data_[static_cast<size_t>(a + cnts[digit])]);
                        data_[static_cast<size_t>(a + cnts[digit])] = t;
                        t = t1;
                    } while (cnts[static_cast<size_t>(i)] > offs[static_cast<size_t>(i)]);
                }
            }
            ptrdiff_t p = a + offs[1];
            for (ptrdiff_t i = 0; i < r; ++i) {
                cnts[static_cast<size_t>(i)] = 0;
                offs[static_cast<size_t>(i)] = 0;
            }
            return p;
        };

        for (ptrdiff_t j = 0; j < length; ++j) {
            ++cnts[getDigit(static_cast<int>(data_[j]), q)];
        }

        ptrdiff_t m = 0;
        ptrdiff_t i = 0;
        ptrdiff_t b = length;
        while (i < length) {
            ptrdiff_t p = (b - i < 1) ? i : dist(i, b, q);
            if (q == 0) {
                m += r;
                ptrdiff_t t = m / r;
                while (t % r == 0) {
                    t /= r;
                    ++q;
                }
                i = b;
                while (b < length && shift(static_cast<int>(data_[static_cast<size_t>(b)]), q + 1) == shift(m, q + 1)) {
                    ++cnts[getDigit(static_cast<int>(data_[static_cast<size_t>(b)]), q)];
                    ++b;
                }
            }
            else {
                b = p;
                --q;
                for (ptrdiff_t j = i; j < b; ++j) {
                    ++cnts[getDigit(static_cast<int>(data_[j]), q)];
                }
            }
        }
    }

    // StacklessBinaryQuickSort.runSort + partition: the binary quicksort queue is replaced by
    // the value m the way the Java code does (each pass partitions [i, b) with bit q, then
    // walks over the elements that share the bits above q with m).
    template<class T = int>
    void StacklessBinaryQuickSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        if constexpr (std::is_same_v<T, int>) {
            for (ptrdiff_t i = 0; i < length; ++i) {
                if (static_cast<int>(data_[i]) < 0) {
                    // "Binary quicksort does not support negative values!"
                    throw WideError(L"\u4e8c\u5206\u5feb\u901f\u6392\u5e8f\u4e0d\u652f\u6301\u8d1f\u6570\uff01");
                }
            }
        }

        auto partition = [&](ptrdiff_t a, ptrdiff_t b, ptrdiff_t bit) -> ptrdiff_t {
            ptrdiff_t i = a - 1;
            ptrdiff_t j = b;
            using std::swap;
            while (true) {
                do {
                    ++i;
                } while (i < j && ((static_cast<int>(data_[static_cast<size_t>(i)]) >> bit) & 1) == 0);
                do {
                    --j;
                } while (j > i && ((static_cast<int>(data_[static_cast<size_t>(j)]) >> bit) & 1) != 0);
                if (i < j) {
                    swap(data_[static_cast<size_t>(i)], data_[static_cast<size_t>(j)]);
                }
                else {
                    return i;
                }
            }
        };

        ptrdiff_t q = NSortHelpers::binaryQuickSortHighBit(data_, 0, length);
        ptrdiff_t m = 0;
        ptrdiff_t i = 0;
        ptrdiff_t b = length;
        while (i < length) {
            ptrdiff_t p = (b - i < 1) ? i : partition(i, b, q);
            if (q == 0) {
                m += 2;
                while (((m >> (q + 1)) & 1) == 0) {
                    ++q;
                }
                i = b;
                while (b < length && ((data_[static_cast<size_t>(b)] >> (q + 1)) == (m >> (q + 1)))) {
                    ++b;
                }
            }
            else {
                b = p;
                --q;
            }
        }
    }

    // StaticSort.runSort + staticSort: distributes the values into auxLen buckets, permutes
    // them into place in one cycle walk per bucket and finishes every bucket with an
    // insertion sort (up to 16 elements) or a heap sort.
    template<class T = int>
    void StaticSort(std::vector<T>& data_) {
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());
        if (dataSize < 2) {
            return;
        }

        // findMinMax(array, a, b)
        int minValue = static_cast<int>(data_[0]);
        int maxValue = minValue;
        for (ptrdiff_t i = 1; i < dataSize; ++i) {
            if (static_cast<int>(data_[i]) < minValue) {
                minValue = static_cast<int>(data_[i]);
            }
            else if (static_cast<int>(data_[i]) > maxValue) {
                maxValue = static_cast<int>(data_[i]);
            }
        }

        ptrdiff_t auxLen = dataSize;
        std::vector<ptrdiff_t> count(static_cast<size_t>(auxLen) + 1, 0);
        std::vector<ptrdiff_t> offset(static_cast<size_t>(auxLen) + 1, 0);

        float bucketRate = static_cast<float>(auxLen) / static_cast<float>(maxValue - minValue + 1);

        for (ptrdiff_t i = 0; i < dataSize; ++i) {
            ptrdiff_t idx = static_cast<ptrdiff_t>(static_cast<float>(static_cast<int>(data_[i]) - minValue) * bucketRate);
            ++count[static_cast<size_t>(idx)];
        }
        offset[0] = 0;
        for (ptrdiff_t i = 1; i < auxLen; ++i) {
            offset[static_cast<size_t>(i)] = count[static_cast<size_t>(i - 1)] + offset[static_cast<size_t>(i - 1)];
        }

        for (ptrdiff_t v = 0; v < auxLen; ++v) {
            while (count[static_cast<size_t>(v)] > 0) {
                ptrdiff_t origin = offset[static_cast<size_t>(v)];
                ptrdiff_t from = origin;
                int num = static_cast<int>(data_[static_cast<size_t>(from)]);
                data_[static_cast<size_t>(from)] = -1;
                do {
                    ptrdiff_t idx = static_cast<ptrdiff_t>(static_cast<float>(num - minValue) * bucketRate);
                    ptrdiff_t to = offset[static_cast<size_t>(idx)];
                    ++offset[static_cast<size_t>(idx)];
                    --count[static_cast<size_t>(idx)];
                    int temp = static_cast<int>(data_[static_cast<size_t>(to)]);
                    data_[static_cast<size_t>(to)] = num;
                    num = temp;
                    from = to;
                } while (from != origin);
            }
        }

        for (ptrdiff_t i = 0; i < auxLen; ++i) {
            ptrdiff_t s = (i > 1) ? offset[static_cast<size_t>(i - 1)] : 0;
            ptrdiff_t e = offset[static_cast<size_t>(i)];
            if (e - s <= 1) {
                continue;
            }
            if (e - s > 16) {
                NSortHelpers::heapSort(data_, s, e, true);
            }
            else {
                NSortHelpers::insertionSort(data_, s, e);
            }
        }
    }

    // TimeSort.runSort: one thread per element sleeps value * A milliseconds, then appends its
    // element to the sorted prefix (the Java code does the same with 10 ms per number and
    // finishes with InsertionSort.customInsertSort).
    template<class T = int>
    void TimeSort(std::vector<T>& data_) {
        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());
        if (sortLength < 2) {
            return;
        }

        constexpr int a = 10;  // Java: @SortMeta defaultAnswer "delay per number in milliseconds"

        std::vector<T> tmp(data_);
        std::atomic<ptrdiff_t> next{ 0 };
        std::mutex reportMutex;

        std::vector<std::thread> threads;
        threads.reserve(static_cast<size_t>(sortLength));
        for (ptrdiff_t i = 0; i < sortLength; ++i) {
            threads.emplace_back([&data_, &tmp, &next, &reportMutex, i]() {
                int value = static_cast<int>(tmp[static_cast<size_t>(i)]);
                if (value > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(value * a));
                }
                std::lock_guard<std::mutex> lock(reportMutex);
                data_[static_cast<size_t>(next.load())] = value;
                next.fetch_add(1);
            });
        }
        for (size_t i = 0; i < threads.size(); ++i) {
            threads[i].join();
        }

        NSortHelpers::insertionSort(data_, 0, sortLength);
    }

} // namespace NVisualSort::NSortAlgorithms
