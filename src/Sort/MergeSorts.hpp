#pragma once
// Ports of the ArrayV (Java, MIT) "merge" category sorting classes,
// https://github.com/Gaming32/ArrayV  (sources: src/main/java/io/github/arrayv/sorts/merge).
// VisualSort - merge sort family (19 classes of the ArrayV "Merge Sorts" package).
//
// ASCII ONLY: do not put non-ASCII characters in this file. MSVC parses sources in the
// system code page (GBK); use \uXXXX escapes if a wide string is ever needed.
//
// This header is #included from Sort.h after SortHelpers.h; it opens the shared namespace
// and closes it again at the end of the file.
//
// Naming: the Java class MergeSort already exists as a C++ sort in NSortAlgorithms, so it
// is ported as MergeSortJava and its menu entry is the shared merge-sort name with the
// "(ArrayV)" suffix (see D:\Temp\merge_register.txt for the display names). Every internal
// helper starts with the PascalCase name of the algorithm it belongs to.
#include "SortHelpers.hpp"
#include <thread>
#include <vector>
#include <cstddef>
#include <algorithm>

namespace NVisualSort::NSortAlgorithms {

    // ------------------------------------------------------------------
    // AndreySort (Andrey Astrelin's in-place merge sort)
    // ------------------------------------------------------------------

