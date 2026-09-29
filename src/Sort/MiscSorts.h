#pragma once
// Ports of the ArrayV (Java, MIT) sorting classes of the "misc" category,
// https://github.com/Gaming32/ArrayV (package io.github.arrayv.sorts.misc).
//   PancakeSort, BurntPancakeSort, PancakeInsertionSort, StalinSort
//
// ASCII ONLY: do not put non-ASCII characters in this file. MSVC parses sources
// in the system code page (GBK); Chinese text would need \uXXXX escapes inside
// a L"..." wide string.
//
// This header is #included from Sort.h after SortHelpers.h, and it is also
// compiled standalone by the check TU, so it pulls in what it needs itself.
#include "SortHelpers.h"

#include <cstddef>
#include <limits>
#include <vector>

namespace NVisualSort::NSortAlgorithms {

    // ==== Pancake Sort ====

    // PancakeSort.sorted: checks whether data_[0 .. length] is non-decreasing
    // (the Java loop compares the pairs (0,1), (1,2), ..., (length-1, length)).
    template<class T>
    bool PancakeSortSorted(std::vector<T>& data_, ptrdiff_t length) {
        ptrdiff_t size = static_cast<ptrdiff_t>(data_.size());

        for (ptrdiff_t i = 0; i < length; ++i) {

            if (i + 1 >= size) {
                break; // defensive: callers always pass length <= size - 1
            }
            if (data_[i] > data_[i + 1]) return false;
        }
        return true;
    }

    // PancakeSort.findMax: index of the leftmost maximum of data_[0 .. end]
    // (Java starts from Integer.MIN_VALUE and only replaces on a strict >).
    template<class T>
    ptrdiff_t PancakeSortFindMax(std::vector<T>& data_, ptrdiff_t end) {
        ptrdiff_t index = 0;
        int max = (std::numeric_limits<int>::min)(); // Integer.MIN_VALUE (parenthesized: Windows.h min macro)

        for (ptrdiff_t i = 0; i <= end; ++i) {

            if (NSortHelpers::CompareValues(static_cast<int>(data_[i]), max) == 1) {
                max = static_cast<int>(data_[i]);
                index = i;
            }
        }
        return index;
    }

