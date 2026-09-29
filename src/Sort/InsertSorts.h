#pragma once
// Ports of the ArrayV (Java, MIT) "insert" category sorting classes,
// https://github.com/Gaming32/ArrayV  (sources: src/Sort/InsertSorts.java counterparts).
// VisualSort - insertion sort family (29 classes of the ArrayV "Insertion Sorts" package).
//
// ASCII ONLY: do not put non-ASCII characters in this file. MSVC parses sources in the
// system code page (GBK); use \uXXXX escapes if a wide string is ever needed.
//
// This header is #included from Sort.h after SortHelpers.h; it opens the shared namespace
// and closes it again at the end of the file.
//
// Naming: the Java classes InsertionSort and ShellSort already exist as C++ sorts in
// NSortAlgorithms, so they are ported as InsertionSortJava / ShellSortJava.
// Every internal helper starts with the PascalCase name of the algorithm it belongs to.
#include "SortHelpers.h"
#include <queue>
#include <thread>

namespace NVisualSort::NSortAlgorithms {

    // InsertionSort.runSort (ArrayV's plain insertion sort; that menu entry already exists).
    template<class T = int>
    void InsertionSortJava(std::vector<T>& data_) {
        if (data_.size() < 2) {
            return;
        }
        NSortHelpers::insertionSort(data_, 0, static_cast<ptrdiff_t>(data_.size()));
    }

    // BinaryInsertionSort.runSort.
    template<class T = int>
    void BinaryInsertionSort(std::vector<T>& data_) {
        if (data_.size() < 2) {
            return;
        }
        NSortHelpers::binaryInsertSort(data_, 0, static_cast<ptrdiff_t>(data_.size()));
    }

    // DoubleInsertionSort.insertionSort: inserts two elements per step from the middle outwards.
    template<class T = int>
    void DoubleInsertionSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        ptrdiff_t start = 0;
        ptrdiff_t end = length;

        ptrdiff_t left = start + (end - start) / 2 - 1;
        ptrdiff_t right = left + 1;

        if (data_[left] > data_[right]) {
            using std::swap;
            swap(data_[left], data_[right]);
        }
        --left;
        ++right;

        T leftItem{};
        T rightItem{};

        while (left >= start && right < end) {
            if (data_[left] > data_[right]) {
                leftItem = data_[right];
                rightItem = data_[left];

                ptrdiff_t pos = left + 1;
                while (pos <= right && data_[pos] <= leftItem) {
                    data_[pos - 1] = data_[pos];
                    ++pos;
                }
                data_[pos - 1] = leftItem;

                pos = right - 1;
                while (pos >= left && data_[pos] >= rightItem) {
                    data_[pos + 1] = data_[pos];
                    --pos;
                }
                data_[pos + 1] = rightItem;
            }
            else {
                leftItem = data_[left];
                rightItem = data_[right];

                ptrdiff_t pos = left + 1;
                while (data_[pos] < leftItem) {
                    data_[pos - 1] = data_[pos];
                    ++pos;
                }
                data_[pos - 1] = leftItem;

                pos = right - 1;
                while (data_[pos] > rightItem) {
                    data_[pos + 1] = data_[pos];
                    --pos;
                }
                data_[pos + 1] = rightItem;
            }

            --left;
            ++right;
        }