    // AndreySort.sort (selection sort used for short runs).
    template<class T>
    void AndreySortSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        while (b > 1) {
            ptrdiff_t k = 0;

            for (ptrdiff_t i = 1; i < b; ++i) {
                if (data_[a + k] > data_[a + i]) {
                    k = i;
                }
            }

            using std::swap;
            swap(data_[a], data_[a + k]);
            ++a;
            --b;
        }
    }

    // AndreySort.aswap
    template<class T>
    void AndreySortAswap(std::vector<T>& data_, ptrdiff_t arr1, ptrdiff_t arr2, ptrdiff_t l) {
        using std::swap;
        while (l-- > 0) {
            swap(data_[arr1++], data_[arr2++]);
        }
    }

    // AndreySort.backmerge: arr1(-l1..0] :merge: arr2(-l2..0] -> arr2(-l2..l1]
    template<class T>
    ptrdiff_t AndreySortBackmerge(std::vector<T>& data_, ptrdiff_t arr1, ptrdiff_t l1,
                                  ptrdiff_t arr2, ptrdiff_t l2) {
        ptrdiff_t arr0 = arr2 + l1;
        using std::swap;

        for (;;) {
            if (data_[arr1] > data_[arr2]) {
                swap(data_[arr1--], data_[arr0--]);
                if (--l1 == 0) {
                    return 0;
                }
            }
            else {
                swap(data_[arr2--], data_[arr0--]);
                if (--l2 == 0) {
                    break;
                }
            }
        }

        ptrdiff_t res = l1;
        do {
            swap(data_[arr1--], data_[arr0--]);
        } while (--l1 != 0);
        return res;
    }

    // AndreySort.rmerge (merge arr[p0..p0+l) by the buffer arr[p0+l..p0+l+r)).
    template<class T>
    void AndreySortRmerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t l, ptrdiff_t r) {
        for (ptrdiff_t i = 0; i < l; i += r) {
            // select smallest arr[p0+n*r]
            ptrdiff_t q = i;
            for (ptrdiff_t j = i + r; j < l; j += r) {
                if (data_[a + q] > data_[a + j]) {
                    q = j;
                }
            }
            if (q != i) {
                AndreySortAswap(data_, a + i, a + q, r);
            }
            if (i != 0) {
                AndreySortAswap(data_, a + l, a + i, r);
                AndreySortBackmerge(data_, a + (l + r - 1), r, a + (i - 1), r);
            }
        }
    }

    // AndreySort.rbnd
    inline ptrdiff_t AndreySortRbnd(ptrdiff_t len) {
        len = len / 2;
        ptrdiff_t k = 0;
        for (ptrdiff_t i = 1; i < len; i *= 2) {
            ++k;
        }
        len /= k;
        for (k = 1; k <= len; k *= 2) {
        }
        return k;
    }

    // AndreySort.msort
    template<class T>
    void AndreySortMsort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t len) {
        if (len < 12) {
            AndreySortSort(data_, a, len);
            return;
        }

        ptrdiff_t r = AndreySortRbnd(len);
        ptrdiff_t lr = (len / r - 1) * r;

        for (ptrdiff_t p = 2; p <= lr; p += 2) {
            if (data_[a + (p - 2)] > data_[a + (p - 1)]) {
                using std::swap;
                swap(data_[a + (p - 2)], data_[a + (p - 1)]);
            }
            if ((p & 2) != 0) {
                continue;
            }

            AndreySortAswap(data_, a + (p - 2), a + p, 2);

            ptrdiff_t m = len - p;
            ptrdiff_t q = 2;

            for (;;) {
                ptrdiff_t q0 = 2 * q;
                if (q0 > m || (p & q0) != 0) {
                    break;
                }
                AndreySortBackmerge(data_, a + (p - q - 1), q, a + (p + q - 1), q);
                q = q0;
            }

            AndreySortBackmerge(data_, a + (p + q - 1), q, a + (p - q - 1), q);
            ptrdiff_t q1 = q;
            q *= 2;

            while ((q & p) == 0) {
                q *= 2;
                AndreySortRmerge(data_, a + (p - q), q, q1);
            }
        }

        ptrdiff_t q1 = 0;
        for (ptrdiff_t q = r; q < lr; q *= 2) {
            if ((lr & q) != 0) {
                q1 += q;
                if (q1 != q) {
                    AndreySortRmerge(data_, a + (lr - q1), q1, r);
                }
            }
        }

        ptrdiff_t s = len - lr;
        AndreySortMsort(data_, a + lr, s);
        AndreySortAswap(data_, a, a + lr, s);
        s += AndreySortBackmerge(data_, a + (s - 1), s, a + (lr - 1), lr - s);
        AndreySortMsort(data_, a, s);
    }

    // AndreySort.runSort.
    template<class T = int>
    void AndreySort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }
        AndreySortMsort(data_, 0, length);
    }

    // ------------------------------------------------------------------
    // BlockSwapMergeSort
    // ------------------------------------------------------------------

    // BlockSwapMergeSort.multiSwap
    template<class T>
    void BlockSwapMergeSortMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
        using std::swap;
        for (ptrdiff_t i = 0; i < len; ++i) {
            swap(data_[a + i], data_[b + i]);
        }
    }

    // BlockSwapMergeSort.binarySearchMid
    template<class T>
    ptrdiff_t BlockSwapMergeSortBinarySearchMid(std::vector<T>& data_, ptrdiff_t start,
                                                ptrdiff_t mid, ptrdiff_t end) {
        ptrdiff_t a = 0;
        ptrdiff_t b = (std::min)(mid - start, end - mid);
        ptrdiff_t m = a + (b - a) / 2;

        while (b > a) {
            if (NSortHelpers::CompareValues(data_[mid - m - 1], data_[mid + m]) == 1) {
                a = m + 1;
            }
            else {
                b = m;
            }
            m = a + (b - a) / 2;
        }

        return m;
    }

    // BlockSwapMergeSort.multiSwapMerge.
    // The Java code recurses in the middle of its loop; the recursion can be O(n)
    // deep, so it is driven by an explicit stack of saved loop states.
    template<class T>
    void BlockSwapMergeSortMultiSwapMerge(std::vector<T>& data_, ptrdiff_t start,
                                          ptrdiff_t mid, ptrdiff_t end) {
        struct Frame {
            ptrdiff_t start;
            ptrdiff_t mid;
            ptrdiff_t end;
            ptrdiff_t m;
        };

        std::vector<Frame> stack;

        ptrdiff_t m = BlockSwapMergeSortBinarySearchMid(data_, start, mid, end);

        for (;;) {
            if (m > 0) {
                BlockSwapMergeSortMultiSwap(data_, mid - m, mid, m);

                stack.push_back(Frame{ start, mid, end, m });

                // enter the recursive call multiSwapMerge(mid, mid + m, end)
                start = mid;
                mid = mid + m;
                m = BlockSwapMergeSortBinarySearchMid(data_, start, mid, end);
            }
            else {
                if (stack.empty()) {
                    break;
                }
                Frame frame = stack.back();
                stack.pop_back();

                start = frame.start;
                mid = frame.mid;
                end = frame.end;
                m = frame.m;

                // resume the loop of the parent frame
                end = mid;
                mid -= m;
                m = BlockSwapMergeSortBinarySearchMid(data_, start, mid, end);
            }
        }
    }

    // BlockSwapMergeSort.multiSwapMergeSort
    template<class T>
    void BlockSwapMergeSortMultiSwapMergeSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t len = b - a;
        ptrdiff_t i;

        for (ptrdiff_t j = 1; j < len; j *= 2) {
            for (i = a; i + 2 * j <= b; i += 2 * j) {
                BlockSwapMergeSortMultiSwapMerge(data_, i, i + j, i + 2 * j);
            }
            if (i + j < b) {
                BlockSwapMergeSortMultiSwapMerge(data_, i, i + j, b);
            }
        }
    }

    // BlockSwapMergeSort.runSort.
    template<class T = int>
    void BlockSwapMergeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }
        BlockSwapMergeSortMultiSwapMergeSort(data_, 0, length);
    }

    // ------------------------------------------------------------------
    // BottomUpMergeSort
    // ------------------------------------------------------------------

    // BottomUpMergeSort.merge (writes into scratchArray; copyLength is a driver field).
    template<class T>
    void BottomUpMergeSortMerge(std::vector<T>& data_, std::vector<T>& scratchArray,
                                ptrdiff_t currentLength, ptrdiff_t index, ptrdiff_t mergeSize,
                                ptrdiff_t& copyLength) {
        ptrdiff_t left = index;
        ptrdiff_t mid = left + (mergeSize / 2);
        ptrdiff_t right = mid;
        ptrdiff_t end = (std::min)(currentLength, index + mergeSize);

        ptrdiff_t scratchIndex = left;

        if (right < end) {
            while (left < mid && right < end) {

                if (data_[left] <= data_[right]) {
                    scratchArray[scratchIndex++] = data_[left++];
                }
                else {
                    scratchArray[scratchIndex++] = data_[right++];
                }
            }
            if (left < mid) {
                while (left < mid) {
                    scratchArray[scratchIndex++] = data_[left++];
                }
            }
            if (right < end) {
                while (right < end) {
                    scratchArray[scratchIndex++] = data_[right++];
                }
            }
        }
        else {
            copyLength = left;
        }
    }

    // BottomUpMergeSort.runSort.
    template<class T = int>
    void BottomUpMergeSort(std::vector<T>& data_) {
        ptrdiff_t currentLength = static_cast<ptrdiff_t>(data_.size());
        if (currentLength < 2) {
            return;
        }

        std::vector<T> scratchArray(currentLength);
        ptrdiff_t copyLength = currentLength;
        ptrdiff_t mergeSize = 2;

        while (mergeSize <= currentLength) {
            copyLength = currentLength;

            for (ptrdiff_t i = 0; i < currentLength; i += mergeSize) {
                BottomUpMergeSortMerge(data_, scratchArray, currentLength, i, mergeSize, copyLength);
            }

            for (ptrdiff_t i = 0; i < copyLength; ++i) {
                data_[i] = scratchArray[i];
            }

            mergeSize *= 2;
        }
        if ((mergeSize / 2) != currentLength) {
            BottomUpMergeSortMerge(data_, scratchArray, currentLength, 0, mergeSize, copyLength);

            for (ptrdiff_t i = 0; i < currentLength; ++i) {
                data_[i] = scratchArray[i];
            }
        }
    }

    // ------------------------------------------------------------------
    // BufferedStoogeSort
    // ------------------------------------------------------------------

    // BufferedStoogeSort.compare
    template<class T>
    int BufferedStoogeSortCompare(std::vector<T>& arr, ptrdiff_t x, ptrdiff_t y) {
        return NSortHelpers::CompareValues(arr[x], arr[y]);
    }

    // BufferedStoogeSort.wrapper (recursion depth is O(log3 n), kept recursive).
    template<class T>
    void BufferedStoogeSortWrapper(std::vector<T>& arr, ptrdiff_t start, ptrdiff_t stop) {
        if (stop - start > 1) {
            if (stop - start == 2 && BufferedStoogeSortCompare(arr, start, stop - 1) == 1) {
                using std::swap;
                swap(arr[start], arr[stop - 1]);
            }
            if (stop - start > 2) {
                // (int) Math.ceil((stop - start) / 3.0) + start
                ptrdiff_t third = start + (stop - start + 2) / 3;
                // (int) Math.ceil((stop - start) / 3.0 * 2.0) + start
                ptrdiff_t twoThird = start + ((stop - start) * 2 + 2) / 3;

                if (twoThird - third < third) {
                    --twoThird;
                }
                if ((stop - start - 2) % 3 == 0) {
                    --twoThird;
                }
                BufferedStoogeSortWrapper(arr, third, twoThird);
                BufferedStoogeSortWrapper(arr, twoThird, stop);

                ptrdiff_t left = third;
                ptrdiff_t right = twoThird;
                ptrdiff_t bufferStart = start;

                using std::swap;
                while (left < twoThird && right < stop) {
                    if (BufferedStoogeSortCompare(arr, left, right) == 1) {
                        swap(arr[bufferStart], arr[right]);
                        ++right;
                    }
                    else {
                        swap(arr[bufferStart], arr[left]);
                        ++left;
                    }
                    ++bufferStart;
                }
                while (right < stop) {
                    swap(arr[bufferStart], arr[right]);
                    ++right;
                    ++bufferStart;
                }

                BufferedStoogeSortWrapper(arr, twoThird, stop);

                left = twoThird - 1;
                right = stop - 1;
                while (right > left && left >= start) {
                    if (BufferedStoogeSortCompare(arr, left, right) == 1) {
                        for (ptrdiff_t i = left; i < right; ++i) {
                            swap(arr[i], arr[i + 1]);
                        }
                        --left;
                    }
                    --right;
                }
            }
        }
    }

    // BufferedStoogeSort.runSort.
    template<class T = int>
    void BufferedStoogeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }
        BufferedStoogeSortWrapper(data_, 0, length);
    }

    // ------------------------------------------------------------------
    // ImprovedInPlaceMergeSort
    // ------------------------------------------------------------------

    // ImprovedInPlaceMergeSort.push
    template<class T>
    void ImprovedInPlaceMergeSortPush(std::vector<T>& data_, ptrdiff_t p, ptrdiff_t a, ptrdiff_t b) {
        if (a == b) {
            return;
        }

        T temp = data_[p];
        data_[p] = data_[a];

        for (ptrdiff_t i = a + 1; i < b; ++i) {
            data_[i - 1] = data_[i];
        }

        data_[b - 1] = temp;
    }

    // ImprovedInPlaceMergeSort.merge
    template<class T>
    void ImprovedInPlaceMergeSortMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = a;
        ptrdiff_t j = m;

        while (i < m && j < b) {

            if (NSortHelpers::CompareValues(data_[i], data_[j]) == 1) {
                ++j;
            }
            else {
                ImprovedInPlaceMergeSortPush(data_, i, m, j);
                ++i;
            }
        }

        while (i < m) {
            ImprovedInPlaceMergeSortPush(data_, i, m, b);
            ++i;
        }
    }

    // ImprovedInPlaceMergeSort.mergeSort
    template<class T>
    void ImprovedInPlaceMergeSortMergeSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t m = a + (b - a) / 2;

        if (b - a > 2) {
            if (b - a > 3) {
                ImprovedInPlaceMergeSortMergeSort(data_, a, m);
            }
            ImprovedInPlaceMergeSortMergeSort(data_, m, b);
        }

        ImprovedInPlaceMergeSortMerge(data_, a, m, b);
    }

    // ImprovedInPlaceMergeSort.runSort.
    template<class T = int>
    void ImprovedInPlaceMergeSort(std::vector<T>& data_) {
        ptrdiff_t currentLength = static_cast<ptrdiff_t>(data_.size());
        if (currentLength < 2) {
            return;
        }
        ImprovedInPlaceMergeSortMergeSort(data_, 0, currentLength);
    }

    // ------------------------------------------------------------------
    // InPlaceMergeSort
    // ------------------------------------------------------------------

    // InPlaceMergeSort.push
    template<class T>
    void InPlaceMergeSortPush(std::vector<T>& data_, ptrdiff_t low, ptrdiff_t high) {
        using std::swap;
        for (ptrdiff_t i = low; i < high; ++i) {
            if (NSortHelpers::CompareValues(data_[i], data_[i + 1]) == 1) {
                swap(data_[i], data_[i + 1]);
            }
        }
    }

    // InPlaceMergeSort.merge
    template<class T>
    void InPlaceMergeSortMerge(std::vector<T>& data_, ptrdiff_t min, ptrdiff_t max, ptrdiff_t mid) {
        ptrdiff_t i = min;
        while (i <= mid) {
            if (NSortHelpers::CompareValues(data_[i], data_[mid + 1]) == 1) {
                using std::swap;
                swap(data_[i], data_[mid + 1]);
                InPlaceMergeSortPush(data_, mid + 1, max);
            }
            ++i;
        }
    }

    // InPlaceMergeSort.mergeSort
    template<class T>
    void InPlaceMergeSortMergeSort(std::vector<T>& data_, ptrdiff_t min, ptrdiff_t max) {
        if (max - min == 0) {
            // only one element, no swap
        }
        else if (max - min == 1) {
            if (NSortHelpers::CompareValues(data_[min], data_[max]) == 1) {
                using std::swap;
                swap(data_[min], data_[max]);
            }
        }
        else {
            ptrdiff_t mid = (min + max) / 2;

            InPlaceMergeSortMergeSort(data_, min, mid);
            InPlaceMergeSortMergeSort(data_, mid + 1, max);
            InPlaceMergeSortMerge(data_, min, max, mid);
        }
    }

    // InPlaceMergeSort.runSort.
    template<class T = int>
    void InPlaceMergeSort(std::vector<T>& data_) {
        ptrdiff_t currentLength = static_cast<ptrdiff_t>(data_.size());
        if (currentLength < 2) {
            return;
        }
        InPlaceMergeSortMergeSort(data_, 0, currentLength - 1);
    }

    // ------------------------------------------------------------------
    // IterativeTopDownMergeSort
    // ------------------------------------------------------------------

    // IterativeTopDownMergeSort.ceilPowerOfTwo
    inline ptrdiff_t IterativeTopDownMergeSortCeilPowerOfTwo(ptrdiff_t x) {
        --x;
        for (ptrdiff_t i = 16; i > 0; i >>= 1) {
            x |= x >> i;
        }
        return ++x;
    }

    // IterativeTopDownMergeSort.merge
    template<class T>
    void IterativeTopDownMergeSortMerge(std::vector<T>& data_, std::vector<T>& tmp,
                                        ptrdiff_t start, ptrdiff_t mid, ptrdiff_t end) {
        ptrdiff_t low = start;
        ptrdiff_t high = mid;

        ptrdiff_t nxt = start;

        for (; low < mid && high < end; ++nxt) {
            if (NSortHelpers::CompareValues(data_[low], data_[high]) == 1) {
                tmp[nxt] = data_[high++];
            }
            else {
                tmp[nxt] = data_[low++];
            }
        }

        if (low >= mid) {
            while (high < end) {
                tmp[nxt++] = data_[high++];
            }
        }
        else {
            while (low < mid) {
                tmp[nxt++] = data_[low++];
            }
        }

        for (ptrdiff_t i = start; i < end; ++i) {
            data_[i] = tmp[i];
        }
    }

    // IterativeTopDownMergeSort.runSortLarge (rational-arithmetic variant).
    template<class T>
    void IterativeTopDownMergeSortRunSortLarge(std::vector<T>& data_, std::vector<T>& tmp,
                                               ptrdiff_t length) {
        for (ptrdiff_t subarrayCount = IterativeTopDownMergeSortCeilPowerOfTwo(length),
                       wholeI = length / subarrayCount, fracI = length % subarrayCount;
             subarrayCount > 1;) {
            for (ptrdiff_t whole = 0, frac = 0; whole < length;) {
                ptrdiff_t start = whole;

                whole += wholeI;
                frac += fracI;
                if (frac >= subarrayCount) {
                    ++whole;
                    frac -= subarrayCount;
                }
                ptrdiff_t mid = whole;

                whole += wholeI;
                frac += fracI;
                if (frac >= subarrayCount) {
                    ++whole;
                    frac -= subarrayCount;
                }
                IterativeTopDownMergeSortMerge(data_, tmp, start, mid, whole);
            }
            subarrayCount >>= 1;
            wholeI <<= 1;
            if (fracI >= subarrayCount) {
                ++wholeI;
                fracI -= subarrayCount;
            }
        }
    }

    // IterativeTopDownMergeSort.mergeSort
    template<class T>
    void IterativeTopDownMergeSortMergeSort(std::vector<T>& data_, std::vector<T>& tmp,
                                            ptrdiff_t length) {
        if (length < 1 << 15) {
            for (ptrdiff_t subarrayCount = IterativeTopDownMergeSortCeilPowerOfTwo(length);
                 subarrayCount > 1; subarrayCount >>= 1) {
                for (ptrdiff_t i = 0; i < subarrayCount; i += 2) {
                    IterativeTopDownMergeSortMerge(data_, tmp,
                        length * i / subarrayCount,
                        length * (i + 1) / subarrayCount,
                        length * (i + 2) / subarrayCount);
                }
            }
        }
        else {
            IterativeTopDownMergeSortRunSortLarge(data_, tmp, length);
        }
    }

    // IterativeTopDownMergeSort.runSort.
    template<class T = int>
    void IterativeTopDownMergeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::vector<T> tmp(length);
        IterativeTopDownMergeSortMergeSort(data_, tmp, length);
    }

    // ------------------------------------------------------------------
    // LazyStableSort (GrailSorting.grailLazyStableSort, in-place, no buffer)
    // ------------------------------------------------------------------

    // GrailSorting.grailMultiSwap
    template<class T>
    void LazyStableSortMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t swapsLeft) {
        using std::swap;
        while (swapsLeft != 0) {
            swap(data_[a++], data_[b++]);
            --swapsLeft;
        }
    }

    // GrailSorting.grailRotate
    template<class T>
    void LazyStableSortRotate(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t lenA, ptrdiff_t lenB) {
        while (lenA != 0 && lenB != 0) {
            if (lenA <= lenB) {
                LazyStableSortMultiSwap(data_, pos, pos + lenA, lenA);
                pos += lenA;
                lenB -= lenA;
            }
            else {
                LazyStableSortMultiSwap(data_, pos + (lenA - lenB), pos + lenA, lenB);
                lenA -= lenB;
            }
        }
    }

    // GrailSorting.grailBinSearch (isLeft determines the direction).
    template<class T>
    ptrdiff_t LazyStableSortBinSearch(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len,
                                      ptrdiff_t keyPos, bool isLeft) {
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
        }
        return right;
    }

    // GrailSorting.grailMergeWithoutBuffer
    template<class T>
    void LazyStableSortMergeWithoutBuffer(std::vector<T>& data_, ptrdiff_t pos,
                                          ptrdiff_t len1, ptrdiff_t len2) {
        if (len1 < len2) {
            while (len1 != 0) {
                // Binary Search left
                ptrdiff_t loc = LazyStableSortBinSearch(data_, pos + len1, len2, pos, true);
                if (loc != 0) {
                    LazyStableSortRotate(data_, pos, len1, loc);
                    pos += loc;
                    len2 -= loc;
                }
                if (len2 == 0) {
                    break;
                }
                do {
                    ++pos;
                    --len1;
                } while (len1 != 0 && data_[pos] <= data_[pos + len1]);
            }
        }
        else {
            while (len2 != 0) {
                // Binary Search right
                ptrdiff_t loc = LazyStableSortBinSearch(data_, pos, len1, pos + (len1 + len2 - 1), false);
                if (loc != len1) {
                    LazyStableSortRotate(data_, pos + loc, len1 - loc, len2);
                    len1 = loc;
                }
                if (len1 == 0) {
                    break;
                }
                do {
                    --len2;
                } while (len2 != 0 && data_[pos + len1 - 1] <= data_[pos + len1 + len2 - 1]);
            }
        }
    }

    // GrailSorting.grailLazyStableSort
    template<class T>
    void LazyStableSortLazyStableSort(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len) {
        for (ptrdiff_t dist = 1; dist < len; dist += 2) {
            if (data_[pos + dist - 1] > data_[pos + dist]) {
                using std::swap;
                swap(data_[pos + (dist - 1)], data_[pos + dist]);
            }
        }

        for (ptrdiff_t part = 2; part < len; part *= 2) {
            ptrdiff_t left = 0;
            ptrdiff_t right = len - 2 * part;

            while (left <= right) {
                LazyStableSortMergeWithoutBuffer(data_, pos + left, part, part);
                left += 2 * part;
            }

            ptrdiff_t rest = len - left;
            if (rest > part) {
                LazyStableSortMergeWithoutBuffer(data_, pos + left, part, rest - part);
            }
        }
    }

    // LazyStableSort.runSort.
    template<class T = int>
    void LazyStableSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }
        LazyStableSortLazyStableSort(data_, 0, length);
    }

    // ------------------------------------------------------------------
    // MergeSort (ArrayV) -> MergeSortJava
    // ------------------------------------------------------------------

    // MergeSort.runSort (that menu entry already exists, so the ArrayV flavour is suffixed).
    template<class T = int>
    void MergeSortJava(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }
        NSortHelpers::mergeSort(data_, length, false);
    }

    // ------------------------------------------------------------------
    // MergeSortParallel
    // ------------------------------------------------------------------

    // MergeSortParallel.merge
    template<class T>
    void MergeSortParallelMerge(std::vector<T>& data_, std::vector<T>& tmp,
                                ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = a, j = m, k = a;

        while (i < m && j < b) {
            if (data_[i] <= data_[j]) {
                tmp[k++] = data_[i++];
            }
            else {
                tmp[k++] = data_[j++];
            }
        }
        while (i < m) {
            tmp[k++] = data_[i++];
        }
        while (j < b) {
            tmp[k++] = data_[j++];
        }

        while (a < b) {
            data_[a] = tmp[a];
            ++a;
        }
    }

    // MergeSortParallel.mergeSort (each split runs on its own thread, as in Java).
    template<class T>
    void MergeSortParallelMergeSort(std::vector<T>& data_, std::vector<T>& tmp,
                                    ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t len = b - a;

        if (len < 2) {
            return;
        }

        ptrdiff_t m = (a + b) / 2;

        std::thread left([&data_, &tmp, a, m]() {
            MergeSortParallelMergeSort(data_, tmp, a, m);
        });
        std::thread right([&data_, &tmp, m, b]() {
            MergeSortParallelMergeSort(data_, tmp, m, b);
        });

        left.join();
        right.join();

        MergeSortParallelMerge(data_, tmp, a, m, b);
    }

    // MergeSortParallel.runSort.
    template<class T = int>
    void MergeSortParallel(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::vector<T> tmp(length);
        MergeSortParallelMergeSort(data_, tmp, 0, length);
    }

    // ------------------------------------------------------------------
    // NewShuffleMergeSort (extends IterativeTopDownMergeSort)
    // ------------------------------------------------------------------

    // NewShuffleMergeSort.rotateEqual
    template<class T>
    void NewShuffleMergeSortRotateEqual(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t size) {
        using std::swap;
        for (ptrdiff_t i = 0; i < size; ++i) {
            swap(data_[a + i], data_[b + i]);
        }
    }

    // NewShuffleMergeSort.rotate
    template<class T>
    void NewShuffleMergeSortRotate(std::vector<T>& data_, ptrdiff_t mid, ptrdiff_t a, ptrdiff_t b) {
        while (a > 0 && b > 0) {
            if (a > b) {
                NewShuffleMergeSortRotateEqual(data_, mid - b, mid, b);
                mid -= b;
                a -= b;
            }
            else {
                NewShuffleMergeSortRotateEqual(data_, mid - a, mid, a);
                mid += a;
                b -= a;
            }
        }
    }

    // NewShuffleMergeSort.shuffleEasy
    template<class T>
    void NewShuffleMergeSortShuffleEasy(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t size) {
        for (ptrdiff_t i = 1; i < size; i *= 3) {
            T val = data_[start + i - 1];
            for (ptrdiff_t j = i * 2 % size; j != i; j = j * 2 % size) {
                T nval = data_[start + j - 1];
                data_[start + j - 1] = val;
                val = nval;
            }
            data_[start + i - 1] = val;
        }
    }

    // NewShuffleMergeSort.shuffle
    template<class T>
    void NewShuffleMergeSortShuffle(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        while (end - start > 1) {
            ptrdiff_t n = (end - start) / 2;
            ptrdiff_t l = 1;
            while (l * 3 - 1 <= 2 * n) {
                l *= 3;
            }
            ptrdiff_t m = (l - 1) / 2;

            NewShuffleMergeSortRotate(data_, start + n, n - m, m);
            NewShuffleMergeSortShuffleEasy(data_, start, l);
            start += l - 1;
        }
    }

    // NewShuffleMergeSort.rotateShuffledEqual
    template<class T>
    void NewShuffleMergeSortRotateShuffledEqual(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t size) {
        using std::swap;
        for (ptrdiff_t i = 0; i < size; i += 2) {
            swap(data_[a + i], data_[b + i]);
        }
    }

    // NewShuffleMergeSort.rotateShuffled
    template<class T>
    void NewShuffleMergeSortRotateShuffled(std::vector<T>& data_, ptrdiff_t mid, ptrdiff_t a, ptrdiff_t b) {
        while (a > 0 && b > 0) {
            if (a > b) {
                NewShuffleMergeSortRotateShuffledEqual(data_, mid - b, mid, b);
                mid -= b;
                a -= b;
            }
            else {
                NewShuffleMergeSortRotateShuffledEqual(data_, mid - a, mid, a);
                mid += a;
                b -= a;
            }
        }
    }

    // NewShuffleMergeSort.rotateShuffledOuter
    template<class T>
    void NewShuffleMergeSortRotateShuffledOuter(std::vector<T>& data_, ptrdiff_t mid,
                                                ptrdiff_t a, ptrdiff_t b) {
        if (a > b) {
            NewShuffleMergeSortRotateShuffledEqual(data_, mid - b, mid + 1, b);
            mid -= b;
            a -= b;
            NewShuffleMergeSortRotateShuffled(data_, mid, a, b);
        }
        else {
            NewShuffleMergeSortRotateShuffledEqual(data_, mid - a, mid + 1, a);
            mid += a + 1;
            b -= a;
            NewShuffleMergeSortRotateShuffled(data_, mid, a, b);
        }
    }

    // NewShuffleMergeSort.unshuffleEasy
    template<class T>
    void NewShuffleMergeSortUnshuffleEasy(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t size) {
        for (ptrdiff_t i = 1; i < size; i *= 3) {
            ptrdiff_t prev = i;
            T val = data_[start + i - 1];
            for (ptrdiff_t j = i * 2 % size; j != i; j = j * 2 % size) {
                data_[start + prev - 1] = data_[start + j - 1];
                prev = j;
            }
            data_[start + prev - 1] = val;
        }
    }

    // NewShuffleMergeSort.unshuffle
    template<class T>
    void NewShuffleMergeSortUnshuffle(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        while (end - start > 1) {
            ptrdiff_t n = (end - start) / 2;
            ptrdiff_t l = 1;
            while (l * 3 - 1 <= 2 * n) {
                l *= 3;
            }
            ptrdiff_t m = (l - 1) / 2;

            NewShuffleMergeSortRotateShuffledOuter(data_, start + 2 * m, 2 * m, 2 * n - 2 * m);
            NewShuffleMergeSortUnshuffleEasy(data_, start, l);
            start += l - 1;
        }
    }

    // NewShuffleMergeSort.mergeUp
    template<class T>
    void NewShuffleMergeSortMergeUp(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, bool type) {
        ptrdiff_t i = start;
        ptrdiff_t j = i + 1;

        while (j < end) {
            int cmp = NSortHelpers::CompareValues(data_[i], data_[j]);
            if (cmp == -1 || (!type && cmp == 0)) {
                ++i;
                if (i == j) {
                    ++j;
                    type = !type;
                }
            }
            else if (end - j == 1) {
                NewShuffleMergeSortRotate(data_, j, j - i, 1);
                break;
            }
            else {
                ptrdiff_t r = 0;
                if (type) {
                    while (j + 2 * r < end && NSortHelpers::CompareValues(data_[j + 2 * r], data_[i]) != 1) {
                        ++r;
                    }
                }
                else {
                    while (j + 2 * r < end && NSortHelpers::CompareValues(data_[j + 2 * r], data_[i]) == -1) {
                        ++r;
                    }
                }
                --j;
                NewShuffleMergeSortUnshuffle(data_, j, j + 2 * r);
                NewShuffleMergeSortRotate(data_, j, j - i, r);
                i += r + 1;
                j += 2 * r + 1;
            }
        }
    }

    // NewShuffleMergeSort.merge (overrides IterativeTopDownMergeSort.merge; tmp is unused).
    template<class T>
    void NewShuffleMergeSortMerge(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t mid, ptrdiff_t end) {
        if (mid - start <= end - mid) {
            NewShuffleMergeSortShuffle(data_, start, end);
            NewShuffleMergeSortMergeUp(data_, start, end, true);
        }
        else {
            NewShuffleMergeSortShuffle(data_, start + 1, end);
            NewShuffleMergeSortMergeUp(data_, start, end, false);
        }
    }

    // NewShuffleMergeSort.runSortLarge (IterativeTopDownMergeSort's rational driver).
    template<class T>
    void NewShuffleMergeSortRunSortLarge(std::vector<T>& data_, ptrdiff_t length) {
        for (ptrdiff_t subarrayCount = IterativeTopDownMergeSortCeilPowerOfTwo(length),
                       wholeI = length / subarrayCount, fracI = length % subarrayCount;
             subarrayCount > 1;) {
            for (ptrdiff_t whole = 0, frac = 0; whole < length;) {
                ptrdiff_t start = whole;

                whole += wholeI;
                frac += fracI;
                if (frac >= subarrayCount) {
                    ++whole;
                    frac -= subarrayCount;
                }
                ptrdiff_t mid = whole;

                whole += wholeI;
                frac += fracI;
                if (frac >= subarrayCount) {
                    ++whole;
                    frac -= subarrayCount;
                }
                NewShuffleMergeSortMerge(data_, start, mid, whole);
            }
            subarrayCount >>= 1;
            wholeI <<= 1;
            if (fracI >= subarrayCount) {
                ++wholeI;
                fracI -= subarrayCount;
            }
        }
    }

    // NewShuffleMergeSort.mergeSort (the inherited driver, calling the shuffle merge).
    template<class T>
    void NewShuffleMergeSortMergeSort(std::vector<T>& data_, ptrdiff_t length) {
        if (length < 1 << 15) {
            for (ptrdiff_t subarrayCount = IterativeTopDownMergeSortCeilPowerOfTwo(length);
                 subarrayCount > 1; subarrayCount >>= 1) {
                for (ptrdiff_t i = 0; i < subarrayCount; i += 2) {
                    NewShuffleMergeSortMerge(data_,
                        length * i / subarrayCount,
                        length * (i + 1) / subarrayCount,
                        length * (i + 2) / subarrayCount);
                }
            }
        }
        else {
            NewShuffleMergeSortRunSortLarge(data_, length);
        }
    }

    // NewShuffleMergeSort.runSort.
    template<class T = int>
    void NewShuffleMergeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }
        NewShuffleMergeSortMergeSort(data_, length);
    }

    // ------------------------------------------------------------------
    // PDMergeSort (pattern-defeating merge sort)
    // ------------------------------------------------------------------

    // PDMergeSort.mergeUp
    template<class T>
    void PDMergeSortMergeUp(std::vector<T>& data_, std::vector<T>& copied,
                            ptrdiff_t start, ptrdiff_t mid, ptrdiff_t end) {
        for (ptrdiff_t i = 0; i < mid - start; ++i) {
            copied[i] = data_[i + start];
        }

        ptrdiff_t bufferPointer = 0;
        ptrdiff_t left = start;
        ptrdiff_t right = mid;

        while (left < right && right < end) {
            if (copied[bufferPointer] <= data_[right]) {
                data_[left++] = copied[bufferPointer++];
            }
            else {
                data_[left++] = data_[right++];
            }
        }

        while (left < right) {
            data_[left++] = copied[bufferPointer++];
        }
    }

    // PDMergeSort.mergeDown
    template<class T>
    void PDMergeSortMergeDown(std::vector<T>& data_, std::vector<T>& copied,
                              ptrdiff_t start, ptrdiff_t mid, ptrdiff_t end) {
        for (ptrdiff_t i = 0; i < end - mid; ++i) {
            copied[i] = data_[i + mid];
        }

        ptrdiff_t bufferPointer = end - mid - 1;
        ptrdiff_t left = mid - 1;
        ptrdiff_t right = end - 1;

        while (right > left && left >= start) {
            if (copied[bufferPointer] >= data_[left]) {
                data_[right--] = copied[bufferPointer--];
            }
            else {
                data_[right--] = data_[left--];
            }
        }

        while (right > left) {
            data_[right--] = copied[bufferPointer--];
        }
    }

    // PDMergeSort.merge
    template<class T>
    void PDMergeSortMerge(std::vector<T>& data_, std::vector<T>& copied,
                          ptrdiff_t leftStart, ptrdiff_t rightStart, ptrdiff_t end) {
        if (end - rightStart < rightStart - leftStart) {
            PDMergeSortMergeDown(data_, copied, leftStart, rightStart, end);
        }
        else {
            PDMergeSortMergeUp(data_, copied, leftStart, rightStart, end);
        }
    }

    // PDMergeSort.identifyRun (returns the start of the next run, or -1).
    template<class T>
    ptrdiff_t PDMergeSortIdentifyRun(std::vector<T>& data_, ptrdiff_t index, ptrdiff_t maxIndex) {
        ptrdiff_t startIndex = index;

        if (index >= maxIndex) {
            return -1;
        }

        bool cmp = data_[index] <= data_[index + 1];
        ++index;

        while (index < maxIndex) {
            bool checkCmp = data_[index] <= data_[index + 1];
            if (checkCmp != cmp) {
                break;
            }
            ++index;
        }

        if (!cmp) {
            // Writes.reversal(array, startIndex, index, ...) is inclusive of index
            NSortHelpers::reverseRange(data_, startIndex, index + 1);
        }
        if (index >= maxIndex) {
            return -1;
        }
        return index + 1;
    }

    // PDMergeSort.findRuns
    template<class T>
    std::vector<ptrdiff_t> PDMergeSortFindRuns(std::vector<T>& data_, ptrdiff_t maxIndex,
                                               ptrdiff_t& runCount) {
        std::vector<ptrdiff_t> runs(static_cast<size_t>(maxIndex / 2 + 2));
        runCount = 0;

        ptrdiff_t lastRun = 0;
        while (lastRun != -1) {
            runs[runCount++] = lastRun;
            ptrdiff_t newRun = PDMergeSortIdentifyRun(data_, lastRun, maxIndex);
            lastRun = newRun;
        }

        return runs;
    }

    // PDMergeSort.runSort.
    template<class T = int>
    void PDMergeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        ptrdiff_t runCount = 0;
        std::vector<ptrdiff_t> runs = PDMergeSortFindRuns(data_, length - 1, runCount);
        std::vector<T> copied(static_cast<size_t>(length / 2));

        while (runCount > 1) {
            for (ptrdiff_t i = 0; i < runCount - 1; i += 2) {
                ptrdiff_t end = i + 2 >= runCount ? length : runs[i + 2];
                PDMergeSortMerge(data_, copied, runs[i], runs[i + 1], end);
            }
            for (ptrdiff_t i = 1, j = 2; i < runCount; ++i, j += 2, --runCount) {
                runs[i] = runs[j];
            }
        }
    }

    // ------------------------------------------------------------------
    // QuadSort
    // ------------------------------------------------------------------

    // QuadSort.runSort (QuadSorting.quadSort).
    template<class T = int>
    void QuadSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }
        NSortHelpers::quadSort(data_, 0, length);
    }

    // ------------------------------------------------------------------
    // RotateMergeSort
    // ------------------------------------------------------------------

    // RotateMergeSort.multiSwap
    template<class T>
    void RotateMergeSortMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
        using std::swap;
        for (ptrdiff_t i = 0; i < len; ++i) {
            swap(data_[a + i], data_[b + i]);
        }
    }

    // RotateMergeSort.rotate
    template<class T>
    void RotateMergeSortRotate(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t l = m - a;
        ptrdiff_t r = b - m;

        while (l > 0 && r > 0) {
            if (r < l) {
                RotateMergeSortMultiSwap(data_, m - r, m, r);
                b -= r;
                m -= r;
                l -= r;
            }
            else {
                RotateMergeSortMultiSwap(data_, a, m, l);
                a += l;
                m += l;
                r -= l;
            }
        }
    }

    // RotateMergeSort.binarySearch
    template<class T>
    ptrdiff_t RotateMergeSortBinarySearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b,
                                          T value, bool left) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            bool comp = left ? NSortHelpers::CompareValues(value, data_[m]) <= 0
                             : NSortHelpers::CompareValues(value, data_[m]) < 0;

            if (comp) {
                b = m;
            }
            else {
                a = m + 1;
            }
        }

        return a;
    }

    // RotateMergeSort.rotateMerge (recursion depth is O(log n), kept recursive).
    template<class T>
    void RotateMergeSortRotateMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t m1, m2, m3;

        if (m - a >= b - m) {
            m1 = a + (m - a) / 2;
            m2 = RotateMergeSortBinarySearch(data_, m, b, data_[m1], true);
            m3 = m1 + (m2 - m);
        }
        else {
            m2 = m + (b - m) / 2;
            m1 = RotateMergeSortBinarySearch(data_, a, m, data_[m2], false);
            m3 = (m2++) - (m - m1);
        }
        RotateMergeSortRotate(data_, m1, m, m2);

        if (m2 - (m3 + 1) > 0 && b - m2 > 0) {
            RotateMergeSortRotateMerge(data_, m3 + 1, m2, b);
        }
        if (m1 - a > 0 && m3 - m1 > 0) {
            RotateMergeSortRotateMerge(data_, a, m1, m3);
        }
    }

    // RotateMergeSort.rotateMergeSort
    template<class T>
    void RotateMergeSortRotateMergeSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t len = b - a;
        ptrdiff_t i;

        for (ptrdiff_t j = 1; j < len; j *= 2) {
            for (i = a; i + 2 * j <= b; i += 2 * j) {
                RotateMergeSortRotateMerge(data_, i, i + j, i + 2 * j);
            }
            if (i + j < b) {
                RotateMergeSortRotateMerge(data_, i, i + j, b);
            }
        }
    }

    // RotateMergeSort.runSort.
    template<class T = int>
    void RotateMergeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }
        RotateMergeSortRotateMergeSort(data_, 0, length);
    }

    // ------------------------------------------------------------------
    // RotateMergeSortParallel
    // ------------------------------------------------------------------

    // RotateMergeSortParallel.multiSwap
    template<class T>
    void RotateMergeSortParallelMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
        using std::swap;
        for (ptrdiff_t i = 0; i < len; ++i) {
            swap(data_[a + i], data_[b + i]);
        }
    }

    // RotateMergeSortParallel.rotate
    template<class T>
    void RotateMergeSortParallelRotate(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t l = m - a;
        ptrdiff_t r = b - m;

        while (l > 0 && r > 0) {
            if (r < l) {
                RotateMergeSortParallelMultiSwap(data_, m - r, m, r);
                b -= r;
                m -= r;
                l -= r;
            }
            else {
                RotateMergeSortParallelMultiSwap(data_, a, m, l);
                a += l;
                m += l;
                r -= l;
            }
        }
    }

    // RotateMergeSortParallel.binarySearch
    template<class T>
    ptrdiff_t RotateMergeSortParallelBinarySearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b,
                                                  T value, bool left) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            bool comp = left ? NSortHelpers::CompareValues(value, data_[m]) <= 0
                             : NSortHelpers::CompareValues(value, data_[m]) < 0;

            if (comp) {
                b = m;
            }
            else {
                a = m + 1;
            }
        }

        return a;
    }

    // RotateMergeSortParallel.rotateMerge (both halves run on their own threads).
    template<class T>
    void RotateMergeSortParallelRotateMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        if (m - a < 1 || b - m < 1) {
            return;
        }

        ptrdiff_t m1, m2, m3;

        if (m - a >= b - m) {
            m1 = a + (m - a) / 2;
            m2 = RotateMergeSortParallelBinarySearch(data_, m, b, data_[m1], true);
            m3 = m1 + (m2 - m);
        }
        else {
            m2 = m + (b - m) / 2;
            m1 = RotateMergeSortParallelBinarySearch(data_, a, m, data_[m2], false);
            m3 = (m2++) - (m - m1);
        }
        RotateMergeSortParallelRotate(data_, m1, m, m2);

        ptrdiff_t la = a, lm1 = m1, lm3 = m3;
        ptrdiff_t rm3 = m3 + 1, rm2 = m2, rb = b;

        std::thread l([&data_, la, lm1, lm3]() {
            RotateMergeSortParallelRotateMerge(data_, la, lm1, lm3);
        });
        std::thread r([&data_, rm3, rm2, rb]() {
            RotateMergeSortParallelRotateMerge(data_, rm3, rm2, rb);
        });

        l.join();
        r.join();
    }

    // RotateMergeSortParallel.rotateMergeSort
    template<class T>
    void RotateMergeSortParallelRotateMergeSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        if (b - a < 2) {
            return;
        }

        ptrdiff_t m = (a + b) / 2;

        std::thread l([&data_, a, m]() {
            RotateMergeSortParallelRotateMergeSort(data_, a, m);
        });
        std::thread r([&data_, m, b]() {
            RotateMergeSortParallelRotateMergeSort(data_, m, b);
        });

        l.join();
        r.join();

        RotateMergeSortParallelRotateMerge(data_, a, m, b);
    }

    // RotateMergeSortParallel.runSort.
    template<class T = int>
    void RotateMergeSortParallel(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }
        RotateMergeSortParallelRotateMergeSort(data_, 0, length);
    }

    // ------------------------------------------------------------------
    // StacklessRotateMergeSort
    // ------------------------------------------------------------------

    // StacklessRotateMergeSort.rotate (IndexedRotations.griesMills: block-swap rotation).
    // Upstream can request a rotation that reaches past the end of the sort
    // range (see the note on StacklessRotateMergeSortReadAt); in ArrayV the
    // swaps land in the unused tail of the backing array, which a std::vector
    // does not have. Skip such a rotation instead of writing outside the
    // vector - memory safety is the only C++-forced difference.
    template<class T>
    void StacklessRotateMergeSortRotate(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        if (a < 0 || m < a || b < m || b > static_cast<ptrdiff_t>(data_.size())) {
            return;
        }

        ptrdiff_t l = m - a;
        ptrdiff_t r = b - m;

        using std::swap;
        while (l > 0 && r > 0) {
            if (r < l) {
                for (ptrdiff_t i = 0; i < r; ++i) {
                    swap(data_[m - r + i], data_[m + i]);
                }
                b -= r;
                m -= r;
                l -= r;
            }
            else {
                for (ptrdiff_t i = 0; i < l; ++i) {
                    swap(data_[a + i], data_[m + i]);
                }
                a += l;
                m += l;
                r -= l;
            }
        }
    }

    // StacklessRotateMergeSort.partitionMerge reads slightly outside its sort
    // window when the second run of a block is shorter than c (upstream can do
    // this because ArrayV's backing array is allocated longer than the current
    // sort length, so the read never faults there). std::vector has no such
    // padding, so clamp the index into the vector; this is the only
    // C++-forced deviation from the Java source and it does not change the
    // values read by any in-range comparison.
    template<class T>
    const T& StacklessRotateMergeSortReadAt(std::vector<T>& data_, ptrdiff_t index) {
        ptrdiff_t size = static_cast<ptrdiff_t>(data_.size());
        if (index < 0) {
            index = 0;
        }
        else if (index >= size) {
            index = size - 1;
        }
        return data_[index];
    }

    // StacklessRotateMergeSort.partitionMerge (select c smallest elements).
    template<class T>
    void StacklessRotateMergeSortPartitionMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m,
                                                ptrdiff_t b, ptrdiff_t c) {
        ptrdiff_t lenA = m - a, lenB = b - m;

        if (lenA < 1 || lenB < 1) {
            return;
        }

        if (lenB < lenA) {
            c = (lenA + lenB) - c;
            ptrdiff_t r1 = 0, r2 = (std::min)(c, lenB);

            while (r1 < r2) {
                ptrdiff_t ml = (r1 + r2) / 2;

                if (NSortHelpers::CompareValues(StacklessRotateMergeSortReadAt(data_, m - (c - ml)),
                                                data_[b - ml - 1]) > 0) {
                    r2 = ml;
                }
                else {
                    r1 = ml + 1;
                }
            }
            // [lenA-(c-r1)][c-r1][lenB-r1][r1]
            // [lenA-(c-r1)][lenB-r1][c-r1][r1]
            StacklessRotateMergeSortRotate(data_, m - (c - r1), m, b - r1);
        }
        else {
            ptrdiff_t r1 = 0, r2 = (std::min)(c, lenA);

            while (r1 < r2) {
                ptrdiff_t ml = (r1 + r2) / 2;

                if (NSortHelpers::CompareValues(data_[a + ml],
                                                StacklessRotateMergeSortReadAt(data_, m + (c - ml) - 1)) > 0) {
                    r2 = ml;
                }
                else {
                    r1 = ml + 1;
                }
            }
            // [r1][lenA-r1][c-r1][lenB-(c-r1)]
            // [r1][c-r1][lenA-r1][lenB-(c-r1)]
            StacklessRotateMergeSortRotate(data_, a + r1, m, m + (c - r1));
        }
    }

    // StacklessRotateMergeSort.rotateMerge
    template<class T>
    void StacklessRotateMergeSortRotateMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t c) {
        ptrdiff_t i;
        for (i = a + 1; i < b && NSortHelpers::CompareValues(data_[i - 1], data_[i]) <= 0; ++i) {
        }
        if (i < b) {
            StacklessRotateMergeSortPartitionMerge(data_, a, i, b, c);
        }
    }

    // StacklessRotateMergeSort.rotatePartitionMergeSort
    template<class T>
    void StacklessRotateMergeSortRotatePartitionMergeSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t len = b - a;

        using std::swap;
        for (ptrdiff_t i = a + 1; i < b; i += 2) {
            if (NSortHelpers::CompareValues(data_[i - 1], data_[i]) > 0) {
                swap(data_[i - 1], data_[i]);
            }
        }

        for (ptrdiff_t j = 2; j < len; j *= 2) {
            ptrdiff_t b1 = 0;

            for (ptrdiff_t i = a; i + j < b; i += 2 * j) {
                b1 = (std::min)(i + 2 * j, b);
                StacklessRotateMergeSortPartitionMerge(data_, i, i + j, b1, j);
            }

            for (ptrdiff_t k = j / 2; k > 1; k /= 2) {
                for (ptrdiff_t i = a; i + k < b1; i += 2 * k) {
                    StacklessRotateMergeSortRotateMerge(data_, i, (std::min)(i + 2 * k, b), k);
                }
            }

            for (ptrdiff_t i = a + 1; i < b1; i += 2) {
                if (NSortHelpers::CompareValues(data_[i - 1], data_[i]) > 0) {
                    swap(data_[i - 1], data_[i]);
                }
            }
        }
    }

    // StacklessRotateMergeSort.runSort.
    template<class T = int>
    void StacklessRotateMergeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }
        StacklessRotateMergeSortRotatePartitionMergeSort(data_, 0, length);
    }

    // ------------------------------------------------------------------
    // StrandSort
    // ------------------------------------------------------------------

    // StrandSort.mergeTo
    template<class T>
    void StrandSortMergeTo(std::vector<T>& data_, std::vector<T>& subList,
                           ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = 0, s = m - a;

        while (i < s && m < b) {
            if (subList[i] < data_[m]) {
                data_[a++] = subList[i++];
            }
            else {
                data_[a++] = data_[m++];
            }
        }

        while (i < s) {
            data_[a++] = subList[i++];
        }
    }

    // StrandSort.runSort (reverses the order of equal items, as in Java).
    template<class T = int>
    void StrandSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::vector<T> subList(static_cast<size_t>(length));

        ptrdiff_t j = length, k = j;

        while (j > 0) {
            subList[0] = data_[0];
            --k;

            for (ptrdiff_t i = 0, p = 0, m = 1; m < j; ++m) {
                if (data_[m] >= subList[i]) {
                    subList[++i] = data_[m];
                    --k;
                }
                else {
                    data_[p++] = data_[m];
                }
            }

            StrandSortMergeTo(data_, subList, k, j, length);
            j = k;
        }
    }

    // ------------------------------------------------------------------
    // TwinSort
    // ------------------------------------------------------------------

    // TwinSort.runSort (TwinSorting.twinsort).
    template<class T = int>
    void TwinSort(std::vector<T>& data_) {
        ptrdiff_t currentLength = static_cast<ptrdiff_t>(data_.size());
        if (currentLength < 2) {
            return;
        }
        NSortHelpers::twinsort(data_, currentLength);
    }

    // ------------------------------------------------------------------
    // WeavedMergeSort
    // ------------------------------------------------------------------

    // WeavedMergeSort.merge (strided bottom-up merge; recursion depth is O(log n)).
    template<class T>
    void WeavedMergeSortMerge(std::vector<T>& data_, std::vector<T>& tmp, ptrdiff_t length,
                              ptrdiff_t residue, ptrdiff_t modulus) {
        if (residue + modulus >= length) {
            return;
        }

        ptrdiff_t low = residue;
        ptrdiff_t high = residue + modulus;
        ptrdiff_t dmodulus = modulus << 1;

        WeavedMergeSortMerge(data_, tmp, length, low, dmodulus);
        WeavedMergeSortMerge(data_, tmp, length, high, dmodulus);

        ptrdiff_t nxt = residue;

        for (; low < length && high < length; nxt += modulus) {
            int cmp = NSortHelpers::CompareValues(data_[low], data_[high]);
            if (cmp == 1 || (cmp == 0 && low > high)) {
                tmp[nxt] = data_[high];
                high += dmodulus;
            }
            else {
                tmp[nxt] = data_[low];
                low += dmodulus;
            }
        }

        if (low >= length) {
            while (high < length) {
                tmp[nxt] = data_[high];
                nxt += modulus;
                high += dmodulus;
            }
        }
        else {
            while (low < length) {
                tmp[nxt] = data_[low];
                nxt += modulus;
                low += dmodulus;
            }
        }

        for (ptrdiff_t i = residue; i < length; i += modulus) {
            data_[i] = tmp[i];
        }
    }

    // WeavedMergeSort.runSort.
    template<class T = int>
    void WeavedMergeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        std::vector<T> tmp(static_cast<size_t>(length));
        WeavedMergeSortMerge(data_, tmp, length, 0, 1);
    }

} // namespace NVisualSort::NSortAlgorithms