    // Pancake: port of PancakeSort.runSort. For every prefix end i (from the
    // back), if data_[0 .. i] is not sorted, the maximum of that prefix is
    // flipped to position i with one or two prefix reversals.
    template<class T = int>
    void PancakeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) return;

        for (ptrdiff_t i = length - 1; i >= 0; --i) {
            if (!PancakeSortSorted(data_, i)) {
                ptrdiff_t index = PancakeSortFindMax(data_, i);

                if (index == 0) {
                    NSortHelpers::reverseRange(data_, 0, i + 1); // reversal(0, i)
                }
                else if (index != i) {
                    NSortHelpers::reverseRange(data_, 0, index + 1); // reversal(0, index)
                    NSortHelpers::reverseRange(data_, 0, i + 1);     // reversal(0, i)
                }
            }
            else break;
        }
    }

    // ==== Burnt Pancake Sort ====

    // Burnt Pancake: port of BurntPancakeSort.runSort. The burnt side of every
    // pancake is encoded by an extra reversal, so each element needs up to four
    // prefix flips: (0, max), (0, i), (0, i - 1) and (0, max - 1). The >= compare
    // picks the rightmost maximum, as in Java.
    template<class T = int>
    void BurntPancakeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) return;

        for (ptrdiff_t i = length - 1; i > 0; --i) {
            ptrdiff_t max = 0;

            for (ptrdiff_t j = max + 1; j <= i; ++j) {
                if (data_[j] >= data_[max]) max = j;
            }

            if (max != i) {
                NSortHelpers::reverseRange(data_, 0, max + 1); // reversal(0, max)
                NSortHelpers::reverseRange(data_, 0, i + 1);   // reversal(0, i)
                NSortHelpers::reverseRange(data_, 0, i);       // reversal(0, i - 1)
                NSortHelpers::reverseRange(data_, 0, max);     // reversal(0, max - 1)
            }
        }
    }

    // ==== Pancake Insertion Sort ====

    // PancakeInsertionSort.monoboundFw: Java's forward monobound binary search;
    // it searches the sorted part of a prefix that is currently flipped, hence
    // the comparisons against data_[end - mid] and the <= direction.
    template<class T>
    ptrdiff_t PancakeInsertionSortMonoboundFw(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, const T& value) {
        ptrdiff_t top = end - start;

        while (top > 1) {
            ptrdiff_t mid = top / 2;

            if (value <= data_[end - mid]) { // compareValueIndex(value, end - mid) <= 0
                end -= mid;
            }
            top -= mid;
        }

        if (value <= data_[end - 1]) {
            return end - 1;
        }
        return end;
    }

    // PancakeInsertionSort.monoboundBw: Java's backward monobound binary search
    // (data_[start + mid] > value means the insertion point is to the right).
    template<class T>
    ptrdiff_t PancakeInsertionSortMonoboundBw(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, const T& value) {
        ptrdiff_t top = end - start;

        while (top > 1) {
            ptrdiff_t mid = top / 2;

            if (data_[start + mid] > value) { // compareIndexValue(start + mid, value) > 0
                start += mid;
            }
            top -= mid;
        }

        if (data_[start] > value) {
            return start + 1;
        }
        return start;
    }

    // PancakeInsertionSort.front: sorts the first three items with a decision
    // tree and reports the current direction of the (flipped) sorted prefix.
    template<class T>
    bool PancakeInsertionSortFront(std::vector<T>& data_, ptrdiff_t length) {
        if (length < 2) {
            return false;
        }
        bool dir = true;
        if (data_[0] > data_[1]) { // compare(0, 1) > 0
            NSortHelpers::reverseRange(data_, 0, 2); // flip(1)
        }
        if (length > 2) {
            if (data_[1] > data_[2]) { // compare(1, 2) > 0
                if (data_[0] > data_[2]) { // compare(0, 2) > 0
                    NSortHelpers::reverseRange(data_, 0, 2); // flip(1)
                    return false;
                }
                else {
                    NSortHelpers::reverseRange(data_, 0, 3); // flip(2)
                    NSortHelpers::reverseRange(data_, 0, 2); // flip(1)
                }
                return false;
            }
            else {
                return true;
            }
        }
        return dir;
    }

    // Pancake Insertion: port of PancakeInsertionSort.runSort. Insertion sort
    // where the sorted prefix is kept flipped whenever that makes the insertion
    // cheaper; every move is a prefix reversal.
    template<class T = int>
    void PancakeInsertionSort(std::vector<T>& data_) {
        ptrdiff_t currentLength = static_cast<ptrdiff_t>(data_.size());
        if (currentLength < 2) return;

        bool dir = PancakeInsertionSortFront(data_, currentLength); // sort the first three items with a decision tree

        for (ptrdiff_t i = 3; i < currentLength; ++i) {
            if (dir) {
                if (data_[i - 1] <= data_[i]) { // compare(i - 1, i) <= 0
                    continue;
                }
                else if (data_[0] > data_[i]) { // compare(0, i) > 0
                    NSortHelpers::reverseRange(data_, 0, i); // flip(i - 1)
                    dir = !dir;
                }
                else {
                    T temp = data_[i]; // Java evaluates array[i] before the search
                    ptrdiff_t idx = PancakeInsertionSortMonoboundFw(data_, 0, i, temp);
                    NSortHelpers::reverseRange(data_, 0, i + 1); // flip(i)
                    ptrdiff_t end = i - idx;
                    NSortHelpers::reverseRange(data_, 0, end + 1); // flip(end)
                    NSortHelpers::reverseRange(data_, 0, end);     // flip(end - 1)
                    dir = !dir;
                }
            }
            else {
                if (data_[i - 1] > data_[i]) { // compare(i - 1, i) > 0
                    continue;
                }
                else if (data_[0] <= data_[i]) { // compare(0, i) <= 0
                    NSortHelpers::reverseRange(data_, 0, i); // flip(i - 1)
                    dir = !dir;
                }
                else {
                    T temp = data_[i]; // Java evaluates array[i] before the search
                    ptrdiff_t idx = PancakeInsertionSortMonoboundBw(data_, 0, i, temp);
                    NSortHelpers::reverseRange(data_, 0, i + 1); // flip(i)
                    ptrdiff_t end = i - idx;
                    NSortHelpers::reverseRange(data_, 0, end + 1); // flip(end)
                    NSortHelpers::reverseRange(data_, 0, end);     // flip(end - 1)
                    dir = !dir;
                }
            }
        }

        if (!dir) { // the array is reversed
            NSortHelpers::reverseRange(data_, 0, currentLength); // flip(currentLength - 1)
        }
    }

    // ==== Stalin Sort ====

    // Stalin: port of StalinSort.runSort. One left-to-right pass: every element
    // that is smaller than its left neighbour is overwritten with that neighbour,
    // so the array ends up holding the running maximum (i.e. non-decreasing).
    // Note this rewrites values, it does not delete or move elements.
    template<class T = int>
    void StalinSort(std::vector<T>& data_) {
        ptrdiff_t currentLength = static_cast<ptrdiff_t>(data_.size());
        if (currentLength < 2) return;

        for (ptrdiff_t i = 1; i < currentLength; ++i) {

            if (data_[i - 1] > data_[i]) {
                data_[i] = data_[i - 1];
            }
        }
    }

} // namespace NVisualSort::NSortAlgorithms