        if (right < end) {
            ptrdiff_t pos = right - 1;
            T current = data_[right];
            // Java runs this loop without a lower bound; the guard keeps it inside the range.
            while (pos >= start && data_[pos] > current) {
                data_[pos + 1] = data_[pos];
                --pos;
            }
            data_[pos + 1] = current;
        }
    }

    // BinaryDoubleInsertionSort.leftBinarySearch (leftmost insertion point for val).
    template<class T>
    ptrdiff_t BinaryDoubleInsertionSortLeftSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;
            NSortHelpers::MarkArray(1, data_, a);
            NSortHelpers::MarkArray(2, data_, m);
            NSortHelpers::MarkArray(3, data_, b);

            if (val <= data_[m]) {
                b = m;
            }
            else {
                a = m + 1;
            }
        }
        return a;
    }

    // BinaryDoubleInsertionSort.rightBinarySearch (rightmost insertion point for val).
    template<class T>
    ptrdiff_t BinaryDoubleInsertionSortRightSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;
            NSortHelpers::MarkArray(1, data_, a);
            NSortHelpers::MarkArray(2, data_, m);
            NSortHelpers::MarkArray(3, data_, b);

            if (val < data_[m]) {
                b = m;
            }
            else {
                a = m + 1;
            }
        }
        return a;
    }

    // BinaryDoubleInsertionSort.insertToLeft.
    template<class T>
    void BinaryDoubleInsertionSortInsertLeft(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& temp) {
        while (a > b) {
            --a;
            data_[a + 1] = data_[a];
        }
        data_[b] = temp;
    }

    // BinaryDoubleInsertionSort.insertToRight.
    template<class T>
    void BinaryDoubleInsertionSortInsertRight(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& temp) {
        while (a < b) {
            ++a;
            data_[a - 1] = data_[a];
        }
        data_[a] = temp;
    }

    // BinaryDoubleInsertionSort.doubleInsertion: two binary-search driven insertions per step.
    template<class T = int>
    void BinaryDoubleInsertionSort(std::vector<T>& data_) {
        ptrdiff_t a = 0;
        ptrdiff_t b = static_cast<ptrdiff_t>(data_.size());
        if (b - a < 2) {
            return;
        }

        ptrdiff_t j = a + (b - a - 2) / 2 + 1;
        ptrdiff_t i = a + (b - a - 1) / 2;

        if (j > i && data_[i] > data_[j]) {
            using std::swap;
            swap(data_[i], data_[j]);
        }

        --i;
        ++j;

        while (j < b) {
            if (data_[i] > data_[j]) {
                T l = data_[j];
                T r = data_[i];

                ptrdiff_t m = BinaryDoubleInsertionSortRightSearch(data_, i + 1, j, l);
                BinaryDoubleInsertionSortInsertRight(data_, i, m - 1, l);
                ptrdiff_t left = BinaryDoubleInsertionSortLeftSearch(data_, m, j, r);
                BinaryDoubleInsertionSortInsertLeft(data_, j, left, r);
            }
            else {
                T l = data_[i];
                T r = data_[j];

                ptrdiff_t m = BinaryDoubleInsertionSortLeftSearch(data_, i + 1, j, l);
                BinaryDoubleInsertionSortInsertRight(data_, i, m - 1, l);
                ptrdiff_t right = BinaryDoubleInsertionSortRightSearch(data_, m, j, r);
                BinaryDoubleInsertionSortInsertLeft(data_, j, right, r);
            }
            --i;
            ++j;
        }
    }

    // BlockInsertionSort.grailRotate / Rotations.holyGriesMills (three reversals).
    template<class T>
    void BlockInsertionSortRotate(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t lenA, ptrdiff_t lenB) {
        NSortHelpers::reverseRange(data_, pos, pos + lenA);
        NSortHelpers::reverseRange(data_, pos + lenA, pos + lenA + lenB);
        NSortHelpers::reverseRange(data_, pos, pos + lenA + lenB);
    }

    // GrailSorting.grailBinSearch (used by BlockInsertionSort through its GrailSorting base class).
    template<class T>
    ptrdiff_t BlockInsertionSortBinSearch(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len, ptrdiff_t keyPos, bool isLeft) {
        ptrdiff_t left = -1;
        ptrdiff_t right = len;

        while (left < right - 1) {
            ptrdiff_t mid = left + ((right - left) >> 1);
            if (isLeft) {
                if (data_[pos + mid] >= data_[keyPos]) {
                    right = mid;
                }
                else {
                    left = mid;
                }
            }
            else {
                if (data_[pos + mid] > data_[keyPos]) {
                    right = mid;
                }
                else {
                    left = mid;
                }
            }
            NSortHelpers::MarkArray(1, data_, pos + mid);
        }
        return right;
    }

    // GrailSorting.grailMergeWithoutBuffer (in place merge used by BlockInsertionSort).
    template<class T>
    void BlockInsertionSortMergeWithoutBuffer(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len1, ptrdiff_t len2) {
        if (len1 < len2) {
            while (len1 != 0) {
                // Binary Search left
                ptrdiff_t loc = BlockInsertionSortBinSearch(data_, pos + len1, len2, pos, true);
                if (loc != 0) {
                    BlockInsertionSortRotate(data_, pos, len1, loc);
                    pos += loc;
                    len2 -= loc;
                }
                if (len2 == 0) break;
                do {
                    ++pos;
                    --len1;
                } while (len1 != 0 && data_[pos] <= data_[pos + len1]);
            }
        }
        else {
            while (len2 != 0) {
                // Binary Search right
                ptrdiff_t loc = BlockInsertionSortBinSearch(data_, pos, len1, pos + (len1 + len2 - 1), false);
                if (loc != len1) {
                    BlockInsertionSortRotate(data_, pos + loc, len1 - loc, len2);
                    len1 = loc;
                }
                if (len1 == 0) break;
                do {
                    --len2;
                } while (len2 != 0 && data_[pos + len1 - 1] <= data_[pos + len1 + len2 - 1]);
            }
        }
    }

    // BlockInsertionSort.insert1.
    template<class T>
    void BlockInsertionSortInsert1(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t l) {
        T tmp = data_[l];
        --l;
        while (l >= a && data_[l] > tmp) {
            data_[l + 1] = data_[l];
            --l;
        }
        data_[l + 1] = tmp;
    }

    // BlockInsertionSort.insert2.
    template<class T>
    void BlockInsertionSortInsert2(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t l, ptrdiff_t r) {
        T tmpL = data_[l];
        --l;
        T tmpR = data_[r];

        while (l >= a && data_[l] > tmpR) {
            data_[l + 2] = data_[l];
            --l;
        }
        data_[l + 2] = tmpR;

        while (l >= a && data_[l] > tmpL) {
            data_[l + 1] = data_[l];
            --l;
        }
        data_[l + 1] = tmpL;
    }

    // BlockInsertionSort.findRun (reverses descending runs so every run is ascending).
    template<class T>
    ptrdiff_t BlockInsertionSortFindRun(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t i = a + 1;
        if (i == b) {
            return i;
        }

        bool descending = data_[i - 1] > data_[i];
        ++i;

        if (descending) {
            while (i < b && data_[i - 1] > data_[i]) {
                ++i;
            }
            NSortHelpers::reverseRange(data_, a, i); // Java reversal is inclusive: [a, i - 1]
        }
        else {
            while (i < b && data_[i - 1] <= data_[i]) {
                ++i;
            }
        }
        return i;
    }

    // BlockInsertionSort.insertionSort (block merges with the grail in place merge).
    template<class T = int>
    void BlockInsertionSort(std::vector<T>& data_) {
        ptrdiff_t b = static_cast<ptrdiff_t>(data_.size());
        if (b < 2) {
            return;
        }

        ptrdiff_t a = 0;
        ptrdiff_t i = BlockInsertionSortFindRun(data_, a, b);

        while (i < b) {
            ptrdiff_t j = BlockInsertionSortFindRun(data_, i, b);
            ptrdiff_t len = j - i;

            if (len < 3) {
                if (len == 2) {
                    BlockInsertionSortInsert2(data_, a, i, i + 1);
                }
                else {
                    BlockInsertionSortInsert1(data_, a, i);
                }
            }
            else {
                BlockInsertionSortMergeWithoutBuffer(data_, a, i - a, len);
            }
            i = j;
        }
    }

    // ShellSort.runSort (that menu entry already exists; default ExtendedCiuraGaps).
    template<class T = int>
    void ShellSortJava(std::vector<T>& data_) {
        if (data_.size() < 2) {
            return;
        }
        NSortHelpers::shellSort(data_, 0, static_cast<ptrdiff_t>(data_.size()));
    }

    // RecursiveShellSort.gappedInsertionSort.
    template<class T>
    void RecursiveShellSortGappedInsertion(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t gap) {
        for (ptrdiff_t i = a + gap; i < b; i += gap) {
            T key = data_[i];
            ptrdiff_t j = i - gap;

            while (j >= a && key < data_[j]) {
                data_[j + gap] = data_[j];
                j -= gap;
            }
            data_[j + gap] = key;
        }
    }

    // RecursiveShellSort.recursiveShellSort (recursion depth is O(log n)).
    template<class T>
    void RecursiveShellSortRec(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, ptrdiff_t g) {
        if (start + g <= end) {
            RecursiveShellSortRec(data_, start, end, 3 * g);
            RecursiveShellSortRec(data_, start + g, end, 3 * g);
            RecursiveShellSortRec(data_, start + (2 * g), end, 3 * g);
            RecursiveShellSortGappedInsertion(data_, start, end, g);
        }
    }

    // RecursiveShellSort.runSort.
    template<class T = int>
    void RecursiveShellSort(std::vector<T>& data_) {
        if (data_.size() < 2) {
            return;
        }
        RecursiveShellSortRec(data_, 0, static_cast<ptrdiff_t>(data_.size()), 1);
    }

    // ShellSortParallel.gappedInsertion (one residue class of a gap pass).
    template<class T>
    void ShellSortParallelGappedInsertion(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t g) {
        for (ptrdiff_t i = a + g; i < b; i += g) {
            if (data_[i - g] > data_[i]) {
                T tmp = data_[i];
                ptrdiff_t j = i;

                do {
                    data_[j] = data_[j - g];
                    j -= g;
                } while (j - g >= a && data_[j - g] > tmp);

                data_[j] = tmp;
            }
        }
    }

    // ShellSortParallel.runSort: every gap pass runs one thread per residue class.
    // The Java code starts min(gap, length - gap) threads at once; here they are started in
    // batches so that a large gap cannot exhaust the system's thread limit (the residue classes
    // are independent, so batching gives exactly the same result).
    template<class T = int>
    void ShellSortParallel(std::vector<T>& data_) {
        ptrdiff_t currentLength = static_cast<ptrdiff_t>(data_.size());
        if (currentLength < 2) {
            return;
        }

        const std::vector<ptrdiff_t>& gaps = NSortHelpers::ExtendedCiuraGaps;

        ptrdiff_t k = 0;
        while (k < static_cast<ptrdiff_t>(gaps.size()) && gaps[k] >= currentLength) {
            ++k;
        }

        constexpr ptrdiff_t batchSize = 64;

        for (; k < static_cast<ptrdiff_t>(gaps.size()); ++k) {
            ptrdiff_t g = gaps[k];
            ptrdiff_t t = (std::min)(g, currentLength - g);
            if (t <= 0) {
                continue;
            }

            for (ptrdiff_t base = 0; base < t; base += batchSize) {
                ptrdiff_t batchEnd = (std::min)(base + batchSize, t);
                std::vector<std::thread> threads;
                threads.reserve(static_cast<size_t>(batchEnd - base));

                for (ptrdiff_t i = base; i < batchEnd; ++i) {
                    threads.emplace_back([&data_, i, currentLength, g]() {
                        ShellSortParallelGappedInsertion(data_, i, currentLength, g);
                    });
                }
                for (std::thread& thread : threads) {
                    thread.join();
                }
            }
        }
    }

    // LibrarySort.shiftExt (G = 15, the "no book" sentinel is length).
    template<class T>
    void LibrarySortShiftExt(std::vector<T>& tmp_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, int valueBound) {
        ptrdiff_t m1 = a + (std::min)(m - a, b - m);

        while (m > a) {
            --b;
            --m;
            tmp_[b] = tmp_[m];
        }
        while (a < m1) {
            tmp_[a] = valueBound;
            ++a;
        }
    }

    // LibrarySort.leftBlockSearch.
    template<class T>
    ptrdiff_t LibrarySortLeftBlockSearch(std::vector<T>& tmp_, ptrdiff_t a, ptrdiff_t b, const T& val) {
        const ptrdiff_t s = 15 + 1;

        while (a < b) {
            ptrdiff_t m = a + (((b - a) / s) / 2) * s;
            NSortHelpers::MarkArray(2, tmp_, m / s);

            if (val <= tmp_[m]) {
                b = m;
            }
            else {
                a = m + s;
            }
        }
        return a;
    }

    // LibrarySort.rightBlockSearch.
    template<class T>
    ptrdiff_t LibrarySortRightBlockSearch(std::vector<T>& tmp_, ptrdiff_t a, ptrdiff_t b, const T& val) {
        const ptrdiff_t s = 15 + 1;

        while (a < b) {
            ptrdiff_t m = a + (((b - a) / s) / 2) * s;
            NSortHelpers::MarkArray(2, tmp_, m / s);

            if (val < tmp_[m]) {
                b = m;
            }
            else {
                a = m + s;
            }
        }
        return a;
    }

    // LibrarySort.locSearch (searches for the sentinel with compareOriginalValues semantics).
    template<class T>
    ptrdiff_t LibrarySortLocSearch(std::vector<T>& tmp_, ptrdiff_t a, ptrdiff_t b, int valueBound) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            if (!(tmp_[m] < valueBound)) { // valueBound <= tmp_[m]
                b = m;
            }
            else {
                a = m + 1;
            }
        }
        return a;
    }

    // LibrarySort.rightBinSearch.
    template<class T>
    ptrdiff_t LibrarySortRightBinSearch(std::vector<T>& tmp_, ptrdiff_t a, ptrdiff_t b, const T& val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            if (val < tmp_[m]) {
                b = m;
            }
            else {
                a = m + 1;
            }
        }
        return a;
    }

    // LibrarySort.insertTo.
    template<class T>
    void LibrarySortInsertTo(std::vector<T>& tmp_, ptrdiff_t a, ptrdiff_t b) {
        T temp = tmp_[a];

        while (a > b) {
            --a;
            tmp_[a + 1] = tmp_[a];
        }
        tmp_[b] = temp;
    }

    // LibrarySort.binaryInsertion.
    template<class T>
    void LibrarySortBinaryInsertion(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        for (ptrdiff_t i = a + 1; i < b; ++i) {
            ptrdiff_t loc = LibrarySortRightBinSearch(data_, a, i, data_[i]);
            LibrarySortInsertTo(data_, i, loc);
        }
    }

    // LibrarySort.retrieve (moves the books out of the library back into the array).
    template<class T>
    void LibrarySortRetrieve(std::vector<T>& data_, std::vector<T>& tmp_, ptrdiff_t i, ptrdiff_t pEnd, int valueBound) {
        const ptrdiff_t G = 15;

        ptrdiff_t loc = i - 1;

        for (ptrdiff_t k = pEnd - (G + 1); k > G;) {
            ptrdiff_t m = LibrarySortLocSearch(tmp_, k - G, k, valueBound) - 1;
            k -= G + 1;

            while (m >= k && loc >= 0) {
                data_[loc] = tmp_[m];
                --loc;
                tmp_[m] = valueBound;
                --m;
            }
        }

        ptrdiff_t m = LibrarySortLocSearch(tmp_, 0, G, valueBound) - 1;
        while (m >= 0 && loc >= 0) {
            data_[loc] = tmp_[m];
            --loc;
            tmp_[m] = valueBound;
            --m;
        }
    }

    // LibrarySort.runSort (G = 15 gaps per block, R = 4 rebalancing factor, no shuffle).
    template<class T = int>
    void LibrarySort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        const ptrdiff_t G = 15;
        const ptrdiff_t R = 4;
        // The Java code fills the gaps with length, but this visualizer's data is 1..length, so
        // a book can hold the sentinel value and would be mistaken for an empty slot (and lost).
        // length + 1 is strictly above every data value, so it can only be a gap.
        const int valueBound = static_cast<int>(length) + 1;

        if constexpr (std::is_same_v<T, int>) {
            for (ptrdiff_t i = 0; i < length; ++i) {
                if (static_cast<int>(data_[i]) > length) {
                    // "data values must not exceed the data size for Library Sort"
                    throw WideError(L"\u56fe\u4e66\u9986\u6392\u5e8f\u8981\u6c42\u6570\u636e\u4e0d\u8d85\u8fc7\u6570\u636e\u91cf\uff01");
                }
            }
        }

        // The Java code allocates length*(G+1)-1 slots but indexes up to (s+1)*(G+1)+G-1 with
        // s == length for length < 32, which is out of bounds there. The full pEnd range is
        // allocated here so every block access below stays inside the buffer.
        std::vector<T> tmp(static_cast<size_t>(length + 1) * (G + 1) + G);
        for (ptrdiff_t k = 0; k < static_cast<ptrdiff_t>(tmp.size()); ++k) {
            tmp[k] = valueBound;
        }

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());

        ptrdiff_t s = length;
        while (s >= 32) {
            s = (s - 1) / R + 1;
        }

        ptrdiff_t i = s;
        ptrdiff_t j = R * i;
        ptrdiff_t pEnd = (s + 1) * (G + 1) + G;
        LibrarySortBinaryInsertion(data_, 0, s);

        for (ptrdiff_t k = 0; k < s; ++k) {
            NSortHelpers::MarkArray(1, data_, k);
            tmp[k * (G + 1) + G] = data_[k];
        }

        for (; i < length; ++i) {
            if (i == j) {
                LibrarySortRetrieve(data_, tmp, i, pEnd, valueBound);

                s = i;
                pEnd = (s + 1) * (G + 1) + G;
                j *= R;

                for (ptrdiff_t k = 0; k < s; ++k) {
                    NSortHelpers::MarkArray(1, data_, k);
                    tmp[k * (G + 1) + G] = data_[k];
                }
            }

            NSortHelpers::MarkArray(1, data_, i);
            ptrdiff_t bLoc = LibrarySortLeftBlockSearch(tmp, G, pEnd - (G + 1), data_[i]);

            if (!(data_[i] < tmp[bLoc]) && !(tmp[bLoc] < data_[i])) { // array[i] == tmp[bLoc]
                ptrdiff_t eqEnd = LibrarySortRightBlockSearch(tmp, bLoc + (G + 1), pEnd - (G + 1), data_[i]);
                ptrdiff_t span = (eqEnd - bLoc) / (G + 1);

                if (span > 1) {
                    bLoc += static_cast<ptrdiff_t>(engine() % static_cast<uint32_t>(span)) * (G + 1);
                }
            }
            ptrdiff_t loc = LibrarySortLocSearch(tmp, bLoc - G, bLoc, valueBound);

            if (loc == bLoc) {
                do {
                    bLoc += G + 1;
                } while (bLoc < pEnd && LibrarySortLocSearch(tmp, bLoc - G, bLoc, valueBound) == bLoc);

                if (bLoc == pEnd) {
                    LibrarySortRetrieve(data_, tmp, i, pEnd, valueBound);

                    s = i;
                    pEnd = (s + 1) * (G + 1) + G;
                    j = R * i;

                    for (ptrdiff_t k = 0; k < s; ++k) {
                        NSortHelpers::MarkArray(1, data_, k);
                        tmp[k * (G + 1) + G] = data_[k];
                    }
                }
                else {
                    ptrdiff_t rotP = LibrarySortLocSearch(tmp, bLoc - G, bLoc, valueBound);
                    ptrdiff_t rotS = bLoc - (std::max)(rotP, bLoc - G / 2);
                    LibrarySortShiftExt(tmp, loc - rotS, bLoc - rotS, bLoc, valueBound);
                }
                --i;
            }
            else {
                tmp[loc] = data_[i];
                ptrdiff_t insLoc = LibrarySortRightBinSearch(tmp, bLoc - G, loc, tmp[loc]);
                LibrarySortInsertTo(tmp, loc, insLoc);
            }
        }
        LibrarySortRetrieve(data_, tmp, length, pEnd, valueBound);
    }

    // SimplifiedLibrarySort.getMinLevel (R = 4).
    inline ptrdiff_t SimplifiedLibrarySortMinLevel(ptrdiff_t n) {
        const ptrdiff_t R = 4;
        while (n >= 32) {
            n = (n - 1) / R + 1;
        }
        return n;
    }

    // SimplifiedLibrarySort.binarySearch.
    template<class T>
    ptrdiff_t SimplifiedLibrarySortBinarySearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;
            NSortHelpers::MarkArray(3, data_, m);

            if (val < data_[m]) {
                b = m;
            }
            else {
                a = m + 1;
            }
        }
        return a;
    }

    // SimplifiedLibrarySort.rebalance.
    template<class T>
    void SimplifiedLibrarySortRebalance(std::vector<T>& data_, std::vector<T>& temp,
        std::vector<ptrdiff_t>& cnts, std::vector<ptrdiff_t>& locs, ptrdiff_t m, ptrdiff_t b) {
        // do a partial sum to find locations
        for (ptrdiff_t i = 0; i < m; ++i) {
            cnts[i + 1] = cnts[i + 1] + cnts[i] + 1;
        }

        // place books in gaps into their correct locations
        for (ptrdiff_t i = m, j = 0; i < b; ++i, ++j) {
            NSortHelpers::MarkArray(2, data_, i);
            temp[cnts[locs[j]]] = data_[i];
            cnts[locs[j]] = cnts[locs[j]] + 1;
        }
        for (ptrdiff_t i = 0; i < m; ++i) {
            NSortHelpers::MarkArray(2, data_, i);
            temp[cnts[i]] = data_[i];
            cnts[i] = cnts[i] + 1;
        }

        // copy back to array & sort the gaps
        for (ptrdiff_t i = 0; i < b; ++i) {
            data_[i] = temp[i];
        }
        NSortHelpers::binaryInsertSort(data_, 0, cnts[0] - 1);
        for (ptrdiff_t i = 0; i < m - 1; ++i) {
            NSortHelpers::binaryInsertSort(data_, cnts[i], cnts[i + 1] - 1);
        }
        NSortHelpers::binaryInsertSort(data_, cnts[m - 1], cnts[m]);

        // reset count array
        for (ptrdiff_t i = 0; i < m + 2; ++i) {
            cnts[i] = 0;
        }
    }

    // SimplifiedLibrarySort.runSort (R = 4 rebalancing factor).
    template<class T = int>
    void SimplifiedLibrarySort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        const ptrdiff_t R = 4;

        if (length < 32) {
            NSortHelpers::binaryInsertSort(data_, 0, length);
            return;
        }

        ptrdiff_t j = SimplifiedLibrarySortMinLevel(length);
        NSortHelpers::binaryInsertSort(data_, 0, j);

        ptrdiff_t maxLevel = j;
        while (maxLevel * R < length) {
            maxLevel *= R;
        }

        std::vector<T> temp(length);
        std::vector<ptrdiff_t> cnts(maxLevel + 2, 0);
        std::vector<ptrdiff_t> locs(length - maxLevel, 0);

        for (ptrdiff_t i = j, k = 0; i < length; ++i) {
            if (R * j == i) {
                SimplifiedLibrarySortRebalance(data_, temp, cnts, locs, j, i);
                j = i;
                k = 0;
            }

            // search which gap a book goes and save the result
            NSortHelpers::MarkArray(2, data_, i);
            ptrdiff_t loc = SimplifiedLibrarySortBinarySearch(data_, 0, j, data_[i]);

            cnts[loc + 1] = cnts[loc + 1] + 1;
            locs[k] = loc;
            ++k;
        }
        SimplifiedLibrarySortRebalance(data_, temp, cnts, locs, j, length);
    }

    // PatienceSort's private binarySearch: the Java code only uses it for its highlight marks
    // (findValue is the value of the pile that is about to be inserted).
    template<class T>
    void PatienceSortMarkSearch(std::vector<T>& data_, const std::vector<std::vector<T>>& piles, const T& findValue) {
        if (piles.empty()) {
            return;
        }

        ptrdiff_t at = static_cast<ptrdiff_t>(piles.size()) / 2;
        ptrdiff_t change = static_cast<ptrdiff_t>(piles.size()) / 4;

        while (change > 0 && NSortHelpers::CompareValues(piles[at].back(), findValue) != 0) {
            NSortHelpers::MarkArray(1, data_, at);

            if (piles[at].back() < findValue) {
                at += change;
            }
            else {
                at -= change;
            }
            change /= 2;
        }
        NSortHelpers::MarkArray(1, data_, at);
    }

    // PatienceSort.runSort: deal the values into piles, then merge them with a min heap.
    template<class T = int>
    void PatienceSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::vector<std::vector<T>> piles;

        // sort into piles
        for (ptrdiff_t x = 0; x < length; ++x) {
            T value = data_[x];

            NSortHelpers::MarkArray(2, data_, x);

            // leftmost pile whose top is >= value (Collections.binarySearch + insertion point)
            ptrdiff_t lo = 0;
            ptrdiff_t hi = static_cast<ptrdiff_t>(piles.size());
            while (lo < hi) {
                ptrdiff_t mid = lo + (hi - lo) / 2;
                if (!(piles[mid].back() < value)) {
                    hi = mid;
                }
                else {
                    lo = mid + 1;
                }
            }
            ptrdiff_t i = lo;

            PatienceSortMarkSearch(data_, piles, value);

            if (i != static_cast<ptrdiff_t>(piles.size())) {
                piles[i].push_back(value);
            }
            else {
                piles.push_back(std::vector<T>{ value });
            }
        }

        // priority queue allows us to retrieve the least pile efficiently
        struct PatienceSortPileCompare {
            const std::vector<std::vector<T>>* piles;
            bool operator()(ptrdiff_t a, ptrdiff_t b) const {
                return (*piles)[b].back() < (*piles)[a].back();
            }
        };

        std::priority_queue<ptrdiff_t, std::vector<ptrdiff_t>, PatienceSortPileCompare> heap(PatienceSortPileCompare{ &piles });
        for (ptrdiff_t p = 0; p < static_cast<ptrdiff_t>(piles.size()); ++p) {
            heap.push(p);
        }

        for (ptrdiff_t c = 0; c < length; ++c) {
            ptrdiff_t p = heap.top();
            heap.pop();

            data_[c] = piles[p].back();
            piles[p].pop_back();

            if (!piles[p].empty()) {
                heap.push(p);
            }
        }
    }

    // ClassicTreeSort.traverse + runSort: unbalanced BST stored in two index arrays
    // (index 0 doubles as the root and as the "no child" marker, as in the Java code).
    template<class T = int>
    void ClassicTreeSort(std::vector<T>& data_) {
        ptrdiff_t n = static_cast<ptrdiff_t>(data_.size());
        if (n < 2) {
            return;
        }

        std::vector<ptrdiff_t> lower(n, 0);
        std::vector<ptrdiff_t> upper(n, 0);

        for (ptrdiff_t i = 1; i < n; ++i) {
            NSortHelpers::MarkArray(2, data_, i);
            ptrdiff_t c = 0;

            while (true) {
                NSortHelpers::MarkArray(1, data_, c);

                std::vector<ptrdiff_t>& next = (data_[i] < data_[c]) ? lower : upper;

                if (next[c] == 0) {
                    next[c] = i;
                    break;
                }
                else {
                    c = next[c];
                }
            }
        }

        // in-order traversal into a temporary array (explicit stack: the tree can be as deep as n)
        std::vector<T> temp(n);
        ptrdiff_t idx = 0;
        std::vector<ptrdiff_t> stack;
        stack.push_back(0);

        while (!stack.empty()) {
            ptrdiff_t r = stack.back();
            stack.pop_back();

            if (r < 0) { // after the left subtree: emit this node
                ptrdiff_t node = -r - 1;
                NSortHelpers::MarkArray(1, data_, node);
                temp[idx] = data_[node];
                ++idx;
            }
            else {
                NSortHelpers::MarkArray(1, data_, r);
                if (upper[r] != 0) {
                    stack.push_back(upper[r]);
                }
                stack.push_back(-r - 1);
                if (lower[r] != 0) {
                    stack.push_back(lower[r]);
                }
            }
        }

        for (ptrdiff_t i = 0; i < n; ++i) {
            data_[i] = temp[i];
        }
    }

    // TreeSort: unbalanced BST of array indices (node 0 is the NULL node).
    template<class T>
    struct TreeSortTree {
        struct Node {
            ptrdiff_t pointer = -1;
            ptrdiff_t left = 0;
            ptrdiff_t right = 0;
        };

        std::vector<Node> nodes;
        std::vector<T>& data_;

        explicit TreeSortTree(std::vector<T>& data) : nodes(1), data_(data) {}

        ptrdiff_t NewNode(ptrdiff_t pointer) {
            nodes.push_back(Node{ pointer, 0, 0 });
            return static_cast<ptrdiff_t>(nodes.size()) - 1;
        }

        // TreeSort.Node.add, written iteratively (the Java recursion is O(n) deep for sorted data).
        ptrdiff_t Add(ptrdiff_t root, ptrdiff_t addPointer) {
            if (root == 0) {
                return NewNode(addPointer);
            }

            ptrdiff_t c = root;
            while (true) {
                NSortHelpers::MarkArray(2, data_, nodes[c].pointer);

                if (data_[addPointer] < data_[nodes[c].pointer]) { // equality goes right: stable
                    if (nodes[c].left == 0) {
                        nodes[c].left = NewNode(addPointer);
                        break;
                    }
                    c = nodes[c].left;
                }
                else {
                    if (nodes[c].right == 0) {
                        nodes[c].right = NewNode(addPointer);
                        break;
                    }
                    c = nodes[c].right;
                }
            }
            return root;
        }

        // TreeSort.Node.writeToArray with an explicit stack (in-order, one value per array slot).
        void WriteToArray(ptrdiff_t root, std::vector<T>& tempArray) {
            ptrdiff_t idx = 0;
            std::vector<ptrdiff_t> stack;
            stack.push_back(root);

            while (!stack.empty()) {
                ptrdiff_t r = stack.back();
                stack.pop_back();

                if (r < 0) {
                    ptrdiff_t node = -r - 1;
                    NSortHelpers::MarkArray(1, data_, nodes[node].pointer);
                    tempArray[idx] = data_[nodes[node].pointer];
                    ++idx;
                }
                else if (r != 0) {
                    if (nodes[r].right != 0) {
                        stack.push_back(nodes[r].right);
                    }
                    stack.push_back(-r - 1);
                    if (nodes[r].left != 0) {
                        stack.push_back(nodes[r].left);
                    }
                }
            }
        }
    };

    // TreeSort.runSort.
    template<class T = int>
    void TreeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        TreeSortTree<T> tree(data_);
        ptrdiff_t root = 0;

        for (ptrdiff_t i = 0; i < length; ++i) {
            NSortHelpers::MarkArray(1, data_, i);
            root = tree.Add(root, i);
        }

        std::vector<T> tempArray(length);
        tree.WriteToArray(root, tempArray);

        for (ptrdiff_t i = 0; i < length; ++i) {
            data_[i] = tempArray[i];
        }
    }

    // AATreeSort: AA tree of array indices (node 0 is the NULL node, its level is -1).
    template<class T>
    struct AATreeSortTree {
        struct Node {
            ptrdiff_t pointer = -1;
            ptrdiff_t left = 0;
            ptrdiff_t right = 0;
            int level = -1;
        };

        std::vector<Node> nodes;
        std::vector<T>& data_;

        explicit AATreeSortTree(std::vector<T>& data) : nodes(1), data_(data) {}

        ptrdiff_t NewNode(ptrdiff_t pointer) {
            nodes.push_back(Node{ pointer, 0, 0, 0 });
            return static_cast<ptrdiff_t>(nodes.size()) - 1;
        }

        // AATreeSort.Node.skew (a single right rotation).
        ptrdiff_t Skew(ptrdiff_t r) {
            ptrdiff_t l = nodes[r].left;

            NSortHelpers::MarkArray(3, data_, nodes[r].pointer);
            NSortHelpers::MarkArray(4, data_, nodes[l].pointer);

            nodes[r].left = nodes[l].right;
            nodes[l].right = r;
            return l;
        }

        // AATreeSort.Node.split (a single left rotation).
        ptrdiff_t Split(ptrdiff_t r) {
            ptrdiff_t rr = nodes[r].right;

            NSortHelpers::MarkArray(3, data_, nodes[r].pointer);
            NSortHelpers::MarkArray(4, data_, nodes[rr].pointer);

            nodes[r].right = nodes[rr].left;
            nodes[rr].left = r;
            nodes[rr].level++;
            return rr;
        }

        // AATreeSort.Node.add (recursion depth is O(log n)).
        ptrdiff_t Add(ptrdiff_t r, ptrdiff_t addPointer) {
            if (r == 0) {
                return NewNode(addPointer);
            }

            NSortHelpers::MarkArray(2, data_, nodes[r].pointer);

            if (data_[addPointer] < data_[nodes[r].pointer]) { // equality goes right: stable
                nodes[r].left = Add(nodes[r].left, addPointer);

                NSortHelpers::MarkArray(2, data_, nodes[r].pointer);

                if (nodes[nodes[r].left].level == nodes[r].level) {
                    if (nodes[r].level != nodes[nodes[r].right].level) {
                        return Skew(r);
                    }
                    // a skew immediately followed by a split can be skipped (the Java optimization)
                    nodes[r].level++;
                    return r;
                }
                return r;
            }

            nodes[r].right = Add(nodes[r].right, addPointer);

            NSortHelpers::MarkArray(2, data_, nodes[r].pointer);

            if (nodes[nodes[nodes[r].right].right].level == nodes[r].level) {
                return Split(r);
            }
            return r;
        }

        // AATreeSort.Node.writeToArray with an explicit stack.
        void WriteToArray(ptrdiff_t root, std::vector<T>& tempArray) {
            ptrdiff_t idx = 0;
            std::vector<ptrdiff_t> stack;
            stack.push_back(root);

            while (!stack.empty()) {
                ptrdiff_t r = stack.back();
                stack.pop_back();

                if (r < 0) {
                    ptrdiff_t node = -r - 1;
                    NSortHelpers::MarkArray(1, data_, nodes[node].pointer);
                    tempArray[idx] = data_[nodes[node].pointer];
                    ++idx;
                }
                else if (r != 0) {
                    if (nodes[r].right != 0) {
                        stack.push_back(nodes[r].right);
                    }
                    stack.push_back(-r - 1);
                    if (nodes[r].left != 0) {
                        stack.push_back(nodes[r].left);
                    }
                }
            }
        }
    };

    // AATreeSort.runSort.
    template<class T = int>
    void AATreeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        AATreeSortTree<T> tree(data_);
        ptrdiff_t root = 0;

        for (ptrdiff_t i = 0; i < length; ++i) {
            NSortHelpers::MarkArray(1, data_, i);
            root = tree.Add(root, i);
        }

        std::vector<T> tempArray(length);
        tree.WriteToArray(root, tempArray);

        for (ptrdiff_t i = 0; i < length; ++i) {
            data_[i] = tempArray[i];
        }
    }

    // AVLTreeSort: AVL tree of array indices (node 0 is the NULL node, balance 0).
    template<class T>
    struct AVLTreeSortTree {
        struct Node {
            ptrdiff_t pointer = -1;
            ptrdiff_t left = 0;
            ptrdiff_t right = 0;
            int balance = 0; // height of the right subtree minus the height of the left subtree
        };

        struct AddResult {
            ptrdiff_t node;
            bool heightChange;
        };

        std::vector<Node> nodes;
        std::vector<T>& data_;

        explicit AVLTreeSortTree(std::vector<T>& data) : nodes(1), data_(data) {}

        ptrdiff_t NewNode(ptrdiff_t pointer) {
            nodes.push_back(Node{ pointer, 0, 0, 0 });
            return static_cast<ptrdiff_t>(nodes.size()) - 1;
        }

        // AVLTreeSort.Node.singleRotateRight.
        ptrdiff_t SingleRotateRight(ptrdiff_t r) {
            ptrdiff_t b = nodes[r].left;

            NSortHelpers::MarkArray(3, data_, nodes[r].pointer);
            NSortHelpers::MarkArray(4, data_, nodes[b].pointer);

            nodes[r].left = nodes[b].right;
            nodes[b].right = r;

            nodes[r].balance = 0;
            nodes[b].balance = 0;
            return b;
        }

        // AVLTreeSort.Node.singleRotateLeft.
        ptrdiff_t SingleRotateLeft(ptrdiff_t r) {
            ptrdiff_t b = nodes[r].right;

            NSortHelpers::MarkArray(3, data_, nodes[r].pointer);
            NSortHelpers::MarkArray(4, data_, nodes[b].pointer);

            nodes[r].right = nodes[b].left;
            nodes[b].left = r;

            nodes[r].balance = 0;
            nodes[b].balance = 0;
            return b;
        }

        // AVLTreeSort.Node.doubleRotateRight (left-right rotation).
        ptrdiff_t DoubleRotateRight(ptrdiff_t r) {
            int oldBBalance = nodes[nodes[nodes[r].left].right].balance;

            ptrdiff_t newLeft = SingleRotateLeft(nodes[r].left);
            nodes[r].left = newLeft;

            ptrdiff_t b = SingleRotateRight(r);

            if (oldBBalance == -1) {
                nodes[nodes[b].right].balance = 1;
            }
            if (oldBBalance == 1) {
                nodes[nodes[b].left].balance = -1;
            }
            return b;
        }

        // AVLTreeSort.Node.doubleRotateLeft (right-left rotation).
        ptrdiff_t DoubleRotateLeft(ptrdiff_t r) {
            int oldBBalance = nodes[nodes[nodes[r].right].left].balance;

            ptrdiff_t newRight = SingleRotateRight(nodes[r].right);
            nodes[r].right = newRight;

            ptrdiff_t b = SingleRotateLeft(r);

            if (oldBBalance == -1) {
                nodes[nodes[b].right].balance = 1;
            }
            if (oldBBalance == 1) {
                nodes[nodes[b].left].balance = -1;
            }
            return b;
        }

        // AVLTreeSort.Node.heightChangeLeft.
        AddResult HeightChangeLeft(ptrdiff_t r) {
            if (nodes[r].balance != -1) {
                nodes[r].balance--;
                return AddResult{ r, nodes[r].balance == -1 };
            }
            if (nodes[nodes[r].left].balance == -1) {
                return AddResult{ SingleRotateRight(r), false };
            }
            return AddResult{ DoubleRotateRight(r), false };
        }

        // AVLTreeSort.Node.heightChangeRight.
        AddResult HeightChangeRight(ptrdiff_t r) {
            if (nodes[r].balance != 1) {
                nodes[r].balance++;
                return AddResult{ r, nodes[r].balance == 1 };
            }
            if (nodes[nodes[r].right].balance == 1) {
                return AddResult{ SingleRotateLeft(r), false };
            }
            return AddResult{ DoubleRotateLeft(r), false };
        }

        // AVLTreeSort.Node.add (recursion depth is O(log n)).
        AddResult Add(ptrdiff_t r, ptrdiff_t addPointer) {
            if (r == 0) {
                return AddResult{ NewNode(addPointer), true };
            }

            NSortHelpers::MarkArray(2, data_, nodes[r].pointer);

            if (data_[addPointer] < data_[nodes[r].pointer]) { // equality goes right: stable
                AddResult container = Add(nodes[r].left, addPointer);

                NSortHelpers::MarkArray(2, data_, nodes[r].pointer);
                nodes[r].left = container.node;

                if (container.heightChange) {
                    return HeightChangeLeft(r);
                }
                return AddResult{ r, false };
            }

            AddResult container = Add(nodes[r].right, addPointer);

            NSortHelpers::MarkArray(2, data_, nodes[r].pointer);
            nodes[r].right = container.node;

            if (container.heightChange) {
                return HeightChangeRight(r);
            }
            return AddResult{ r, false };
        }

        // AVLTreeSort.Node.writeToArray with an explicit stack.
        void WriteToArray(ptrdiff_t root, std::vector<T>& tempArray) {
            ptrdiff_t idx = 0;
            std::vector<ptrdiff_t> stack;
            stack.push_back(root);

            while (!stack.empty()) {
                ptrdiff_t r = stack.back();
                stack.pop_back();

                if (r < 0) {
                    ptrdiff_t node = -r - 1;
                    NSortHelpers::MarkArray(1, data_, nodes[node].pointer);
                    tempArray[idx] = data_[nodes[node].pointer];
                    ++idx;
                }
                else if (r != 0) {
                    if (nodes[r].right != 0) {
                        stack.push_back(nodes[r].right);
                    }
                    stack.push_back(-r - 1);
                    if (nodes[r].left != 0) {
                        stack.push_back(nodes[r].left);
                    }
                }
            }
        }
    };

    // AVLTreeSort.runSort.
    template<class T = int>
    void AVLTreeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        AVLTreeSortTree<T> tree(data_);
        ptrdiff_t root = 0;

        for (ptrdiff_t i = 0; i < length; ++i) {
            NSortHelpers::MarkArray(1, data_, i);
            root = tree.Add(root, i).node;
        }

        std::vector<T> tempArray(length);
        tree.WriteToArray(root, tempArray);

        for (ptrdiff_t i = 0; i < length; ++i) {
            data_[i] = tempArray[i];
        }
    }

    // RedBlackTreeSort: red-black tree of array indices (node 0 is the NULL node, black).
    template<class T>
    struct RedBlackTreeSortTree {
        struct Node {
            ptrdiff_t pointer = -1;
            ptrdiff_t left = 0;
            ptrdiff_t right = 0;
            bool isRed = false;
        };

        struct AddResult {
            ptrdiff_t node;
            bool needsFix;
        };

        std::vector<Node> nodes;
        std::vector<T>& data_;

        explicit RedBlackTreeSortTree(std::vector<T>& data) : nodes(1), data_(data) {}

        ptrdiff_t NewNode(ptrdiff_t pointer) {
            nodes.push_back(Node{ pointer, 0, 0, true });
            return static_cast<ptrdiff_t>(nodes.size()) - 1;
        }

        // RedBlackTreeSort.Node.singleRotateRight.
        ptrdiff_t SingleRotateRight(ptrdiff_t r) {
            ptrdiff_t b = nodes[r].left;

            NSortHelpers::MarkArray(3, data_, nodes[r].pointer);
            NSortHelpers::MarkArray(4, data_, nodes[b].pointer);

            nodes[r].left = nodes[b].right;
            nodes[b].right = r;

            nodes[b].isRed = false;
            nodes[r].isRed = true;
            return b;
        }

        // RedBlackTreeSort.Node.singleRotateLeft.
        ptrdiff_t SingleRotateLeft(ptrdiff_t r) {
            ptrdiff_t b = nodes[r].right;

            NSortHelpers::MarkArray(3, data_, nodes[r].pointer);
            NSortHelpers::MarkArray(4, data_, nodes[b].pointer);

            nodes[r].right = nodes[b].left;
            nodes[b].left = r;

            nodes[b].isRed = false;
            nodes[r].isRed = true;
            return b;
        }

        // RedBlackTreeSort.Node.doubleRotateRight (left-right rotation).
        ptrdiff_t DoubleRotateRight(ptrdiff_t r) {
            ptrdiff_t newLeft = SingleRotateLeft(nodes[r].left);
            nodes[r].left = newLeft;

            return SingleRotateRight(r);
        }

        // RedBlackTreeSort.Node.doubleRotateLeft (right-left rotation).
        ptrdiff_t DoubleRotateLeft(ptrdiff_t r) {
            ptrdiff_t newRight = SingleRotateRight(nodes[r].right);
            nodes[r].right = newRight;

            return SingleRotateLeft(r);
        }

        // RedBlackTreeSort.Node.add (recursion depth is O(log n)).
        AddResult Add(ptrdiff_t r, ptrdiff_t addPointer) {
            if (r == 0) {
                return AddResult{ NewNode(addPointer), false };
            }

            NSortHelpers::MarkArray(2, data_, nodes[r].pointer);

            // top-down recoloring: prevents cascading rotations without losing black balance
            if (!nodes[r].isRed && nodes[nodes[r].left].isRed && nodes[nodes[r].right].isRed) {
                NSortHelpers::MarkArray(3, data_, nodes[nodes[r].left].pointer);
                NSortHelpers::MarkArray(4, data_, nodes[nodes[r].right].pointer);

                nodes[r].isRed = true;
                nodes[nodes[r].left].isRed = false;
                nodes[nodes[r].right].isRed = false;
            }

            if (data_[addPointer] < data_[nodes[r].pointer]) { // equality goes right: stable
                AddResult container = Add(nodes[r].left, addPointer);

                NSortHelpers::MarkArray(2, data_, nodes[r].pointer);
                nodes[r].left = container.node;

                if (container.needsFix) {
                    if (nodes[nodes[nodes[r].left].left].isRed) {
                        return AddResult{ SingleRotateRight(r), false };
                    }
                    return AddResult{ DoubleRotateRight(r), false };
                }
                return AddResult{ r, nodes[r].isRed && nodes[nodes[r].left].isRed };
            }

            AddResult container = Add(nodes[r].right, addPointer);

            NSortHelpers::MarkArray(2, data_, nodes[r].pointer);
            nodes[r].right = container.node;

            if (container.needsFix) {
                if (nodes[nodes[nodes[r].right].right].isRed) {
                    return AddResult{ SingleRotateLeft(r), false };
                }
                return AddResult{ DoubleRotateLeft(r), false };
            }
            return AddResult{ r, nodes[r].isRed && nodes[nodes[r].right].isRed };
        }

        // RedBlackTreeSort.Node.writeToArray with an explicit stack.
        void WriteToArray(ptrdiff_t root, std::vector<T>& tempArray) {
            ptrdiff_t idx = 0;
            std::vector<ptrdiff_t> stack;
            stack.push_back(root);

            while (!stack.empty()) {
                ptrdiff_t r = stack.back();
                stack.pop_back();

                if (r < 0) {
                    ptrdiff_t node = -r - 1;
                    NSortHelpers::MarkArray(1, data_, nodes[node].pointer);
                    tempArray[idx] = data_[nodes[node].pointer];
                    ++idx;
                }
                else if (r != 0) {
                    if (nodes[r].right != 0) {
                        stack.push_back(nodes[r].right);
                    }
                    stack.push_back(-r - 1);
                    if (nodes[r].left != 0) {
                        stack.push_back(nodes[r].left);
                    }
                }
            }
        }
    };

    // RedBlackTreeSort.runSort.
    template<class T = int>
    void RedBlackTreeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        RedBlackTreeSortTree<T> tree(data_);
        ptrdiff_t root = 0;

        for (ptrdiff_t i = 0; i < length; ++i) {
            NSortHelpers::MarkArray(1, data_, i);
            root = tree.Add(root, i).node;

            NSortHelpers::MarkArray(2, data_, tree.nodes[root].pointer);
            tree.nodes[root].isRed = false; // the root of a red-black tree is always black
        }

        std::vector<T> tempArray(length);
        tree.WriteToArray(root, tempArray);

        for (ptrdiff_t i = 0; i < length; ++i) {
            data_[i] = tempArray[i];
        }
    }

    // SplaySort: splay tree of VALUES (the Java class stores the array values themselves).
    // The Java recursion is replaced by an equivalent iterative bottom-up splay, so a
    // degenerate (O(n) deep) tree cannot overflow the stack.
    template<class T>
    struct SplaySortTree {
        struct Node {
            T key{};
            ptrdiff_t left = 0;
            ptrdiff_t right = 0;
            ptrdiff_t parent = 0;
        };

        std::vector<Node> nodes;
        ptrdiff_t root = 0;

        SplaySortTree() : nodes(1) {}

        ptrdiff_t NewNode(const T& key) {
            nodes.push_back(Node{ key, 0, 0, 0 });
            return static_cast<ptrdiff_t>(nodes.size()) - 1;
        }

        // SplaySort.leftRotate.
        void RotateLeft(ptrdiff_t x) {
            ptrdiff_t y = nodes[x].right;

            nodes[x].right = nodes[y].left;
            if (nodes[y].left != 0) {
                nodes[nodes[y].left].parent = x;
            }
            nodes[y].parent = nodes[x].parent;
            if (nodes[x].parent != 0) {
                if (nodes[nodes[x].parent].left == x) {
                    nodes[nodes[x].parent].left = y;
                }
                else {
                    nodes[nodes[x].parent].right = y;
                }
            }
            nodes[y].left = x;
            nodes[x].parent = y;
        }

        // SplaySort.rightRotate.
        void RotateRight(ptrdiff_t x) {
            ptrdiff_t y = nodes[x].left;

            nodes[x].left = nodes[y].right;
            if (nodes[y].right != 0) {
                nodes[nodes[y].right].parent = x;
            }
            nodes[y].parent = nodes[x].parent;
            if (nodes[x].parent != 0) {
                if (nodes[nodes[x].parent].left == x) {
                    nodes[nodes[x].parent].left = y;
                }
                else {
                    nodes[nodes[x].parent].right = y;
                }
            }
            nodes[y].right = x;
            nodes[x].parent = y;
        }

        // SplaySort.splay: descend to the last accessed node, then splay it to the root.
        ptrdiff_t Splay(ptrdiff_t r, const T& key) {
            if (r == 0) {
                return 0;
            }

            ptrdiff_t cur = r;
            while (true) {
                if (nodes[cur].key > key) {
                    if (nodes[cur].left == 0) {
                        break;
                    }
                    cur = nodes[cur].left;
                }
                else {
                    if (nodes[cur].right == 0) {
                        break;
                    }
                    cur = nodes[cur].right;
                }
            }

            while (nodes[cur].parent != 0) {
                ptrdiff_t p = nodes[cur].parent;
                ptrdiff_t g = nodes[p].parent;

                if (g == 0) {
                    if (nodes[p].left == cur) {
                        RotateRight(p);
                    }
                    else {
                        RotateLeft(p);
                    }
                }
                else if ((nodes[g].left == p) == (nodes[p].left == cur)) {
                    // zig-zig: rotate the grandparent first, then the parent
                    if (nodes[g].left == p) {
                        RotateRight(g);
                        RotateRight(p);
                    }
                    else {
                        RotateLeft(g);
                        RotateLeft(p);
                    }
                }
                else {
                    // zig-zag: rotate the parent first (bringing cur up), then the grandparent
                    if (nodes[g].left == p) {
                        RotateLeft(p);
                        RotateRight(g);
                    }
                    else {
                        RotateRight(p);
                        RotateLeft(g);
                    }
                }
            }
            return cur;
        }

        // SplaySort.insertRec (already iterative apart from the splay).
        void Insert(const T& key) {
            if (root == 0) {
                root = NewNode(key);
                return;
            }

            ptrdiff_t r = Splay(root, key);
            ptrdiff_t n = NewNode(key);

            if (nodes[r].key > key) {
                nodes[n].right = r;
                nodes[n].left = nodes[r].left;
                if (nodes[n].left != 0) {
                    nodes[nodes[n].left].parent = n;
                }
                nodes[r].left = 0;
            }
            else {
                nodes[n].left = r;
                nodes[n].right = nodes[r].right;
                if (nodes[n].right != 0) {
                    nodes[nodes[n].right].parent = n;
                }
                nodes[r].right = 0;
            }
            nodes[r].parent = n;
            root = n;
        }

        // SplaySort.traverseRec with an explicit stack (the splay tree can be O(n) deep).
        void Traverse(std::vector<T>& data_) {
            ptrdiff_t index = 0;
            std::vector<ptrdiff_t> stack;
            stack.push_back(root);

            while (!stack.empty()) {
                ptrdiff_t r = stack.back();
                stack.pop_back();

                if (r < 0) {
                    ptrdiff_t node = -r - 1;
                    data_[index] = nodes[node].key;
                    ++index;
                }
                else if (r != 0) {
                    if (nodes[r].right != 0) {
                        stack.push_back(nodes[r].right);
                    }
                    stack.push_back(-r - 1);
                    if (nodes[r].left != 0) {
                        stack.push_back(nodes[r].left);
                    }
                }
            }
        }
    };

    // SplaySort.runSort.
    template<class T = int>
    void SplaySort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        SplaySortTree<T> tree;

        for (ptrdiff_t i = 0; i < length; ++i) {
            NSortHelpers::MarkArray(2, data_, i);
            tree.Insert(data_[i]);
            NSortHelpers::MarkArray(1, data_, 0); // Java's treeWrite(..., 1) mark
        }

        tree.Traverse(data_);
    }

    // HanoiSort: state of the tower-of-hanoi influenced sort. The main array is the first
    // peg (with sp as its stack pointer), stack2/stack3 are the two auxiliary pegs.
    template<class T>
    struct HanoiSortState {
        std::vector<T>& data_;
        ptrdiff_t length = 0;
        std::vector<T> stack2;
        std::vector<T> stack3;
        ptrdiff_t sp = 0;       // stack pointer of the main array
        ptrdiff_t unsorted = 0; // where the unsorted portion of the main array begins
        T target{};
        ptrdiff_t targetMoves = 0;

        explicit HanoiSortState(std::vector<T>& data) : data_(data) {}

        // HanoiSort.moveFromMain (returns the number of consecutive duplicates moved).
        ptrdiff_t MoveFromMain(std::vector<T>& stack, bool checkUnsorted) {
            ptrdiff_t duplicates = 1;

            stack.push_back(data_[sp]);
            ++sp;
            NSortHelpers::MarkArray(1, data_, sp);

            bool endOnLength = (sp >= length) || (checkUnsorted && sp >= unsorted);
            while (!endOnLength && !(data_[sp] < stack.back()) && !(stack.back() < data_[sp])) {
                ++duplicates;
                stack.push_back(data_[sp]);
                ++sp;
                NSortHelpers::MarkArray(1, data_, sp);
                endOnLength = (sp >= length) || (checkUnsorted && sp >= unsorted);
            }
            return duplicates;
        }

        // HanoiSort.moveToMain.
        void MoveToMain(std::vector<T>& stack) {
            --sp;
            NSortHelpers::MarkArray(1, data_, sp);
            data_[sp] = stack.back();
            stack.pop_back();

            while (!stack.empty() && !(stack.back() < data_[sp]) && !(data_[sp] < stack.back())) {
                --sp;
                NSortHelpers::MarkArray(1, data_, sp);
                data_[sp] = stack.back();
                stack.pop_back();
            }
        }

        // HanoiSort.moveBetweenStacks.
        void MoveBetweenStacks(std::vector<T>& from, std::vector<T>& to) {
            to.push_back(from.back());
            from.pop_back();

            while (!from.empty() && !(from.back() < to.back()) && !(to.back() < from.back())) {
                to.push_back(from.back());
                from.pop_back();
            }
        }

        // HanoiSort.validNumberMoves (true when moves is of the form (2^n) - 1).
        bool ValidNumberMoves(ptrdiff_t moves) const {
            while (moves != 0) {
                if (moves % 2 == 0) {
                    return false;
                }
                moves /= 2;
            }
            return true;
        }

        // HanoiSort.getHeight (log_2 of the number of moves plus one).
        ptrdiff_t GetHeight(ptrdiff_t movesPlus1) const {
            ptrdiff_t height = 0;
            while (movesPlus1 > 1) {
                movesPlus1 /= 2;
                ++height;
            }
            return height;
        }

        // HanoiSort.endConMet.
        bool EndConMet(int endCon, ptrdiff_t moves) const {
            if (!ValidNumberMoves(moves)) {
                return false;
            }

            switch (endCon) {
            case 1: return stack2.empty() || !(stack2.back() < target); // target <= stack2.peek()
            case 2: return moves == targetMoves;
            case 3: return stack2.empty();
            default: throw WideError(L"HanoiSort: invalid end condition");
            }
        }

        // HanoiSort.hanoi (iterative: the required recursion depth is unknown).
        ptrdiff_t Hanoi(int startStack, bool goRight, int endCon) {
            ptrdiff_t moves = 0;
            int minPoleLoc = startStack;

            if (!EndConMet(endCon, moves)) {
                moves++;
                switch (minPoleLoc) {
                case 1:
                    if (goRight) {
                        MoveFromMain(stack2, true);
                        minPoleLoc = 2;
                    }
                    else {
                        MoveFromMain(stack3, true);
                        minPoleLoc = 3;
                    }
                    break;
                case 2:
                    if (goRight) {
                        MoveBetweenStacks(stack2, stack3);
                        minPoleLoc = 3;
                    }
                    else {
                        MoveToMain(stack2);
                        minPoleLoc = 1;
                    }
                    break;
                case 3:
                    if (goRight) {
                        MoveToMain(stack3);
                        minPoleLoc = 1;
                    }
                    else {
                        MoveBetweenStacks(stack3, stack2);
                        minPoleLoc = 2;
                    }
                    break;
                }
            }

            while (!EndConMet(endCon, moves)) {
                moves += 2;
                switch (minPoleLoc) {
                case 1:
                    if (!stack2.empty() &&
                        (stack3.empty() || stack2.back() < stack3.back())) {
                        MoveBetweenStacks(stack2, stack3);
                    }
                    else {
                        MoveBetweenStacks(stack3, stack2);
                    }
                    if (goRight) {
                        MoveFromMain(stack2, true);
                        minPoleLoc = 2;
                    }
                    else {
                        MoveFromMain(stack3, true);
                        minPoleLoc = 3;
                    }
                    break;
                case 2:
                    if (stack3.empty() ||
                        (sp < unsorted && data_[sp] < stack3.back())) {
                        MoveFromMain(stack3, true);
                    }
                    else {
                        MoveToMain(stack3);
                    }
                    if (goRight) {
                        MoveBetweenStacks(stack2, stack3);
                        minPoleLoc = 3;
                    }
                    else {
                        MoveToMain(stack2);
                        minPoleLoc = 1;
                    }
                    break;
                case 3:
                    if (stack2.empty() ||
                        (sp < unsorted && data_[sp] < stack2.back())) {
                        MoveFromMain(stack2, true);
                    }
                    else {
                        MoveToMain(stack2);
                    }
                    if (goRight) {
                        MoveToMain(stack3);
                        minPoleLoc = 1;
                    }
                    else {
                        MoveBetweenStacks(stack3, stack2);
                        minPoleLoc = 2;
                    }
                    break;
                }
            }
            return moves;
        }

        // HanoiSort.removeFromMainStack.
        void RemoveFromMainStack() {
            NSortHelpers::MarkArray(2, data_, sp);
            target = data_[sp];

            ptrdiff_t moves = Hanoi(2, true, 1);
            ptrdiff_t height = GetHeight(moves + 1);
            targetMoves = moves;
            bool evenHeight = height % 2 == 0;

            NSortHelpers::MarkArray(1, data_, sp);

            if (evenHeight) { // move smaller elements to stack3, if necessary
                Hanoi(1, true, 2);
            }
            unsorted += MoveFromMain(stack2, false); // move the next element(s) to stack2
            Hanoi(3, evenHeight, 2);                 // move the smaller elements back to stack2
        }

        // HanoiSort.returnToMainStack.
        void ReturnToMainStack() {
            ptrdiff_t moves = Hanoi(2, true, 3);
            ptrdiff_t height = GetHeight(moves + 1);

            if (height % 2 == 1) { // odd height case: moved to stack3
                targetMoves = moves;
                Hanoi(3, true, 2);
            } // in the even case it is already on the main stack
        }
    };

    // HanoiSort.runSort.
    template<class T = int>
    void HanoiSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        HanoiSortState<T> state(data_);
        state.length = length;

        while (state.unsorted < length) {
            state.RemoveFromMainStack();
        }
        state.ReturnToMainStack();
    }

} // namespace NVisualSort::NSortAlgorithms
