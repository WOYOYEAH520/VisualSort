#pragma once
// Ports of the ArrayV (Java, MIT) "hybrid" sorting category (41 classes):
// AdaptiveGrailSort, BinaryMergeSort, BufferPartitionMergeSort, ChaliceSort,
// CircularGrailSort, CocktailMergeSort, DropMergeSort, EctaSort, FifthMergeSort,
// FlanSort, FluxSort, GrailSort, HybridCombSort, ImprovedBlockSelectionSort,
// IntroCircleSort{Iterative,Recursive}, IntroSort, KotaSort, LazierestSort,
// LaziestSort, MedianMergeSort, MergeInsertionSort, OptimizedBottomUpMergeSort,
// OptimizedDualPivotQuickSort, OptimizedLazyStableSort, OptimizedRotateMergeSort,
// OptimizedWeaveMergeSort, PDQBranchedSort, PDQBranchlessSort,
// ParallelBlockMergeSort, ParallelGrailSort, RemiSort, SqrtSort,
// StacklessDualPivotQuickSort, StacklessHybridQuickSort, SynchronousSqrtSort,
// TimSort, UnstableGrailSort, WeaveMergeSort, WikiSort, YujisBufferedMergeSort2.
// ASCII ONLY: comments in English; any Chinese text lives in \uXXXX escapes
// inside L"..." wide strings.
#include "SortHelpers.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <random>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace NVisualSort::NSortAlgorithms {
    // ==================== AdaptiveGrailSort ====================
// Ports of ArrayV (Java, MIT) hybrid sorting classes: AdaptiveGrailSort.
// "Adaptive Grail Sort" by aphitorite (The Holy Grail Sort Project) - an O(1) space,
// stable, O(n log n) worst case block merge sort that is adaptive to existing runs.
// ASCII ONLY: no non-ASCII characters anywhere in this file.


    // AdaptiveGrailSort.Subarray
    enum class AdaptiveGrailSortSubarray { Left, Right };

    // AdaptiveGrailSort.multiSwap
    template<class T>
    void AdaptiveGrailSortMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
        using std::swap;
        for (ptrdiff_t i = 0; i < len; i++)
            swap(data_[a + i], data_[b + i]);
    }

    // AdaptiveGrailSort.multiTriSwap (changes len sized blocks order ABC -> BCA)
    template<class T>
    void AdaptiveGrailSortMultiTriSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t c, ptrdiff_t len) {
        for (ptrdiff_t i = 0; i < len; i++) {
            T temp = data_[a + i];
            data_[a + i] = data_[b + i];
            data_[b + i] = data_[c + i];
            data_[c + i] = temp;
        }
    }

    // AdaptiveGrailSort.insertTo (Java's array[(a--)-1] read: write index a, then decrement a)
    template<class T>
    void AdaptiveGrailSortInsertTo(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        T temp = data_[a];
        while (a > b) {
            data_[a] = data_[a - 1];
            --a;
        }
        data_[b] = temp;
    }

    // AdaptiveGrailSort.insertToBW (Java's array[(a++)+1] read: write index a, then increment a)
    template<class T>
    void AdaptiveGrailSortInsertToBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        T temp = data_[a];
        while (a < b) {
            data_[a] = data_[a + 1];
            ++a;
        }
        data_[a] = temp;
    }

    // AdaptiveGrailSort.shift
    template<class T>
    void AdaptiveGrailSortShift(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        using std::swap;
        while (m < b) {
            swap(data_[a], data_[m]);
            ++a;
            ++m;
        }
    }

    // AdaptiveGrailSort.rotate
    template<class T>
    void AdaptiveGrailSortRotate(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t l = m - a, r = b - m;

        while (l > 1 && r > 1) {
            if (r < l) {
                AdaptiveGrailSortMultiSwap(data_, m - r, m, r);
                b -= r;
                m -= r;
                l -= r;
            }
            else {
                AdaptiveGrailSortMultiSwap(data_, a, m, l);
                a += l;
                m += l;
                r -= l;
            }
        }

        if (r == 1)      AdaptiveGrailSortInsertTo(data_, m, a);
        else if (l == 1) AdaptiveGrailSortInsertToBW(data_, a, b - 1);
    }

    // AdaptiveGrailSort.leftBinarySearch
    template<class T>
    ptrdiff_t AdaptiveGrailSortLeftBinarySearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            if (val <= data_[m])
                b = m;
            else
                a = m + 1;
        }

        return a;
    }

    // AdaptiveGrailSort.rightBinarySearch
    template<class T>
    ptrdiff_t AdaptiveGrailSortRightBinarySearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            if (val < data_[m])
                b = m;
            else
                a = m + 1;
        }

        return a;
    }

    // AdaptiveGrailSort.buildUniqueRun
    template<class T>
    ptrdiff_t AdaptiveGrailSortBuildUniqueRun(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t n) {
        ptrdiff_t nKeys = 1, i = a + 1;

        // build run at start
        if (data_[i - 1] < data_[i]) {
            i++;
            nKeys++;

            while (nKeys < n && data_[i - 1] < data_[i]) {
                i++;
                nKeys++;
            }
        }
        else if (data_[i - 1] > data_[i]) {
            i++;
            nKeys++;

            while (nKeys < n && data_[i - 1] > data_[i]) {
                i++;
                nKeys++;
            }
            NSortHelpers::reverseRange(data_, a, i);
        }

        return nKeys;
    }

    // AdaptiveGrailSort.buildUniqueRunBW
    template<class T>
    ptrdiff_t AdaptiveGrailSortBuildUniqueRunBW(std::vector<T>& data_, ptrdiff_t b, ptrdiff_t n) {
        ptrdiff_t nKeys = 1, i = b - 1;

        // build run at end
        if (data_[i - 1] < data_[i]) {
            i--;
            nKeys++;

            while (nKeys < n && data_[i - 1] < data_[i]) {
                i--;
                nKeys++;
            }
        }
        else if (data_[i - 1] > data_[i]) {
            i--;
            nKeys++;

            while (nKeys < n && data_[i - 1] > data_[i]) {
                i--;
                nKeys++;
            }
            NSortHelpers::reverseRange(data_, i, b);
        }

        return nKeys;
    }

    // AdaptiveGrailSort.findKeys
    template<class T>
    ptrdiff_t AdaptiveGrailSortFindKeys(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t nKeys, ptrdiff_t n) {
        ptrdiff_t p = a, pEnd = a + nKeys;

        for (ptrdiff_t i = pEnd; i < b && nKeys < n; i++) {
            NSortHelpers::MarkArray(1, data_, i);
            ptrdiff_t loc = AdaptiveGrailSortLeftBinarySearch(data_, p, pEnd, data_[i]);

            if (pEnd == loc || data_[i] != data_[loc]) {
                AdaptiveGrailSortRotate(data_, p, pEnd, i);
                ptrdiff_t inc = i - pEnd;
                loc += inc;
                p += inc;
                pEnd += inc;

                AdaptiveGrailSortInsertTo(data_, pEnd, loc);
                nKeys++;
                pEnd++;
            }
        }
        AdaptiveGrailSortRotate(data_, a, p, pEnd);
        return nKeys;
    }

    // special thanks to @MP for this idea
    // AdaptiveGrailSort.findKeysBW
    template<class T>
    ptrdiff_t AdaptiveGrailSortFindKeysBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t nKeys, ptrdiff_t n) {
        ptrdiff_t p = b - nKeys, pEnd = b;

        for (ptrdiff_t i = p - 1; i >= a && nKeys < n; i--) {
            NSortHelpers::MarkArray(1, data_, i);
            ptrdiff_t loc = AdaptiveGrailSortLeftBinarySearch(data_, p, pEnd, data_[i]);

            if (pEnd == loc || data_[i] != data_[loc]) {
                AdaptiveGrailSortRotate(data_, i + 1, p, pEnd);
                ptrdiff_t inc = p - (i + 1);
                loc -= inc;
                pEnd -= inc;
                p -= inc + 1;
                nKeys++;

                AdaptiveGrailSortInsertToBW(data_, i, loc - 1);
            }
        }
        AdaptiveGrailSortRotate(data_, p, pEnd, b);
        return nKeys;
    }

    // instead of insertion level find & create runs divisible by minRun
    // AdaptiveGrailSort.buildRuns (Java field this.minRun is passed in as minRun)
    template<class T>
    void AdaptiveGrailSortBuildRuns(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t minRun) {
        ptrdiff_t i = a + 1, j = a;

        while (i < b) {
            // Java: Reads.compareIndices(array, i-1, i++, ...) == 1 - the index side
            // effect must happen before the increment is visible to the loop below.
            bool descending = data_[i - 1] > data_[i];
            i++;

            if (descending) {
                while (i < b && data_[i - 1] > data_[i]) i++;
                NSortHelpers::reverseRange(data_, j, i);
            }
            else while (i < b && data_[i - 1] <= data_[i]) i++;

            if (i < b) j = i - (i - j - 1) % minRun - 1; // a%b, if(a%b == 0) -> a = b

            while (i - j < minRun && i < b) {
                ptrdiff_t loc = AdaptiveGrailSortRightBinarySearch(data_, j, i, data_[i]);
                AdaptiveGrailSortInsertTo(data_, i, loc);
                i++;
            }
            j = i;
            i++;
        }
    }

    // AdaptiveGrailSort.binaryInsertion
    template<class T>
    void AdaptiveGrailSortBinaryInsertion(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        for (ptrdiff_t i = a + 1; i < b; i++) {
            ptrdiff_t loc = AdaptiveGrailSortRightBinarySearch(data_, a, i, data_[i]);
            AdaptiveGrailSortInsertTo(data_, i, loc);
        }
    }

    // AdaptiveGrailSort.mergeWithBufRest
    template<class T>
    void AdaptiveGrailSortMergeWithBufRest(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b,
                                           ptrdiff_t p, ptrdiff_t pLen) {
        using std::swap;
        ptrdiff_t i = 0, j = m, k = a;

        while (i < pLen && j < b) {
            if (data_[p + i] <= data_[j]) {
                swap(data_[k], data_[p + i]);
                k++;
                i++;
            }
            else {
                swap(data_[k], data_[j]);
                k++;
                j++;
            }
        }
        while (i < pLen) {
            swap(data_[k], data_[p + i]);
            k++;
            i++;
        }
    }

    // AdaptiveGrailSort.mergeWithBuf
    template<class T>
    void AdaptiveGrailSortMergeWithBuf(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t l = m - a;
        AdaptiveGrailSortMultiSwap(data_, p, a, l);
        AdaptiveGrailSortMergeWithBufRest(data_, a, m, b, p, l);
    }

    // AdaptiveGrailSort.mergeWithBufBW
    template<class T>
    void AdaptiveGrailSortMergeWithBufBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        using std::swap;
        ptrdiff_t pLen = b - m;
        AdaptiveGrailSortMultiSwap(data_, m, p, pLen);

        ptrdiff_t i = pLen - 1, j = m - 1, k = b - 1;

        while (i >= 0 && j >= a) {
            if (data_[p + i] >= data_[j]) {
                swap(data_[k], data_[p + i]);
                k--;
                i--;
            }
            else {
                swap(data_[k], data_[j]);
                k--;
                j--;
            }
        }
        while (i >= 0) {
            swap(data_[k], data_[p + i]);
            k--;
            i--;
        }
    }

    // AdaptiveGrailSort.inPlaceMerge
    template<class T>
    void AdaptiveGrailSortInPlaceMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = a, j = m, k;

        while (i < j && j < b) {
            if (data_[i] > data_[j]) {
                k = AdaptiveGrailSortLeftBinarySearch(data_, j + 1, b, data_[i]);
                AdaptiveGrailSortRotate(data_, i, j, k);

                i += k - j;
                j = k;
            }
            else i++;
        }
    }

    // AdaptiveGrailSort.inPlaceMergeBW
    template<class T>
    void AdaptiveGrailSortInPlaceMergeBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = m - 1, j = b - 1, k;

        while (j > i && i >= a) {
            if (data_[i] > data_[j]) {
                k = AdaptiveGrailSortRightBinarySearch(data_, a, i, data_[j]);
                AdaptiveGrailSortRotate(data_, k, i + 1, j + 1);

                j -= (i + 1) - k;
                i = k - 1;
            }
            else j--;
        }
    }

    // AdaptiveGrailSort.mergeWithoutBuf
    template<class T>
    void AdaptiveGrailSortMergeWithoutBuf(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        if (m - a > b - m) AdaptiveGrailSortInPlaceMergeBW(data_, a, m, b);
        else               AdaptiveGrailSortInPlaceMerge(data_, a, m, b);
    }

    // AdaptiveGrailSort.checkSorted
    template<class T>
    bool AdaptiveGrailSortCheckSorted(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        return data_[m - 1] > data_[m];
    }

    // AdaptiveGrailSort.checkReverseBounds
    template<class T>
    bool AdaptiveGrailSortCheckReverseBounds(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        if (data_[a] > data_[b - 1]) {
            AdaptiveGrailSortRotate(data_, a, m, b);
            return false;
        }

        return true;
    }

    // AdaptiveGrailSort.checkBounds
    template<class T>
    bool AdaptiveGrailSortCheckBounds(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        return AdaptiveGrailSortCheckSorted(data_, a, m, b)
            && AdaptiveGrailSortCheckReverseBounds(data_, a, m, b);
    }

    // AdaptiveGrailSort.grailGetSubarray
    template<class T>
    AdaptiveGrailSortSubarray AdaptiveGrailSortGrailGetSubarray(std::vector<T>& data_, ptrdiff_t t, ptrdiff_t mKey) {
        if (data_[t] < data_[mKey])
            return AdaptiveGrailSortSubarray::Left;

        else return AdaptiveGrailSortSubarray::Right;
    }

    // returns mKey final position
    // AdaptiveGrailSort.blockSelectSort
    template<class T>
    ptrdiff_t AdaptiveGrailSortBlockSelectSort(std::vector<T>& data_, ptrdiff_t p, ptrdiff_t t, ptrdiff_t r,
                                               ptrdiff_t d, ptrdiff_t lCount, ptrdiff_t bCount, ptrdiff_t bLen) {
        using std::swap;
        ptrdiff_t mKey = lCount;

        for (ptrdiff_t j = 0, k = lCount + 1; j < k - 1; j++) {
            ptrdiff_t min = j;

            for (ptrdiff_t i = (std::max)(lCount - r, j + 1); i < k; i++) {
                int comp = NSortHelpers::CompareValues(data_[p + d + i * bLen], data_[p + d + min * bLen]);

                if (comp < 0 || (comp == 0 && data_[t + i] < data_[t + min]))
                    min = i;
            }

            if (min != j) {
                AdaptiveGrailSortMultiSwap(data_, p + j * bLen, p + min * bLen, bLen);
                swap(data_[t + j], data_[t + min]);

                if (k < bCount && min == k - 1) k++;
            }
            if (min == mKey) mKey = j;
        }

        return t + mKey;
    }

    // special thanks to @Anonymous0726 for this idea
    // AdaptiveGrailSort.grailSortKeys
    template<class T>
    void AdaptiveGrailSortGrailSortKeys(std::vector<T>& data_, ptrdiff_t b, ptrdiff_t p, ptrdiff_t mKey) {
        using std::swap;
        swap(data_[p], data_[mKey]);
        ptrdiff_t i = mKey, j = i + 1, k = p + 1;

        while (j < b) {
            if (data_[j] < data_[p]) {
                swap(data_[i], data_[j]);
                i++;
            }
            else {
                swap(data_[k], data_[j]);
                k++;
            }

            j++;
        }

        AdaptiveGrailSortMultiSwap(data_, i, p, b - i);
    }

    // AdaptiveGrailSort.grailSortKeysWithoutBuf
    template<class T>
    void AdaptiveGrailSortGrailSortKeysWithoutBuf(std::vector<T>& data_, ptrdiff_t b, ptrdiff_t mKey) {
        ptrdiff_t i = mKey, j = i + 1;

        while (j < b) {
            if (data_[j] < data_[i]) {
                AdaptiveGrailSortInsertTo(data_, j, i);
                i++;
            }

            j++;
        }
    }

    // special thanks to @Anonymous0726 for this idea
    // AdaptiveGrailSort.grailMergeBlocks
    template<class T>
    ptrdiff_t AdaptiveGrailSortGrailMergeBlocks(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        using std::swap;
        ptrdiff_t i = a, j = m;

        while (i < m && j < b) {
            if (data_[i] <= data_[j]) {
                swap(data_[p], data_[i]);
                p++;
                i++;
            }
            else {
                swap(data_[p], data_[j]);
                p++;
                j++;
            }
        }

        if (i > p) {
            while (i < m) {
                swap(data_[p], data_[i]);
                p++;
                i++;
            }
        }
        return j;
    }

    // same as grailMergeBlocks() except reverses equal items order
    // AdaptiveGrailSort.grailMergeBlocksRev
    template<class T>
    ptrdiff_t AdaptiveGrailSortGrailMergeBlocksRev(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        using std::swap;
        ptrdiff_t i = a, j = m;

        while (i < m && j < b) {
            if (data_[i] < data_[j]) {
                swap(data_[p], data_[i]);
                p++;
                i++;
            }
            else {
                swap(data_[p], data_[j]);
                p++;
                j++;
            }
        }

        if (i > p) {
            while (i < m) {
                swap(data_[p], data_[i]);
                p++;
                i++;
            }
        }
        return j;
    }

    // is never called if m-a || b-m <= bLen
    // should never be called if (m-a)%bLen != 0
    // AdaptiveGrailSort.grailBlockMerge
    template<class T>
    void AdaptiveGrailSortGrailBlockMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b,
                                          ptrdiff_t t, ptrdiff_t p, ptrdiff_t bLen) {
        ptrdiff_t b1 = b - (b - m - 1) % bLen - 1,
                  i = a + bLen, j = a, key = t - 1,
                  lCount = (m - i) / bLen, bCount = (b1 - i) / bLen, l = -1, r = lCount - 1;

        AdaptiveGrailSortMultiTriSwap(data_, p, m - bLen, a, bLen);
        AdaptiveGrailSortInsertToBW(data_, t, t + lCount - 1);

        ptrdiff_t mKey = AdaptiveGrailSortBlockSelectSort(data_, i, t, 1, bLen - 1, lCount, bCount, bLen);
        AdaptiveGrailSortSubarray frag = AdaptiveGrailSortSubarray::Left;

        while (l < lCount && r < bCount) {
            if (frag == AdaptiveGrailSortSubarray::Left) {
                do {
                    j += bLen;
                    l++;
                    key++;
                } while (l < lCount && AdaptiveGrailSortGrailGetSubarray(data_, key, mKey) == AdaptiveGrailSortSubarray::Left);

                if (l == lCount) {
                    i = AdaptiveGrailSortGrailMergeBlocks(data_, i, j, b, i - bLen);
                    AdaptiveGrailSortMergeWithBufRest(data_, i - bLen, i, b, p, bLen);
                }
                else i = AdaptiveGrailSortGrailMergeBlocks(data_, i, j, j + bLen - 1, i - bLen);

                frag = AdaptiveGrailSortSubarray::Right;
            }
            else {
                do {
                    j += bLen;
                    r++;
                    key++;
                } while (r < bCount && AdaptiveGrailSortGrailGetSubarray(data_, key, mKey) == AdaptiveGrailSortSubarray::Right);

                if (r == bCount) {
                    AdaptiveGrailSortShift(data_, i - bLen, i, b);
                    AdaptiveGrailSortMultiSwap(data_, p, b - bLen, bLen);
                }
                else i = AdaptiveGrailSortGrailMergeBlocksRev(data_, i, j, j + bLen - 1, i - bLen);

                frag = AdaptiveGrailSortSubarray::Left;
            }
        }

        AdaptiveGrailSortGrailSortKeys(data_, t + bCount, p, mKey);
    }

    // old
    // AdaptiveGrailSort.grailBlockMergeWithoutBuf
    // Upstream note (kept as-is): this path addresses bCount tag slots at t, but its caller
    // only owns `keys` slots there, and bCount = (b1-j)/bLen + 1 can exceed keys when the
    // truncating bLen = 2*j/tLen leaves a partial block. For few-distinct-value inputs the
    // Java would raise ArrayIndexOutOfBoundsException; the port matches it index for index.
    template<class T>
    void AdaptiveGrailSortGrailBlockMergeWithoutBuf(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b,
                                                    ptrdiff_t t, ptrdiff_t bLen) {
        ptrdiff_t a1 = a + (m - a) % bLen, b1 = b - (b - m) % bLen,
                  i = a, j = a1, key = t,
                  lCount = (m - j) / bLen + 1, bCount = (b1 - j) / bLen + 1, l = 0, r = lCount;

        ptrdiff_t mKey = AdaptiveGrailSortBlockSelectSort(data_, j, t, 0, 0, lCount - 1, bCount - 1, bLen);
        AdaptiveGrailSortSubarray frag = AdaptiveGrailSortSubarray::Left;

        while (l < lCount && r < bCount) {
            AdaptiveGrailSortSubarray next = AdaptiveGrailSortGrailGetSubarray(data_, key, mKey);
            key++;

            if (next == frag) {
                if (frag == AdaptiveGrailSortSubarray::Left) l++;
                else                                         r++;
                i = j;
            }
            else { // grailMergeBlocksWithoutBuf()
                ptrdiff_t m2 = j, b2 = j + bLen, k = 0;

                if (frag == AdaptiveGrailSortSubarray::Left) {
                    while (i < m2 && m2 < b2) {
                        if (data_[i] > data_[m2]) {
                            k = AdaptiveGrailSortLeftBinarySearch(data_, m2 + 1, b2, data_[i]);
                            AdaptiveGrailSortRotate(data_, i, m2, k);

                            i += k - m2;
                            m2 = k;
                        }
                        else i++;
                    }
                }
                else {
                    while (i < m2 && m2 < b2) {
                        if (data_[i] >= data_[m2]) {
                            k = AdaptiveGrailSortRightBinarySearch(data_, m2 + 1, b2, data_[i]);
                            AdaptiveGrailSortRotate(data_, i, m2, k);

                            i += k - m2;
                            m2 = k;
                        }
                        else i++;
                    }
                }

                if (i < m2) { // right side is merged first
                    if (next == AdaptiveGrailSortSubarray::Left) l++;
                    else                                         r++;
                }
                else {
                    if (frag == AdaptiveGrailSortSubarray::Left) l++;
                    else                                         r++;
                    frag = next;
                }
            }

            j += bLen;
        }

        if (l < lCount) AdaptiveGrailSortInPlaceMergeBW(data_, a, b1, b);
        AdaptiveGrailSortGrailSortKeysWithoutBuf(data_, t + bCount - 1, mKey);
    }

    // AdaptiveGrailSort.smartMerge
    template<class T>
    void AdaptiveGrailSortSmartMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        if (AdaptiveGrailSortCheckBounds(data_, a, m, b)) {
            a = AdaptiveGrailSortRightBinarySearch(data_, a, m - 1, data_[m]);
            AdaptiveGrailSortMergeWithBuf(data_, a, m, b, p);
        }
    }

    // AdaptiveGrailSort.smartMergeBW
    template<class T>
    void AdaptiveGrailSortSmartMergeBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        if (AdaptiveGrailSortCheckBounds(data_, a, m, b)) {
            b = AdaptiveGrailSortLeftBinarySearch(data_, m + 1, b, data_[m - 1]);
            AdaptiveGrailSortMergeWithBufBW(data_, a, m, b, p);
        }
    }

    // AdaptiveGrailSort.smartBlockMerge
    template<class T>
    void AdaptiveGrailSortSmartBlockMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b,
                                          ptrdiff_t t, ptrdiff_t p, ptrdiff_t bLen) {
        if (AdaptiveGrailSortCheckBounds(data_, a, m, b)) {
            ptrdiff_t n = AdaptiveGrailSortRightBinarySearch(data_, a, m - 1, data_[m]);
            b = AdaptiveGrailSortLeftBinarySearch(data_, m + 1, b, data_[m - 1]);

            if (AdaptiveGrailSortCheckReverseBounds(data_, n, m, b)) {
                if (m - n <= bLen || b - m <= bLen) {
                    if (b - m < m - n) AdaptiveGrailSortMergeWithBufBW(data_, n, m, b, p);
                    else               AdaptiveGrailSortMergeWithBuf(data_, n, m, b, p);
                }
                else {
                    n -= (n - a) % bLen;
                    AdaptiveGrailSortGrailBlockMerge(data_, n, m, b, t, p, bLen);
                }
            }
        }
    }

    // AdaptiveGrailSort.smartBlockMergeWithoutBuf
    template<class T>
    void AdaptiveGrailSortSmartBlockMergeWithoutBuf(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b,
                                                    ptrdiff_t t, ptrdiff_t bLen) {
        if (AdaptiveGrailSortCheckBounds(data_, a, m, b)) {
            a = AdaptiveGrailSortRightBinarySearch(data_, a, m - 1, data_[m]);

            if (m - a <= bLen) AdaptiveGrailSortInPlaceMerge(data_, a, m, b);
            else               AdaptiveGrailSortGrailBlockMergeWithoutBuf(data_, a, m, b, t, bLen);
        }
    }

    // AdaptiveGrailSort.smartInPlaceMerge
    template<class T>
    void AdaptiveGrailSortSmartInPlaceMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        if (AdaptiveGrailSortCheckSorted(data_, a, m, b))
            AdaptiveGrailSortInPlaceMergeBW(data_, a, m, b);
    }

    // AdaptiveGrailSort.redistBuffer
    template<class T>
    void AdaptiveGrailSortRedistBuffer(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t rPos = AdaptiveGrailSortLeftBinarySearch(data_, m, b, data_[a]);
        AdaptiveGrailSortRotate(data_, a, m, rPos);

        ptrdiff_t dist = rPos - m;
        a += dist;
        m += dist;

        ptrdiff_t a1 = a + (m - a) / 2;
        rPos = AdaptiveGrailSortLeftBinarySearch(data_, m, b, data_[a1]);
        AdaptiveGrailSortRotate(data_, a1, m, rPos);

        dist = rPos - m;
        a1 += dist;
        m += dist;

        AdaptiveGrailSortMergeWithoutBuf(data_, a, a1 - dist, a1);
        AdaptiveGrailSortMergeWithoutBuf(data_, a1, m, b);
    }

    // AdaptiveGrailSort.redistBufferBW
    template<class T>
    void AdaptiveGrailSortRedistBufferBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t rPos = AdaptiveGrailSortRightBinarySearch(data_, a, m, data_[b - 1]);
        AdaptiveGrailSortRotate(data_, rPos, m, b);

        ptrdiff_t dist = m - rPos;
        b -= dist;
        m -= dist;

        ptrdiff_t b1 = m + (b - m) / 2;
        rPos = AdaptiveGrailSortRightBinarySearch(data_, a, m, data_[b1 - 1]);
        AdaptiveGrailSortRotate(data_, rPos, m, b1);

        dist = m - rPos;
        b1 -= dist;
        m -= dist;

        AdaptiveGrailSortMergeWithoutBuf(data_, b1, b1 + dist, b);
        AdaptiveGrailSortMergeWithoutBuf(data_, a, m, b1);
    }

    // AdaptiveGrailSort.inPlaceMergeSort
    template<class T>
    void AdaptiveGrailSortInPlaceMergeSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t minRun) {
        AdaptiveGrailSortBuildRuns(data_, a, b, minRun);

        ptrdiff_t len = b - a;
        ptrdiff_t i = a;
        for (ptrdiff_t j = minRun; j < len; j *= 2) {
            for (i = a; i + 2 * j <= b; i += 2 * j)
                AdaptiveGrailSortSmartInPlaceMerge(data_, i, i + j, i + 2 * j);

            if (i + j < b)
                AdaptiveGrailSortSmartInPlaceMerge(data_, i, i + j, b);
        }
    }

    // AdaptiveGrailSort.grailAdaptiveSortWithoutBuf
    template<class T>
    void AdaptiveGrailSortGrailAdaptiveSortWithoutBuf(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b,
                                                      ptrdiff_t keys, ptrdiff_t ideal, bool bwBuf, ptrdiff_t minRun) {
        ptrdiff_t len = b - a, bLen;
        for (bLen = (std::min)(keys, minRun); 2 * bLen <= keys; bLen *= 2);
        ptrdiff_t tLen = keys - bLen;

        ptrdiff_t i = 0, j = minRun,
                  t = 0, p = 0, a1 = 0, b1 = 0;

        if (bwBuf) {
            p = b - bLen; a1 = a; b1 = p - tLen; t = b1;
        }
        else {
            p = a + tLen; a1 = p + bLen; b1 = b; t = a;
        }

        // insertion level
        AdaptiveGrailSortBuildRuns(data_, a1, b1, minRun);

        // merge with buffer level
        while (j <= bLen && j < len) {
            for (i = a1; i + 2 * j <= b1; i += 2 * j)
                AdaptiveGrailSortSmartMerge(data_, i, i + j, i + 2 * j, p);

            if (i + j < b1)
                AdaptiveGrailSortSmartMergeBW(data_, i, i + j, b1, p);

            j *= 2;
        }

        if (bLen / 2 >= minRun && bLen / 2 >= (keys + 1) / 2) {
            AdaptiveGrailSortBinaryInsertion(data_, p, p + bLen);

            bLen /= 2;
            tLen = keys - bLen;
            p += bLen;
        }

        // block merge level
        while (tLen >= 2 * j / bLen - 1 && j < len) {
            for (i = a1; i + 2 * j <= b1; i += 2 * j)
                AdaptiveGrailSortSmartBlockMerge(data_, i, i + j, i + 2 * j, t, p, bLen);

            if (i + j < b1) {
                if (b1 - (i + j) > bLen)
                    AdaptiveGrailSortSmartBlockMerge(data_, i, i + j, b1, t, p, bLen);

                else AdaptiveGrailSortSmartMergeBW(data_, i, i + j, b1, p);
            }

            j *= 2;
        }

        AdaptiveGrailSortBinaryInsertion(data_, p, p + bLen);
        tLen = keys - keys % 2;

        // block merge w/o buffer level
        while (j < len) {
            bLen = 2 * j / tLen;

            for (i = a1; i + 2 * j <= b1; i += 2 * j)
                AdaptiveGrailSortSmartBlockMergeWithoutBuf(data_, i, i + j, i + 2 * j, t, bLen);

            if (i + j < b1) {
                if (b1 - (i + j) > bLen)
                    AdaptiveGrailSortSmartBlockMergeWithoutBuf(data_, i, i + j, b1, t, bLen);

                else AdaptiveGrailSortSmartInPlaceMerge(data_, i, i + j, b1);
            }

            j *= 2;
        }

        // buffer redistribution
        if (bwBuf) {
            a = AdaptiveGrailSortRightBinarySearch(data_, a, b1, data_[b1]);
            if (keys >= ideal / 2) AdaptiveGrailSortRedistBufferBW(data_, a, b1, b);
            else                   AdaptiveGrailSortMergeWithoutBuf(data_, a, b1, b);
        }
        else {
            b = AdaptiveGrailSortLeftBinarySearch(data_, a1, b, data_[a1 - 1]);
            if (keys >= ideal / 2) AdaptiveGrailSortRedistBuffer(data_, a, a1, b);
            else                   AdaptiveGrailSortMergeWithoutBuf(data_, a, a1, b);
        }
    }

    // AdaptiveGrailSort.grailAdaptiveSort
    template<class T>
    void AdaptiveGrailSortGrailAdaptiveSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t len = b - a;

        // insertion on small len
        if (len < 31) {
            AdaptiveGrailSortBinaryInsertion(data_, a, b);
            return;
        }
        // mini adaptive grail sort
        if (len < 63) {
            ptrdiff_t minRun = (len + 1) / 2;
            AdaptiveGrailSortBuildRuns(data_, a, b, minRun);

            ptrdiff_t m = a + minRun;
            if (AdaptiveGrailSortCheckBounds(data_, a, m, b))
                AdaptiveGrailSortRedistBufferBW(data_, a, m, b);

            return;
        }

        // calculate optimal minRun & block len
        ptrdiff_t minRun;
        for (minRun = len; minRun >= 32; minRun = (minRun + 1) / 2);

        ptrdiff_t bLen;
        for (bLen = minRun; bLen * bLen < len; bLen *= 2);

        ptrdiff_t tLen = len / bLen - 2,
                  ideal = tLen + bLen;

        // choose direction to find keys
        bool bwBuf;
        ptrdiff_t rRun = AdaptiveGrailSortBuildUniqueRunBW(data_, b, ideal), lRun = 0;

        if (rRun == ideal) bwBuf = true;
        else {
            lRun = AdaptiveGrailSortBuildUniqueRun(data_, a, ideal);

            if (lRun == ideal) bwBuf = false;
            else bwBuf = (rRun < 16 && lRun < 16) || rRun >= lRun;
        }

        // find bLen + tLen unique buffer keys
        ptrdiff_t keys = bwBuf ? AdaptiveGrailSortFindKeysBW(data_, a, b, rRun, ideal)
                               : AdaptiveGrailSortFindKeys(data_, a, b, lRun, ideal);

        if (keys < ideal) {
            if (keys == 1)            return;
            else if (keys <= 4)       AdaptiveGrailSortInPlaceMergeSort(data_, a, b, minRun);
            else                      AdaptiveGrailSortGrailAdaptiveSortWithoutBuf(data_, a, b, keys, ideal, bwBuf, minRun);
            return;
        }

        ptrdiff_t i = 0, j = minRun,
                  t = 0, p = 0, a1 = 0, b1 = 0;

        if (bwBuf) {
            p = b - bLen; a1 = a; b1 = p - tLen; t = b1;
        }
        else {
            p = a + tLen; a1 = p + bLen; b1 = b; t = a;
        }

        // insertion level
        AdaptiveGrailSortBuildRuns(data_, a1, b1, minRun);

        // merge with buffer level
        while (j <= bLen && j < len) {
            for (i = a1; i + 2 * j <= b1; i += 2 * j)
                AdaptiveGrailSortSmartMerge(data_, i, i + j, i + 2 * j, p);

            if (i + j < b1)
                AdaptiveGrailSortSmartMergeBW(data_, i, i + j, b1, p);

            j *= 2;
        }

        // block merge level
        while (j < len) {
            for (i = a1; i + 2 * j <= b1; i += 2 * j)
                AdaptiveGrailSortSmartBlockMerge(data_, i, i + j, i + 2 * j, t, p, bLen);

            if (i + j < b1) {
                if (b1 - (i + j) > bLen)
                    AdaptiveGrailSortSmartBlockMerge(data_, i, i + j, b1, t, p, bLen);

                else AdaptiveGrailSortSmartMergeBW(data_, i, i + j, b1, p);
            }

            j *= 2;
        }

        AdaptiveGrailSortBinaryInsertion(data_, p, p + bLen);

        // buffer redistribution
        if (bwBuf) {
            a = AdaptiveGrailSortRightBinarySearch(data_, a, b1, data_[b1]);
            AdaptiveGrailSortRedistBufferBW(data_, a, b1, b);
        }
        else {
            b = AdaptiveGrailSortLeftBinarySearch(data_, a1, b, data_[a1 - 1]);
            AdaptiveGrailSortRedistBuffer(data_, a, a1, b);
        }
    }

    // AdaptiveGrailSort.runSort
    template<class T = int>
    void AdaptiveGrailSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        AdaptiveGrailSortGrailAdaptiveSort(data_, 0, static_cast<ptrdiff_t>(data_.size()));
    }

    // ==================== FifthMergeSort + ChaliceSort ====================
// Ports of ArrayV (Java, MIT) hybrid sorting classes: FifthMergeSort, ChaliceSort.
// FifthMergeSort's entry point is defined first because ChaliceSort calls it for
// ranges of 32..127 elements.


    // =========================================================================
    // FifthMergeSort (Josiah "Gaming32" Glosson, ArrayV, MIT)
    // =========================================================================

    // FifthMergeSort.IndexPair
    struct FifthMergeSortIndexPair {
        ptrdiff_t aEnd;
        ptrdiff_t bEnd;
    };

    // FifthMergeSort.mergeInPlaceForwards
    template<class T>
    void FifthMergeSortMergeInPlaceForwards(std::vector<T>& data_, ptrdiff_t buffer, ptrdiff_t start, ptrdiff_t mid, ptrdiff_t end) {
        ptrdiff_t left = start, right = mid;
        while (left < mid && right < end) {
            if (data_[left] <= data_[right]) {
                data_[buffer] = data_[left];
                ++buffer;
                ++left;
                NSortHelpers::MarkArray(3, data_, buffer);
            }
            else {
                data_[buffer] = data_[right];
                ++buffer;
                ++right;
                NSortHelpers::MarkArray(3, data_, buffer);
            }
        }
        // Highlights.clearAllMarks();

        while (left < mid) {
            data_[buffer] = data_[left];
            ++buffer;
            ++left;
        }
        while (right < end) {
            data_[buffer] = data_[right];
            ++buffer;
            ++right;
        }
    }

    // FifthMergeSort.mergeInPlaceBackwards
    template<class T>
    FifthMergeSortIndexPair FifthMergeSortMergeInPlaceBackwards(std::vector<T>& data_, ptrdiff_t buffer, ptrdiff_t bufferLen, ptrdiff_t mid, ptrdiff_t end) {
        ptrdiff_t left = mid - 1, right = end - 1;
        while (buffer > right && right >= mid) {
            if (data_[left] > data_[right]) {
                NSortHelpers::MarkArray(3, data_, buffer);
                data_[buffer] = data_[left];
                --buffer;
                --left;
            }
            else {
                NSortHelpers::MarkArray(3, data_, buffer);
                data_[buffer] = data_[right];
                --buffer;
                --right;
            }
        }
        // Highlights.clearAllMarks();
        if (right == left) {
            while (right >= 0) {
                data_[buffer] = data_[right];
                --buffer;
                --right;
            }
        }
        else if (right < mid) {
            while (left >= 0) {
                data_[buffer] = data_[left];
                --buffer;
                --left;
            }
        }
        return FifthMergeSortIndexPair{left + 1, right + 1};
    }

    // FifthMergeSort.mergeForwardsWithBuffer (buffer may alias data_)
    template<class T>
    void FifthMergeSortMergeForwardsWithBuffer(std::vector<T>& data_, std::vector<T>& buffer, ptrdiff_t dest, ptrdiff_t left, ptrdiff_t leftEnd, ptrdiff_t mid, ptrdiff_t end) {
        ptrdiff_t right = mid;
        while (left < leftEnd && right < end) {
            NSortHelpers::MarkArray(2, data_, left);
            NSortHelpers::MarkArray(3, data_, right);
            if (buffer[left] <= data_[right]) {
                data_[dest] = buffer[left];
                ++dest;
                ++left;
            }
            else {
                data_[dest] = data_[right];
                ++dest;
                ++right;
            }
        }
        // Highlights.clearMark(3);

        while (left < leftEnd) {
            NSortHelpers::MarkArray(2, data_, left);
            data_[dest] = buffer[left];
            ++dest;
            ++left;
        }
    }

    // FifthMergeSort.merge
    template<class T>
    void FifthMergeSortMerge(std::vector<T>& data_, std::vector<T>& buffer, ptrdiff_t chunkOffset, ptrdiff_t start, ptrdiff_t mid, ptrdiff_t end, bool fromBuffer) {
        std::vector<T>* from;
        std::vector<T>* to;
        ptrdiff_t writepos;
        if (fromBuffer) {
            from = &buffer;
            to = &data_;
            writepos = start;
            start -= chunkOffset;
            mid -= chunkOffset;
            end -= chunkOffset;
        }
        else {
            from = &data_;
            to = &buffer;
            writepos = start - chunkOffset;
        }

        ptrdiff_t left = start, right = mid;
        while (left < mid && right < end) {
            if ((*from)[left] <= (*from)[right]) {
                (*to)[writepos] = (*from)[left];
                ++writepos;
                ++left;
            }
            else {
                (*to)[writepos] = (*from)[right];
                ++writepos;
                ++right;
            }
        }
        // Highlights.clearMark(2);

        while (left < mid) {
            (*to)[writepos] = (*from)[left];
            ++writepos;
            ++left;
        }
        while (right < end) {
            (*to)[writepos] = (*from)[right];
            ++writepos;
            ++right;
        }
    }

    // FifthMergeSort.pingPong
    template<class T>
    void FifthMergeSortPingPong(std::vector<T>& data_, std::vector<T>& buffer, ptrdiff_t start, ptrdiff_t end) {
        ptrdiff_t i;
        for (i = start; i + 8 < end; i += 8) {
            NSortHelpers::binaryInsertSort(data_, i, i + 8); // inserter.customBinaryInsert
        }
        if (end - i > 1) {
            NSortHelpers::binaryInsertSort(data_, i, end);
        }

        ptrdiff_t length = end - start;
        bool fromBuffer = false;
        for (ptrdiff_t gap = 8; gap < length; gap *= 2) {
            ptrdiff_t fullMerge = gap * 2;
            for (i = start; i + fullMerge < end; i += fullMerge) {
                FifthMergeSortMerge(data_, buffer, start, i, i + gap, i + fullMerge, fromBuffer);
            }
            if (i + gap < end) {
                FifthMergeSortMerge(data_, buffer, start, i, i + gap, end, fromBuffer);
            }
            else {
                if (fromBuffer) {
                    // Writes.arraycopy(buffer, i - start, array, i, end - i, ...)
                    for (ptrdiff_t k = 0; k < end - i; ++k)
                        data_[i + k] = buffer[i - start + k];
                }
                else {
                    // Writes.arraycopy(array, i, buffer, i - start, end - i, ...)
                    for (ptrdiff_t k = 0; k < end - i; ++k)
                        buffer[i - start + k] = data_[i + k];
                }
            }
            fromBuffer = !fromBuffer;
        }
        if (fromBuffer) {
            // Writes.arraycopy(buffer, 0, array, start, length, ...)
            for (ptrdiff_t k = 0; k < length; ++k)
                data_[start + k] = buffer[k];
        }
    }

    // port of FifthMergeSort.runSort (the class's own driver is fifthMergeSort)
    template<class T = int>
    void FifthMergeSort(std::vector<T>& data_) {
        ptrdiff_t currentLength = static_cast<ptrdiff_t>(data_.size());
        if (currentLength < 2) return;

        ptrdiff_t fifthLen = currentLength / 5;
        ptrdiff_t bufferLen = currentLength - fifthLen * 4;
        std::vector<T> buffer(bufferLen);

        FifthMergeSortPingPong(data_, buffer, 0, bufferLen);
        for (ptrdiff_t i = 0, start = bufferLen; i < 4; ++i, start += fifthLen) {
            FifthMergeSortPingPong(data_, buffer, start, start + fifthLen);
        }

        // Writes.arraycopy(array, 0, buffer, 0, bufferLen, ...)
        for (ptrdiff_t k = 0; k < bufferLen; ++k)
            buffer[k] = data_[k];
        ptrdiff_t twoFifths = 2 * fifthLen;
        for (ptrdiff_t i = 0, start = bufferLen; i < 2; ++i, start += twoFifths) {
            FifthMergeSortMergeInPlaceForwards(data_, start - bufferLen, start, start + fifthLen, start + twoFifths);
        }

        FifthMergeSortIndexPair finalMerge = FifthMergeSortMergeInPlaceBackwards(data_, currentLength - 1, bufferLen, twoFifths, 2 * twoFifths);
        if (finalMerge.bEnd > 0) {
            FifthMergeSortMergeForwardsWithBuffer(data_, data_, bufferLen, 0, finalMerge.aEnd, twoFifths, currentLength);
        }

        FifthMergeSortMergeForwardsWithBuffer(data_, buffer, 0, 0, bufferLen, bufferLen, currentLength);
    }

    // =========================================================================
    // ChaliceSort (aphitorite, ArrayV, MIT): stable merge sort using an
    // O(cbrt n) dynamic external buffer. Extends BlockMergeSorting, whose
    // block* helpers live in NSortHelpers and are called instead of re-ported.
    // =========================================================================

    // ChaliceSort.ceilCbrt
    inline ptrdiff_t ChaliceSortCeilCbrt(ptrdiff_t n) {
        ptrdiff_t a = 0, b = 11;

        while (a < b) {
            ptrdiff_t m = (a + b) / 2;

            if ((static_cast<ptrdiff_t>(1) << (3 * m)) >= n) b = m;
            else a = m + 1;
        }

        return static_cast<ptrdiff_t>(1) << a;
    }

    // ChaliceSort.calcKeys (assumes keys needed is <= n/4)
    inline ptrdiff_t ChaliceSortCalcKeys(ptrdiff_t bLen, ptrdiff_t n) {
        ptrdiff_t a = 1, b = n / 4;

        while (a < b) {
            ptrdiff_t m = (a + b) / 2;

            if ((n - 4 * m - 1) / bLen - 2 < m) b = m;
            else a = m + 1;
        }

        return a;
    }

    // ChaliceSort.laziestSortExt
    template<class T>
    void ChaliceSortLaziestSortExt(std::vector<T>& data_, std::vector<T>& tmp, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t s = static_cast<ptrdiff_t>(tmp.size());

        for (ptrdiff_t i = a; i < b; i += s) {
            ptrdiff_t j = (std::min)(b, i + s);
            NSortHelpers::blockBinaryInsertion(data_, i, j);
            if (i > a) NSortHelpers::blockMergeBWExt(data_, tmp, a, i, j);
        }
    }

    // ChaliceSort.findKeysSm
    template<class T>
    std::pair<ptrdiff_t, ptrdiff_t> ChaliceSortFindKeysSm(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t a1, ptrdiff_t b1, bool full, ptrdiff_t n) {
        ptrdiff_t p = a, pEnd = 0;

        if (full) {
            for (; p < b; ++p) {
                NSortHelpers::MarkArray(1, data_, p);
                ptrdiff_t loc = NSortHelpers::blockLeftBinSearch(data_, a1, b1, data_[p]);

                if (loc == b1 || NSortHelpers::CompareValues(data_[p], data_[loc]) != 0) {
                    pEnd = p + 1;
                    break;
                }
            }
            if (pEnd != 0) {
                for (ptrdiff_t i = pEnd; i < b && pEnd - p < n; ++i) {
                    NSortHelpers::MarkArray(1, data_, i);
                    ptrdiff_t loc = NSortHelpers::blockLeftBinSearch(data_, a1, b1, data_[i]);

                    if (loc == b1 || NSortHelpers::CompareValues(data_[i], data_[loc]) != 0) {
                        loc = NSortHelpers::blockLeftBinSearch(data_, p, pEnd, data_[i]);

                        if (loc == pEnd || NSortHelpers::CompareValues(data_[i], data_[loc]) != 0) {
                            NSortHelpers::blockRotate(data_, p, pEnd, i);

                            ptrdiff_t len1 = i - pEnd;
                            p += len1;
                            loc += len1;
                            pEnd = i + 1;

                            NSortHelpers::blockInsertTo(data_, i, loc);
                        }
                    }
                }
            }
            else pEnd = p;
        }
        else {
            pEnd = p + 1;

            for (ptrdiff_t i = pEnd; i < b && pEnd - p < n; ++i) {
                NSortHelpers::MarkArray(1, data_, i);
                ptrdiff_t loc = NSortHelpers::blockLeftBinSearch(data_, p, pEnd, data_[i]);

                if (loc == pEnd || NSortHelpers::CompareValues(data_[i], data_[loc]) != 0) {
                    NSortHelpers::blockRotate(data_, p, pEnd, i);

                    ptrdiff_t len1 = i - pEnd;
                    p += len1;
                    loc += len1;
                    pEnd = i + 1;

                    NSortHelpers::blockInsertTo(data_, i, loc);
                }
            }
        }
        return std::pair<ptrdiff_t, ptrdiff_t>(p, pEnd);
    }

    // ChaliceSort.findKeys (searches for n keys s blocks at a time)
    template<class T>
    ptrdiff_t ChaliceSortFindKeys(std::vector<T>& data_, std::vector<T>& tmp, ptrdiff_t a, ptrdiff_t b, ptrdiff_t n, ptrdiff_t s) {
        std::pair<ptrdiff_t, ptrdiff_t> t = ChaliceSortFindKeysSm(data_, a, b, 0, 0, false, (std::min)(n, s));
        ptrdiff_t p = t.first, pEnd = t.second;

        if (s < n && pEnd - p == s) {
            for (n -= s; ; n -= s) {
                t = ChaliceSortFindKeysSm(data_, pEnd, b, p, pEnd, true, (std::min)(s, n));
                ptrdiff_t keys = t.second - t.first;

                if (keys == 0) break;

                if (keys < s || n == s) {
                    NSortHelpers::blockRotate(data_, pEnd, t.first, t.second);

                    t.first = pEnd;
                    pEnd += keys;

                    NSortHelpers::blockMergeBWExt(data_, tmp, p, t.first, pEnd); // merge can be done inplace + stable
                    break;
                }
                else {
                    NSortHelpers::blockRotate(data_, p, pEnd, t.first);

                    p += t.first - pEnd;
                    pEnd = t.second;

                    NSortHelpers::blockMergeBWExt(data_, tmp, p, t.first, pEnd);
                }
            }
        }
        NSortHelpers::blockRotate(data_, a, p, pEnd);
        return pEnd - p;
    }

    // ChaliceSort.findBitsSm
    template<class T>
    std::pair<ptrdiff_t, ptrdiff_t> ChaliceSortFindBitsSm(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t a1, bool bw, ptrdiff_t n) {
        ptrdiff_t p = a, pEnd, cmp = bw ? -1 : 1;

        while (p < b && NSortHelpers::CompareValues(data_[p], data_[a1]) != cmp) ++p;
        ++a1;

        if (p < b) {
            pEnd = p + 1;

            for (ptrdiff_t i = pEnd; i < b && pEnd - p < n; ++i) {
                if (NSortHelpers::CompareValues(data_[i], data_[a1]) == cmp) {
                    NSortHelpers::blockRotate(data_, p, pEnd, i);

                    p += i - pEnd;
                    pEnd = i + 1;
                    ++a1;
                }
            }
        }
        else pEnd = p;

        return std::pair<ptrdiff_t, ptrdiff_t>(p, pEnd);
    }

    // ChaliceSort.findBits
    template<class T>
    ptrdiff_t ChaliceSortFindBits(std::vector<T>& data_, std::vector<T>& tmp, ptrdiff_t a, ptrdiff_t b, ptrdiff_t n, ptrdiff_t s) {
        ChaliceSortLaziestSortExt(data_, tmp, a, a + n);

        ptrdiff_t a0 = a, a1 = a + n, c = 0, c0 = 0;

        for (ptrdiff_t i = 0; c < n && i < 2; ++i) {
            ptrdiff_t p = a1, pEnd = p;

            while (true) {
                std::pair<ptrdiff_t, ptrdiff_t> t = ChaliceSortFindBitsSm(data_, pEnd, b, a0, i == 1, (std::min)(s, n - c));
                ptrdiff_t bits = t.second - t.first;

                if (bits == 0) break;

                a0 += bits;
                c += bits;

                if (bits < s || c == n) {
                    NSortHelpers::blockRotate(data_, pEnd, t.first, t.second);

                    t.first = pEnd;
                    pEnd += bits;

                    break;
                }
                else {
                    NSortHelpers::blockRotate(data_, p, pEnd, t.first);

                    p += t.first - pEnd;
                    pEnd = t.second;
                }
            }
            NSortHelpers::blockRotate(data_, a1, p, pEnd);
            a1 += pEnd - p;

            if (i == 0) c0 = c;
        }

        // returns the count of ascending pairs of elements NOT how many bits found

        if (c < n) return -1;

        else {
            NSortHelpers::blockMultiSwap(data_, a + c0, a + n + c0, n - c0);
            return c0;
        }
    }

    // ChaliceSort.bitReversal
    template<class T>
    void ChaliceSortBitReversal(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t len = b - a, m = 0;
        ptrdiff_t d1 = len >> 1, d2 = d1 + (d1 >> 1);

        for (ptrdiff_t i = 1; i < len - 1; ++i) {
            ptrdiff_t j = d1;

            for (ptrdiff_t k = i, n = d2; (k & 1) == 0; j -= n, k >>= 1, n >>= 1) {
            }
            m += j;
            if (m > i) {
                using std::swap;
                swap(data_[a + i], data_[a + m]);
            }
        }
    }

    // ChaliceSort.unshuffle (b-a is even)
    template<class T>
    void ChaliceSortUnshuffle(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t len = (b - a) >> 1, c = 0;

        for (ptrdiff_t n = 2; len > 0; len >>= 1, n *= 2) {
            if ((len & 1) == 1) {
                ptrdiff_t a1 = a + c;

                ChaliceSortBitReversal(data_, a1, a1 + n);
                ChaliceSortBitReversal(data_, a1, a1 + n / 2);
                ChaliceSortBitReversal(data_, a1 + n / 2, a1 + n);
                NSortHelpers::blockRotate(data_, a + c / 2, a1, a1 + n / 2);

                c += n;
            }
        }
    }

    // ChaliceSort.redistBuffer
    template<class T>
    void ChaliceSortRedistBuffer(std::vector<T>& data_, std::vector<T>& tmp, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t s = static_cast<ptrdiff_t>(tmp.size());

        while (m - a > s && m < b) {
            ptrdiff_t i = NSortHelpers::blockLeftBinSearch(data_, m, b, data_[a + s]);
            NSortHelpers::blockRotate(data_, a + s, m, i);

            ptrdiff_t t = i - m;
            m = i;

            NSortHelpers::blockMergeFWExt(data_, tmp, a, a + s, m);
            a += t + s;
        }
        if (m < b) NSortHelpers::blockMergeFWExt(data_, tmp, a, m, b);
    }

    // ChaliceSort.dualMergeBW
    template<class T>
    void ChaliceSortDualMergeBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t i = m - 1;
        --b;

        while (p > b + 1 && b >= m) {
            NSortHelpers::MarkArray(2, data_, i);

            if (NSortHelpers::CompareValues(data_[b], data_[i]) >= 0) {
                --p;
                using std::swap;
                swap(data_[p], data_[b]);
                --b;
            }
            else {
                --p;
                using std::swap;
                swap(data_[p], data_[i]);
                --i;
            }
        }

        if (b < m) NSortHelpers::blockShiftBW(data_, a, i + 1, p);

        else {
            ++i;
            ++b;
            p = m - (i - a);

            while (a < i && m < b) {
                NSortHelpers::MarkArray(2, data_, m);

                if (NSortHelpers::CompareValues(data_[a], data_[m]) <= 0) {
                    using std::swap;
                    swap(data_[p], data_[a]);
                    ++p;
                    ++a;
                }
                else {
                    using std::swap;
                    swap(data_[p], data_[m]);
                    ++p;
                    ++m;
                }
            }
            while (a < i) {
                using std::swap;
                swap(data_[p], data_[a]);
                ++p;
                ++a;
            }
        }
    }

    // ChaliceSort.dualMergeBWExt
    template<class T>
    void ChaliceSortDualMergeBWExt(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t i = m - 1;
        --b;

        while (p > b + 1 && b >= m) {
            NSortHelpers::MarkArray(2, data_, i);

            if (NSortHelpers::CompareValues(data_[b], data_[i]) >= 0) {
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

        if (b < m) NSortHelpers::blockShiftBWExt(data_, a, i + 1, p);

        else {
            ++i;
            ++b;
            p = m - (i - a);

            while (a < i && m < b) {
                NSortHelpers::MarkArray(2, data_, m);

                if (NSortHelpers::CompareValues(data_[a], data_[m]) <= 0) {
                    data_[p] = data_[a];
                    ++p;
                    ++a;
                }
                else {
                    data_[p] = data_[m];
                    ++p;
                    ++m;
                }
            }
            while (a < i) {
                data_[p] = data_[a];
                ++p;
                ++a;
            }
        }
    }

    // ChaliceSort.smartMerge
    template<class T>
    ptrdiff_t ChaliceSortSmartMerge(std::vector<T>& data_, ptrdiff_t p, ptrdiff_t a, ptrdiff_t m, bool rev) {
        ptrdiff_t i = m, cmp = rev ? 0 : 1;

        while (a < m) {
            NSortHelpers::MarkArray(2, data_, i);

            if (NSortHelpers::CompareValues(data_[a], data_[i]) < cmp) {
                data_[p] = data_[a];
                ++p;
                ++a;
            }
            else {
                data_[p] = data_[i];
                ++p;
                ++i;
            }
        }

        return i;
    }

    // ChaliceSort.smartTailMerge
    template<class T>
    void ChaliceSortSmartTailMerge(std::vector<T>& data_, std::vector<T>& tmp, ptrdiff_t p, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = m, bLen = static_cast<ptrdiff_t>(tmp.size());

        while (a < m && i < b) {
            NSortHelpers::MarkArray(2, data_, i);

            if (NSortHelpers::CompareValues(data_[a], data_[i]) <= 0) {
                data_[p] = data_[a];
                ++p;
                ++a;
            }
            else {
                data_[p] = data_[i];
                ++p;
                ++i;
            }
        }
        if (a < m) {
            if (a > p) NSortHelpers::blockShiftFWExt(data_, p, a, m);
            // Writes.arraycopy(tmp, 0, array, b-bLen, bLen, ...)
            for (ptrdiff_t k = 0; k < bLen; ++k)
                data_[b - bLen + k] = tmp[k];
        }
        else {
            a = 0;

            while (a < bLen && i < b) {
                NSortHelpers::MarkArray(2, data_, i);

                if (NSortHelpers::CompareValues(tmp[a], data_[i]) <= 0) {
                    data_[p] = tmp[a];
                    ++p;
                    ++a;
                }
                else {
                    data_[p] = data_[i];
                    ++p;
                    ++i;
                }
            }
            while (a < bLen) {
                data_[p] = tmp[a];
                ++p;
                ++a;
            }
        }
    }

    // ChaliceSort.blockCycle
    template<class T>
    void ChaliceSortBlockCycle(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t t, ptrdiff_t tIdx, ptrdiff_t tLen, ptrdiff_t bLen) {
        for (ptrdiff_t i = 0; i < tLen - 1; ++i) {
            if (NSortHelpers::CompareValues(data_[t + i], data_[tIdx + i]) > 0 ||
                (i > 0 && NSortHelpers::CompareValues(data_[t + i], data_[tIdx + i - 1]) < 0)) {

                // Writes.arraycopy(array, a+i*bLen, array, a-bLen, bLen, ...)
                for (ptrdiff_t k = 0; k < bLen; ++k)
                    data_[a - bLen + k] = data_[a + i * bLen + k];

                ptrdiff_t val = i, next = NSortHelpers::blockLeftBinSearch(data_, tIdx, tIdx + tLen, data_[t + i]) - tIdx;

                do {
                    // Writes.arraycopy(array, a+next*bLen, array, a+val*bLen, bLen, ...)
                    for (ptrdiff_t k = 0; k < bLen; ++k)
                        data_[a + val * bLen + k] = data_[a + next * bLen + k];
                    using std::swap;
                    swap(data_[t + i], data_[t + next]);

                    val = next;
                    next = NSortHelpers::blockLeftBinSearch(data_, tIdx, tIdx + tLen, data_[t + i]) - tIdx;
                }
                while (next != i);

                // Writes.arraycopy(array, a-bLen, array, a+val*bLen, bLen, ...)
                for (ptrdiff_t k = 0; k < bLen; ++k)
                    data_[a + val * bLen + k] = data_[a - bLen + k];
            }
        }
    }

    // ChaliceSort.blockMerge
    template<class T>
    void ChaliceSortBlockMerge(std::vector<T>& data_, std::vector<T>& tmp, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t tl, ptrdiff_t tLen, ptrdiff_t t, ptrdiff_t tIdx, ptrdiff_t bp1, ptrdiff_t bp2, ptrdiff_t bLen) {
        if (b - m <= bLen) {
            // Highlights.clearMark(2);
            NSortHelpers::blockMergeBWExt(data_, tmp, a, m, b);
            return;
        }

        NSortHelpers::blockInsertTo(data_, t + tl - 1, t);

        ptrdiff_t i = a + bLen - 1, j = m + bLen - 1, ti = t, tj = t + tl, tp = tIdx;

        while (ti < t + tl && tj < t + tLen) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) <= 0) {
                using std::swap;
                swap(data_[tp], data_[ti]);
                ++tp;
                ++ti;
                i += bLen;
            }
            else {
                using std::swap;
                swap(data_[tp], data_[tj]);
                ++tp;
                ++tj;
                swap(data_[bp1], data_[bp2]);
                j += bLen;
            }
            ++bp1; ++bp2;
        }
        while (ti < t + tl) {
            using std::swap;
            swap(data_[tp], data_[ti]);
            ++tp;
            ++ti;
            ++bp1; ++bp2;
        }
        while (tj < t + tLen) {
            using std::swap;
            swap(data_[tp], data_[tj]);
            ++tp;
            ++tj;
            swap(data_[bp1], data_[bp2]);
            ++bp1; ++bp2;
        }
        t ^= tIdx; tIdx ^= t; t ^= tIdx;

        NSortHelpers::heapSort(data_, tIdx, tIdx + tLen, true); // MaxHeapSort.customHeapSort(array, tIdx, tIdx+tLen, 1)

        // Writes.arraycopy(array, m-bLen, tmp, 0, bLen, ...)
        for (ptrdiff_t k = 0; k < bLen; ++k)
            tmp[k] = data_[m - bLen + k];
        // Writes.arraycopy(array, a, array, m-bLen, bLen, ...)
        for (ptrdiff_t k = 0; k < bLen; ++k)
            data_[m - bLen + k] = data_[a + k];

        ChaliceSortBlockCycle(data_, a + bLen, t, tIdx, tLen, bLen);
        NSortHelpers::blockMultiSwap(data_, t, tIdx, tLen);

        bp1 -= tLen; bp2 -= tLen;

        ptrdiff_t f = a + bLen, a1 = f, bp3 = bp2 + tLen;

        bool rev = NSortHelpers::CompareValues(data_[bp1], data_[bp2]) > 0;

        while (true) {
            do {
                if (rev) {
                    using std::swap;
                    swap(data_[bp1], data_[bp2]);
                }

                ++bp1; ++bp2;
                a1 += bLen;
            }
            while (bp2 < bp3 && NSortHelpers::CompareValues(data_[bp1], data_[bp2]) == (rev ? 1 : -1));

            if (bp2 == bp3) {
                ChaliceSortSmartTailMerge(data_, tmp, f - bLen, f, rev ? f : a1, b);
                return;
            }
            f = ChaliceSortSmartMerge(data_, f - bLen, f, a1, rev);
            rev = !rev;
        }
    }

    // ChaliceSort.blockCycleEasy
    template<class T>
    void ChaliceSortBlockCycleEasy(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t t, ptrdiff_t tIdx, ptrdiff_t tLen, ptrdiff_t bLen) {
        for (ptrdiff_t i = 0; i < tLen - 1; ++i) {
            if (NSortHelpers::CompareValues(data_[t + i], data_[tIdx + i]) > 0 ||
                (i > 0 && NSortHelpers::CompareValues(data_[t + i], data_[tIdx + i - 1]) < 0)) {

                ptrdiff_t next = NSortHelpers::blockLeftBinSearch(data_, tIdx, tIdx + tLen, data_[t + i]) - tIdx;

                do {
                    NSortHelpers::blockMultiSwap(data_, a + i * bLen, a + next * bLen, bLen);
                    using std::swap;
                    swap(data_[t + i], data_[t + next]);

                    next = NSortHelpers::blockLeftBinSearch(data_, tIdx, tIdx + tLen, data_[t + i]) - tIdx;
                }
                while (next != i);
            }
        }
    }

    // ChaliceSort.inPlaceMergeBW (Chalice's own 4/5-arg variant, distinct from
    // BlockMergeSorting.inPlaceMergeBW: it takes a `rev` flag and returns f)
    template<class T>
    ptrdiff_t ChaliceSortInPlaceMergeBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, bool rev) {
        ptrdiff_t f = rev ? NSortHelpers::blockRightBinSearch(data_, m, b, data_[m - 1])
                          : NSortHelpers::blockLeftBinSearch(data_, m, b, data_[m - 1]);
        b = f;

        while (b > m && m > a) {
            ptrdiff_t i = rev ? NSortHelpers::blockLeftBinSearch(data_, a, m, data_[b - 1])
                              : NSortHelpers::blockRightBinSearch(data_, a, m, data_[b - 1]);

            NSortHelpers::blockRotate(data_, i, m, b);

            ptrdiff_t t = m - i;
            m = i;
            b -= t + 1;

            if (m == a) break;

            b = rev ? NSortHelpers::blockRightBinSearch(data_, m, b, data_[m - 1])
                    : NSortHelpers::blockLeftBinSearch(data_, m, b, data_[m - 1]);
        }

        return f;
    }

    // ChaliceSort.blockMergeEasy
    template<class T>
    void ChaliceSortBlockMergeEasy(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t lenA, ptrdiff_t lenB, ptrdiff_t tl, ptrdiff_t tLen, ptrdiff_t t, ptrdiff_t tIdx, ptrdiff_t bp1, ptrdiff_t bp2, ptrdiff_t bLen) {
        if (b - m <= bLen) {
            ChaliceSortInPlaceMergeBW(data_, a, m, b, false);
            return;
        }

        ptrdiff_t a1 = a + lenA, b1 = b - lenB;

        ptrdiff_t i = a1 + bLen - 1, j = m + bLen - 1, ti = tIdx, tj = tIdx + tl, tp = t;

        while (ti < tIdx + tl && tj < tIdx + tLen) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) <= 0) {
                using std::swap;
                swap(data_[ti], data_[tp]);
                ++ti;
                ++tp;
                i += bLen;
            }
            else {
                using std::swap;
                swap(data_[tj], data_[tp]);
                ++tj;
                ++tp;
                swap(data_[bp1], data_[bp2]);
                j += bLen;
            }
            ++bp1; ++bp2;
        }
        while (ti < tIdx + tl) {
            using std::swap;
            swap(data_[ti], data_[tp]);
            ++ti;
            ++tp;
            ++bp1; ++bp2;
        }
        while (tj < tIdx + tLen) {
            using std::swap;
            swap(data_[tj], data_[tp]);
            ++tj;
            ++tp;
            swap(data_[bp1], data_[bp2]);
            ++bp1; ++bp2;
        }
        t ^= tIdx; tIdx ^= t; t ^= tIdx;

        NSortHelpers::heapSort(data_, tIdx, tIdx + tLen, true); // MaxHeapSort.customHeapSort(array, tIdx, tIdx+tLen, 1)

        ChaliceSortBlockCycleEasy(data_, a1, t, tIdx, tLen, bLen);
        NSortHelpers::blockMultiSwap(data_, t, tIdx, tLen);

        bp1 -= tLen; bp2 -= tLen;

        ptrdiff_t f = a1, a2 = f, bp3 = bp2 + tLen;

        bool rev = NSortHelpers::CompareValues(data_[bp1], data_[bp2]) > 0;

        while (true) {
            do {
                if (rev) {
                    using std::swap;
                    swap(data_[bp1], data_[bp2]);
                }

                ++bp1; ++bp2;
                a2 += bLen;
            }
            while (bp2 < bp3 && NSortHelpers::CompareValues(data_[bp1], data_[bp2]) == (rev ? 1 : -1));

            if (bp2 == bp3) {
                if (!rev) ChaliceSortInPlaceMergeBW(data_, a1, b1, b, false);
                NSortHelpers::blockInPlaceMerge(data_, a, a1, b);

                return;
            }
            f = ChaliceSortInPlaceMergeBW(data_, f, a2, a2 + bLen, rev);
            rev = !rev;
        }
    }

    // port of ChaliceSort.runSort
    template<class T = int>
    void ChaliceSort(std::vector<T>& data_) {
        ptrdiff_t a = 0, b = static_cast<ptrdiff_t>(data_.size()), n = b - a;

        if (n < 2) return;

        if (n < 128) {
            if (n < 32) {
                NSortHelpers::blockBinaryInsertion(data_, a, b);
            }
            else {
                // FifthMergeSort smallSort; smallSort.fifthMergeSort(array, n);
                FifthMergeSort(data_);
            }

            return;
        }

        ptrdiff_t cbrt = 2 * ChaliceSortCeilCbrt(n / 4), bLen = 2 * cbrt;
        ptrdiff_t kLen = ChaliceSortCalcKeys(bLen, n);

        std::vector<T> tmp(bLen);

        ptrdiff_t keys = ChaliceSortFindKeys(data_, tmp, a, b, 2 * kLen, cbrt);

        if (keys < 8) { // need at least 8 keys to perform a block merge
            for (ptrdiff_t j = 1; j < n; j *= 2)
                for (ptrdiff_t i = a + j; i < b; i += 2 * j)
                    ChaliceSortInPlaceMergeBW(data_, i - j, i, (std::min)(i + j, b), false);

            return;
        }
        else if (keys < 2 * kLen) {
            keys -= keys % 4;
            kLen = keys / 2;
        }

        // bit buffer length always equal to key buffer length
        ptrdiff_t a1 = a + keys, a2 = a1 + keys;
        ptrdiff_t bSep = ChaliceSortFindBits(data_, tmp, a1, b, kLen, cbrt);

        if (bSep == -1) { // if we cant find enough bits we dont have to sort
            ChaliceSortLaziestSortExt(data_, tmp, a, a2);
            NSortHelpers::blockInPlaceMerge(data_, a, a2, b);

            return;
        }

        // [a][ keys ][a1][ bits ][a2][extbuf][a3][main sequence][b]
        ptrdiff_t a3 = a2 + bLen, i;
        ptrdiff_t j = 1;
        n = b - a3;

        // advanced build blocks
        NSortHelpers::blockBinaryInsertion(data_, a2, a3);
        // Writes.arraycopy(array, a2, tmp, 0, bLen, ...)
        for (ptrdiff_t k = 0; k < bLen; ++k)
            tmp[k] = data_[a2 + k];

        for (; j < cbrt; j *= 2) {
            ptrdiff_t p = (std::max)(static_cast<ptrdiff_t>(2), j);

            for (i = a3; i + 2 * j < b; i += 2 * j)
                NSortHelpers::blockMergeWithBufFWExt(data_, i, i + j, i + 2 * j, i - p);

            if (i + j < b) NSortHelpers::blockMergeWithBufFWExt(data_, i, i + j, b, i - p);
            else           NSortHelpers::blockShiftFWExt(data_, i - p, i, b);

            a3 -= p; b -= p;
        }

        i = b - n % (2 * j);

        if (i + j < b) NSortHelpers::blockMergeWithBufBWExt(data_, i, i + j, b, b + j);
        else           NSortHelpers::blockShiftBWExt(data_, i, b, b + j);

        for (i -= 2 * j; i >= a3; i -= 2 * j)
            NSortHelpers::blockMergeWithBufBWExt(data_, i, i + j, i + 2 * j, i + 3 * j);

        a3 += j; b += j; j *= 2;

        for (i = a3; i + 2 * j < b; i += 2 * j)
            NSortHelpers::blockMergeWithBufFWExt(data_, i, i + j, i + 2 * j, i - j);

        if (i + j < b) NSortHelpers::blockMergeWithBufFWExt(data_, i, i + j, b, i - j);
        else           NSortHelpers::blockShiftFWExt(data_, i - j, i, b);

        a3 -= j; b -= j; j *= 2;

        i = b - n % (2 * j);

        if (i + j < b) ChaliceSortDualMergeBWExt(data_, i, i + j, b, b + j / 2);
        else           NSortHelpers::blockShiftBWExt(data_, i, b, b + j / 2);

        for (i -= 2 * j; i >= a3; i -= 2 * j)
            ChaliceSortDualMergeBWExt(data_, i, i + j, i + 2 * j, i + 2 * j + j / 2);

        a3 += j / 2; b += j / 2; j *= 2;

        // advanced build blocks (in-place using keys)
        if (keys >= j) {
            NSortHelpers::blockRotate(data_, a, a1, a3);
            a2 = a1 + bLen;

            if (kLen >= j) {
                for (ptrdiff_t mLvl = 2 * j; j < kLen; j *= 2) {
                    ptrdiff_t p = (std::max)(mLvl, j);

                    for (i = a3; i + 2 * j < b; i += 2 * j)
                        NSortHelpers::blockMergeWithBufFW(data_, i, i + j, i + 2 * j, i - p);

                    if (i + j < b) NSortHelpers::blockMergeWithBufFW(data_, i, i + j, b, i - p);
                    else           NSortHelpers::blockShiftFW(data_, i - p, i, b);

                    a3 -= p; b -= p;
                }

                i = b - n % (2 * j);

                if (i + j < b) NSortHelpers::blockMergeWithBufBW(data_, i, i + j, b, b + j);
                else           NSortHelpers::blockShiftBW(data_, i, b, b + j);

                for (i -= 2 * j; i >= a3; i -= 2 * j)
                    NSortHelpers::blockMergeWithBufBW(data_, i, i + j, i + 2 * j, i + 3 * j);

                a3 += j; b += j; j *= 2;
            }
            if (keys >= j) {
                for (i = a3; i + 2 * j < b; i += 2 * j)
                    NSortHelpers::blockMergeWithBufFW(data_, i, i + j, i + 2 * j, i - j);

                if (i + j < b) NSortHelpers::blockMergeWithBufFW(data_, i, i + j, b, i - j);
                else           NSortHelpers::blockShiftFW(data_, i - j, i, b);

                a3 -= j; b -= j; j *= 2;

                i = b - n % (2 * j);

                if (i + j < b) ChaliceSortDualMergeBW(data_, i, i + j, b, b + j / 2);
                else           NSortHelpers::blockShiftBW(data_, i, b, b + j / 2);

                for (i -= 2 * j; i >= a3; i -= 2 * j)
                    ChaliceSortDualMergeBW(data_, i, i + j, i + 2 * j, i + 2 * j + j / 2);

                a3 += j / 2; b += j / 2; j *= 2;
            }

            NSortHelpers::blockRotate(data_, a, a2, a3);
            a2 = a1 + keys;

            NSortHelpers::heapSort(data_, a, a1, true); // MaxHeapSort.customHeapSort(array, a, a1, 1)
        }
        // Writes.arraycopy(tmp, 0, array, a2, bLen, ...)
        for (ptrdiff_t k = 0; k < bLen; ++k)
            data_[a2 + k] = tmp[k];

        // main block merge
        ChaliceSortUnshuffle(data_, a, a1);
        ptrdiff_t limit = bLen * (kLen + 2);

        for (ptrdiff_t k = j / bLen - 1; j < n && (std::min)(2 * j, n) <= limit; j *= 2, k = 2 * k + 1) {
            for (i = a3; i + 2 * j <= b; i += 2 * j)
                ChaliceSortBlockMerge(data_, tmp, i, i + j, i + 2 * j, k, 2 * k, a, a + kLen, a1, a1 + kLen, bLen);

            if (i + j < b)
                ChaliceSortBlockMerge(data_, tmp, i, i + j, b, k, (b - i - 1) / bLen - 1, a, a + kLen, a1, a1 + kLen, bLen);
        }

        // in-place block merge
        for (; j < n; j *= 2) {
            bLen = (2 * j) / kLen;
            ptrdiff_t lenA = j % bLen, lenB = lenA;

            for (i = a3; i + 2 * j <= b; i += 2 * j)
                ChaliceSortBlockMergeEasy(data_, i, i + j, i + 2 * j, lenA, lenB, kLen / 2, kLen, a, a + kLen, a1, a1 + kLen, bLen);

            if (i + j < b)
                ChaliceSortBlockMergeEasy(data_, i, i + j, b, lenA, (b - i - j) % bLen, kLen / 2, kLen / 2 + (b - i - j) / bLen, a, a + kLen, a1, a1 + kLen, bLen);
        }

        // cleaning up
        NSortHelpers::blockMultiSwap(data_, a1 + bSep, a1 + kLen + bSep, kLen - bSep); // restore bit buffer initial position
        ChaliceSortLaziestSortExt(data_, tmp, a, a3);
        ChaliceSortRedistBuffer(data_, tmp, a, a3, b);
    }

    // ==================== ParallelGrailSort + ParallelBlockMergeSort + SynchronousSqrtSort ====================
// Ports of ArrayV (Java, MIT) hybrid sorting classes:
//   ParallelGrailSort, ParallelBlockMergeSort, SynchronousSqrtSort.
// ASCII ONLY. All namespace-scope helpers are prefixed with the owning
// algorithm's PascalCase name so no two algorithms can collide.


    // ==================================================================
    // ParallelGrailSort (parallel variant of Grail sort)
    // ==================================================================

    // ParallelGrailSort.sqrt
    inline ptrdiff_t ParallelGrailSortSqrt(ptrdiff_t n) {
        ptrdiff_t a = 0, b = (std::min)(static_cast<ptrdiff_t>(46341), n);

        while (a < b) {
            ptrdiff_t m = (a + b) / 2;

            if (m * m >= n) b = m;
            else            a = m + 1;
        }

        return a;
    }

    // ParallelGrailSort.shiftFW
    template<class T>
    void ParallelGrailSortShiftFW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        while (m < b) {
            using std::swap;
            swap(data_[a], data_[m]);
            ++a;
            ++m;
        }
    }

    // ParallelGrailSort.shiftBW
    template<class T>
    void ParallelGrailSortShiftBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        while (m > a) {
            --b;
            --m;
            using std::swap;
            swap(data_[b], data_[m]);
        }
    }

    // ParallelGrailSort.multiSwap
    template<class T>
    void ParallelGrailSortMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
        for (ptrdiff_t i = 0; i < len; ++i) {
            using std::swap;
            swap(data_[a + i], data_[b + i]);
        }
    }

    // ParallelGrailSort.rotate
    template<class T>
    void ParallelGrailSortRotate(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t l = m - a, r = b - m;

        while (l > 0 && r > 0) {
            if (r < l) {
                ParallelGrailSortMultiSwap(data_, m - r, m, r);
                b -= r;
                m -= r;
                l -= r;
            }
            else {
                ParallelGrailSortMultiSwap(data_, a, m, l);
                a += l;
                m += l;
                r -= l;
            }
        }
    }

    // ParallelGrailSort.insertTo
    template<class T>
    void ParallelGrailSortInsertTo(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        T temp = data_[a];
        while (a > b) {
            data_[a] = data_[a - 1];
            --a;
        }
        data_[b] = temp;
    }

    // ParallelGrailSort.leftBinSearch
    template<class T>
    ptrdiff_t ParallelGrailSortLeftBinSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            if (NSortHelpers::CompareValues(val, data_[m]) <= 0)
                b = m;
            else
                a = m + 1;
        }

        return a;
    }

    // ParallelGrailSort.rightBinSearch
    template<class T>
    ptrdiff_t ParallelGrailSortRightBinSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            if (NSortHelpers::CompareValues(val, data_[m]) < 0)
                b = m;
            else
                a = m + 1;
        }

        return a;
    }

    // ParallelGrailSort.binaryInsertion
    template<class T>
    void ParallelGrailSortBinaryInsertion(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        for (ptrdiff_t i = a + 1; i < b; ++i) {
            ptrdiff_t loc = ParallelGrailSortRightBinSearch(data_, a, i, data_[i]);
            ParallelGrailSortInsertTo(data_, i, loc);
        }
    }

    // ParallelGrailSort.mergeFW
    template<class T>
    ptrdiff_t ParallelGrailSortMergeFW(std::vector<T>& data_, ptrdiff_t p, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, bool fwEq) {
        ptrdiff_t i = a, j = m;

        while (i < m && j < b) {
            using std::swap;
            if (NSortHelpers::CompareValues(data_[i], data_[j]) < (fwEq ? 1 : 0)) {
                swap(data_[p], data_[i]);
                ++p;
                ++i;
            }
            else {
                swap(data_[p], data_[j]);
                ++p;
                ++j;
            }
        }

        ptrdiff_t f = i < m ? i : j;
        if (i < m && p < i) ParallelGrailSortShiftFW(data_, p, i, m);

        return f;
    }

    // ParallelGrailSort.inPlaceMergeFW
    template<class T>
    ptrdiff_t ParallelGrailSortInPlaceMergeFW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, bool fwEq) {
        ptrdiff_t i = a, j = m, k;

        while (i < j && j < b) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) > (fwEq ? 0 : -1)) {
                k = fwEq ? ParallelGrailSortLeftBinSearch(data_, j + 1, b, data_[i])
                         : ParallelGrailSortRightBinSearch(data_, j + 1, b, data_[i]);

                ParallelGrailSortRotate(data_, i, j, k);

                i += k - j;
                j = k;
            }
            else ++i;
        }

        return i;
    }

    // ParallelGrailSort.inPlaceMergeBW
    template<class T>
    void ParallelGrailSortInPlaceMergeBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, bool fwEq) {
        ptrdiff_t i = m - 1, j = b - 1, k;

        while (j > i && i >= a) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) > (fwEq ? 0 : -1)) {
                k = fwEq ? ParallelGrailSortRightBinSearch(data_, a, i, data_[j])
                         : ParallelGrailSortLeftBinSearch(data_, a, i, data_[j]);

                ParallelGrailSortRotate(data_, k, i + 1, j + 1);

                j -= (i + 1) - k;
                i = k - 1;
            }
            else --j;
        }
    }

    // ParallelGrailSort.findKeys
    template<class T>
    ptrdiff_t ParallelGrailSortFindKeys(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t n) {
        ptrdiff_t p = a, nKeys = 1, pEnd = a + nKeys;

        for (ptrdiff_t i = pEnd; i < b && nKeys < n; ++i) {
            NSortHelpers::MarkArray(1, data_, i);
            ptrdiff_t loc = ParallelGrailSortLeftBinSearch(data_, p, pEnd, data_[i]);

            if (pEnd == loc || NSortHelpers::CompareValues(data_[i], data_[loc]) != 0) {
                ParallelGrailSortRotate(data_, p, pEnd, i);
                ptrdiff_t inc = i - pEnd;
                loc += inc;
                p += inc;
                pEnd += inc;

                ParallelGrailSortInsertTo(data_, pEnd, loc);
                ++nKeys;
                ++pEnd;
            }
        }
        ParallelGrailSortRotate(data_, a, p, pEnd);
        return nKeys;
    }

    // ParallelGrailSort.blockSelect
    template<class T>
    void ParallelGrailSortBlockSelect(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t t, ptrdiff_t bLen) {
        for (ptrdiff_t j = a; j < b; j += bLen) {
            ptrdiff_t min = j;

            for (ptrdiff_t i = min + bLen; i < b; i += bLen) {
                int cmp = NSortHelpers::CompareValues(data_[i], data_[min]);

                if (cmp < 0 || (cmp == 0 && NSortHelpers::CompareValues(data_[t + (i - a) / bLen], data_[t + (min - a) / bLen]) < 0))
                    min = i;
            }

            if (min != j) {
                ParallelGrailSortMultiSwap(data_, j, min, bLen);
                using std::swap;
                swap(data_[t + (j - a) / bLen], data_[t + (min - a) / bLen]);
            }
        }
    }

    // ParallelGrailSort.blockMerge
    template<class T>
    void ParallelGrailSortBlockMerge(std::vector<T>& data_, ptrdiff_t t, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t bLen) {
        ptrdiff_t a1 = a + (m - a) % bLen;
        ptrdiff_t b1 = b - (b - m) % bLen;
        ptrdiff_t i = a1, l = i - bLen, r = m;

        T mKey = data_[t + (m - i) / bLen];
        ptrdiff_t f = a;
        bool frag = true;

        ParallelGrailSortBlockSelect(data_, a1, b1, t, bLen);

        while (l < m && r < b1) {
            bool curr = NSortHelpers::CompareValues(data_[t], mKey) < 0;
            ++t;

            if (frag != curr) {
                f = ParallelGrailSortMergeFW(data_, f - bLen, f, i, i + bLen, frag);

                if (f < i) {
                    ParallelGrailSortShiftBW(data_, f, i, i + bLen);
                    f += bLen;
                }
                else frag = curr;

                if (frag) r += bLen;
                else      l += bLen;
            }
            else {
                ParallelGrailSortShiftFW(data_, f - bLen, f, i);
                f = i;

                if (frag) l += bLen;
                else      r += bLen;
            }
            i += bLen;
        }

        if (l < m) {
            f = ParallelGrailSortMergeFW(data_, f - bLen, f, b1, b, true);
            if (f >= b1) ParallelGrailSortShiftFW(data_, f - bLen, f, b);
        }
        else ParallelGrailSortShiftFW(data_, f - bLen, f, b);
    }

    // ParallelGrailSort.blockMergeFewKeys
    template<class T>
    void ParallelGrailSortBlockMergeFewKeys(std::vector<T>& data_, ptrdiff_t t, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t bLen) {
        ptrdiff_t a1 = a + (m - a) % bLen;
        ptrdiff_t b1 = b - (b - m) % bLen;
        ptrdiff_t i = a1, l = i - bLen, r = m;

        T mKey = data_[t + (m - i) / bLen];
        ptrdiff_t f = a;
        bool frag = true;

        ParallelGrailSortBlockSelect(data_, a1, b1, t, bLen);

        while (l < m && r < b1) {
            bool curr = NSortHelpers::CompareValues(data_[t], mKey) < 0;
            ++t;

            if (frag != curr) {
                bool tmp = frag;

                if (f == i || NSortHelpers::CompareValues(data_[i - 1], data_[i + bLen - 1]) < (frag ? 1 : 0))
                    frag = curr;

                f = ParallelGrailSortInPlaceMergeFW(data_, f, i, i + bLen, tmp);

                if (frag) r += bLen;
                else      l += bLen;
            }
            else {
                f = i;

                if (frag) l += bLen;
                else      r += bLen;
            }
            i += bLen;
        }

        if (l < m) ParallelGrailSortInPlaceMergeBW(data_, f, b1, b, true);
    }

    // ParallelGrailSort.redistFW
    template<class T>
    void ParallelGrailSortRedistFW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ParallelGrailSortBinaryInsertion(data_, a, m);
        ParallelGrailSortInPlaceMergeFW(data_, a, m, b, true);
    }

    // ParallelGrailSort.redistBW
    template<class T>
    void ParallelGrailSortRedistBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ParallelGrailSortBinaryInsertion(data_, m, b);
        ParallelGrailSortInPlaceMergeBW(data_, a, m, b, false);
    }

    // ParallelGrailSort.lazyStableSort (Java runs it on a LazyStableSort thread;
    // recursion depth is O(log n), which is fine for the C++ stack).
    template<class T>
    void ParallelGrailSortLazyStableSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        if (b - a <= 16) {
            ParallelGrailSortBinaryInsertion(data_, a, b);
            return;
        }

        ptrdiff_t m = (a + b) / 2;

        // Java: LazyStableSort left(a, m) and right(m, b), started and joined.
        std::vector<std::thread> threads;
        threads.emplace_back([&data_, a, m] { ParallelGrailSortLazyStableSort(data_, a, m); });
        threads.emplace_back([&data_, m, b] { ParallelGrailSortLazyStableSort(data_, m, b); });
        for (std::thread& th : threads) th.join();

        ParallelGrailSortInPlaceMergeFW(data_, a, m, b, true);
    }

    // ParallelGrailSort.grailCommonSort (Java runs it on a GrailCommonSort thread)
    template<class T>
    void ParallelGrailSortGrailCommonSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t nKeys) {
        ptrdiff_t len = b - a;

        if (len <= 16) {
            ParallelGrailSortBinaryInsertion(data_, a, b);
            return;
        }

        ptrdiff_t bLen = ParallelGrailSortSqrt(len);
        ptrdiff_t tLen = len / bLen;

        ptrdiff_t idl = bLen + tLen;
        bool strat1 = nKeys >= idl;
        if (!strat1) idl = nKeys;

        ptrdiff_t keys = ParallelGrailSortFindKeys(data_, a, b, idl);
        ptrdiff_t a1 = a + keys;
        ptrdiff_t m = (a1 + b) / 2;

        if (strat1 && keys == idl) {
            // Java: GrailCommonSort left(a1, m, keys) and right(m, b, keys).
            std::vector<std::thread> threads;
            threads.emplace_back([&data_, a1, m, keys] { ParallelGrailSortGrailCommonSort(data_, a1, m, keys); });
            threads.emplace_back([&data_, m, b, keys] { ParallelGrailSortGrailCommonSort(data_, m, b, keys); });
            for (std::thread& th : threads) th.join();

            ParallelGrailSortBlockMerge(data_, a, a1, m, b, bLen);

            m = ParallelGrailSortLeftBinSearch(data_, a + tLen, b - bLen, data_[a + tLen - 1]);

            // Java: RedistFW kBuf(a, a+tLen, m) and RedistBW mBuf(m, b-bLen, b).
            std::vector<std::thread> redist;
            redist.emplace_back([&data_, a, tLen, m] { ParallelGrailSortRedistFW(data_, a, a + tLen, m); });
            redist.emplace_back([&data_, m, b, bLen] { ParallelGrailSortRedistBW(data_, m, b - bLen, b); });
            for (std::thread& th : redist) th.join();
        }
        else if (keys > 4) {
            bLen = (b - a1 - 1) / (keys - keys % 2) + 1;

            std::vector<std::thread> threads;
            threads.emplace_back([&data_, a1, m, keys] { ParallelGrailSortGrailCommonSort(data_, a1, m, keys); });
            threads.emplace_back([&data_, m, b, keys] { ParallelGrailSortGrailCommonSort(data_, m, b, keys); });
            for (std::thread& th : threads) th.join();

            ParallelGrailSortBlockMergeFewKeys(data_, a, a1, m, b, bLen);
            ParallelGrailSortRedistFW(data_, a, a1, b);
        }
        else if (keys > 1) ParallelGrailSortLazyStableSort(data_, a, b);
    }

    // port of ParallelGrailSort.runSort
    template<class T = int>
    void ParallelGrailSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;

        ParallelGrailSortGrailCommonSort(data_, 0, static_cast<ptrdiff_t>(data_.size()), 46341);
    }


    // ==================================================================
    // ParallelBlockMergeSort
    // ==================================================================

    // ParallelBlockMergeSort.sqrt
    inline ptrdiff_t ParallelBlockMergeSortSqrt(ptrdiff_t n) {
        ptrdiff_t a = 0, b = (std::min)(static_cast<ptrdiff_t>(46341), n);

        while (a < b) {
            ptrdiff_t m = (a + b) / 2;

            if (m * m >= n) b = m;
            else            a = m + 1;
        }

        return a;
    }

    // ParallelBlockMergeSort.multiSwap
    template<class T>
    void ParallelBlockMergeSortMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
        for (ptrdiff_t i = 0; i < len; ++i) {
            using std::swap;
            swap(data_[a + i], data_[b + i]);
        }
    }

    // ParallelBlockMergeSort.rotate
    template<class T>
    void ParallelBlockMergeSortRotate(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t l = m - a, r = b - m;

        while (l > 0 && r > 0) {
            if (r < l) {
                ParallelBlockMergeSortMultiSwap(data_, m - r, m, r);
                b -= r;
                m -= r;
                l -= r;
            }
            else {
                ParallelBlockMergeSortMultiSwap(data_, a, m, l);
                a += l;
                m += l;
                r -= l;
            }
        }
    }

    // ParallelBlockMergeSort.inPlaceMergeFW
    template<class T>
    void ParallelBlockMergeSortInPlaceMergeFW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = a, j = m, k;

        while (i < j && j < b) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) > 0) {
                k = j;
                // Java: while(++k < b && compareIndices(i, k) > 0);
                while (true) {
                    ++k;
                    if (!(k < b && NSortHelpers::CompareValues(data_[i], data_[k]) > 0)) break;
                }

                ParallelBlockMergeSortRotate(data_, i, j, k);

                i += k - j;
                j = k;
            }
            else ++i;
        }
    }

    // ParallelBlockMergeSort.inPlaceMergeBW
    template<class T>
    void ParallelBlockMergeSortInPlaceMergeBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = m - 1, j = b - 1, k;

        while (j > i && i >= a) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) > 0) {
                k = i;
                // Java: while(--k >= a && compareIndices(k, j) > 0);
                while (true) {
                    --k;
                    if (!(k >= a && NSortHelpers::CompareValues(data_[k], data_[j]) > 0)) break;
                }

                ParallelBlockMergeSortRotate(data_, k + 1, i + 1, j + 1);

                j -= i - k;
                i = k;
            }
            else --j;
        }
    }

    // ParallelBlockMergeSort.mergeFW
    template<class T>
    void ParallelBlockMergeSortMergeFW(std::vector<T>& data_, ptrdiff_t p, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t len2 = m - a, pEnd = p + len2;

        ParallelBlockMergeSortMultiSwap(data_, p, a, len2);

        using std::swap;
        while (p < pEnd && m < b) {
            if (NSortHelpers::CompareValues(data_[p], data_[m]) <= 0) {
                swap(data_[a], data_[p]);
                ++a;
                ++p;
            }
            else {
                swap(data_[a], data_[m]);
                ++a;
                ++m;
            }
        }
        while (p < pEnd) {
            swap(data_[a], data_[p]);
            ++a;
            ++p;
        }
    }

    // ParallelBlockMergeSort.mergeBW
    template<class T>
    void ParallelBlockMergeSortMergeBW(std::vector<T>& data_, ptrdiff_t p, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t len2 = b - m, pEnd = p + len2 - 1;

        ParallelBlockMergeSortMultiSwap(data_, p, m, len2);

        --m;
        using std::swap;
        while (pEnd >= p && m >= a) {
            if (NSortHelpers::CompareValues(data_[pEnd], data_[m]) >= 0) {
                --b;
                swap(data_[b], data_[pEnd]);
                --pEnd;
            }
            else {
                --b;
                swap(data_[b], data_[m]);
                --m;
            }
        }
        while (pEnd >= p) {
            --b;
            swap(data_[b], data_[pEnd]);
            --pEnd;
        }
    }

    // ParallelBlockMergeSort.findKeys
    template<class T>
    ptrdiff_t ParallelBlockMergeSortFindKeys(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t n) {
        ptrdiff_t p = a, found = 1, pEnd = p + found, i = pEnd;

        while (found < n) {
            while (i < b && NSortHelpers::CompareValues(data_[pEnd - 1], data_[i]) == 0) ++i;
            if (i == b) break;

            ParallelBlockMergeSortRotate(data_, p, pEnd, i);

            p += i - pEnd;
            ++i;
            pEnd = i;

            ++found;
        }
        ParallelBlockMergeSortRotate(data_, a, p, pEnd);

        return found;
    }

    // ParallelBlockMergeSort.selectMin
    template<class T>
    ptrdiff_t ParallelBlockMergeSortSelectMin(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t bLen) {
        ptrdiff_t min = a;
        for (ptrdiff_t i = min + bLen; i < b; i += bLen)
            if (NSortHelpers::CompareValues(data_[i], data_[min]) < 0)
                min = i;

        return min;
    }

    // BinaryInsertionSort.customBinaryInsert (used by ParallelBlockMergeSort)
    template<class T>
    void ParallelBlockMergeSortBinaryInsert(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        for (ptrdiff_t i = start; i < end; ++i) {
            T num = data_[i];
            ptrdiff_t lo = start, hi = i;

            while (lo < hi) {
                ptrdiff_t mid = lo + ((hi - lo) / 2); // avoid int overflow!

                if (num < data_[mid]) hi = mid; // do NOT move equal elements to right of inserted element
                else                  lo = mid + 1;
            }

            ptrdiff_t j = i - 1;
            while (j >= lo) {
                data_[j + 1] = data_[j];
                --j;
            }
            data_[lo] = num;
        }
    }

    // ParallelBlockMergeSort.blockMerge
    template<class T>
    void ParallelBlockMergeSortBlockMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        if (NSortHelpers::CompareValues(data_[m - 1], data_[m]) <= 0) return;

        else if (NSortHelpers::CompareValues(data_[a], data_[b - 1]) > 0) {
            ParallelBlockMergeSortRotate(data_, a, m, b);
            return;
        }

        ptrdiff_t len1 = m - a;

        ptrdiff_t bLen = ParallelBlockMergeSortSqrt(len1);
        ptrdiff_t tLen = len1 / bLen;
        ptrdiff_t idl = bLen + tLen;

        ptrdiff_t keys = ParallelBlockMergeSortFindKeys(data_, a, m, idl);
        ptrdiff_t a1 = a + keys;
        len1 -= keys;

        if (keys == idl) {
            ptrdiff_t b1 = b - (b - m) % bLen;

            ptrdiff_t t = a;
            ptrdiff_t p = a + tLen;

            ptrdiff_t i = a1 + (len1 - 1) % bLen + 1;

            for (ptrdiff_t j = i, k = t; j < m; j += bLen, ++k) {
                using std::swap;
                swap(data_[j], data_[k]);
            }

            while (i < m && m < b1) {
                if (NSortHelpers::CompareValues(data_[i - 1], data_[m + bLen - 1]) > 0) {
                    ParallelBlockMergeSortMultiSwap(data_, i, m, bLen);
                    ParallelBlockMergeSortMergeBW(data_, p, a1, i, i + bLen);

                    m += bLen;
                }
                else {
                    ptrdiff_t min = ParallelBlockMergeSortSelectMin(data_, i, m, bLen);

                    if (min != i) ParallelBlockMergeSortMultiSwap(data_, i, min, bLen);
                    using std::swap;
                    swap(data_[t], data_[i]);
                    ++t;
                }
                i += bLen;
            }
            if (i < m) {
                do {
                    ptrdiff_t min = ParallelBlockMergeSortSelectMin(data_, i, m, bLen);

                    ParallelBlockMergeSortMultiSwap(data_, i, min, bLen);
                    using std::swap;
                    swap(data_[t], data_[i]);
                    ++t;
                    i += bLen;
                } while (i < m);

                ParallelBlockMergeSortMergeBW(data_, p, a1, b1, b);
            }
            else {
                while (m < b1 && NSortHelpers::CompareValues(data_[m - bLen], data_[m]) > 0) {
                    ParallelBlockMergeSortMergeBW(data_, p, a1, m, m + bLen);
                    m += bLen;
                }
                if (m == b1) ParallelBlockMergeSortMergeBW(data_, p, a1, b1, b);
                else         ParallelBlockMergeSortMergeFW(data_, p, m - bLen + 1, m, b);
            }

            ParallelBlockMergeSortBinaryInsert(data_, p, a1);
            ParallelBlockMergeSortInPlaceMergeFW(data_, a, a + keys, b);
        }
        else if (keys > 1) {
            bLen = (len1 - 1) / keys + 1;
            ptrdiff_t b1 = b - (b - m) % bLen;

            ptrdiff_t t = a;
            ptrdiff_t i = a1 + (len1 - 1) % bLen + 1;

            for (ptrdiff_t j = i, k = t; j < m; j += bLen, ++k) {
                using std::swap;
                swap(data_[j], data_[k]);
            }

            while (i < m && m < b1) {
                if (NSortHelpers::CompareValues(data_[i - 1], data_[m + bLen - 1]) > 0) {
                    ParallelBlockMergeSortMultiSwap(data_, i, m, bLen);
                    ParallelBlockMergeSortInPlaceMergeBW(data_, a1, i, i + bLen);

                    m += bLen;
                }
                else {
                    ptrdiff_t min = ParallelBlockMergeSortSelectMin(data_, i, m, bLen);

                    if (min != i) ParallelBlockMergeSortMultiSwap(data_, i, min, bLen);
                    using std::swap;
                    swap(data_[t], data_[i]);
                    ++t;
                }
                i += bLen;
            }
            if (i < m) {
                do {
                    ptrdiff_t min = ParallelBlockMergeSortSelectMin(data_, i, m, bLen);

                    ParallelBlockMergeSortMultiSwap(data_, i, min, bLen);
                    using std::swap;
                    swap(data_[t], data_[i]);
                    ++t;
                    i += bLen;
                } while (i < m);

                ParallelBlockMergeSortInPlaceMergeBW(data_, a1, b1, b);
            }
            else {
                while (m < b1 && NSortHelpers::CompareValues(data_[m - bLen], data_[m]) > 0) {
                    ParallelBlockMergeSortInPlaceMergeBW(data_, a1, m, m + bLen);
                    m += bLen;
                }
                if (m == b1) ParallelBlockMergeSortInPlaceMergeBW(data_, a1, b1, b);
                else         ParallelBlockMergeSortInPlaceMergeFW(data_, m - bLen + 1, m, b);
            }
            ParallelBlockMergeSortInPlaceMergeFW(data_, a, a + keys, b);
        }
        else ParallelBlockMergeSortInPlaceMergeFW(data_, a, m, b);
    }

    // ParallelBlockMergeSort.blockMergeSort (Java runs it on a BlockMergeSort thread)
    template<class T>
    void ParallelBlockMergeSortBlockMergeSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        if (b - a < 32) {
            ParallelBlockMergeSortBinaryInsert(data_, a, b);

            return;
        }

        ptrdiff_t m = a + (b - a) / 2;

        // Java: BlockMergeSort left(a, m) and right(m, b), started and joined.
        // Recursion depth is O(log n), which is fine.
        std::vector<std::thread> threads;
        threads.emplace_back([&data_, a, m] { ParallelBlockMergeSortBlockMergeSort(data_, a, m); });
        threads.emplace_back([&data_, m, b] { ParallelBlockMergeSortBlockMergeSort(data_, m, b); });
        for (std::thread& th : threads) th.join();

        ParallelBlockMergeSortBlockMerge(data_, a, m, b);
    }

    // port of ParallelBlockMergeSort.runSort
    template<class T = int>
    void ParallelBlockMergeSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;

        ParallelBlockMergeSortBlockMergeSort(data_, 0, static_cast<ptrdiff_t>(data_.size()));
    }


    // ==================================================================
    // SynchronousSqrtSort (extends BlockMergeSorting -> NSortHelpers block*)
    // ==================================================================

    // SynchronousSqrtSort.smartMergeBW
    template<class T>
    ptrdiff_t SynchronousSqrtSortSmartMergeBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p, bool rev) {
        ptrdiff_t i = m - 1, j = b - 1;
        int cmp = rev ? -1 : 0;

        while (i >= a && j >= m) {
            NSortHelpers::MarkArray(2, data_, i);

            if (NSortHelpers::CompareValues(data_[i], data_[j]) > cmp) {
                --p;
                data_[p] = data_[i];
                --i;
            }
            else {
                --p;
                data_[p] = data_[j];
                --j;
            }
        }
        return i + 1;
    }

    // SynchronousSqrtSort.blockSelection
    template<class T>
    void SynchronousSqrtSortBlockSelection(std::vector<T>& data_, std::vector<ptrdiff_t>& tags,
                                           ptrdiff_t a, ptrdiff_t b, ptrdiff_t bLen, ptrdiff_t t, ptrdiff_t tj) {
        ptrdiff_t tagLen = static_cast<ptrdiff_t>(tags.size());

        ptrdiff_t lim = (std::min)(tj + 1, tagLen - t);
        for (ptrdiff_t i = 0; i < lim; ++i)
            tags[t + i] = i + (i <= tj / 2 ? 0 : tagLen);

        for (ptrdiff_t j = a, p = a; j < b - bLen; j += bLen) {
            ptrdiff_t min = p == j ? j + bLen : j;

            for (ptrdiff_t i = min + bLen; i < b; i += bLen) {
                if (i != p) {
                    int cmp = NSortHelpers::CompareValues(data_[i], data_[min]);

                    if (cmp == -1 || (cmp == 0 && tags[t + (i - a) / bLen] < tags[t + (min - a) / bLen]))
                        min = i;
                }
            }
            if (min > j) {
                if (p == j) {
                    for (ptrdiff_t k = 0; k < bLen; ++k) data_[j + k] = data_[min + k];
                    tags[t + (j - a) / bLen] = tags[t + (min - a) / bLen];

                    p = min;
                }
                else {
                    NSortHelpers::blockMultiSwap(data_, j, min, bLen);
                    using std::swap;
                    swap(tags[t + (j - a) / bLen], tags[t + (min - a) / bLen]);
                }
            }
        }
    }

    // SynchronousSqrtSort.mergeBlocksBW
    template<class T>
    void SynchronousSqrtSortMergeBlocksBW(std::vector<T>& data_, std::vector<ptrdiff_t>& tags,
                                          ptrdiff_t a, ptrdiff_t b, ptrdiff_t ti, ptrdiff_t tb, ptrdiff_t bLen) {
        ptrdiff_t tj = tb - 1, mkv = static_cast<ptrdiff_t>(tags.size());
        ptrdiff_t f = b, a1 = f - bLen;
        bool rev = tags[tj] < mkv;

        while (true) {
            do {
                --tj;
                a1 -= bLen;
            } while (tj >= ti && (rev ? tags[tj] < mkv : tags[tj] >= mkv));

            if (tj < ti) {
                NSortHelpers::blockShiftBWExt(data_, a, f, f + bLen);
                break;
            }
            f = SynchronousSqrtSortSmartMergeBW(data_, a1, a1 + bLen, f, f + bLen, rev);
            rev = !rev;
        }
    }

    // port of SynchronousSqrtSort.runSort
    template<class T = int>
    void SynchronousSqrtSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;

        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        if (length <= 16) {
            NSortHelpers::blockBinaryInsertion(data_, 0, length);
            return;
        }

        ptrdiff_t bLen = 1;
        while (bLen * bLen < length) bLen *= 2;
        ptrdiff_t mod = length % bLen;

        ptrdiff_t a = bLen + mod, b = length, len = b - a;
        ptrdiff_t i = 0, j = 1;

        std::vector<T> temp(bLen + mod);
        std::vector<ptrdiff_t> tags((length - 1) / bLen + 1);

        NSortHelpers::blockBinaryInsertion(data_, 0, a);
        for (ptrdiff_t k = 0; k < a; ++k) temp[k] = data_[k];

        ptrdiff_t p = 0;
        for (; j < bLen; j *= 2) {
            p = (std::max)(static_cast<ptrdiff_t>(2), j);

            for (i = a; i + 2 * j < b; i += 2 * j)
                NSortHelpers::blockMergeWithBufFWExt(data_, i, i + j, i + 2 * j, i - p);

            if (i + j < b) NSortHelpers::blockMergeWithBufFWExt(data_, i, i + j, b, i - p);
            else           NSortHelpers::blockShiftFWExt(data_, i - p, i, b);

            a -= p; b -= p;
        }

        p = len % (2 * j);
        i = b - p;

        if (i + j < b) NSortHelpers::blockMergeWithBufBWExt(data_, i, i + j, b, b + j);
        else           NSortHelpers::blockShiftBWExt(data_, i, b, b + j);

        for (i -= 2 * j; i >= a; i -= 2 * j)
            NSortHelpers::blockMergeWithBufBWExt(data_, i, i + j, i + 2 * j, i + 3 * j);

        a += j; b += j; j *= 2;

        for (ptrdiff_t tj = 4; j < len; j *= 2, tj *= 2) {
            ptrdiff_t ti = 0;
            for (i = a; i + 2 * j < b; i += 2 * j, ti += tj)
                SynchronousSqrtSortBlockSelection(data_, tags, i - bLen, i + 2 * j, bLen, ti, tj);

            bool noFrag = i + j < b;
            p = (b - i) / bLen;

            if (noFrag) SynchronousSqrtSortBlockSelection(data_, tags, i - bLen, b, bLen, ti, tj);

            a -= bLen; b -= bLen; i -= bLen;

            if (noFrag) SynchronousSqrtSortMergeBlocksBW(data_, tags, i, b, ti, ti + p, bLen);

            for (i -= 2 * j, ti -= tj; i >= a; i -= 2 * j, ti -= tj)
                SynchronousSqrtSortMergeBlocksBW(data_, tags, i, i + 2 * j, ti, ti + tj, bLen);

            a += bLen; b += bLen;
        }
        p = 0; i = 0; j = a;

        while (i < a && j < b) {
            NSortHelpers::MarkArray(2, data_, i);

            if (NSortHelpers::CompareValues(temp[i], data_[j]) <= 0) {
                data_[p] = temp[i];
                ++p;
                ++i;
            }
            else {
                data_[p] = data_[j];
                ++p;
                ++j;
            }
        }
        while (i < a) {
            NSortHelpers::MarkArray(2, data_, i);
            data_[p] = temp[i];
            ++p;
            ++i;
        }
    }

    // ==================== SqrtSort + EctaSort + CircularGrailSort ====================
// Ports of ArrayV (Java, MIT) hybrid sorting classes: SqrtSort, EctaSort, CircularGrailSort.
// ASCII ONLY - do not put non-ASCII characters in this file.


    // ============================== SqrtSort ==============================
    // ArrayV SqrtSort (c) 2014 Andrey Astrelin, refactored by MusicTheorist.
    // Stable O(N*log(N)) sort using O(sqrt(N)) extra memory.

    // SqrtSort.java class SqrtState: mutable state threaded through the merge helpers
    // (Java carries these in a small object returned by sqrtSmartMergeWithXBuf).
    struct SqrtSortState {
        ptrdiff_t leftOverLen;
        ptrdiff_t leftOverFrag;
    };

    // Writes.arraycopy (ArrayV Writes.java): memmove semantics. Java copies backwards
    // when src == dest and destPos >= srcPos, forwards otherwise.
    template<class T>
    void SqrtSortArrayCopy(std::vector<T>& src, ptrdiff_t srcPos,
                           std::vector<T>& dest, ptrdiff_t destPos, ptrdiff_t length) {
        if (&src != &dest || destPos < srcPos) {
            for (ptrdiff_t i = 0; i < length; ++i) dest[destPos + i] = src[srcPos + i];
        }
        else {
            for (ptrdiff_t i = length - 1; i >= 0; --i) dest[destPos + i] = src[srcPos + i];
        }
    }

    // SqrtSort.sqrtSwap (auxwrite is animation-only in Java and is dropped here).
    template<class T>
    void SqrtSortSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        using std::swap;
        swap(data_[a], data_[b]);
    }

    // SqrtSort.sqrtSwap called with the tags array.
    inline void SqrtSortTagsSwap(std::vector<ptrdiff_t>& tags, ptrdiff_t a, ptrdiff_t b) {
        using std::swap;
        swap(tags[a], tags[b]);
    }

    // SqrtSort.sqrtMultiSwap
    template<class T>
    void SqrtSortMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t swapsLeft) {
        while (swapsLeft != 0) {
            SqrtSortSwap(data_, a, b);
            ++a;
            ++b;
            --swapsLeft;
        }
    }

    // SqrtSort.sqrtInsertSort -> InsertionSort.customInsertSort(array, pos, len, 0.25, auxwrite).
    // The Java call passes (pos, len) straight to InsertionSorting.insertionSort(array, start, end);
    // kept verbatim (in this algorithm the call site always has pos == 0).
    template<class T>
    void SqrtSortInsertSort(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len) {
        NSortHelpers::insertionSort(data_, pos, len);
    }

    // SqrtSort.sqrtMergeRight
    template<class T>
    void SqrtSortMergeRight(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t leftLen,
                            ptrdiff_t rightLen, ptrdiff_t dist) {
        ptrdiff_t mergedPos = leftLen + rightLen + dist - 1;
        ptrdiff_t right = leftLen + rightLen - 1;
        ptrdiff_t left = leftLen - 1;

        while (left >= 0) {
            NSortHelpers::MarkArray(2, data_, pos + left);
            NSortHelpers::MarkArray(3, data_, pos + right);

            if (right < leftLen || data_[pos + left] > data_[pos + right]) {
                data_[pos + mergedPos] = data_[pos + left];
                --mergedPos;
                --left;
            }
            else {
                data_[pos + mergedPos] = data_[pos + right];
                --mergedPos;
                --right;
            }
        }

        if (right != mergedPos) {
            while (right >= leftLen) {
                data_[pos + mergedPos] = data_[pos + right];
                --mergedPos;
                --right;
                NSortHelpers::MarkArray(2, data_, pos + right);
            }
        }
    }

    // SqrtSort.sqrtMergeLeftWithXBuf
    // arr[dist..-1] - free, arr[0, leftEnd - 1] ++ arr[leftEnd, leftEnd + rightEnd - 1]
    // -> arr[dist, dist + leftEnd + rightEnd - 1]
    template<class T>
    void SqrtSortMergeLeftWithXBuf(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t leftEnd,
                                   ptrdiff_t rightEnd, ptrdiff_t dist) {
        ptrdiff_t left = 0;
        ptrdiff_t right = leftEnd;
        rightEnd += leftEnd;

        while (right < rightEnd) {
            if (left == leftEnd || data_[pos + left] > data_[pos + right]) {
                data_[pos + dist] = data_[pos + right];
                ++dist;
                ++right;
            }
            else {
                data_[pos + dist] = data_[pos + left];
                ++dist;
                ++left;
            }

            NSortHelpers::MarkArray(2, data_, pos + left);
            NSortHelpers::MarkArray(3, data_, pos + right);
        }

        if (dist != left) {
            while (left < leftEnd) {
                data_[pos + dist] = data_[pos + left];
                ++dist;
                ++left;
                NSortHelpers::MarkArray(2, data_, pos + left);
            }
        }
    }

    // SqrtSort.sqrtMergeDown
    // arr[0,L1-1] ++ arr2[0,L2-1] -> arr[-L1,L2-1],  arr2 is "before" arr1
    template<class T>
    void SqrtSortMergeDown(std::vector<T>& data_, ptrdiff_t arrPos, std::vector<T>& buffer,
                           ptrdiff_t bufPos, ptrdiff_t leftLen, ptrdiff_t rightLen) {
        ptrdiff_t arrMerge = 0;
        ptrdiff_t bufMerge = 0;
        ptrdiff_t dist = 0 - rightLen;

        while (bufMerge < rightLen) {
            if (arrMerge == leftLen || data_[arrPos + arrMerge] >= buffer[bufPos + bufMerge]) {
                data_[arrPos + dist] = buffer[bufPos + bufMerge];
                ++dist;
                ++bufMerge;
            }
            else {
                data_[arrPos + dist] = data_[arrPos + arrMerge];
                ++dist;
                ++arrMerge;
            }

            NSortHelpers::MarkArray(2, data_, arrPos + arrMerge);
            NSortHelpers::MarkArray(3, buffer, bufPos + bufMerge);
        }

        if (dist != arrMerge) {
            while (arrMerge < leftLen) {
                data_[arrPos + dist] = data_[arrPos + arrMerge];
                ++dist;
                ++arrMerge;
                NSortHelpers::MarkArray(2, data_, arrPos + arrMerge);
            }
        }
    }

    // SqrtSort.sqrtSmartMergeWithXBuf - returns the leftover length and the leftover fragment.
    template<class T>
    SqrtSortState SqrtSortSmartMergeWithXBuf(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t leftOverLen,
                                             ptrdiff_t leftOverFrag, ptrdiff_t blockLen) {
        ptrdiff_t dist = 0 - blockLen;
        ptrdiff_t left = 0;
        ptrdiff_t right = leftOverLen;
        ptrdiff_t leftEnd = right;
        ptrdiff_t rightEnd = right + blockLen;
        ptrdiff_t typeFrag = 1 - leftOverFrag;  // 1 if inverted

        while (left < leftEnd && right < rightEnd) {
            if (NSortHelpers::CompareValues(data_[pos + left], data_[pos + right]) - typeFrag < 0) {
                data_[pos + dist] = data_[pos + left];
                ++dist;
                ++left;
            }
            else {
                data_[pos + dist] = data_[pos + right];
                ++dist;
                ++right;
            }

            NSortHelpers::MarkArray(2, data_, pos + left);
            NSortHelpers::MarkArray(3, data_, pos + right);
        }

        ptrdiff_t length;
        ptrdiff_t fragment = leftOverFrag;

        if (left < leftEnd) {
            length = leftEnd - left;

            while (left < leftEnd) {
                --rightEnd;
                --leftEnd;
                data_[pos + rightEnd] = data_[pos + leftEnd];
                NSortHelpers::MarkArray(2, data_, pos + leftEnd);
            }
        }
        else {
            length = rightEnd - right;
            fragment = typeFrag;
        }

        return SqrtSortState{ length, fragment };
    }

    // SqrtSort.sqrtMergeBuffersLeftWithXBuf
    // arr - starting array. arr[0 - regBlockLen..-1] - buffer.
    // regBlockLen - length of regular blocks. First blockCount blocks are stable sorted
    // by 1st elements and key-coded. keysPos < midkey means stream A. aBlockCount are
    // regular blocks from stream A. lastLen is the length of the last (irregular) block
    // from stream B, that should go before aBlockCount blocks.
    template<class T>
    void SqrtSortMergeBuffersLeftWithXBuf(std::vector<ptrdiff_t>& keys, ptrdiff_t midkey,
                                          std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t blockCount,
                                          ptrdiff_t regBlockLen, ptrdiff_t aBlockCount, ptrdiff_t lastLen) {
        if (blockCount == 0) {
            ptrdiff_t aBlocksLen = aBlockCount * regBlockLen;
            SqrtSortMergeLeftWithXBuf(data_, pos, aBlocksLen, lastLen, 0 - regBlockLen);
            return;
        }

        ptrdiff_t leftOverLen = regBlockLen;
        ptrdiff_t leftOverFrag = (keys[0] < midkey) ? 0 : 1;
        ptrdiff_t processIndex = regBlockLen;

        ptrdiff_t restToProcess;

        for (ptrdiff_t keyIndex = 1; keyIndex < blockCount; ++keyIndex, processIndex += regBlockLen) {
            restToProcess = processIndex - leftOverLen;
            ptrdiff_t nextFrag = (keys[keyIndex] < midkey) ? 0 : 1;

            if (nextFrag == leftOverFrag) {
                SqrtSortArrayCopy(data_, pos + restToProcess, data_, pos + restToProcess - regBlockLen, leftOverLen);

                restToProcess = processIndex;
                leftOverLen = regBlockLen;
            }
            else {
                SqrtSortState results = SqrtSortSmartMergeWithXBuf(data_, pos + restToProcess, leftOverLen,
                                                                   leftOverFrag, regBlockLen);

                leftOverLen = results.leftOverLen;
                leftOverFrag = results.leftOverFrag;
            }
        }

        restToProcess = processIndex - leftOverLen;

        if (lastLen != 0) {
            if (leftOverFrag != 0) {
                SqrtSortArrayCopy(data_, pos + restToProcess, data_, pos + restToProcess - regBlockLen, leftOverLen);

                restToProcess = processIndex;
                leftOverLen = regBlockLen * aBlockCount;
                leftOverFrag = 0;
            }
            else {
                leftOverLen += regBlockLen * aBlockCount;
            }
            SqrtSortMergeLeftWithXBuf(data_, pos + restToProcess, leftOverLen, lastLen, 0 - regBlockLen);
        }
        else {
            SqrtSortArrayCopy(data_, pos + restToProcess, data_, pos + restToProcess - regBlockLen, leftOverLen);
        }
    }

    // SqrtSort.sqrtBuildBlocks
    // build blocks of length buildLen
    // input: [-buildLen,-1] elements are buffer
    // output: first buildLen elements are buffer, blocks 2 * buildLen and last subblock sorted
    template<class T>
    void SqrtSortBuildBlocks(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len, ptrdiff_t buildLen) {
        ptrdiff_t extraDist, part;

        for (ptrdiff_t dist = 1; dist < len; dist += 2) {
            extraDist = 0;
            if (data_[pos + (dist - 1)] > data_[pos + dist]) extraDist = 1;

            data_[pos + dist - 3] = data_[pos + dist - 1 + extraDist];
            data_[pos + dist - 2] = data_[pos + dist - extraDist];
        }
        if (len % 2 != 0) data_[pos + len - 3] = data_[pos + len - 1];

        pos -= 2;

        for (part = 2; part < buildLen; part *= 2) {
            ptrdiff_t left = 0;
            ptrdiff_t right = len - 2 * part;

            while (left <= right) {
                SqrtSortMergeLeftWithXBuf(data_, pos + left, part, part, 0 - part);
                left += 2 * part;
            }

            ptrdiff_t rest = len - left;

            if (rest > part) {
                SqrtSortMergeLeftWithXBuf(data_, pos + left, part, rest - part, 0 - part);
            }
            else {
                while (left < len) {
                    data_[pos + left - part] = data_[pos + left];
                    ++left;
                }
            }

            pos -= part;
        }
        ptrdiff_t restToBuild = len % (2 * buildLen);
        ptrdiff_t leftOverPos = len - restToBuild;

        if (restToBuild <= buildLen) {
            SqrtSortArrayCopy(data_, pos + leftOverPos, data_, pos + leftOverPos + buildLen, restToBuild);
        }
        else {
            SqrtSortMergeRight(data_, pos + leftOverPos, buildLen, restToBuild - buildLen, buildLen);
        }

        while (leftOverPos > 0) {
            leftOverPos -= 2 * buildLen;
            SqrtSortMergeRight(data_, pos + leftOverPos, buildLen, buildLen, buildLen);
        }
    }

    // SqrtSort.sqrtCombineBlocks
    // keys are on the left of arr. Blocks of length buildLen combined in pairs.
    // buildLen and numKeys are powers of 2. (2 * buildLen / regBlockLen) keys are guaranteed.
    template<class T>
    void SqrtSortCombineBlocks(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len, ptrdiff_t buildLen,
                               ptrdiff_t regBlockLen, std::vector<ptrdiff_t>& tags) {
        ptrdiff_t combineLen = len / (2 * buildLen);
        ptrdiff_t leftOver = len % (2 * buildLen);

        if (leftOver <= buildLen) {
            len -= leftOver;
            leftOver = 0;
        }

        ptrdiff_t leftIndex = 0;

        for (ptrdiff_t i = 0; i <= combineLen; ++i) {
            if (i == combineLen && leftOver == 0) break;

            ptrdiff_t blockPos = pos + i * 2 * buildLen;
            ptrdiff_t blockCount = (i == combineLen ? leftOver : 2 * buildLen) / regBlockLen;

            ptrdiff_t tagIndex = blockCount + (i == combineLen ? 1 : 0);
            for (ptrdiff_t j = 0; j <= tagIndex; ++j) tags[j] = j;

            ptrdiff_t midkey = buildLen / regBlockLen;

            for (tagIndex = 1; tagIndex < blockCount; ++tagIndex) {
                leftIndex = tagIndex - 1;

                for (ptrdiff_t rightIndex = tagIndex; rightIndex < blockCount; ++rightIndex) {
                    int rightComp = NSortHelpers::CompareValues(data_[blockPos + leftIndex * regBlockLen],
                                                                data_[blockPos + rightIndex * regBlockLen]);
                    if (rightComp > 0 || (rightComp == 0 && tags[leftIndex] > tags[rightIndex])) leftIndex = rightIndex;
                }

                if (leftIndex != tagIndex - 1) {
                    SqrtSortMultiSwap(data_, blockPos + (tagIndex - 1) * regBlockLen,
                                      blockPos + leftIndex * regBlockLen, regBlockLen);
                    SqrtSortTagsSwap(tags, tagIndex - 1, leftIndex);
                }
            }
            ptrdiff_t aBlockCount = 0;
            ptrdiff_t lastLen = 0;

            if (i == combineLen) lastLen = leftOver % regBlockLen;

            if (lastLen != 0) {
                while (aBlockCount < blockCount && data_[blockPos + blockCount * regBlockLen] <
                        data_[blockPos + (blockCount - aBlockCount - 1) * regBlockLen]) {
                    ++aBlockCount;
                }
            }
            SqrtSortMergeBuffersLeftWithXBuf(tags, midkey, data_, blockPos, blockCount - aBlockCount,
                                             regBlockLen, aBlockCount, lastLen);
        }
        for (leftIndex = len - 1; leftIndex >= 0; --leftIndex) {
            data_[pos + leftIndex] = data_[pos + leftIndex - regBlockLen];
        }
    }

    // SqrtSort.sqrtCommonSort (recursion depth is O(log log n), so it stays recursive).
    template<class T>
    void SqrtSortCommonSort(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len,
                            std::vector<T>& extBuf, ptrdiff_t extBufPos, std::vector<ptrdiff_t>& tags) {
        if (len <= 16) {
            SqrtSortInsertSort(data_, pos, len);
            return;
        }

        ptrdiff_t blockLen = 1;
        while ((blockLen * blockLen) < len) blockLen *= 2;

        SqrtSortArrayCopy(data_, pos, extBuf, extBufPos, blockLen);

        SqrtSortCommonSort(extBuf, extBufPos, blockLen, data_, pos, tags);

        SqrtSortBuildBlocks(data_, pos + blockLen, len - blockLen, blockLen);

        ptrdiff_t buildLen = blockLen;

        while (len > (buildLen *= 2)) {
            SqrtSortCombineBlocks(data_, pos + blockLen, len - blockLen, buildLen, blockLen, tags);
        }
        SqrtSortMergeDown(data_, pos + blockLen, extBuf, extBufPos, len - blockLen, blockLen);
    }

    // port of SqrtSort.runSort
    template<class T = int>
    void SqrtSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;

        ptrdiff_t len = static_cast<ptrdiff_t>(data_.size());
        ptrdiff_t bufferLen = 1;

        while (bufferLen * bufferLen < len) bufferLen *= 2;
        ptrdiff_t numKeys = (len - 1) / bufferLen + 2;

        std::vector<T> extBuf(bufferLen);
        std::vector<ptrdiff_t> tags(numKeys);

        SqrtSortCommonSort(data_, 0, len, extBuf, 0, tags);
    }

    // =============================== EctaSort ===============================
    // ArrayV EctaSort (c) 2020-2021 aphitorite. Standalone implementation: it does not use
    // KotaSorting helpers; its own merge/block-cycle machinery is ported below.

    // EctaSort.getMinRun
    inline ptrdiff_t EctaSortGetMinRun(ptrdiff_t n) {
        ptrdiff_t mRun = n;
        while (mRun >= 32) mRun = (mRun + 1) / 2;
        return mRun;
    }

    // Writes.arraycopy (ArrayV Writes.java): memmove semantics (see SqrtSortArrayCopy).
    template<class T>
    void EctaSortArrayCopy(std::vector<T>& src, ptrdiff_t srcPos,
                           std::vector<T>& dest, ptrdiff_t destPos, ptrdiff_t length) {
        if (&src != &dest || destPos < srcPos) {
            for (ptrdiff_t i = 0; i < length; ++i) dest[destPos + i] = src[srcPos + i];
        }
        else {
            for (ptrdiff_t i = length - 1; i >= 0; --i) dest[destPos + i] = src[srcPos + i];
        }
    }

    // EctaSort.shift
    template<class T>
    void EctaSortShift(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        while (m < b) {
            data_[a] = data_[m];
            ++a;
            ++m;
        }
    }

    // EctaSort.shiftBW
    template<class T>
    void EctaSortShiftBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        while (m > a) {
            --b;
            --m;
            data_[b] = data_[m];
        }
    }

    // BinaryDoubleInsertionSort.leftBinarySearch (ported locally: the left search sets
    // hi = mid on val <= array[mid]).
    template<class T>
    ptrdiff_t EctaSortLeftBinarySearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            NSortHelpers::MarkArray(1, data_, a);
            NSortHelpers::MarkArray(2, data_, m);
            NSortHelpers::MarkArray(3, data_, b);

            if (val <= data_[m]) b = m;
            else a = m + 1;
        }

        return a;
    }

    // BinaryDoubleInsertionSort.rightBinarySearch (val < array[mid] sets hi = mid).
    template<class T>
    ptrdiff_t EctaSortRightBinarySearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            NSortHelpers::MarkArray(1, data_, a);
            NSortHelpers::MarkArray(2, data_, m);
            NSortHelpers::MarkArray(3, data_, b);

            if (val < data_[m]) b = m;
            else a = m + 1;
        }

        return a;
    }

    // BinaryDoubleInsertionSort.insertToLeft
    template<class T>
    void EctaSortInsertToLeft(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& temp) {
        while (a > b) {
            --a;
            data_[a + 1] = data_[a];
        }
        data_[b] = temp;
    }

    // BinaryDoubleInsertionSort.insertToRight
    template<class T>
    void EctaSortInsertToRight(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, const T& temp) {
        while (a < b) {
            data_[a] = data_[a + 1];
            ++a;
        }
        data_[a] = temp;
    }

    // BinaryDoubleInsertionSort.doubleInsertion (customDoubleInsert(array, a, b, 0.5)).
    template<class T>
    void EctaSortDoubleInsert(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        if (b - a < 2) return;

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

                ptrdiff_t m = EctaSortRightBinarySearch(data_, i + 1, j, l);
                EctaSortInsertToRight(data_, i, m - 1, l);
                EctaSortInsertToLeft(data_, j, EctaSortLeftBinarySearch(data_, m, j, r), r);
            }
            else {
                T l = data_[i];
                T r = data_[j];

                ptrdiff_t m = EctaSortLeftBinarySearch(data_, i + 1, j, l);
                EctaSortInsertToRight(data_, i, m - 1, l);
                EctaSortInsertToLeft(data_, j, EctaSortRightBinarySearch(data_, m, j, r), r);
            }
            --i;
            ++j;
        }
    }

    // EctaSort.mergeTo
    template<class T>
    void EctaSortMergeTo(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t i = a, j = m;

        while (i < m && j < b) {
            if (data_[i] <= data_[j]) {
                data_[p] = data_[i];
                ++p;
                ++i;
            }
            else {
                data_[p] = data_[j];
                ++p;
                ++j;
            }
        }
        while (i < m) {
            data_[p] = data_[i];
            ++p;
            ++i;
        }
        while (j < b) {
            data_[p] = data_[j];
            ++p;
            ++j;
        }
    }

    // EctaSort.pingPongMerge
    template<class T>
    void EctaSortPingPongMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m1, ptrdiff_t m2,
                               ptrdiff_t m3, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t p1 = p + m2 - a;
        ptrdiff_t pEnd = p + b - a;

        EctaSortMergeTo(data_, a, m1, m2, p);
        EctaSortMergeTo(data_, m2, m3, b, p1);
        EctaSortMergeTo(data_, p, p1, pEnd, a);
    }

    // EctaSort.merge
    template<class T>
    void EctaSortMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t len = b - m;
        ptrdiff_t pEnd = p + len - 1;
        {
            ptrdiff_t srcPos = m;
            --m;
            EctaSortArrayCopy(data_, srcPos, data_, p, len);
        }

        while (m >= a && pEnd >= p) {
            if (data_[m] > data_[pEnd]) {
                --b;
                data_[b] = data_[m];
                --m;
            }
            else {
                --b;
                data_[b] = data_[pEnd];
                --pEnd;
            }
        }
        while (pEnd >= p) {
            --b;
            data_[b] = data_[pEnd];
            --pEnd;
        }
    }

    // EctaSort.mergeFromBuf
    template<class T>
    void EctaSortMergeFromBuf(std::vector<T>& data_, std::vector<T>& buf, ptrdiff_t a, ptrdiff_t m,
                              ptrdiff_t b, ptrdiff_t bufLen) {
        ptrdiff_t i = 0;

        while (i < bufLen && m < b) {
            NSortHelpers::MarkArray(2, data_, i);

            if (buf[i] <= data_[m]) {
                data_[a] = buf[i];
                ++a;
                ++i;
            }
            else {
                data_[a] = data_[m];
                ++a;
                ++m;
            }
        }
        while (i < bufLen) {
            NSortHelpers::MarkArray(2, data_, i);
            data_[a] = buf[i];
            ++a;
            ++i;
        }
    }

    // EctaSort.dualMergeFromBufBW
    template<class T>
    void EctaSortDualMergeFromBufBW(std::vector<T>& data_, std::vector<T>& buf, ptrdiff_t a,
                                    ptrdiff_t a1, ptrdiff_t m, ptrdiff_t b, ptrdiff_t bufLen) {
        ptrdiff_t i = bufLen - 1;
        {
            ptrdiff_t mOld = m;
            --m;
            bufLen -= b - mOld;
        }

        while (i >= bufLen && m >= a1) {
            NSortHelpers::MarkArray(2, data_, i);

            if (buf[i] > data_[m]) {
                --b;
                data_[b] = buf[i];
                --i;
            }
            else {
                --b;
                data_[b] = data_[m];
                --m;
            }
        }
        if (m < a1) {
            while (i >= 0) {
                --b;
                data_[b] = buf[i];
                --i;
            }
        }
        else {
            EctaSortMergeFromBuf(data_, buf, a, a1, b, bufLen);
        }
    }

    // EctaSort.mergeSort - returns the run length j.
    template<class T>
    ptrdiff_t EctaSortMergeSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t p,
                                ptrdiff_t mRun, ptrdiff_t bufLen) {
        ptrdiff_t i = a, j = mRun;

        for (; i + j <= b; i += j) {
            EctaSortDoubleInsert(data_, i, i + j);
        }
        EctaSortDoubleInsert(data_, i, b);

        while (4 * j <= bufLen) {
            for (i = a; i + 4 * j <= b; i += 4 * j) {
                EctaSortPingPongMerge(data_, i, i + j, i + 2 * j, i + 3 * j, i + 4 * j, p);
            }

            if (i + 3 * j < b) {
                EctaSortPingPongMerge(data_, i, i + j, i + 2 * j, i + 3 * j, b, p);
            }
            else if (i + 2 * j < b) {
                EctaSortPingPongMerge(data_, i, i + j, i + 2 * j, b, b, p);
            }
            else if (i + j < b) {
                EctaSortMerge(data_, i, i + j, b, p);
            }

            j *= 4;
        }
        while (j <= bufLen) {
            for (i = a; i + 2 * j <= b; i += 2 * j) {
                EctaSortMerge(data_, i, i + j, i + 2 * j, p);
            }

            if (i + j < b) {
                EctaSortMerge(data_, i, i + j, b, p);
            }

            j *= 2;
        }

        return j;
    }

    // EctaSort.blockCycle
    template<class T>
    void EctaSortBlockCycle(std::vector<T>& data_, std::vector<ptrdiff_t>& keys, ptrdiff_t a,
                            ptrdiff_t bLen, ptrdiff_t t, ptrdiff_t p, bool excl, bool fw) {
        ptrdiff_t s = fw ? bLen : -bLen;

        for (ptrdiff_t i = 0; i < t; ++i) {
            if (i != keys[i]) {
                EctaSortArrayCopy(data_, a + i * s, data_, p, bLen);
                ptrdiff_t j = i, next = keys[i];

                do {
                    if (!(excl && j == t - 1)) {
                        EctaSortArrayCopy(data_, a + next * s, data_, a + j * s, bLen);
                    }
                    NSortHelpers::MarkArray(2, data_, j);
                    keys[j] = j;

                    j = next;
                    next = keys[next];
                } while (next != i);

                EctaSortArrayCopy(data_, p, data_, a + j * s, bLen);
                NSortHelpers::MarkArray(2, data_, j);
                keys[j] = j;
            }
        }
    }

    // EctaSort.ectaMergeFW
    template<class T>
    void EctaSortMergeFW(std::vector<T>& data_, std::vector<ptrdiff_t>& tags, ptrdiff_t a,
                         ptrdiff_t m, ptrdiff_t b, ptrdiff_t bLen) {
        ptrdiff_t i = a, j = m, t = 0, tc = 0;
        ptrdiff_t s[2] = { 2 * bLen, 0 };
        ptrdiff_t p[2] = { a - 2 * bLen, m };

        do {
            ptrdiff_t c = s[0] < bLen ? 1 : 0;

            for (ptrdiff_t k = 0; k < bLen; ++k) {
                if (i < m && j < b) {
                    if (data_[i] <= data_[j]) {
                        data_[p[c] + k] = data_[i];
                        ++i;
                        ++s[0];
                    }
                    else {
                        data_[p[c] + k] = data_[j];
                        ++j;
                        ++s[1];
                    }
                }
                else if (i < m) {
                    data_[p[c] + k] = data_[i];
                    ++i;
                    ++s[0];
                }
                else {
                    data_[p[c] + k] = data_[j];
                    ++j;
                    ++s[1];
                }
            }
            p[c] += bLen;
            s[c] -= bLen;

            NSortHelpers::MarkArray(2, data_, tc);
            if (c == 0) {
                tags[tc] = t;
                ++t;
            }
            else {
                tags[tc] = -1;
            }
            ++tc;
        } while (i < m || j < b);

        if (s[0] > 0) {
            tags[tc] = t;
            ++t;
        }

        for (ptrdiff_t k = 2; k < tc; ++k) {
            if (tags[k] == -1) {
                NSortHelpers::MarkArray(2, data_, k);
                tags[k] = t;
                ++t;
            }
        }
        EctaSortBlockCycle(data_, tags, a - 2 * bLen, bLen, t, b - bLen, s[0] > 0, true);
    }

    // EctaSort.ectaMergeBW
    template<class T>
    void EctaSortMergeBW(std::vector<T>& data_, std::vector<ptrdiff_t>& tags, ptrdiff_t a,
                         ptrdiff_t m, ptrdiff_t b, ptrdiff_t bLen) {
        ptrdiff_t i = b - 1, j = m - 1, t = 0, tc = 0;
        ptrdiff_t s[2] = { 2 * bLen, 0 };
        ptrdiff_t p[2] = { b + 2 * bLen, m };

        do {
            ptrdiff_t c = s[0] < bLen ? 1 : 0;

            for (ptrdiff_t k = 1; k <= bLen; ++k) {
                if (i >= m && j >= a) {
                    if (data_[i] >= data_[j]) {
                        data_[p[c] - k] = data_[i];
                        --i;
                        ++s[0];
                    }
                    else {
                        data_[p[c] - k] = data_[j];
                        --j;
                        ++s[1];
                    }
                }
                else if (i >= m) {
                    data_[p[c] - k] = data_[i];
                    --i;
                    ++s[0];
                }
                else {
                    data_[p[c] - k] = data_[j];
                    --j;
                    ++s[1];
                }
            }
            p[c] -= bLen;
            s[c] -= bLen;

            NSortHelpers::MarkArray(2, data_, tc);
            if (c == 0) {
                tags[tc] = t;
                ++t;
            }
            else {
                tags[tc] = -1;
            }
            ++tc;
        } while (i >= m || j >= a);

        if (s[0] > 0) {
            tags[tc] = t;
            ++t;
        }

        for (ptrdiff_t k = 2; k < tc; ++k) {
            if (tags[k] == -1) {
                NSortHelpers::MarkArray(2, data_, k);
                tags[k] = t;
                ++t;
            }
        }
        EctaSortBlockCycle(data_, tags, b + bLen, bLen, t, a, s[0] > 0, false);
    }

    // port of EctaSort.runSort
    template<class T = int>
    void EctaSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;

        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        if (length < 256) {
            if (length <= 32) {
                EctaSortDoubleInsert(data_, 0, length);
            }
            else {
                ptrdiff_t mRun = EctaSortGetMinRun(length), bufLen = length / 2;
                std::vector<T> buf(bufLen);

                EctaSortArrayCopy(data_, bufLen, buf, 0, bufLen);
                EctaSortMergeSort(data_, 0, bufLen, bufLen, mRun, bufLen);

                EctaSortArrayCopy(buf, 0, data_, bufLen, bufLen);
                EctaSortArrayCopy(data_, 0, buf, 0, bufLen);
                EctaSortMergeSort(data_, bufLen, length, 0, mRun, bufLen);

                EctaSortMergeFromBuf(data_, buf, 0, bufLen, length, bufLen);
            }
            return;
        }

        ptrdiff_t mRun = EctaSortGetMinRun(length), bLen = mRun;
        for (; bLen * bLen < length / 2; bLen *= 2);
        ptrdiff_t bufLen = 2 * bLen + length % bLen;

        ptrdiff_t a = bufLen, b = length, len = b - a;

        std::vector<T> buf(bufLen);
        std::vector<ptrdiff_t> tags(len / bLen + 1);

        EctaSortArrayCopy(data_, a, buf, 0, bufLen);
        EctaSortMergeSort(data_, 0, a, a, EctaSortGetMinRun(bufLen), bufLen);

        EctaSortArrayCopy(buf, 0, data_, a, bufLen);
        EctaSortArrayCopy(data_, 0, buf, 0, bufLen);

        ptrdiff_t i = a, j = EctaSortMergeSort(data_, a, b, 0, mRun, bufLen);
        bool bw = false;

        while (j < len) {
            for (i = a; i + 2 * j <= b; i += 2 * j) {
                EctaSortMergeFW(data_, tags, i, i + j, i + 2 * j, bLen);
            }

            if (i + j < b) EctaSortMergeFW(data_, tags, i, i + j, b, bLen);
            else            EctaSortShift(data_, i - 2 * bLen, i, b);

            j *= 2;
            a -= 2 * bLen;
            b -= 2 * bLen;

            if (j >= len) {
                bw = true;
                break;
            }

            for (i = a; i + 2 * j <= b; i += 2 * j) { }

            if (i + j < b) EctaSortMergeBW(data_, tags, i, i + j, b, bLen);
            else           EctaSortShiftBW(data_, i, b, b + 2 * bLen);

            for (i -= 2 * j; i >= a; i -= 2 * j) {
                EctaSortMergeBW(data_, tags, i, i + j, i + 2 * j, bLen);
            }

            j *= 2;
            a += 2 * bLen;
            b += 2 * bLen;
        }
        if (bw) EctaSortDualMergeFromBufBW(data_, buf, 0, a, b, length, bufLen);
        else    EctaSortMergeFromBuf(data_, buf, 0, a, b, bufLen);
    }

    // =========================== CircularGrailSort ===========================
    // ArrayV CircularGrailSort (c) 2021 aphitorite. Positions are taken modulo n, which
    // lets the merges run past the end of the array ("circular" workspace).

    // CircularGrailSort.circSwap
    template<class T>
    void CircularGrailSortCircSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t n) {
        using std::swap;
        swap(data_[a % n], data_[b % n]);
    }

    // CircularGrailSort.circCompareIndices
    template<class T>
    int CircularGrailSortCircCompare(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t n) {
        return NSortHelpers::CompareValues(data_[a % n], data_[b % n]);
    }

    // CircularGrailSort.shiftFW
    template<class T>
    void CircularGrailSortShiftFW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t n) {
        while (m < b) {
            CircularGrailSortCircSwap(data_, a, m, n);
            ++a;
            ++m;
        }
    }

    // CircularGrailSort.shiftBW
    template<class T>
    void CircularGrailSortShiftBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t n) {
        while (m > a) {
            --b;
            --m;
            CircularGrailSortCircSwap(data_, b, m, n);
        }
    }

    // CircularGrailSort.insertion
    template<class T>
    void CircularGrailSortInsertion(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t n) {
        for (ptrdiff_t i = a + 1; i < b; ++i) {
            while (i > a && CircularGrailSortCircCompare(data_, i - 1, i, n) > 0) {
                ptrdiff_t old = i;
                --i;
                CircularGrailSortCircSwap(data_, old, i, n);
            }
        }
    }

    // CircularGrailSort.multiSwap
    template<class T>
    void CircularGrailSortMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len, ptrdiff_t n) {
        for (ptrdiff_t i = 0; i < len; ++i) {
            CircularGrailSortCircSwap(data_, a + i, b + i, n);
        }
    }

    // CircularGrailSort.rotate
    template<class T>
    void CircularGrailSortRotate(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t n) {
        ptrdiff_t l = m - a, r = b - m;

        while (l > 0 && r > 0) {
            if (r < l) {
                CircularGrailSortMultiSwap(data_, m - r, m, r, n);
                b -= r;
                m -= r;
                l -= r;
            }
            else {
                CircularGrailSortMultiSwap(data_, a, m, l, n);
                a += l;
                m += l;
                r -= l;
            }
        }
    }

    // CircularGrailSort.inPlaceMerge
    template<class T>
    void CircularGrailSortInPlaceMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t n) {
        ptrdiff_t i = a, j = m, k;

        while (i < j && j < b) {
            if (CircularGrailSortCircCompare(data_, i, j, n) > 0) {
                k = j;
                ++k;
                while (k < b && CircularGrailSortCircCompare(data_, i, k, n) > 0) ++k;

                CircularGrailSortRotate(data_, i, j, k, n);

                i += k - j;
                j = k;
            }
            else {
                ++i;
            }
        }
    }

    // CircularGrailSort.merge - returns the new i (or j when the left run is exhausted).
    template<class T>
    ptrdiff_t CircularGrailSortMerge(std::vector<T>& data_, ptrdiff_t p, ptrdiff_t a, ptrdiff_t m,
                                     ptrdiff_t b, bool full, ptrdiff_t n) {
        ptrdiff_t i = a, j = m;

        while (i < m && j < b) {
            if (CircularGrailSortCircCompare(data_, i, j, n) <= 0) {
                ptrdiff_t oldP = p, oldI = i;
                ++p;
                ++i;
                CircularGrailSortCircSwap(data_, oldP, oldI, n);
            }
            else {
                ptrdiff_t oldP = p, oldJ = j;
                ++p;
                ++j;
                CircularGrailSortCircSwap(data_, oldP, oldJ, n);
            }
        }
        if (i < m) {
            if (i > p) CircularGrailSortShiftFW(data_, p, i, m, n);
        }
        else if (full) {
            CircularGrailSortShiftFW(data_, p, j, b, n);
        }

        return i < m ? i : j;
    }

    // CircularGrailSort.blockLessThan
    template<class T>
    bool CircularGrailSortBlockLessThan(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b,
                                        ptrdiff_t bLen, ptrdiff_t n) {
        int cmp = CircularGrailSortCircCompare(data_, a, b, n);

        return cmp == -1 || (cmp == 0 && CircularGrailSortCircCompare(data_, a + bLen - 1, b + bLen - 1, n) == -1);
    }

    // CircularGrailSort.blockMerge
    template<class T>
    void CircularGrailSortBlockMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b,
                                     ptrdiff_t bLen, ptrdiff_t n) {
        ptrdiff_t b1 = b - (b - m - 1) % bLen - 1;

        if (b1 > m) {
            ptrdiff_t b2 = b1;

            for (ptrdiff_t i = m - bLen; i > a && CircularGrailSortBlockLessThan(data_, b1, i, bLen, n);
                 i -= bLen, b2 -= bLen) { }

            for (ptrdiff_t j = a; j < b1 - bLen; j += bLen) {
                ptrdiff_t min = j;

                for (ptrdiff_t i = min + bLen; i < b1; i += bLen) {
                    if (CircularGrailSortBlockLessThan(data_, i, min, bLen, n)) min = i;
                }

                if (min != j) CircularGrailSortMultiSwap(data_, j, min, bLen, n);
            }
            ptrdiff_t f = a;

            for (ptrdiff_t i = a + bLen; i < b2; i += bLen) {
                f = CircularGrailSortMerge(data_, f - bLen, f, i, i + bLen, false, n);

                if (f < i) {
                    CircularGrailSortShiftBW(data_, f, i, i + bLen, n);
                    f += bLen;
                }
            }
            CircularGrailSortMerge(data_, f - bLen, f, b1, b, true, n);
        }
        else {
            CircularGrailSortMerge(data_, a - bLen, a, m, b, true, n);
        }
    }

    // port of CircularGrailSort.runSort
    template<class T = int>
    void CircularGrailSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;

        ptrdiff_t n = static_cast<ptrdiff_t>(data_.size());

        if (n <= 16) {
            CircularGrailSortInsertion(data_, 0, n, n);
            return;
        }

        ptrdiff_t bLen = 1;
        for (; bLen * bLen < n; bLen *= 2);

        ptrdiff_t i = bLen, j = 1, len = n - i, b = n;

        while (j <= bLen) {
            for (; i + 2 * j < b; i += 2 * j) {
                CircularGrailSortMerge(data_, i - j, i, i + j, i + 2 * j, true, n);
            }
            if (i + j < b) {
                CircularGrailSortMerge(data_, i - j, i, i + j, b, true, n);
            }
            else {
                CircularGrailSortShiftFW(data_, i - j, i, b, n);
            }

            i = b + bLen - j;
            b = i + len;
            j *= 2;
        }
        while (j < len) {
            for (; i + 2 * j < b; i += 2 * j) {
                CircularGrailSortBlockMerge(data_, i, i + j, i + 2 * j, bLen, n);
            }
            if (i + j < b) {
                CircularGrailSortBlockMerge(data_, i, i + j, b, bLen, n);
            }
            else {
                CircularGrailSortShiftFW(data_, i - bLen, i, b, n);
            }

            i = b;
            b += len;
            j *= 2;
        }
        CircularGrailSortInsertion(data_, i - bLen, i, n);
        CircularGrailSortInPlaceMerge(data_, i - bLen, i, b, n);

        CircularGrailSortRotate(data_, 0, (i - bLen) % n, n, n);
    }

    // ==================== FlanSort + RemiSort + BufferPartitionMergeSort ====================
// Ports of ArrayV (Java, MIT) hybrid sorting classes: FlanSort, RemiSort,
// BufferPartitionMergeSort.


    // ===================== FlanSort =====================

    // FlanSort.G / FlanSort.R (private final fields)
    inline constexpr ptrdiff_t FlanSortG = 14;
    inline constexpr ptrdiff_t FlanSortR = 4;

    // FlanSort.medianOfThree
    template<class T>
    ptrdiff_t FlanSortMedianOfThree(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        if (NSortHelpers::CompareValues(data_[m], data_[a]) > 0) {
            if (NSortHelpers::CompareValues(data_[m], data_[b]) < 0)
                return m;
            if (NSortHelpers::CompareValues(data_[a], data_[b]) > 0)
                return a;
            else
                return b;
        }
        else {
            if (NSortHelpers::CompareValues(data_[m], data_[b]) > 0)
                return m;
            if (NSortHelpers::CompareValues(data_[a], data_[b]) < 0)
                return a;
            else
                return b;
        }
    }

    // FlanSort.ninther
    template<class T>
    ptrdiff_t FlanSortNinther(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t s = (b - a) / 9;

        ptrdiff_t a1 = FlanSortMedianOfThree(data_, a, a + s, a + 2 * s);
        ptrdiff_t m1 = FlanSortMedianOfThree(data_, a + 3 * s, a + 4 * s, a + 5 * s);
        ptrdiff_t b1 = FlanSortMedianOfThree(data_, a + 6 * s, a + 7 * s, a + 8 * s);

        return FlanSortMedianOfThree(data_, a1, m1, b1);
    }

    // FlanSort.medianOfThreeNinthers
    template<class T>
    ptrdiff_t FlanSortMedianOfThreeNinthers(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t s = (b - a) / 3;

        ptrdiff_t a1 = FlanSortNinther(data_, a, a + s);
        ptrdiff_t m1 = FlanSortNinther(data_, a + s, a + 2 * s);
        ptrdiff_t b1 = FlanSortNinther(data_, a + 2 * s, b);

        return FlanSortMedianOfThree(data_, a1, m1, b1);
    }

    // FlanSort.shiftBW
    template<class T>
    void FlanSortShiftBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        while (m > a) {
            --b;
            --m;
            using std::swap;
            swap(data_[b], data_[m]);
        }
    }

    // FlanSort.leftBlockSearch
    template<class T>
    ptrdiff_t FlanSortLeftBlockSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, T val) {
        ptrdiff_t s = FlanSortG + 1;

        while (a < b) {
            ptrdiff_t m = a + (((b - a) / s) / 2) * s;
            NSortHelpers::MarkArray(3, data_, m);

            if (NSortHelpers::CompareValues(val, data_[m]) <= 0)
                b = m;
            else
                a = m + s;
        }

        return a;
    }

    // FlanSort.rightBlockSearch
    template<class T>
    ptrdiff_t FlanSortRightBlockSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, T val) {
        ptrdiff_t s = FlanSortG + 1;

        while (a < b) {
            ptrdiff_t m = a + (((b - a) / s) / 2) * s;
            NSortHelpers::MarkArray(3, data_, m);

            if (NSortHelpers::CompareValues(val, data_[m]) < 0)
                b = m;
            else
                a = m + s;
        }

        return a;
    }

    // FlanSort.rightBinSearch
    template<class T>
    ptrdiff_t FlanSortRightBinSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, T val, bool bw) {
        int cmp = bw ? 1 : -1;

        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;
            NSortHelpers::MarkArray(3, data_, m);

            if (NSortHelpers::CompareValues(val, data_[m]) == cmp)
                b = m;
            else
                a = m + 1;
        }

        return a;
    }

    // FlanSort.insertTo
    template<class T>
    void FlanSortInsertTo(std::vector<T>& data_, T tmp, ptrdiff_t a, ptrdiff_t b) {
        while (a > b) {
            data_[a] = data_[a - 1];
            --a;
        }
        data_[b] = tmp;
    }

    // FlanSort.binaryInsertion
    template<class T>
    void FlanSortBinaryInsertion(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        for (ptrdiff_t i = a + 1; i < b; i++) {
            T tmp = data_[i];
            ptrdiff_t loc = FlanSortRightBinSearch(data_, a, i, data_[i], false);
            FlanSortInsertTo(data_, tmp, i, loc);
        }
    }

    // FlanSort.kWayMerge (in-place variant; the inherited MultiWayMergeSorting
    // siftDown is NSortHelpers::kWaySiftDown, same parameter list)
    template<class T>
    void FlanSortKWayMerge(std::vector<T>& data_, std::vector<ptrdiff_t>& heap, std::vector<ptrdiff_t>& pa,
                           ptrdiff_t s, ptrdiff_t b, ptrdiff_t p, ptrdiff_t size) {
        if (size < 2) {
            if (size == 1) {
                while (pa[0] < b) {
                    using std::swap;
                    swap(data_[p], data_[pa[0]]);
                    ++p;
                    ++pa[0];
                }
            }
            return;
        }
        ptrdiff_t a = pa[0];

        for (ptrdiff_t i = 0; i < size; i++)
            heap[i] = i;

        for (ptrdiff_t i = (size - 1) / 2; i >= 0; i--)
            NSortHelpers::kWaySiftDown(data_, heap, pa, heap[i], i, size);

        while (size > 0) {
            ptrdiff_t min = heap[0];

            {
                using std::swap;
                swap(data_[p], data_[pa[min]]);
            }
            ++p;
            pa[min] = pa[min] + 1;

            if (pa[min] == (std::min)(a + (min + 1) * s, b)) {
                --size;
                NSortHelpers::kWaySiftDown(data_, heap, pa, heap[size], 0, size);
            }
            else
                NSortHelpers::kWaySiftDown(data_, heap, pa, heap[0], 0, size);
        }
    }

    // FlanSort.retrieve
    template<class T>
    void FlanSortRetrieve(std::vector<T>& data_, ptrdiff_t i, ptrdiff_t p, ptrdiff_t pEnd, T bsv, bool bw) {
        ptrdiff_t j = i - 1, m;

        for (ptrdiff_t k = pEnd - (FlanSortG + 1); k > p + FlanSortG;) {
            m = FlanSortRightBinSearch(data_, k - FlanSortG, k, bsv, bw) - 1;
            k -= FlanSortG + 1;

            while (m >= k) {
                using std::swap;
                swap(data_[j], data_[m]);
                --j;
                --m;
            }
        }

        m = FlanSortRightBinSearch(data_, p, p + FlanSortG, bsv, bw) - 1;
        while (m >= p) {
            using std::swap;
            swap(data_[j], data_[m]);
            --j;
            --m;
        }
    }

    // FlanSort.librarySort (Java's `new Random()` becomes the one shared engine)
    template<class T>
    void FlanSortLibrarySort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t p, T bsv, bool bw,
                             std::mt19937& engine) {
        ptrdiff_t len = b - a;

        if (len < 32) {
            FlanSortBinaryInsertion(data_, a, b);
            return;
        }

        ptrdiff_t s = len;
        while (s >= 32) s = (s - 1) / FlanSortR + 1;

        ptrdiff_t i = a + s, j = a + FlanSortR * s, pEnd = p + (s + 1) * (FlanSortG + 1) + FlanSortG;
        FlanSortBinaryInsertion(data_, a, i);
        for (ptrdiff_t k = 0; k < s; k++) { // scatter elements to make G sized gaps b/w them
            using std::swap;
            swap(data_[a + k], data_[p + k * (FlanSortG + 1) + FlanSortG]);
        }

        while (i < b) {
            if (i == j) { // rebalancing (retrieve from buffer & rescatter)
                FlanSortRetrieve(data_, i, p, pEnd, bsv, bw);

                s = i - a;
                pEnd = p + (s + 1) * (FlanSortG + 1) + FlanSortG;
                j = a + (j - a) * FlanSortR;

                for (ptrdiff_t k = 0; k < s; k++) {
                    using std::swap;
                    swap(data_[a + k], data_[p + k * (FlanSortG + 1) + FlanSortG]);
                }
            }

            ptrdiff_t bLoc = FlanSortLeftBlockSearch(data_, p + FlanSortG, pEnd - (FlanSortG + 1), data_[i]); // search gap location

            if (NSortHelpers::CompareValues(data_[i], data_[bLoc]) == 0) { // handle equal values to prevent worst case O(n^2)
                ptrdiff_t eqEnd = FlanSortRightBlockSearch(data_, bLoc + (FlanSortG + 1), pEnd - (FlanSortG + 1), data_[i]); // find the endpoint of the gaps with equal head element
                bLoc += NSortHelpers::bogoRandInt(engine, 0, (eqEnd - bLoc) / (FlanSortG + 1)) * (FlanSortG + 1); // choose a random gap from the range of gaps
            }

            ptrdiff_t loc = FlanSortRightBinSearch(data_, bLoc - FlanSortG, bLoc, bsv, bw); // search next empty space in gap

            if (loc == bLoc) { // if there is no empty space filled elements in gap are split
                do bLoc += FlanSortG + 1;
                while (bLoc < pEnd && FlanSortRightBinSearch(data_, bLoc - FlanSortG, bLoc, bsv, bw) == bLoc);

                if (bLoc == pEnd) { // rebalancing
                    FlanSortRetrieve(data_, i, p, pEnd, bsv, bw);

                    s = i - a;
                    pEnd = p + (s + 1) * (FlanSortG + 1) + FlanSortG;
                    j = a + (j - a) * FlanSortR;

                    for (ptrdiff_t k = 0; k < s; k++) {
                        using std::swap;
                        swap(data_[a + k], data_[p + k * (FlanSortG + 1) + FlanSortG]);
                    }
                }
                else { // if a gap is full find next non full gap to the right & shift the space down
                    ptrdiff_t rotP = FlanSortRightBinSearch(data_, bLoc - FlanSortG, bLoc, bsv, bw);
                    ptrdiff_t rotS = bLoc - (std::max)(rotP, bLoc - FlanSortG / 2); // for odd G whether its floor or ceil(G/2) doesnt matter
                    FlanSortShiftBW(data_, loc - rotS, bLoc - rotS, bLoc);
                }
            }
            else {
                T t = data_[i];
                data_[i] = data_[loc];
                ++i;
                FlanSortInsertTo(data_, t, loc, FlanSortRightBinSearch(data_, bLoc - FlanSortG, loc, t, false));
            }
        }
        FlanSortRetrieve(data_, b, p, pEnd, bsv, bw);
    }

    // port of FlanSort.runSort
    template<class T = int>
    void FlanSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) return;

        std::mt19937 engine(NSortHelpers::SortRandomEngine::GetSortRandom<T>());

        std::vector<ptrdiff_t> pa(FlanSortG + 2);
        std::vector<ptrdiff_t> heap(FlanSortG + 2);

        ptrdiff_t a = 0, b = length;

        while (b - a >= 32) {
            T piv = data_[FlanSortMedianOfThreeNinthers(data_, a, b)];

            // partition -> [a][E > piv][i][E == piv][j][E < piv][b]
            ptrdiff_t i1 = a, i = a - 1, j = b, j1 = b;

            for (;;) {
                while (true) {
                    ++i;
                    if (!(i < j)) break;
                    int cmp = NSortHelpers::CompareValues(data_[i], piv);
                    if (cmp == 0) {
                        using std::swap;
                        swap(data_[i1], data_[i]);
                        ++i1;
                    }
                    else if (cmp < 0) break;
                }

                while (true) {
                    --j;
                    if (!(j > i)) break;
                    int cmp = NSortHelpers::CompareValues(data_[j], piv);
                    if (cmp == 0) {
                        --j1;
                        using std::swap;
                        swap(data_[j1], data_[j]);
                    }
                    else if (cmp > 0) break;
                }

                if (i < j) {
                    using std::swap;
                    swap(data_[i], data_[j]);
                }
                else {
                    if (i1 == b) return;
                    else if (j < i) ++j;

                    while (i1 > a) {
                        --i;
                        --i1;
                        using std::swap;
                        swap(data_[i], data_[i1]);
                    }
                    while (j1 < b) {
                        using std::swap;
                        swap(data_[j], data_[j1]);
                        ++j;
                        ++j1;
                    }

                    break;
                }
            }

            ptrdiff_t left = i - a, right = b - j, m, kCnt = 0;

            if (left <= right) { // sort the smaller partition using larger partition as space
                m = b - left;
                left = (std::max)((right + 1) / (FlanSortG + 1), static_cast<ptrdiff_t>(16));

                for (ptrdiff_t k = a; k < i; k += left) {
                    FlanSortLibrarySort(data_, k, (std::min)(k + left, i), j, piv, true, engine);
                    pa[kCnt] = k;
                    ++kCnt;
                }

                FlanSortKWayMerge(data_, heap, pa, left, i, m, kCnt);

                // swap items eq to pivot next to sorted area
                // eq items zone: [i][E == piv][j][E < piv][m][sorted area]
                if (j - i < m - j) {
                    while (i < j) {
                        using std::swap;
                        swap(data_[i], data_[m - 1]);
                        ++i;
                        --m;
                    }
                    b = m;
                }
                else {
                    while (m > j) {
                        using std::swap;
                        swap(data_[i], data_[m - 1]);
                        ++i;
                        --m;
                    }
                    b = i;
                }
            }
            else {
                m = a + right;
                right = (std::max)((left + 1) / (FlanSortG + 1), static_cast<ptrdiff_t>(16));

                for (ptrdiff_t k = j; k < b; k += right) {
                    FlanSortLibrarySort(data_, k, (std::min)(k + right, b), a, piv, false, engine);
                    pa[kCnt] = k;
                    ++kCnt;
                }

                FlanSortKWayMerge(data_, heap, pa, right, b, a, kCnt);

                // eq items zone: [sorted area][m][E > piv][i][E == piv][j]
                if (i - m < j - i) {
                    while (m < i) {
                        using std::swap;
                        swap(data_[m], data_[j - 1]);
                        ++m;
                        --j;
                    }
                    a = j;
                }
                else {
                    while (j > i) {
                        using std::swap;
                        swap(data_[m], data_[j - 1]);
                        ++m;
                        --j;
                    }
                    a = m;
                }
            }
        }
        FlanSortBinaryInsertion(data_, a, b);
    }

    // ===================== RemiSort =====================

    // RemiSort.ceilCbrt
    inline ptrdiff_t RemiSortCeilCbrt(ptrdiff_t n) {
        ptrdiff_t a = 0, b = (std::min)(static_cast<ptrdiff_t>(1291), n);

        while (a < b) {
            ptrdiff_t m = (a + b) / 2;

            if (m * m * m >= n) b = m;
            else                a = m + 1;
        }

        return a;
    }

    // RemiSort.siftDown (the private keys-based overload used by tableSort; the
    // heap-array siftDown inherited from MultiWayMergeSorting is
    // NSortHelpers::kWaySiftDown, same parameter list)
    template<class T>
    void RemiSortSiftDown(std::vector<T>& data_, std::vector<ptrdiff_t>& keys, ptrdiff_t r, ptrdiff_t len,
                          ptrdiff_t a, ptrdiff_t t) {
        ptrdiff_t j = r;

        while (2 * j + 1 < len) {
            j = 2 * j + 1;

            if (j + 1 < len) {
                int cmp = NSortHelpers::CompareValues(data_[a + keys[j + 1]], data_[a + keys[j]]);

                if (cmp > 0 || (cmp == 0 && keys[j + 1] > keys[j])) j++;
            }
        }

        int cmp = NSortHelpers::CompareValues(data_[a + t], data_[a + keys[j]]);
        while (cmp > 0 || (cmp == 0 && t > keys[j])) {
            j = (j - 1) / 2;
            cmp = NSortHelpers::CompareValues(data_[a + t], data_[a + keys[j]]);
        }

        for (ptrdiff_t t2; j > r; j = (j - 1) / 2) {
            t2 = keys[j];
            keys[j] = t;
            t = t2;
        }
        keys[r] = t;
    }

    // RemiSort.tableSort
    template<class T>
    void RemiSortTableSort(std::vector<T>& data_, std::vector<ptrdiff_t>& keys, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t len = b - a;

        for (ptrdiff_t i = (len - 1) / 2; i >= 0; i--) {
            ptrdiff_t t = keys[i];
            RemiSortSiftDown(data_, keys, i, len, a, t);
        }

        for (ptrdiff_t i = len - 1; i > 0; i--) {
            ptrdiff_t t = keys[i];
            keys[i] = keys[0];
            RemiSortSiftDown(data_, keys, 0, i, a, t);
        }

        for (ptrdiff_t i = 0; i < len; i++) {
            if (i != keys[i]) {
                T t = data_[a + i];
                ptrdiff_t j = i, next = keys[i];

                do {
                    data_[a + j] = data_[a + next];
                    keys[j] = j;

                    j = next;
                    next = keys[next];
                } while (next != i);

                data_[a + j] = t;
                keys[j] = j;
            }
        }
    }

    // RemiSort.blockCycle
    template<class T>
    void RemiSortBlockCycle(std::vector<T>& data_, std::vector<T>& buf, std::vector<ptrdiff_t>& keys,
                            ptrdiff_t a, ptrdiff_t bLen, ptrdiff_t bCnt) {
        for (ptrdiff_t i = 0; i < bCnt; i++) {
            if (i != keys[i]) {
                for (ptrdiff_t k = 0; k < bLen; k++) buf[k] = data_[a + i * bLen + k];
                ptrdiff_t j = i, next = keys[i];

                do {
                    for (ptrdiff_t k = 0; k < bLen; k++) data_[a + j * bLen + k] = data_[a + next * bLen + k];
                    keys[j] = j;

                    j = next;
                    next = keys[next];
                } while (next != i);

                for (ptrdiff_t k = 0; k < bLen; k++) data_[a + j * bLen + k] = buf[k];
                keys[j] = j;
            }
        }
    }

    // RemiSort.kWayMerge (merges runs, stashes the tail in buf, then relocates
    // the buffered blocks with blockCycle)
    template<class T>
    void RemiSortKWayMerge(std::vector<T>& data_, std::vector<T>& buf, std::vector<ptrdiff_t>& keys,
                           std::vector<ptrdiff_t>& heap, ptrdiff_t b, std::vector<ptrdiff_t>& pa,
                           std::vector<ptrdiff_t>& p, ptrdiff_t bLen, ptrdiff_t rLen) {
        ptrdiff_t k = static_cast<ptrdiff_t>(p.size()), size = k, a = pa[0], a1 = pa[1];

        for (ptrdiff_t i = 0; i < k; i++)
            heap[i] = i;

        for (ptrdiff_t i = (k - 1) / 2; i >= 0; i--)
            NSortHelpers::kWaySiftDown(data_, heap, pa, heap[i], i, k);

        for (ptrdiff_t i = 0; i < rLen; i++) {
            ptrdiff_t min = heap[0];

            NSortHelpers::MarkArray(2, data_, pa[min]);

            buf[i] = data_[pa[min]];
            pa[min] = pa[min] + 1;

            if (pa[min] == (std::min)(a + (min + 1) * rLen, b)) {
                --size;
                NSortHelpers::kWaySiftDown(data_, heap, pa, heap[size], 0, size);
            }
            else
                NSortHelpers::kWaySiftDown(data_, heap, pa, heap[0], 0, size);
        }
        ptrdiff_t t = 0, cnt = 0, c = 0;
        while (pa[c] - p[c] < bLen) c++;

        do {
            ptrdiff_t min = heap[0];

            NSortHelpers::MarkArray(2, data_, pa[min]);
            NSortHelpers::MarkArray(3, data_, p[c]);

            data_[p[c]] = data_[pa[min]];
            pa[min] = pa[min] + 1;
            p[c] = p[c] + 1;

            if (pa[min] == (std::min)(a + (min + 1) * rLen, b)) {
                --size;
                NSortHelpers::kWaySiftDown(data_, heap, pa, heap[size], 0, size);
            }
            else
                NSortHelpers::kWaySiftDown(data_, heap, pa, heap[0], 0, size);

            ++cnt;
            if (cnt == bLen) {
                keys[t] = (c > 0) ? p[c] / bLen - bLen - 1 : -1;
                ++t;

                c = 0;
                cnt = 0;
                while (pa[c] - p[c] < bLen) c++;
            }
        } while (size > 0);

        while (cnt-- > 0) {
            p[c] = p[c] - 1;
            --b;
            data_[b] = data_[p[c]];
        }
        pa[k - 1] = b;
        keys[static_cast<ptrdiff_t>(keys.size()) - 1] = -1;

        t = 0;
        while (keys[t] != -1) t++;

        for (ptrdiff_t i = 1, j = a; j < p[0]; i++) {
            while (p[i] < pa[i]) {
                keys[t] = p[i] / bLen - bLen;
                ++t;
                while (keys[t] != -1) t++;

                for (ptrdiff_t kk = 0; kk < bLen; kk++) data_[p[i] + kk] = data_[j + kk];
                p[i] = p[i] + bLen;

                j += bLen;
            }
        }
        for (ptrdiff_t kk = 0; kk < rLen; kk++) data_[a + kk] = buf[kk];

        RemiSortBlockCycle(data_, buf, keys, a1, bLen, (b - a1) / bLen);
    }

    // port of RemiSort.runSort
    template<class T = int>
    void RemiSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) return;

        ptrdiff_t a = 0, b = length;

        ptrdiff_t bLen = RemiSortCeilCbrt(length);
        ptrdiff_t rLen = bLen * bLen;
        ptrdiff_t rCnt = (length - 1) / rLen + 1;

        if (rCnt < 2) {
            std::vector<ptrdiff_t> keys(length);

            for (ptrdiff_t i = 0; i < static_cast<ptrdiff_t>(keys.size()); i++)
                keys[i] = i;

            RemiSortTableSort(data_, keys, a, b);
            return;
        }

        std::vector<ptrdiff_t> keys(rLen);
        std::vector<T> buf(rLen);

        std::vector<ptrdiff_t> heap(rCnt);
        std::vector<ptrdiff_t> p(rCnt);
        std::vector<ptrdiff_t> pa(rCnt);

        for (ptrdiff_t i = 0; i < static_cast<ptrdiff_t>(keys.size()); i++)
            keys[i] = i;

        ptrdiff_t j = 0;
        for (ptrdiff_t i = a; i < b; i += rLen) {
            RemiSortTableSort(data_, keys, i, (std::min)(i + rLen, b));
            pa[j] = i;
            ++j;
        }
        for (ptrdiff_t i = 0; i < rCnt; i++) p[i] = pa[i];

        RemiSortKWayMerge(data_, buf, keys, heap, b, pa, p, bLen, rLen);
    }

    // ===================== BufferPartitionMergeSort =====================

    // BufferPartitionMergeSort.shiftBW
    template<class T>
    void BufferPartitionMergeSortShiftBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        while (m > a) {
            --b;
            --m;
            using std::swap;
            swap(data_[b], data_[m]);
        }
    }

    // BufferPartitionMergeSort.multiSwap
    template<class T>
    void BufferPartitionMergeSortMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
        for (ptrdiff_t i = 0; i < len; i++) {
            using std::swap;
            swap(data_[a + i], data_[b + i]);
        }
    }

    // BufferPartitionMergeSort.rotate (its own in-place rotation)
    template<class T>
    void BufferPartitionMergeSortRotate(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t l = m - a, r = b - m;

        while (l > 0 && r > 0) {
            if (r < l) {
                BufferPartitionMergeSortMultiSwap(data_, m - r, m, r);
                b -= r;
                m -= r;
                l -= r;
            }
            else {
                BufferPartitionMergeSortMultiSwap(data_, a, m, l);
                a += l;
                m += l;
                r -= l;
            }
        }
    }

    // BufferPartitionMergeSort.inPlaceMerge
    template<class T>
    void BufferPartitionMergeSortInPlaceMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = a, j = m, k;

        while (i < j && j < b) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) > 0) {
                k = j;
                while (++k < b && NSortHelpers::CompareValues(data_[i], data_[k]) > 0);

                BufferPartitionMergeSortRotate(data_, i, j, k);

                i += k - j;
                j = k;
            }
            else i++;
        }
    }

    // BufferPartitionMergeSort.medianOfThree
    template<class T>
    void BufferPartitionMergeSortMedianOfThree(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t m = a + (b - 1 - a) / 2;

        if (NSortHelpers::CompareValues(data_[a], data_[m]) == 1) {
            using std::swap;
            swap(data_[a], data_[m]);
        }

        if (NSortHelpers::CompareValues(data_[m], data_[b - 1]) == 1) {
            using std::swap;
            swap(data_[m], data_[b - 1]);

            if (NSortHelpers::CompareValues(data_[a], data_[m]) == 1)
                return;
        }

        using std::swap;
        swap(data_[a], data_[m]);
    }

    // BufferPartitionMergeSort.medianOfMedians (lite version; insSort's
    // customInsertSort is NSortHelpers::insertionSort)
    template<class T>
    void BufferPartitionMergeSortMedianOfMedians(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t s) {
        ptrdiff_t end = b, start = a, i, j;
        bool ad = true;

        while (end - start > 1) {
            j = start;
            NSortHelpers::MarkArray(2, data_, j);
            for (i = start; i + 2 * s <= end; i += s) {
                NSortHelpers::insertionSort(data_, i, i + s);
                {
                    using std::swap;
                    swap(data_[j], data_[i + s / 2]);
                }
                ++j;
                NSortHelpers::MarkArray(2, data_, j);
            }
            if (i < end) {
                NSortHelpers::insertionSort(data_, i, end);
                {
                    ptrdiff_t mIdx = i + (end - (ad ? 1 : 0) - i) / 2;
                    using std::swap;
                    swap(data_[j], data_[mIdx]);
                }
                ++j;
                NSortHelpers::MarkArray(2, data_, j);
                if ((end - i) % 2 == 0) ad = !ad;
            }
            end = j;
        }
    }

    // BufferPartitionMergeSort.partition
    template<class T>
    ptrdiff_t BufferPartitionMergeSortPartition(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t i = a, j = b;
        NSortHelpers::MarkArray(3, data_, a);

        while (true) {
            do {
                i++;
                NSortHelpers::MarkArray(1, data_, i);
            } while (i < j && NSortHelpers::CompareValues(data_[i], data_[a]) == 1);

            do {
                j--;
                NSortHelpers::MarkArray(2, data_, j);
            } while (j >= i && NSortHelpers::CompareValues(data_[j], data_[a]) == -1);

            if (i < j) {
                using std::swap;
                swap(data_[i], data_[j]);
            }
            else return j;
        }
    }

    // BufferPartitionMergeSort.quickSelect
    template<class T>
    ptrdiff_t BufferPartitionMergeSortQuickSelect(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t m) {
        bool badPartition = false, mom = false;
        ptrdiff_t m1 = (m + b + 1) / 2;

        while (true) {
            if (badPartition) {
                BufferPartitionMergeSortMedianOfMedians(data_, a, b, 5);
                mom = true;
            }
            else BufferPartitionMergeSortMedianOfThree(data_, a, b);

            ptrdiff_t p = BufferPartitionMergeSortPartition(data_, a, b);
            {
                using std::swap;
                swap(data_[a], data_[p]);
            }

            ptrdiff_t l = (std::max)(static_cast<ptrdiff_t>(1), p - a);
            ptrdiff_t r = (std::max)(static_cast<ptrdiff_t>(1), b - (p + 1));
            badPartition = !mom && (l / r >= 16 || r / l >= 16);

            if (p >= m && p < m1) return p;
            else if (p < m) a = p + 1;
            else            b = p;
        }
    }

    // BufferPartitionMergeSort.merge
    template<class T>
    void BufferPartitionMergeSortMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t i = a, j = m;

        while (i < m && j < b) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) <= 0) {
                using std::swap;
                swap(data_[p], data_[i]);
                ++p;
                ++i;
            }
            else {
                using std::swap;
                swap(data_[p], data_[j]);
                ++p;
                ++j;
            }
        }

        while (i < m) {
            using std::swap;
            swap(data_[p], data_[i]);
            ++p;
            ++i;
        }
        while (j < b) {
            using std::swap;
            swap(data_[p], data_[j]);
            ++p;
            ++j;
        }
    }

    // BufferPartitionMergeSort.mergeFW
    template<class T>
    ptrdiff_t BufferPartitionMergeSortMergeFW(std::vector<T>& data_, ptrdiff_t p, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = a, j = m;

        while (i < m && j < b) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) <= 0) {
                using std::swap;
                swap(data_[p], data_[i]);
                ++p;
                ++i;
            }
            else {
                using std::swap;
                swap(data_[p], data_[j]);
                ++p;
                ++j;
            }
        }

        if (i < m) return i;
        else       return j;
    }

    // BufferPartitionMergeSort.getMinLevel
    inline ptrdiff_t BufferPartitionMergeSortGetMinLevel(ptrdiff_t n) {
        while (n >= 32) n = (n + 3) / 4;
        return n;
    }

    // BufferPartitionMergeSort.mergeSort (binInsSort's customBinaryInsert is
    // NSortHelpers::binaryInsertSort)
    template<class T>
    void BufferPartitionMergeSortMergeSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t len = b - a;
        if (len < 2) return;

        ptrdiff_t i, pos, j = BufferPartitionMergeSortGetMinLevel(len);

        for (i = a; i + j <= b; i += j)
            NSortHelpers::binaryInsertSort(data_, i, i + j);
        NSortHelpers::binaryInsertSort(data_, i, b);

        while (j < len) {
            pos = p;
            for (i = a; i + 2 * j <= b; i += 2 * j, pos += 2 * j)
                BufferPartitionMergeSortMerge(data_, i, i + j, i + 2 * j, pos);
            if (i + j < b)
                BufferPartitionMergeSortMerge(data_, i, i + j, b, pos);
            else
                while (i < b) {
                    using std::swap;
                    swap(data_[i], data_[pos]);
                    ++i;
                    ++pos;
                }

            j *= 2;

            pos = a;
            for (i = p; i + 2 * j <= p + len; i += 2 * j, pos += 2 * j)
                BufferPartitionMergeSortMerge(data_, i, i + j, i + 2 * j, pos);
            if (i + j < p + len)
                BufferPartitionMergeSortMerge(data_, i, i + j, p + len, pos);
            else
                while (i < p + len) {
                    using std::swap;
                    swap(data_[i], data_[pos]);
                    ++i;
                    ++pos;
                }

            j *= 2;
        }
    }

    // BufferPartitionMergeSort.sort
    template<class T>
    void BufferPartitionMergeSortSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t minLvl = static_cast<ptrdiff_t>(std::sqrt(static_cast<double>(b - a)));

        ptrdiff_t m = (a + b + 1) / 2;
        BufferPartitionMergeSortMergeSort(data_, m, b, a);

        while (m - a > minLvl) {
            ptrdiff_t m1 = (a + m + 1) / 2;

            m1 = BufferPartitionMergeSortQuickSelect(data_, a, m, m1);
            BufferPartitionMergeSortMergeSort(data_, m1, m, a);

            ptrdiff_t bSize = m1 - a;
            ptrdiff_t m2 = (std::min)(m1 + bSize, b);
            m1 = BufferPartitionMergeSortMergeFW(data_, a, m1, m, m2);

            while (m1 < m) {
                BufferPartitionMergeSortShiftBW(data_, m1, m, m2);
                m1 = m2 - (m - m1);
                a = m1 - bSize;
                m = m2;

                if (m == b) break;

                m2 = (std::min)(m2 + bSize, b);
                m1 = BufferPartitionMergeSortMergeFW(data_, a, m1, m, m2);
            }
            m = m1;
            a = m1 - bSize;
        }
        NSortHelpers::binaryInsertSort(data_, a, m);
        BufferPartitionMergeSortInPlaceMerge(data_, a, m, b);
    }

    // port of BufferPartitionMergeSort.runSort
    template<class T = int>
    void BufferPartitionMergeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) return;

        BufferPartitionMergeSortSort(data_, 0, length);
    }

    // ==================== FluxSort + OptimizedRotateMergeSort + LazierestSort + LaziestSort + MedianMergeSort ====================
// Ports of ArrayV (Java, MIT) hybrid sorting classes: FluxSort,
// OptimizedRotateMergeSort, LazierestSort, LaziestSort, MedianMergeSort.


    // ---------------------------------------------------------------------
    // FluxSort (extends QuadSorting; fluxsort 1.1.3.3 by Igor van den Hoven)
    // ---------------------------------------------------------------------

    // QuadSorting.quadSortSwap. SortHelpers.h has the other QuadSorting pieces
    // (quadTailSwap / quadSwap / quadTailMerge / quadMerge / quadSort) but not
    // this one, so it is ported locally with the FluxSort prefix.
    template<class T>
    void FluxSortQuadSortSwap(std::vector<T>& data_, std::vector<T>& swap, ptrdiff_t start, ptrdiff_t length) {
        if (length < 16) {
            NSortHelpers::quadTailSwap(data_, start, length);
        }

        else if (length < 256) {
            if (NSortHelpers::quadSwap(data_, start, length) == 0)
                NSortHelpers::quadTailMerge(data_, swap, start, length, 16);
        }

        else {
            if (NSortHelpers::quadSwap(data_, start, length) == 0)
                NSortHelpers::quadMerge(data_, swap, start, length, 16);
        }
    }

    // FluxSort.medianOfThree (bit-trick median; compareIndices values are mapped
    // to 0/1 exactly like the Java (cmp+1)/2).
    template<class T>
    ptrdiff_t FluxSortMedianOfThree(std::vector<T>& data_, ptrdiff_t v0, ptrdiff_t v1, ptrdiff_t v2) {
        ptrdiff_t t[2];
        int val;

        val = (NSortHelpers::CompareValues(data_[v0], data_[v1]) + 1) / 2; t[0] = val; t[1] = val ^ 1;
        val = (NSortHelpers::CompareValues(data_[v0], data_[v2]) + 1) / 2; t[0] += val;

        if (t[0] == 1) return v0;

        val = (NSortHelpers::CompareValues(data_[v1], data_[v2]) + 1) / 2; t[1] += val;

        return t[1] == 1 ? v1 : v2;
    }

    // FluxSort.medianOfFive
    template<class T>
    ptrdiff_t FluxSortMedianOfFive(std::vector<T>& data_, ptrdiff_t v0, ptrdiff_t v1, ptrdiff_t v2, ptrdiff_t v3, ptrdiff_t v4) {
        ptrdiff_t t[4];
        int val;

        val = (NSortHelpers::CompareValues(data_[v0], data_[v1]) + 1) / 2; t[0] = val; t[1] = val ^ 1;
        val = (NSortHelpers::CompareValues(data_[v0], data_[v2]) + 1) / 2; t[0] += val; t[2] = val ^ 1;
        val = (NSortHelpers::CompareValues(data_[v0], data_[v3]) + 1) / 2; t[0] += val; t[3] = val ^ 1;
        val = (NSortHelpers::CompareValues(data_[v0], data_[v4]) + 1) / 2; t[0] += val;

        if (t[0] == 2) return v0;

        val = (NSortHelpers::CompareValues(data_[v1], data_[v2]) + 1) / 2; t[1] += val; t[2] += val ^ 1;
        val = (NSortHelpers::CompareValues(data_[v1], data_[v3]) + 1) / 2; t[1] += val; t[3] += val ^ 1;
        val = (NSortHelpers::CompareValues(data_[v1], data_[v4]) + 1) / 2; t[1] += val;

        if (t[1] == 2) return v1;

        val = (NSortHelpers::CompareValues(data_[v2], data_[v3]) + 1) / 2; t[2] += val; t[3] += val ^ 1;
        val = (NSortHelpers::CompareValues(data_[v2], data_[v4]) + 1) / 2; t[2] += val;

        if (t[2] == 2) return v2;

        val = (NSortHelpers::CompareValues(data_[v3], data_[v4]) + 1) / 2; t[3] += val;

        return t[3] == 2 ? v3 : v4;
    }

    // FluxSort.medianOfFifteen
    template<class T>
    ptrdiff_t FluxSortMedianOfFifteen(std::vector<T>& data_, ptrdiff_t ptx, ptrdiff_t nmemb) {
        ptrdiff_t v0, v1, v2, v3, v4, div = nmemb / 16;

        v0 = FluxSortMedianOfThree(data_, ptx + div * 2, ptx + div * 1, ptx + div * 3);
        v1 = FluxSortMedianOfThree(data_, ptx + div * 5, ptx + div * 4, ptx + div * 6);
        v2 = FluxSortMedianOfThree(data_, ptx + div * 8, ptx + div * 7, ptx + div * 9);
        v3 = FluxSortMedianOfThree(data_, ptx + div * 11, ptx + div * 10, ptx + div * 12);
        v4 = FluxSortMedianOfThree(data_, ptx + div * 14, ptx + div * 13, ptx + div * 15);

        return FluxSortMedianOfFive(data_, v2, v0, v1, v3, v4);
    }

    // FluxSort.medianOfNine
    template<class T>
    ptrdiff_t FluxSortMedianOfNine(std::vector<T>& data_, ptrdiff_t ptx, ptrdiff_t nmemb) {
        ptrdiff_t v0, v1, v2, div = nmemb / 16;

        v0 = FluxSortMedianOfThree(data_, ptx + div * 2, ptx + div * 1, ptx + div * 4);
        v1 = FluxSortMedianOfThree(data_, ptx + div * 8, ptx + div * 6, ptx + div * 10);
        v2 = FluxSortMedianOfThree(data_, ptx + div * 14, ptx + div * 12, ptx + div * 15);

        return FluxSortMedianOfThree(data_, v0, v1, v2);
    }

    // FluxSort.fluxAnalyze (Java's "pta, ++pta" argument pair is split so that the
    // comparison uses the old pta and the pre-increment is performed afterwards).
    template<class T>
    bool FluxSortFluxAnalyze(std::vector<T>& data_, ptrdiff_t nmemb) {
        ptrdiff_t cnt, balance = 0;
        ptrdiff_t pta;

        pta = 0;
        cnt = nmemb;

        while (--cnt > 0) {
            if (NSortHelpers::CompareValues(data_[pta], data_[pta + 1]) > 0) balance++;
            ++pta;
        }

        if (balance == 0) return false;

        if (balance == nmemb - 1) {
            NSortHelpers::reverseRange(data_, 0, nmemb);
            return false;
        }

        if (balance <= nmemb / 6 || balance >= nmemb / 6 * 5) {
            NSortHelpers::quadSort(data_, 0, nmemb);
            return false;
        }

        return true;
    }

    // FluxSort.fluxPartition. The Java dispatches on `main == array`, which C++
    // cannot test through references, so the caller passes mainIsArray instead;
    // `array` is always the data vector, `swap` always the shared buffer.
    // Recursion depth is O(log n): a side is recursed into only when it is at most
    // 16/17 of nmemb (aSize > sSize/16 and vice versa).
    template<class T>
    void FluxSortFluxPartition(std::vector<T>& array, std::vector<T>& swap, bool mainIsArray, ptrdiff_t start, ptrdiff_t nmemb) {
        const ptrdiff_t FLUX_OUT = 24;

        int val;
        ptrdiff_t aSize, sSize;
        ptrdiff_t pta, pts, ptx, pte;

        std::vector<T>& main = mainIsArray ? array : swap;

        ptx = mainIsArray ? start : 0;

        T piv = main[nmemb > 1024 ? FluxSortMedianOfFifteen(main, ptx, nmemb) : FluxSortMedianOfNine(main, ptx, nmemb)];

        pte = ptx + nmemb;

        pta = start;
        pts = 0;

        while (ptx < pte) {
            NSortHelpers::MarkArray(3, main, ptx);
            val = (NSortHelpers::CompareValues(main[ptx], piv) + 1) / 2;

            array[pta] = main[ptx];
            pta += val ^ 1;
            NSortHelpers::MarkArray(2, swap, pts);
            swap[pts] = main[ptx];
            pts += val;

            ptx++;
        }

        sSize = pts;
        aSize = nmemb - sSize;

        if (aSize <= sSize / 16 || sSize <= FLUX_OUT) {
            for (ptrdiff_t i = 0; i < sSize; ++i) array[pta + i] = swap[i];
            FluxSortQuadSortSwap(array, swap, pta, sSize);
        }
        else
            FluxSortFluxPartition(array, swap, false, pta, sSize);

        if (sSize <= aSize / 16 || aSize <= FLUX_OUT)
            FluxSortQuadSortSwap(array, swap, start, aSize);
        else
            FluxSortFluxPartition(array, swap, true, start, aSize);
    }

    // port of FluxSort.runSort (FluxSort.fluxsort)
    template<class T = int>
    void FluxSort(std::vector<T>& data_) {
        ptrdiff_t nmemb = static_cast<ptrdiff_t>(data_.size());
        if (nmemb < 2) return;

        if (nmemb < 32)
            NSortHelpers::quadSort(data_, 0, nmemb);

        else if (FluxSortFluxAnalyze(data_, nmemb)) {
            std::vector<T> swap(nmemb);

            FluxSortFluxPartition(data_, swap, true, 0, nmemb);
        }
    }

    // ---------------------------------------------------------------------
    // OptimizedRotateMergeSort
    // ---------------------------------------------------------------------

    // OptimizedRotateMergeSort.rotateInPlace
    template<class T>
    void OptimizedRotateMergeSortRotateInPlace(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t lenA, ptrdiff_t lenB) {
        if (lenA < 1 || lenB < 1) return;

        ptrdiff_t a = pos,
                  b = pos + lenA - 1,
                  c = pos + lenA,
                  d = pos + lenA + lenB - 1;

        while (a < b && c < d) {
            T tmp = data_[b];
            data_[b] = data_[a]; --b;
            data_[a] = data_[c]; ++a;
            data_[c] = data_[d]; ++c;
            data_[d] = tmp; --d;
        }
        while (a < b) {
            T tmp = data_[b];
            data_[b] = data_[a]; --b;
            data_[a] = data_[d]; ++a;
            data_[d] = tmp; --d;
        }
        while (c < d) {
            T tmp = data_[c];
            data_[c] = data_[d]; ++c;
            data_[d] = data_[a]; --d;
            data_[a] = tmp; ++a;
        }
        if (a < d) { // dont count reversals that dont do anything
            NSortHelpers::reverseRange(data_, a, d + 1);
        }
    }

    // OptimizedRotateMergeSort.rotate (uses the run's temp buffer; falls back to
    // rotateInPlace when the block that would be buffered does not fit)
    template<class T>
    void OptimizedRotateMergeSortRotate(std::vector<T>& data_, std::vector<T>& tmp, ptrdiff_t pos, ptrdiff_t left, ptrdiff_t right) {
        if (left < 1 || right < 1) return;

        ptrdiff_t pta = pos, ptb = pos + left, ptc = pos + right, ptd = ptb + right;

        if (left < right) {
            ptrdiff_t bridge = right - left;

            if (bridge < left) {
                ptrdiff_t loop = left;

                if (bridge > static_cast<ptrdiff_t>(tmp.size())) {
                    OptimizedRotateMergeSortRotateInPlace(data_, pos, left, right);
                    return;
                }

                for (ptrdiff_t i = 0; i < bridge; ++i) tmp[i] = data_[ptb + i];

                while (loop-- > 0) {
                    --ptc; --ptd; data_[ptc] = data_[ptd];
                    --ptb; data_[ptd] = data_[ptb];
                }
                for (ptrdiff_t i = 0; i < bridge; ++i) data_[pta + i] = tmp[i];
            }
            else {
                if (left > static_cast<ptrdiff_t>(tmp.size())) {
                    OptimizedRotateMergeSortRotateInPlace(data_, pos, left, right);
                    return;
                }

                for (ptrdiff_t i = 0; i < left; ++i) tmp[i] = data_[pta + i];
                for (ptrdiff_t i = 0; i < right; ++i) data_[pta + i] = data_[ptb + i];
                for (ptrdiff_t i = 0; i < left; ++i) data_[ptc + i] = tmp[i];
            }
        }
        else if (right < left) {
            ptrdiff_t bridge = left - right;

            if (bridge < right) {
                if (bridge > static_cast<ptrdiff_t>(tmp.size())) {
                    OptimizedRotateMergeSortRotateInPlace(data_, pos, left, right);
                    return;
                }

                ptrdiff_t loop = right;

                for (ptrdiff_t i = 0; i < bridge; ++i) tmp[i] = data_[ptc + i];

                while (loop-- > 0) {
                    data_[ptc] = data_[pta]; ++ptc;
                    data_[pta] = data_[ptb]; ++pta; ++ptb;
                }
                for (ptrdiff_t i = 0; i < bridge; ++i) data_[ptd - bridge + i] = tmp[i];
            }
            else {
                if (right > static_cast<ptrdiff_t>(tmp.size())) {
                    OptimizedRotateMergeSortRotateInPlace(data_, pos, left, right);
                    return;
                }

                for (ptrdiff_t i = 0; i < right; ++i) tmp[i] = data_[ptb + i];
                while (left-- > 0) {
                    --ptd; --ptb; data_[ptd] = data_[ptb];
                }
                for (ptrdiff_t i = 0; i < right; ++i) data_[pta + i] = tmp[i];
            }
        }
        else {
            while (left-- > 0) {
                using std::swap;
                swap(data_[pta], data_[ptb]);
                ++pta; ++ptb;
            }
        }
    }

    // OptimizedRotateMergeSort.mergeUp
    template<class T>
    void OptimizedRotateMergeSortMergeUp(std::vector<T>& data_, std::vector<T>& tmp, ptrdiff_t start, ptrdiff_t mid, ptrdiff_t end) {
        for (ptrdiff_t i = 0; i < mid - start; ++i) {
            NSortHelpers::MarkArray(1, data_, i + start);
            tmp[i] = data_[i + start];
        }

        ptrdiff_t bufferPointer = 0;
        ptrdiff_t left = start;
        ptrdiff_t right = mid;

        while (left < right && right < end) {
            NSortHelpers::MarkArray(2, data_, right);
            if (NSortHelpers::CompareValues(tmp[bufferPointer], data_[right]) <= 0) {
                data_[left] = tmp[bufferPointer];
                ++left; ++bufferPointer;
            }
            else {
                data_[left] = data_[right];
                ++left; ++right;
            }
        }

        while (left < right) {
            data_[left] = tmp[bufferPointer];
            ++left; ++bufferPointer;
        }
    }

    // OptimizedRotateMergeSort.mergeDown
    template<class T>
    void OptimizedRotateMergeSortMergeDown(std::vector<T>& data_, std::vector<T>& tmp, ptrdiff_t start, ptrdiff_t mid, ptrdiff_t end) {
        for (ptrdiff_t i = 0; i < end - mid; ++i) {
            NSortHelpers::MarkArray(1, data_, i + mid);
            tmp[i] = data_[i + mid];
        }

        ptrdiff_t bufferPointer = end - mid - 1;
        ptrdiff_t left = mid - 1;
        ptrdiff_t right = end - 1;

        while (right > left && left >= start) {
            NSortHelpers::MarkArray(2, data_, left);
            if (NSortHelpers::CompareValues(tmp[bufferPointer], data_[left]) >= 0) {
                data_[right] = tmp[bufferPointer];
                --right; --bufferPointer;
            }
            else {
                data_[right] = data_[left];
                --right; --left;
            }
        }

        while (right > left) {
            data_[right] = tmp[bufferPointer];
            --right; --bufferPointer;
        }
    }

    // OptimizedRotateMergeSort.monoboundLeft
    template<class T>
    ptrdiff_t OptimizedRotateMergeSortMonoboundLeft(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, T value) {
        ptrdiff_t top, mid;

        top = end - start;

        while (top > 1) {
            mid = top / 2;

            if (NSortHelpers::CompareValues(value, data_[end - mid]) <= 0) {
                end -= mid;
            }
            top -= mid;
        }

        if (NSortHelpers::CompareValues(value, data_[end - 1]) <= 0) {
            return end - 1;
        }
        return end;
    }

    // OptimizedRotateMergeSort.monoboundRight
    template<class T>
    ptrdiff_t OptimizedRotateMergeSortMonoboundRight(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, T value) {
        ptrdiff_t top, mid;

        top = end - start;

        while (top > 1) {
            mid = top / 2;

            if (NSortHelpers::CompareValues(data_[start + mid], value) <= 0) {
                start += mid;
            }
            top -= mid;
        }

        if (NSortHelpers::CompareValues(data_[start], value) <= 0) {
            return start + 1;
        }
        return start;
    }

    // OptimizedRotateMergeSort.leftExpSearch
    template<class T>
    ptrdiff_t OptimizedRotateMergeSortLeftExpSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, T val) {
        ptrdiff_t i = 1;
        while (a - 1 + i < b && NSortHelpers::CompareValues(val, data_[a - 1 + i]) >= 0) i *= 2;

        return OptimizedRotateMergeSortMonoboundRight(data_, a + i / 2, (std::min)(b, a - 1 + i), val);
    }

    // OptimizedRotateMergeSort.rightExpSearch
    template<class T>
    ptrdiff_t OptimizedRotateMergeSortRightExpSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, T val) {
        ptrdiff_t i = 1;
        while (b - i >= a && NSortHelpers::CompareValues(val, data_[b - i]) <= 0) i *= 2;

        return OptimizedRotateMergeSortMonoboundLeft(data_, (std::max)(a, b - i + 1), b - i / 2, val);
    }

    // OptimizedRotateMergeSort.merge (recursive; each recursive call halves the
    // longer run, so the recursion depth is O(log n))
    template<class T>
    void OptimizedRotateMergeSortMerge(std::vector<T>& data_, std::vector<T>& tmp, ptrdiff_t start, ptrdiff_t mid, ptrdiff_t end) {
        if (start >= mid) return;

        end = OptimizedRotateMergeSortRightExpSearch(data_, mid, end, data_[mid - 1]);
        if (end < mid) return;
        start = OptimizedRotateMergeSortLeftExpSearch(data_, start, mid, data_[mid]);
        if (NSortHelpers::CompareValues(data_[start], data_[end - 1]) > 0) {
            OptimizedRotateMergeSortRotate(data_, tmp, start, mid - start, end - mid);
            return;
        }

        ptrdiff_t llen = mid - start, rlen = end - mid;

        if (((llen < rlen) ? llen : rlen) > static_cast<ptrdiff_t>(tmp.size())) {
            ptrdiff_t m1, m2, m3;

            if (mid - start >= end - mid) {
                m1 = start + (mid - start) / 2;
                m2 = OptimizedRotateMergeSortMonoboundLeft(data_, mid, end, data_[m1]);
                m3 = m1 + (m2 - mid);
            }
            else {
                m2 = mid + (end - mid) / 2;
                m1 = OptimizedRotateMergeSortMonoboundRight(data_, start, mid, data_[m2]);
                m3 = m2 - (mid - m1);
                ++m2;
            }
            OptimizedRotateMergeSortRotate(data_, tmp, m1, mid - m1, m2 - mid);
            OptimizedRotateMergeSortMerge(data_, tmp, m3 + 1, m2, end);
            OptimizedRotateMergeSortMerge(data_, tmp, start, m1, m3);
        }
        else {
            if (end - mid < mid - start) {
                OptimizedRotateMergeSortMergeDown(data_, tmp, start, mid, end);
            }
            else {
                OptimizedRotateMergeSortMergeUp(data_, tmp, start, mid, end);
            }
        }
    }

    // OptimizedRotateMergeSort.insertionSort (insertion with a reversal pre-pass
    // and monobound-right placement; called with ranges of at least 2 elements)
    template<class T>
    void OptimizedRotateMergeSortInsertionSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t i = a + 1;

        if (NSortHelpers::CompareValues(data_[i - 1], data_[i]) == 1) {
            ++i;
            while (i < b && NSortHelpers::CompareValues(data_[i - 1], data_[i]) == 1) i++;
            NSortHelpers::reverseRange(data_, a, i);
        }
        else {
            ++i;
            while (i < b && NSortHelpers::CompareValues(data_[i - 1], data_[i]) <= 0) i++;
        }

        while (i < b) {
            ptrdiff_t dest = OptimizedRotateMergeSortMonoboundRight(data_, a, i, data_[i]);
            T saved = data_[i];
            for (ptrdiff_t k = i; k > dest; --k) data_[k] = data_[k - 1];
            data_[dest] = saved;
            i++;
        }
    }

    // port of OptimizedRotateMergeSort.runSort (the Java menu defaultAnswer for
    // the temp array length is 64; MIN_RUN is 32)
    template<class T = int>
    void OptimizedRotateMergeSort(std::vector<T>& data_) {
        const ptrdiff_t MIN_RUN = 32;

        ptrdiff_t currentLength = static_cast<ptrdiff_t>(data_.size());
        if (currentLength < 2) return;

        std::vector<T> tmp(64);

        ptrdiff_t i;
        for (i = 0; i + MIN_RUN < currentLength; i += MIN_RUN) {
            OptimizedRotateMergeSortInsertionSort(data_, i, i + MIN_RUN);
        }
        if (i + 1 < currentLength) {
            OptimizedRotateMergeSortInsertionSort(data_, i, currentLength);
        }

        ptrdiff_t gap, fullMerge;
        for (gap = MIN_RUN; gap < currentLength; gap = fullMerge) {
            fullMerge = gap * 2;
            for (i = 0; i + fullMerge < currentLength; i += fullMerge) {
                OptimizedRotateMergeSortMerge(data_, tmp, i, i + gap, i + fullMerge);
            }
            if (i + gap < currentLength) {
                OptimizedRotateMergeSortMerge(data_, tmp, i, i + gap, currentLength);
            }
        }
    }

    // ---------------------------------------------------------------------
    // LazierestSort
    // ---------------------------------------------------------------------

    // LazierestSort.ceilCbrt
    inline int LazierestSortCeilCbrt(int n) {
        int a = 0, b = (std::min)(1291, n);

        while (a < b) {
            int m = (a + b) / 2;

            if (m * m * m >= n) b = m;
            else                a = m + 1;
        }

        return a;
    }

    // LazierestSort.insertTo
    template<class T>
    void LazierestSortInsertTo(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        T temp = data_[a];
        while (a > b) {
            --a;
            data_[a + 1] = data_[a];
        }
        data_[b] = temp;
    }

    // LazierestSort.multiSwap
    template<class T>
    void LazierestSortMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
        for (ptrdiff_t i = 0; i < len; i++) {
            using std::swap;
            swap(data_[a + i], data_[b + i]);
        }
    }

    // LazierestSort.rotate
    template<class T>
    void LazierestSortRotate(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t l = m - a, r = b - m;

        while (l > 0 && r > 0) {
            if (r < l) {
                LazierestSortMultiSwap(data_, m - r, m, r);
                b -= r;
                m -= r;
                l -= r;
            }
            else {
                LazierestSortMultiSwap(data_, a, m, l);
                a += l;
                m += l;
                r -= l;
            }
        }
    }

    // LazierestSort.leftBinSearch
    template<class T>
    ptrdiff_t LazierestSortLeftBinSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, T val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            if (NSortHelpers::CompareValues(val, data_[m]) <= 0)
                b = m;
            else
                a = m + 1;
        }

        return a;
    }

    // LazierestSort.rightBinSearch
    template<class T>
    ptrdiff_t LazierestSortRightBinSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, T val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            if (NSortHelpers::CompareValues(val, data_[m]) < 0)
                b = m;
            else
                a = m + 1;
        }

        return a;
    }

    // LazierestSort.leftExpSearch
    template<class T>
    ptrdiff_t LazierestSortLeftExpSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, T val) {
        ptrdiff_t i = 1;
        while (a - 1 + i < b && NSortHelpers::CompareValues(val, data_[a - 1 + i]) > 0) i *= 2;

        return LazierestSortLeftBinSearch(data_, a + i / 2, (std::min)(b, a - 1 + i), val);
    }

    // LazierestSort.rightExpSearch
    template<class T>
    ptrdiff_t LazierestSortRightExpSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, T val) {
        ptrdiff_t i = 1;
        while (b - i >= a && NSortHelpers::CompareValues(val, data_[b - i]) < 0) i *= 2;

        return LazierestSortRightBinSearch(data_, (std::max)(a, b - i + 1), b - i / 2, val);
    }

    // LazierestSort.binaryInsertion
    template<class T>
    void LazierestSortBinaryInsertion(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        for (ptrdiff_t i = a + 1; i < b; i++) {
            LazierestSortInsertTo(data_, i, LazierestSortRightBinSearch(data_, a, i, data_[i]));
        }
    }

    // LazierestSort.inPlaceMergeFW. Java calls IndexedRotations.cycleReverse(array,
    // i, j, k), the three-reversal rotation that moves [j, k) in front of [i, j);
    // NSortHelpers::grailRotate(data_, i, j - i, k - j) performs the same rotation.
    template<class T>
    void LazierestSortInPlaceMergeFW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = a, j = m, k;

        while (i < j && j < b) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) == 1) {
                k = LazierestSortLeftExpSearch(data_, j + 1, b, data_[i]);
                NSortHelpers::grailRotate(data_, i, j - i, k - j);

                i += k - j;
                j = k;
            }
            else i++;
        }
    }

    // LazierestSort.inPlaceMergeBW
    template<class T>
    void LazierestSortInPlaceMergeBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = m - 1, j = b - 1, k;

        while (j > i && i >= a) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) > 0) {
                k = LazierestSortRightExpSearch(data_, a, i, data_[j]);
                LazierestSortRotate(data_, k, i + 1, j + 1);

                j -= (i + 1) - k;
                i = k - 1;
            }
            else j--;
        }
    }

    // LazierestSort.inPlaceMerge
    template<class T>
    void LazierestSortInPlaceMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        if (b - m < m - a) LazierestSortInPlaceMergeBW(data_, a, m, b);
        else               LazierestSortInPlaceMergeFW(data_, a, m, b);
    }

    // LazierestSort.fragmentedMerge
    template<class T>
    void LazierestSortFragmentedMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t s) {
        ptrdiff_t i = a + (m - a) % s;

        while (i < m) {
            ptrdiff_t j = LazierestSortLeftExpSearch(data_, m, b, data_[i]);
            NSortHelpers::grailRotate(data_, i, m - i, j - m);

            ptrdiff_t rLen = j - m;
            j = i;
            i += rLen;
            m += rLen;

            LazierestSortInPlaceMerge(data_, a, j, i);
            a = i;
            i += s;
        }
        LazierestSortInPlaceMerge(data_, (std::max)(a, i - s), i, b);
    }

    // LazierestSort.lazierestStableSort
    template<class T>
    void LazierestSortLazierestStableSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t len = b - a;
        ptrdiff_t s = static_cast<ptrdiff_t>(LazierestSortCeilCbrt(static_cast<int>(len)));
        ptrdiff_t s1 = s * s;

        for (ptrdiff_t i = len % s; i <= b; i += s)
            LazierestSortBinaryInsertion(data_, (std::max)(a, i - s), i);

        for (ptrdiff_t i = b - s, j = b; i > a; i -= s) {
            if (j - i == s1) {
                j -= s1;
                i -= s;
            }
            LazierestSortInPlaceMergeFW(data_, (std::max)(a, i - s), i, j);
        }

        for (ptrdiff_t i = b - s1; i > a; i -= s1)
            LazierestSortFragmentedMerge(data_, (std::max)(a, i - s1), i, b, s);
    }

    // port of LazierestSort.runSort
    template<class T = int>
    void LazierestSort(std::vector<T>& data_) {
        ptrdiff_t currentLength = static_cast<ptrdiff_t>(data_.size());
        if (currentLength < 2) return;

        if (currentLength <= 16) LazierestSortBinaryInsertion(data_, 0, currentLength);
        else                     LazierestSortLazierestStableSort(data_, 0, currentLength);
    }

    // ---------------------------------------------------------------------
    // LaziestSort
    // ---------------------------------------------------------------------

    // LaziestSort.insertTo
    template<class T>
    void LaziestSortInsertTo(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        T temp = data_[a];
        while (a > b) {
            --a;
            data_[a + 1] = data_[a];
        }
        data_[b] = temp;
    }

    // LaziestSort.multiSwap
    template<class T>
    void LaziestSortMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
        for (ptrdiff_t i = 0; i < len; i++) {
            using std::swap;
            swap(data_[a + i], data_[b + i]);
        }
    }

    // LaziestSort.rotate
    template<class T>
    void LaziestSortRotate(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t l = m - a, r = b - m;

        while (l > 0 && r > 0) {
            if (r < l) {
                LaziestSortMultiSwap(data_, m - r, m, r);
                b -= r;
                m -= r;
                l -= r;
            }
            else {
                LaziestSortMultiSwap(data_, a, m, l);
                a += l;
                m += l;
                r -= l;
            }
        }
    }

    // LaziestSort.leftBinSearch
    template<class T>
    ptrdiff_t LaziestSortLeftBinSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, T val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            if (NSortHelpers::CompareValues(val, data_[m]) <= 0)
                b = m;
            else
                a = m + 1;
        }

        return a;
    }

    // LaziestSort.rightBinSearch
    template<class T>
    ptrdiff_t LaziestSortRightBinSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, T val) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            if (NSortHelpers::CompareValues(val, data_[m]) < 0)
                b = m;
            else
                a = m + 1;
        }

        return a;
    }

    // LaziestSort.leftExpSearch
    template<class T>
    ptrdiff_t LaziestSortLeftExpSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, T val) {
        ptrdiff_t i = 1;
        while (a - 1 + i < b && NSortHelpers::CompareValues(val, data_[a - 1 + i]) > 0) i *= 2;

        return LaziestSortLeftBinSearch(data_, a + i / 2, (std::min)(b, a - 1 + i), val);
    }

    // LaziestSort.binaryInsertion
    template<class T>
    void LaziestSortBinaryInsertion(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        for (ptrdiff_t i = a + 1; i < b; i++) {
            LaziestSortInsertTo(data_, i, LaziestSortRightBinSearch(data_, a, i, data_[i]));
        }
    }

    // LaziestSort.inPlaceMerge
    template<class T>
    void LaziestSortInPlaceMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = a, j = m, k;

        while (i < j && j < b) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) == 1) {
                k = LaziestSortLeftExpSearch(data_, j + 1, b, data_[i]);
                LaziestSortRotate(data_, i, j, k);

                i += k - j;
                j = k;
            }
            else i++;
        }
    }

    // LaziestSort.laziestStableSort
    template<class T>
    void LaziestSortLaziestStableSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        ptrdiff_t len = end - start;
        if (len <= 16) {
            LaziestSortBinaryInsertion(data_, start, end);
            return;
        }

        ptrdiff_t i, blockLen = (std::max)(static_cast<ptrdiff_t>(16),
                                           static_cast<ptrdiff_t>(std::sqrt(static_cast<double>(len))));
        for (i = start; i + 2 * blockLen < end; i += blockLen) {
            LaziestSortBinaryInsertion(data_, i, i + blockLen);
        }
        LaziestSortBinaryInsertion(data_, i, end);

        while (i - blockLen >= start) {
            LaziestSortInPlaceMerge(data_, i - blockLen, i, end);
            i -= blockLen;
        }
    }

    // port of LaziestSort.runSort
    template<class T = int>
    void LaziestSort(std::vector<T>& data_) {
        ptrdiff_t currentLength = static_cast<ptrdiff_t>(data_.size());
        if (currentLength < 2) return;

        LaziestSortLaziestStableSort(data_, 0, currentLength);
    }

    // ---------------------------------------------------------------------
    // MedianMergeSort
    // ---------------------------------------------------------------------

    // MedianMergeSort.medianOfThree
    template<class T>
    void MedianMergeSortMedianOfThree(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t m = a + (b - 1 - a) / 2;
        using std::swap;

        if (NSortHelpers::CompareValues(data_[a], data_[m]) == 1)
            swap(data_[a], data_[m]);

        if (NSortHelpers::CompareValues(data_[m], data_[b - 1]) == 1) {
            swap(data_[m], data_[b - 1]);

            if (NSortHelpers::CompareValues(data_[a], data_[m]) == 1)
                return;
        }

        swap(data_[a], data_[m]);
    }

    // MedianMergeSort.medianOfMedians (lite version; the InsertionSort instance
    // maps to NSortHelpers::insertionSort, i.e. InsertionSorting.insertionSort)
    template<class T>
    void MedianMergeSortMedianOfMedians(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t s) {
        ptrdiff_t end = b, start = a, i, j;
        bool ad = true;
        using std::swap;

        while (end - start > 1) {
            j = start;
            NSortHelpers::MarkArray(2, data_, j);
            for (i = start; i + 2 * s <= end; i += s) {
                NSortHelpers::insertionSort(data_, i, i + s);
                swap(data_[j], data_[i + s / 2]);
                ++j;
                NSortHelpers::MarkArray(2, data_, j);
            }
            if (i < end) {
                NSortHelpers::insertionSort(data_, i, end);
                swap(data_[j], data_[i + (end - (ad ? 1 : 0) - i) / 2]);
                ++j;
                NSortHelpers::MarkArray(2, data_, j);
                if ((end - i) % 2 == 0) ad = !ad;
            }
            end = j;
        }
    }

    // MedianMergeSort.partition
    template<class T>
    ptrdiff_t MedianMergeSortPartition(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t i = a - 1;
        ptrdiff_t j = b;
        NSortHelpers::MarkArray(3, data_, p);
        using std::swap;

        while (true) {
            do {
                i++;
                NSortHelpers::MarkArray(1, data_, i);
            } while (i < j && NSortHelpers::CompareValues(data_[i], data_[p]) == -1);

            do {
                j--;
                NSortHelpers::MarkArray(2, data_, j);
            } while (j >= i && NSortHelpers::CompareValues(data_[j], data_[p]) == 1);

            if (i < j) swap(data_[i], data_[j]);
            else       return j;
        }
    }

    // MedianMergeSort.merge (ping-pong merge, merging into the other half)
    template<class T>
    void MedianMergeSortMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t i = a, j = m;
        using std::swap;

        while (i < m && j < b) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) <= 0) {
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

    // MedianMergeSort.getMinLevel
    inline ptrdiff_t MedianMergeSortGetMinLevel(ptrdiff_t n) {
        while (n >= 32) n = (n + 3) / 4;
        return n;
    }

    // MedianMergeSort.mergeSort (the BinaryInsertionSort instance maps to
    // NSortHelpers::binaryInsertSort, i.e. BinaryInsertionSorting.binaryInsertSort)
    template<class T>
    void MedianMergeSortMergeSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t p) {
        ptrdiff_t length = b - a;
        if (length < 2) return;

        ptrdiff_t i, pos, j = MedianMergeSortGetMinLevel(length);
        using std::swap;

        for (i = a; i + j <= b; i += j)
            NSortHelpers::binaryInsertSort(data_, i, i + j);
        NSortHelpers::binaryInsertSort(data_, i, b);

        while (j < length) {
            pos = p;
            for (i = a; i + 2 * j <= b; i += 2 * j, pos += 2 * j)
                MedianMergeSortMerge(data_, i, i + j, i + 2 * j, pos);
            if (i + j < b)
                MedianMergeSortMerge(data_, i, i + j, b, pos);
            else
                while (i < b) {
                    swap(data_[i], data_[pos]);
                    ++i; ++pos;
                }

            j *= 2;

            pos = a;
            for (i = p; i + 2 * j <= p + length; i += 2 * j, pos += 2 * j)
                MedianMergeSortMerge(data_, i, i + j, i + 2 * j, pos);
            if (i + j < p + length)
                MedianMergeSortMerge(data_, i, i + j, p + length, pos);
            else
                while (i < p + length) {
                    swap(data_[i], data_[pos]);
                    ++i; ++pos;
                }

            j *= 2;
        }
    }

    // MedianMergeSort.medianMergeSort
    template<class T>
    void MedianMergeSortMedianMergeSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t start = a, end = b;
        bool badPartition = false, mom = false;
        using std::swap;

        while (end - start > 16) {
            if (badPartition) {
                MedianMergeSortMedianOfMedians(data_, start, end, 5);
                mom = true;
            }
            else MedianMergeSortMedianOfThree(data_, start, end);

            ptrdiff_t p = MedianMergeSortPartition(data_, start + 1, end, start);
            swap(data_[start], data_[p]);

            ptrdiff_t left  = p - start;
            ptrdiff_t right = end - (p + 1);
            badPartition = !mom && ((left == 0 || right == 0) || (left / right >= 16 || right / left >= 16));

            if (left <= right) {
                MedianMergeSortMergeSort(data_, start, p, p + 1);
                start = p + 1;
            }
            else {
                MedianMergeSortMergeSort(data_, p + 1, end, 2 * p + 1 - end);
                end = p;
            }
        }
        NSortHelpers::binaryInsertSort(data_, start, end);
    }

    // port of MedianMergeSort.runSort
    template<class T = int>
    void MedianMergeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) return;

        MedianMergeSortMedianMergeSort(data_, 0, length);
    }

    // ==================== IntroSort family + PDQ + stackless quicksorts ====================
// Ports of ArrayV (Java, MIT) hybrid sorting classes:
//   IntroSort, IntroCircleSortRecursive, IntroCircleSortIterative,
//   PDQBranchedSort, PDQBranchlessSort, OptimizedDualPivotQuickSort,
//   StacklessDualPivotQuickSort, StacklessHybridQuickSort
// https://github.com/Gaming32/ArrayV
//
// ASCII ONLY: do not put non-ASCII characters in this file. MSVC parses sources in the
// system code page (GBK); use \uXXXX escapes if a wide string is ever needed.
//
// This header is #included from Sort.h after SortHelpers.h; it opens the shared namespace
// and closes it again at the end of the file.


    // ==========================================================================
    // IntroSort
    // ==========================================================================

    // IntroSort.floorLogBaseTwo (Java computes (int) Math.floor(Math.log(a) / Math.log(2))).
    inline ptrdiff_t IntroSortFloorLogBaseTwo(ptrdiff_t a) {
        return static_cast<ptrdiff_t>(std::floor(std::log(static_cast<double>(a)) / std::log(2.0)));
    }

    // IntroSort.gccmedianof3 (unused in the Java class as well; kept for faithfulness).
    template<class T>
    T IntroSortGccMedianOf3(std::vector<T>& data_, ptrdiff_t& middle, ptrdiff_t left, ptrdiff_t mid, ptrdiff_t right) {
        using std::swap;

        if (data_[left] < data_[mid]) {
            if (data_[mid] < data_[right]) {
                swap(data_[left], data_[mid]);
            }
            else if (data_[left] < data_[right]) {
                swap(data_[left], data_[right]);
            }
        }
        else if (data_[left] < data_[right]) {
            middle = left;
            NSortHelpers::MarkArray(3, data_, left);
            return data_[left];
        }
        else if (data_[mid] < data_[right]) {
            swap(data_[left], data_[right]);
        }
        else {
            swap(data_[left], data_[mid]);
        }
        middle = left;
        NSortHelpers::MarkArray(3, data_, left);
        return data_[left];
    }

    // IntroSort.medianof3 (this.middle is the IntroSort field, updated here).
    template<class T>
    T IntroSortMedianOf3(std::vector<T>& data_, ptrdiff_t& middle, ptrdiff_t left, ptrdiff_t mid, ptrdiff_t right) {
        using std::swap;

        if (data_[right] < data_[left]) {
            swap(data_[left], data_[right]);
        }
        if (data_[mid] < data_[left]) {
            swap(data_[mid], data_[left]);
        }
        if (data_[right] < data_[mid]) {
            swap(data_[right], data_[mid]);
        }
        middle = mid;
        NSortHelpers::MarkArray(3, data_, mid);
        return data_[mid];
    }

    // IntroSort.partition (x is the pivot value selected by medianof3).
    template<class T>
    ptrdiff_t IntroSortPartition(std::vector<T>& data_, ptrdiff_t& middle, ptrdiff_t lo, ptrdiff_t hi, const T& x) {
        using std::swap;

        ptrdiff_t i = lo, j = hi;
        while (true) {
            while (data_[i] < x) {
                NSortHelpers::MarkArray(1, data_, i);
                ++i;
            }

            --j;

            while (x < data_[j]) {
                NSortHelpers::MarkArray(2, data_, j);
                --j;
            }

            if (!(i < j)) {
                NSortHelpers::MarkArray(1, data_, i);
                return i;
            }

            // Follow the pivot and highlight it.
            if (i == middle) {
                NSortHelpers::MarkArray(3, data_, j);
            }
            if (j == middle) {
                NSortHelpers::MarkArray(3, data_, i);
            }

            swap(data_[i], data_[j]);
            ++i;
        }
    }

    // IntroSort.introsortLoop. The recursion is on the right part only and the
    // depthLimit is consumed by every level, so the depth is bounded by
    // 2 * floorLog2(length): recursion is kept.
    template<class T>
    void IntroSortLoop(std::vector<T>& data_, ptrdiff_t& middle, ptrdiff_t lo, ptrdiff_t hi, ptrdiff_t depthLimit) {
        while (hi - lo > 16) { // sizeThreshold
            if (depthLimit == 0) {
                NSortHelpers::heapSort(data_, lo, hi, true);
                return;
            }
            depthLimit--;
            T x = IntroSortMedianOf3(data_, middle, lo, lo + ((hi - lo) / 2), hi - 1);
            ptrdiff_t p = IntroSortPartition(data_, middle, lo, hi, x);
            IntroSortLoop(data_, middle, p, hi, depthLimit);
            hi = p;
        }
        return;
    }

    // IntroSort.runSort.
    template<class T = int>
    void IntroSort(std::vector<T>& data_) {
        const ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        ptrdiff_t middle = 0;

        IntroSortLoop(data_, middle, 0, length, 2 * IntroSortFloorLogBaseTwo(length));

        // InsertionSort.customInsertSort(array, 0, length, 0.5, false).
        NSortHelpers::insertionSort(data_, 0, length);
    }

    // ==========================================================================
    // IntroCircleSortRecursive
    // ==========================================================================

    // IntroCircleSortRecursive.runSort.
    template<class T = int>
    void IntroCircleSortRecursive(std::vector<T>& data_) {
        const ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        // this.end = length;
        ptrdiff_t threshold = 0, n = 1;
        for (; n < length; n *= 2, ++threshold) { }

        threshold /= 2;
        ptrdiff_t iterations = 0;

        ptrdiff_t numSwaps = 0;
        do {
            iterations++;

            if (iterations >= threshold) {
                // BinaryInsertionSort.customBinaryInsert(array, 0, length, 0.1).
                NSortHelpers::binaryInsertSort(data_, 0, length);
                break;
            }

            // this.circleSortRoutine(array, 0, n - 1, 0, 1): the 4th argument is the
            // initial swap count, the 5th is the sleep (dropped).
            numSwaps = NSortHelpers::circleSortRoutine(data_, 0, n - 1, length, 0);
        } while (numSwaps != 0);
    }

    // ==========================================================================
    // IntroCircleSortIterative
    // ==========================================================================

    // IterativeCircleSorting.circleSortRoutine (ported locally: the shared
    // NSortHelpers::circleSortIterative is void, but the Java driver needs the
    // returned swap count to decide whether to keep iterating).
    template<class T>
    ptrdiff_t IntroCircleSortIterativeRoutine(std::vector<T>& data_, ptrdiff_t length, ptrdiff_t end) {
        ptrdiff_t swapCount = 0;
        for (ptrdiff_t gap = length / 2; gap > 0; gap /= 2) {
            for (ptrdiff_t start = 0; start + gap < end; start += 2 * gap) {
                ptrdiff_t high = start + 2 * gap - 1;
                ptrdiff_t low = start;

                while (low < high) {
                    if (high < end && data_[low] > data_[high]) {
                        using std::swap;
                        swap(data_[low], data_[high]);
                        ++swapCount;
                    }

                    ++low;
                    --high;
                }
            }
        }
        return swapCount;
    }

    // IntroCircleSortIterative.runSort.
    template<class T = int>
    void IntroCircleSortIterative(std::vector<T>& data_) {
        const ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        // this.end = length;
        ptrdiff_t threshold = 0, n = 1;
        for (; n < length; n *= 2, ++threshold) { }

        threshold /= 2;
        ptrdiff_t iterations = 0;

        ptrdiff_t numSwaps = 0;
        do {
            iterations++;

            if (iterations >= threshold) {
                // BinaryInsertionSort.customBinaryInsert(array, 0, length, 0.1).
                NSortHelpers::binaryInsertSort(data_, 0, length);
                break;
            }

            // this.circleSortRoutine(array, n, 1): n is the working length, this.end is length.
            numSwaps = IntroCircleSortIterativeRoutine(data_, n, length);
        } while (numSwaps != 0);
    }

    // ==========================================================================
    // PDQBranchedSort / PDQBranchlessSort
    // ==========================================================================

    // PDQBranchedSort.runSort (MaxHeapSort is supplied by pdqLoop's heap fallback).
    template<class T = int>
    void PDQBranchedSort(std::vector<T>& data_) {
        const ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        NSortHelpers::pdqSort(data_, 0, length, false);
    }

    // PDQBranchlessSort.runSort (visualizeAux/deleteAux are visualization only).
    template<class T = int>
    void PDQBranchlessSort(std::vector<T>& data_) {
        const ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        NSortHelpers::pdqSort(data_, 0, length, true);
    }

    // ==========================================================================
    // OptimizedDualPivotQuickSort
    // ==========================================================================

    // One pending dualPivot(array, left, right, divisor) call.
    struct OptimizedDualPivotQuickSortRange {
        ptrdiff_t left;
        ptrdiff_t right;
        ptrdiff_t divisor;
    };

    // OptimizedDualPivotQuickSort.dualPivot. The Java method recurses on both halves
    // AND on the middle range, which is O(n) worst-case depth, so the recursion is
    // converted to an explicit stack. Sub-ranges are pushed in reverse Java call
    // order (left, right, middle) so they are processed in the original order.
    template<class T>
    void OptimizedDualPivotQuickSortDualPivot(std::vector<T>& data_, ptrdiff_t left0, ptrdiff_t right0, ptrdiff_t divisor0) {
        using std::swap;

        std::vector<OptimizedDualPivotQuickSortRange> stack;
        stack.push_back({ left0, right0, divisor0 });

        while (!stack.empty()) {
            OptimizedDualPivotQuickSortRange range = stack.back();
            stack.pop_back();

            ptrdiff_t left = range.left;
            ptrdiff_t right = range.right;
            ptrdiff_t divisor = range.divisor;

            ptrdiff_t length = right - left;

            // insertion sort for tiny array
            if (length < 27) {
                // Highlights.clearMark(2) is dropped.
                // InsertionSort.customInsertSort(array, left, right + 1, 0.333, false).
                NSortHelpers::insertionSort(data_, left, right + 1);
                continue;
            }

            ptrdiff_t third = length / divisor;

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

            NSortHelpers::MarkArray(2, data_, less);
            NSortHelpers::MarkArray(3, data_, great);

            // sorting
            for (ptrdiff_t k = less; k <= great; ++k) {
                if (data_[k] < pivot1) {
                    swap(data_[k], data_[less]);
                    ++less;
                    NSortHelpers::MarkArray(2, data_, less);
                }
                else if (data_[k] > pivot2) {
                    while (k < great && data_[great] > pivot2) {
                        --great;
                        NSortHelpers::MarkArray(3, data_, great);
                    }
                    swap(data_[k], data_[great]);
                    --great;
                    NSortHelpers::MarkArray(3, data_, great);

                    if (data_[k] < pivot1) {
                        swap(data_[k], data_[less]);
                        ++less;
                        NSortHelpers::MarkArray(2, data_, less);
                    }
                }
            }
            // Highlights.clearAllMarks() is dropped.

            // swaps
            ptrdiff_t dist = great - less;

            if (dist < 13) {
                divisor++;
            }
            swap(data_[less - 1], data_[left]);
            swap(data_[great + 1], data_[right]);

            // subarrays: pushed in reverse of the Java call order
            // (left half, right half, middle range), so the stack processes
            // left, then right, then the middle range exactly like the Java
            // recursion.
            if (pivot1 < pivot2) {
                stack.push_back({ less, great, divisor });
            }
            stack.push_back({ great + 2, right, divisor });
            stack.push_back({ left, less - 2, divisor });

            NSortHelpers::MarkArray(2, data_, less);
            NSortHelpers::MarkArray(3, data_, great);

            // equal elements. Java runs this pass after the left/right recursions
            // and before the middle one; the pass only touches [less, great] (the
            // middle range) while the pushed ranges are [left, less-2] and
            // [great+2, right] and the pivots are local copies, so running it at
            // this point gives an identical result.
            if (dist > length - 13 && pivot1 != pivot2) {
                for (ptrdiff_t k = less; k <= great; ++k) {
                    if (data_[k] == pivot1) {
                        swap(data_[k], data_[less]);
                        ++less;
                        NSortHelpers::MarkArray(2, data_, less);
                    }
                    else if (data_[k] == pivot2) {
                        swap(data_[k], data_[great]);
                        --great;
                        NSortHelpers::MarkArray(3, data_, great);

                        if (data_[k] == pivot1) {
                            swap(data_[k], data_[less]);
                            ++less;
                            NSortHelpers::MarkArray(2, data_, less);
                        }
                    }
                }
            }
            // Highlights.clearAllMarks() is dropped.
        }
    }

    // OptimizedDualPivotQuickSort.runSort.
    template<class T = int>
    void OptimizedDualPivotQuickSort(std::vector<T>& data_) {
        const ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        OptimizedDualPivotQuickSortDualPivot(data_, 0, length - 1, 3);
    }

    // ==========================================================================
    // StacklessDualPivotQuickSort
    // ==========================================================================

    // StacklessDualPivotQuickSort.partition (p is the index of the stored maximum).
    template<class T>
    ptrdiff_t StacklessDualPivotQuickSortPartition(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t p) {
        using std::swap;

        ptrdiff_t m1 = (a + a + b) / 3, m2 = (a + b + b) / 3;

        if (data_[m1] > data_[m2]) {
            swap(data_[m1], data_[a]);
            --b;
            swap(data_[m2], data_[b]);
        }
        else {
            swap(data_[m2], data_[a]);
            --b;
            swap(data_[m1], data_[b]);
        }

        ptrdiff_t i = a, j = b;

        for (ptrdiff_t k = i + 1; k < j; ++k) {
            if (data_[k] < data_[b]) {
                ++i;
                swap(data_[k], data_[i]);
            }
            else if (data_[k] >= data_[a]) {
                do {
                    --j;
                    NSortHelpers::MarkArray(3, data_, j);
                } while (j > k && data_[j] >= data_[a]);

                swap(data_[k], data_[j]);
                // Highlights.clearMark(3) is dropped.

                if (data_[k] < data_[b]) {
                    ++i;
                    swap(data_[k], data_[i]);
                }
            }
        }

        swap(data_[a], data_[i]);
        T t = data_[b];
        data_[b] = data_[j];
        data_[j] = data_[p];
        data_[p] = t;

        return i;
    }

    // StacklessDualPivotQuickSort.leftBinSearch.
    template<class T>
    ptrdiff_t StacklessDualPivotQuickSortLeftBinSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t p) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            if (data_[p] <= data_[m]) {
                b = m;
            }
            else {
                a = m + 1;
            }
        }

        return a;
    }

    // StacklessDualPivotQuickSort.quickSort (already an iterative do-while driver).
    template<class T>
    void StacklessDualPivotQuickSortQuickSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        using std::swap;

        T max = data_[a];

        for (ptrdiff_t i = a + 1; i < b; ++i) {
            NSortHelpers::MarkArray(1, data_, i);

            if (data_[i] > max) max = data_[i];
        }
        for (ptrdiff_t i = b - 1; i >= 0; --i) {
            NSortHelpers::MarkArray(1, data_, i);

            if (data_[i] == max) {
                --b;
                swap(data_[i], data_[b]);
            }
        }

        ptrdiff_t b1 = b;
        bool med = true; // flag to improve pivot selection in the case of many similar elements

        do {
            while (b1 - a > 24) {
                if (!med) {
                    swap(data_[a], data_[(a + a + b1) / 3]);
                }

                b1 = StacklessDualPivotQuickSortPartition(data_, a, b1, b);
            }
            // BinaryInsertionSort.customBinaryInsert(array, a, b1, 0.25).
            NSortHelpers::binaryInsertSort(data_, a, b1);

            a = b1 + 1;
            if (a >= b) {
                if (a - 1 < b) {
                    swap(data_[a - 1], data_[b]);
                }
                return;
            }

            b1 = StacklessDualPivotQuickSortLeftBinSearch(data_, a, b, a - 1);
            swap(data_[a - 1], data_[b]);

            med = true;
            while (a < b1 && data_[a - 1] == data_[a]) {
                med = false;
                ++a;
            }
            if (a == b1) med = true;
        } while (true);
    }

    // StacklessDualPivotQuickSort.runSort.
    template<class T = int>
    void StacklessDualPivotQuickSort(std::vector<T>& data_) {
        const ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        StacklessDualPivotQuickSortQuickSort(data_, 0, length);
    }

    // ==========================================================================
    // StacklessHybridQuickSort
    // ==========================================================================

    // StacklessHybridQuickSort.medianOfThree.
    template<class T>
    void StacklessHybridQuickSortMedianOfThree(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        using std::swap;

        ptrdiff_t m = a + (b - 1 - a) / 2;

        if (data_[a] > data_[m]) {
            swap(data_[a], data_[m]);
        }

        if (data_[m] > data_[b - 1]) {
            swap(data_[m], data_[b - 1]);

            if (data_[a] > data_[m]) {
                return;
            }
        }

        swap(data_[a], data_[m]);
    }

    // StacklessHybridQuickSort.partition.
    template<class T>
    ptrdiff_t StacklessHybridQuickSortPartition(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        using std::swap;

        ptrdiff_t i = a, j = b;

        StacklessHybridQuickSortMedianOfThree(data_, a, b);
        NSortHelpers::MarkArray(3, data_, a);

        do {
            do {
                ++i;
                NSortHelpers::MarkArray(1, data_, i);
            } while (i < j && data_[i] < data_[a]);

            do {
                --j;
                NSortHelpers::MarkArray(2, data_, j);
            } while (j >= i && data_[j] >= data_[a]);

            if (i < j) {
                swap(data_[i], data_[j]);
            }
            else {
                swap(data_[a], data_[j]);
                // Highlights.clearMark(3) is dropped.
                return j;
            }
        } while (true);
    }

    // StacklessHybridQuickSort.leftBinSearch.
    template<class T>
    ptrdiff_t StacklessHybridQuickSortLeftBinSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t p) {
        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;

            if (data_[p] <= data_[m]) {
                b = m;
            }
            else {
                a = m + 1;
            }
        }

        return a;
    }

    // StacklessHybridQuickSort.quickSort (already an iterative do-while driver).
    template<class T>
    void StacklessHybridQuickSortQuickSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        using std::swap;

        T max = data_[a];

        for (ptrdiff_t i = a + 1; i < b; ++i) {
            NSortHelpers::MarkArray(1, data_, i);

            if (data_[i] > max) max = data_[i];
        }
        for (ptrdiff_t i = b - 1; i >= 0; --i) {
            NSortHelpers::MarkArray(1, data_, i);

            if (data_[i] == max) {
                --b;
                swap(data_[i], data_[b]);
            }
        }

        ptrdiff_t b1 = b;
        bool med = true; // flag to improve pivot selection in the case of many similar elements

        do {
            while (b1 - a > 16) {
                if (med) {
                    StacklessHybridQuickSortMedianOfThree(data_, a, b1);
                }

                ptrdiff_t p = StacklessHybridQuickSortPartition(data_, a, b1);
                swap(data_[p], data_[b]);

                b1 = p;
            }
            // BinaryInsertionSort.customBinaryInsert(array, a, b1, 0.25).
            NSortHelpers::binaryInsertSort(data_, a, b1);

            a = b1 + 1;
            if (a >= b) {
                if (a - 1 < b) {
                    swap(data_[a - 1], data_[b]);
                }
                return;
            }

            b1 = StacklessHybridQuickSortLeftBinSearch(data_, a, b, a - 1);
            swap(data_[a - 1], data_[b]);

            med = true;
            while (a < b1 && data_[a - 1] == data_[a]) {
                med = false;
                ++a;
            }
            if (a == b1) med = true;
        } while (true);
    }

    // StacklessHybridQuickSort.runSort.
    template<class T = int>
    void StacklessHybridQuickSort(std::vector<T>& data_) {
        const ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) {
            return;
        }

        StacklessHybridQuickSortQuickSort(data_, 0, length);
    }

    // ==================== BinaryMergeSort, CocktailMergeSort, DropMergeSort, GrailSort, HybridCombSort, KotaSort, TimSort, UnstableGrailSort, WikiSort ====================
// Ports of ArrayV (Java, MIT) hybrid sorting classes: BinaryMergeSort,
// CocktailMergeSort, DropMergeSort, GrailSort, HybridCombSort, KotaSort,
// OptimizedBottomUpMergeSort, TimSort, UnstableGrailSort, WikiSort.


    // CocktailShakerSort.smartCocktailShaker (ported for CocktailMergeSort, which
    // builds the TimSort runs with it; Delays omitted, highlight marks kept).
    template<class T>
    void CocktailMergeSortSmartCocktailShaker(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        ptrdiff_t i = start;
        while (i < ((end / 2) + start)) {
            bool sorted = true;
            for (ptrdiff_t j = i; j < end + start - i - 1; j++) {
                if (data_[j] > data_[j + 1]) {
                    using std::swap;
                    swap(data_[j], data_[j + 1]);
                    sorted = false;
                }

               NSortHelpers::MarkArray(1, data_, j);
               NSortHelpers::MarkArray(2, data_, j + 1);
            }
            for (ptrdiff_t j = end + start - i - 1; j > i; j--) {
                if (data_[j] < data_[j - 1]) {
                    using std::swap;
                    swap(data_[j], data_[j - 1]);
                    sorted = false;
                }

               NSortHelpers::MarkArray(1, data_, j);
               NSortHelpers::MarkArray(2, data_, j - 1);
            }
            if (sorted) break;
            else i++;
        }
    }

    // port of CocktailMergeSort.runSort
    template<class T = int>
    void CocktailMergeSort(std::vector<T>& data_) {
        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());
        if (sortLength < 2) return;

        ptrdiff_t minRunLen = NSortHelpers::timMinRunLength(sortLength);

        if (sortLength == minRunLen) {
            // CocktailShakerSort.runSort(array, sortLength, ...) is the same
            // smartCocktailShaker over [0, sortLength) (only the sleep differs).
            CocktailMergeSortSmartCocktailShaker(data_, 0, sortLength);
        } else {
            ptrdiff_t i = 0;
            for (; i <= (sortLength - minRunLen); i += minRunLen) {
                CocktailMergeSortSmartCocktailShaker(data_, i, i + minRunLen);
            }
            if (i + minRunLen > sortLength) {
                CocktailMergeSortSmartCocktailShaker(data_, i, sortLength);
            }

            NSortHelpers::timSort(data_, 0, sortLength);
        }
    }

    // port of BinaryMergeSort.runSort (MergeSorting.mergeSort with binary = true)
    template<class T = int>
    void BinaryMergeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) return;

        NSortHelpers::mergeSort(data_, length, true);
    }

    // port of HybridCombSort.runSort (CombSorting.combSort, shrink = 1.3, hybrid)
    template<class T = int>
    void HybridCombSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) return;

        NSortHelpers::combSort(data_, length, 1.3, true);
    }

    // port of TimSort.runSort (TimSorting instance + static sort over [0, length)).
    template<class T = int>
    void TimSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) return;

        NSortHelpers::timSort(data_, 0, length);
    }

    // port of WikiSort.runSort (WikiSorting.sort; the Java menu question asks for
    // the external buffer size with defaultAnswer = 0, so cache = 0 -> in-place)
    template<class T = int>
    void WikiSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) return;

        NSortHelpers::wikiSort(data_, 0, length, 0);
    }

    // port of KotaSort.runSort (the Delays.getSleepRatio() == 55.1 branch is a
    // hidden tester path that sorts nothing, so the normal kotaSort is used)
    template<class T = int>
    void KotaSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) return;

        NSortHelpers::kotaSort(data_, 0, length);
    }

    // port of UnstableGrailSort.runSort (UnstableGrailSorting.grailCommonSort)
    template<class T = int>
    void UnstableGrailSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) return;

        NSortHelpers::unstableGrailCommonSort(data_, 0, length);
    }

    // port of GrailSort.runSort (GrailSorting.grailCommonSort). The Java menu
    // question ("external buffer type", defaultAnswer = 0) selects the default
    // in-place branch; the static (1) and dynamic (2) buffer branches need the
    // interactive menu, so the default branch is ported here.
    template<class T = int>
    void GrailSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) return;

        NSortHelpers::grailCommonSort(data_, 0, length, static_cast<std::vector<T>*>(nullptr), 0, 0);
    }

    // port of DropMergeSort.runSort
    template<class T = int>
    void DropMergeSort(std::vector<T>& data_) {
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());
        if (length < 2) return;

        const ptrdiff_t RECENCY = 8;
        const ptrdiff_t EARLY_OUT_TEST_AT = 4;
        const double EARLY_OUT_DISORDER_FRACTION = 0.6;

        // Java's List<Integer> `dropped` holds element values.
        std::vector<T> dropped;

        ptrdiff_t num_dropped_in_a_row = 0;
        ptrdiff_t read = 0;
        ptrdiff_t write = 0;

        ptrdiff_t iteration = 0;
        ptrdiff_t early_out_stop = length / EARLY_OUT_TEST_AT;

        while (read < length) {
           NSortHelpers::MarkArray(2, data_, read);
            iteration += 1;
            if (iteration == early_out_stop
                && static_cast<double>(dropped.size()) > static_cast<double>(read) * EARLY_OUT_DISORDER_FRACTION) {
                // We have seen a lot of the elements and dropped a lot of them.
                // This doesn't look good. Abort.
                for (size_t i = 0; i < dropped.size(); i++) {
                    data_[write] = dropped[i];
                    ++write;
                }
                dropped.clear();
                NSortHelpers::pdqSort(data_, 0, length, false);
                return;
            }

            if (write == 0 || data_[read] >= data_[write - 1]) {
                // The element is in order - keep it:
                data_[write] = data_[read];
                ++write;
                ++read;
                num_dropped_in_a_row = 0;
            } else {
                if (num_dropped_in_a_row == 0 && write >= 2 && data_[read] >= data_[write - 2]) {
                    // Quick undo: drop previously accepted element, and overwrite with new one
                    dropped.push_back(data_[write - 1]);
                    data_[write - 1] = data_[read];
                    ++read;
                    continue;
                }

                if (num_dropped_in_a_row < RECENCY) {
                    dropped.push_back(data_[read]);
                    ++read;
                    num_dropped_in_a_row++;
                } else {
                    // We accepted something num_dropped_in_row elements back that made us
                    // drop all RECENCY subsequent items. Accepting that element was
                    // obviously a mistake - so let's undo it!

                    // Undo dropping the last num_dropped_in_row elements:
                    ptrdiff_t trunc_to_length = static_cast<ptrdiff_t>(dropped.size()) - num_dropped_in_a_row;
                    dropped.resize(static_cast<size_t>(trunc_to_length));
                    read -= num_dropped_in_a_row;

                    ptrdiff_t num_backtracked = 1;
                    write--;

                    // Java keeps max_of_dropped as a raw int initialised to the INDEX
                    // `read` (a known quirk of this sort); raw int comparisons mirror it.
                    int max_of_dropped = static_cast<int>(read);
                    for (ptrdiff_t i = read + 1; i <= read + num_dropped_in_a_row; i++) {
                        if (static_cast<int>(data_[i]) > max_of_dropped) {
                            max_of_dropped = static_cast<int>(data_[i]);
                        }
                    }

                    while (write >= 1 && max_of_dropped < static_cast<int>(data_[write - 1])) {
                        --write;
                       NSortHelpers::MarkArray(1, data_, write);
                        num_backtracked++;
                    }

                    for (ptrdiff_t i = write; i < write + num_backtracked; i++) {
                       NSortHelpers::MarkArray(1, data_, i);
                        dropped.push_back(data_[i]);
                    }

                    num_dropped_in_a_row = 0;
                }
            }
        }

        for (size_t i = 0; i < dropped.size(); i++) {
            data_[write + static_cast<ptrdiff_t>(i)] = dropped[i];
        }

        NSortHelpers::pdqSort(data_, write, length, false);

        std::vector<T> buffer(dropped.size());

        for (size_t i = 0; i < dropped.size(); i++) {
            buffer[i] = data_[write + static_cast<ptrdiff_t>(i)];
        }

        ptrdiff_t i = static_cast<ptrdiff_t>(buffer.size()) - 1;
        ptrdiff_t j = write - 1;
        ptrdiff_t k = length - 1;

        while (i >= 0) {
            if (j < 0 || buffer[i] > data_[j]) {
                data_[k] = buffer[i];
                --k;
                --i;
            } else {
               NSortHelpers::MarkArray(2, data_, j);
                data_[k] = data_[j];
                --k;
                --j;
            }
        }
    }

    // OptimizedBottomUpMergeSort.merge (merges c[lt:md] and c[md+1:rt] into d[lt:rt])
    template<class T>
    void OptimizedBottomUpMergeSortMerge(std::vector<T>& c, std::vector<T>& d, ptrdiff_t lt, ptrdiff_t md,
                                         ptrdiff_t rt, bool activeSound) {
        ptrdiff_t i = lt,   // cursor for first segment
                  j = md + 1, // cursor for second
                  k = lt;     // cursor for result

        // merge until i or j exits its segment
        while ((i <= md) && (j <= rt)) {
            if (c[i] <= c[j]) {
                if (activeSound) {
                    d[k] = c[i];
                    ++k;
                    ++i;
                    NSortHelpers::MarkArray(1, d, i);
                    NSortHelpers::MarkArray(2, d, j);
                } else {
                    d[k] = c[i];
                    ++k;
                    ++i;
                    NSortHelpers::MarkArray(1, d, k - 1);
                }
            } else {
                if (activeSound) {
                    d[k] = c[j];
                    ++k;
                    ++j;
                    NSortHelpers::MarkArray(1, d, j);
                    NSortHelpers::MarkArray(2, d, i);
                } else {
                    d[k] = c[j];
                    ++k;
                    ++j;
                    NSortHelpers::MarkArray(1, d, k - 1);
                }
            }
        }
        // take care of left overs --- tjr code: only one while loop actually runs
        while (i <= md) {
            d[k] = c[i];
            ++k;
            ++i;
        }
        while (j <= rt) {
            d[k] = c[j];
            ++k;
            ++j;
        }
    }

    // OptimizedBottomUpMergeSort.mergePass (merges adjacent segments of size s)
    template<class T>
    void OptimizedBottomUpMergeSortMergePass(std::vector<T>& x, std::vector<T>& y, ptrdiff_t s, ptrdiff_t n,
                                             bool activeSound) {
        ptrdiff_t i = 0;

        while (i <= n - 2 * s) {
            // Merge two adjacent segments of size s
            OptimizedBottomUpMergeSortMerge(x, y, i, i + s - 1, i + 2 * s - 1, activeSound);
            i = i + 2 * s;
        }
        // fewer than 2s elements remain
        if (i + s < n) {
            OptimizedBottomUpMergeSortMerge(x, y, i, i + s - 1, n - 1, activeSound);
        } else {
            for (ptrdiff_t j = i; j <= n - 1; j++) {
                y[j] = x[j]; // copy last segment to y
                NSortHelpers::MarkArray(1, y, j);
            }
        }
    }

    // port of OptimizedBottomUpMergeSort.stableSort (+ runSort)
    template<class T = int>
    void OptimizedBottomUpMergeSort(std::vector<T>& data_) {
        ptrdiff_t n = static_cast<ptrdiff_t>(data_.size());
        if (n < 2) return;

        if (n < 16) {
            // The Java code passes (0, 16) here, which only stays in bounds
            // because ArrayV's backing array is longer than the sorted length;
            // the range is clamped to n (the visible prefix ends up sorted
            // either way).
            NSortHelpers::binaryInsertSort(data_, 0, n);
            return;
        }

        // Sort data_[0:n] using merge sort.
        ptrdiff_t s = 16; // segment size
        std::vector<T> b(static_cast<size_t>(n));
        ptrdiff_t i;

        for (i = 0; i <= n - 16; i += 16) {
            NSortHelpers::binaryInsertSort(data_, i, i + 16);
        }
        NSortHelpers::binaryInsertSort(data_, i, n);

        while (s < n) {
            OptimizedBottomUpMergeSortMergePass(data_, b, s, n, true); // merge from a to b
            s += s;                                                    // double the segment size
            OptimizedBottomUpMergeSortMergePass(b, data_, s, n, false); // merge from b to a
            s += s;                                                     // again, double the segment size
        }
    }

    // ==================== WeaveMergeSort + OptimizedWeaveMergeSort + OptimizedLazyStableSort + MergeInsertionSort + ImprovedBlockSelectionSort + YujisBufferedMergeSort2 ====================
// Ports of ArrayV (Java, MIT) "Hybrid Sorts" category classes
// (https://github.com/Gaming32/ArrayV, D:\Temp\sorts\hybrid) into VisualSort:
//   WeaveMergeSort, OptimizedWeaveMergeSort, OptimizedLazyStableSort,
//   MergeInsertionSort, ImprovedBlockSelectionSort, YujisBufferedMergeSort2.
//
// One function template per Java sort class, in NVisualSort::NSortAlgorithms.
// Every namespace-scope helper is prefixed with its owning algorithm name.
//
// ASCII ONLY: this file contains no non-ASCII characters.
//
// Included from Sort.h after SortHelpers.h; opens/closes its own namespace so it
// also compiles stand-alone (see D:\Temp\check_parts\p9\main.cpp).


    // Writes.multiSwap(array, pos, to, ...): moves the element at 'pos' to 'to'
    // through a chain of adjacent swaps (NOT a single swap of two elements).
    template<class T>
    void WeaveMergeSortMultiSwap(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t to) {
        using std::swap;
        if (to - pos > 0) {
            for (ptrdiff_t i = pos; i < to; i++) swap(data_[i], data_[i + 1]);
        }
        else {
            for (ptrdiff_t i = pos; i > to; i--) swap(data_[i], data_[i - 1]);
        }
    }

    // WeaveMergeSort.weaveInsert
    template<class T>
    void WeaveMergeSortWeaveInsert(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        using std::swap;

        for (ptrdiff_t j = start; j < end; j++) {
            ptrdiff_t pos = j;

            NSortHelpers::MarkArray(1, data_, j);

            while (pos > start && NSortHelpers::CompareValues(data_[pos], data_[pos - 1]) < 1) {
                swap(data_[pos], data_[pos - 1]);
                pos--;
            }
        }
    }

    // WeaveMergeSort.weaveMerge
    template<class T>
    void WeaveMergeSortWeaveMerge(std::vector<T>& data_, ptrdiff_t min, ptrdiff_t max, ptrdiff_t mid) {
        ptrdiff_t i = 1;
        ptrdiff_t target = (mid - min);

        while (i <= target) {
            WeaveMergeSortMultiSwap(data_, mid + i, min + (i * 2) - 1);
            i++;
        }

        WeaveMergeSortWeaveInsert(data_, min, max + 1);
    }

    // WeaveMergeSort.weaveMergeSort (recursion depth is O(log n))
    template<class T>
    void WeaveMergeSortWeaveMergeSort(std::vector<T>& data_, ptrdiff_t min, ptrdiff_t max) {
        if (max - min == 0) {
            // only one element: the Java code only sleeps here (no swap)
        }
        else if (max - min == 1) { // only two elements, swap them if needed
            if (NSortHelpers::CompareValues(data_[min], data_[max]) == 1) {
                using std::swap;
                swap(data_[min], data_[max]);
            }
        }
        else {
            ptrdiff_t mid = (min + max) / 2; // The midpoint

            WeaveMergeSortWeaveMergeSort(data_, min, mid);     // sort the left side
            WeaveMergeSortWeaveMergeSort(data_, mid + 1, max); // sort the right side
            WeaveMergeSortWeaveMerge(data_, min, max, mid);    // combine them
        }
    }

    // port of WeaveMergeSort.runSort
    template<class T = int>
    void WeaveMergeSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        WeaveMergeSortWeaveMergeSort(data_, 0, length - 1);
    }

    // OptimizedWeaveMergeSort.insertTo
    template<class T>
    void OptimizedWeaveMergeSortInsertTo(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        T temp = data_[a];
        while (a > b) {
            // Java: Writes.write(array, a, array[--a]) - the target index is
            // evaluated before the pre-decrement of the source.
            data_[a] = data_[a - 1];
            --a;
        }
        data_[b] = temp;
    }

    // OptimizedWeaveMergeSort.multiSwap
    template<class T>
    void OptimizedWeaveMergeSortMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
        using std::swap;
        for (ptrdiff_t i = 0; i < len; i++)
            swap(data_[a + i], data_[b + i]);
    }

    // OptimizedWeaveMergeSort.rotate
    template<class T>
    void OptimizedWeaveMergeSortRotate(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t l = m - a, r = b - m;

        while (l > 0 && r > 0) {
            if (r < l) {
                OptimizedWeaveMergeSortMultiSwap(data_, m - r, m, r);
                b -= r;
                m -= r;
                l -= r;
            }
            else {
                OptimizedWeaveMergeSortMultiSwap(data_, a, m, l);
                a += l;
                m += l;
                r -= l;
            }
        }
    }

    // OptimizedWeaveMergeSort.bitReversal (power-of-two ranges only, O(n))
    template<class T>
    void OptimizedWeaveMergeSortBitReversal(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        using std::swap;

        ptrdiff_t len = b - a, m = 0;
        ptrdiff_t d1 = len >> 1, d2 = d1 + (d1 >> 1);

        for (ptrdiff_t i = 1; i < len - 1; i++) {
            ptrdiff_t j = d1;

            for (ptrdiff_t k = i, n = d2; (k & 1) == 0; j -= n, k >>= 1, n >>= 1) {
                // empty body (Java for loop with an empty body)
            }
            m += j;
            if (m > i) swap(data_[a + i], data_[a + m]);
        }
    }

    // OptimizedWeaveMergeSort.weaveInsert
    template<class T>
    void OptimizedWeaveMergeSortWeaveInsert(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, bool right) {
        ptrdiff_t i = a, j = i + 1;

        while (j < b) {
            while (i < j && NSortHelpers::CompareValues(data_[i], data_[j]) < (right ? 1 : 0)) i++;

            if (i == j) {
                right = !right;
                j++;
            }
            else {
                // Java: this.insertTo(array, j, i++) - the old i is passed, then i is incremented.
                OptimizedWeaveMergeSortInsertTo(data_, j, i);
                i++;
                j += 2;
            }
        }
    }

    // OptimizedWeaveMergeSort.weaveMerge: 000111 -> 010101 (T), 00011 -> 01010 (T),
    // 00111 -> 10101 (F)
    template<class T>
    void OptimizedWeaveMergeSortWeaveMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        if (b - a < 2) return;

        ptrdiff_t a1 = a, b1 = b;
        bool right = true;

        if ((b - a) % 2 == 1) {
            if (m - a < b - m) {
                a1--;
                right = false;
            }
            else b1++;
        }

        for (ptrdiff_t e = b1, f; e - a1 > 2; e = f) {
            m = (a1 + e) / 2;
            // Java: 1 << (int)(Math.log(m-a1)/Math.log(2)) - exact integer equivalent:
            // the largest power of two <= m-a1 (m-a1 >= 1 here).
            ptrdiff_t p = 1;
            while (p * 2 <= m - a1) p *= 2;

            OptimizedWeaveMergeSortRotate(data_, m - p, m, e - p);
            m = e - p;
            f = m - p;

            OptimizedWeaveMergeSortBitReversal(data_, f, m);
            OptimizedWeaveMergeSortBitReversal(data_, m, e);
            OptimizedWeaveMergeSortBitReversal(data_, f, e);
        }
        OptimizedWeaveMergeSortWeaveInsert(data_, a, b, right);
    }

    // port of OptimizedWeaveMergeSort.runSort
    template<class T = int>
    void OptimizedWeaveMergeSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t n = static_cast<ptrdiff_t>(data_.size());

        // Java: 1 << (int)(Math.log(n-1)/Math.log(2) + 1) - exact integer
        // equivalent: the smallest power of two >= n.
        ptrdiff_t d = 1;
        while (d < n) d *= 2;

        while (d > 1) {
            ptrdiff_t i = 0, dec = 0;

            while (i < n) {
                ptrdiff_t j = i;
                dec += n;
                while (dec >= d) {
                    dec -= d;
                    j++;
                }
                ptrdiff_t k = j;
                dec += n;
                while (dec >= d) {
                    dec -= d;
                    k++;
                }
                OptimizedWeaveMergeSortWeaveMerge(data_, i, j, k);
                i = k;
            }
            d /= 2;
        }
    }

    // OptimizedLazyStableSort.insertionSort (the class's own adaptive variant;
    // it is NOT the same routine as NSortHelpers::insertionSort)
    template<class T>
    void OptimizedLazyStableSortInsertionSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        using std::swap;
        // C++ bounds guard: Java reads array[a+1] when b-a == 1, which is only
        // legal because ArrayV's backing array is longer than the sorted range.
        if (b - a < 2) return;

        ptrdiff_t i = a + 1;
        // Java: Reads.compareIndices(array, i-1, i++, ...) - the old i is read,
        // then i is incremented before the branch is taken.
        int cmp = NSortHelpers::CompareValues(data_[i - 1], data_[i]);
        i++;
        if (cmp == 1) {
            while (i < b && NSortHelpers::CompareValues(data_[i - 1], data_[i]) == 1) i++;
            // Writes.reversal(array, a, i-1): reverse [a, i-1] inclusive
            for (ptrdiff_t lo = a, hi = i - 1; lo < hi; ++lo, --hi) swap(data_[lo], data_[hi]);
        }
        else while (i < b && NSortHelpers::CompareValues(data_[i - 1], data_[i]) <= 0) i++;

        while (i < b) {
            T current = data_[i];
            ptrdiff_t pos = i - 1;
            while (pos >= a && NSortHelpers::CompareValues(data_[pos], current) > 0) {
                data_[pos + 1] = data_[pos];
                pos--;
            }
            data_[pos + 1] = current;

            i++;
        }
    }

    // OptimizedLazyStableSort.grailLazyStableSort (32-element insertion runs then
    // in-place merges with GrailSorting.grailMergeWithoutBuffer; the same merge
    // routine is shared in the Java code through the GrailSorting base class)
    template<class T>
    void OptimizedLazyStableSortLazyStableSort(std::vector<T>& data_, ptrdiff_t pos, ptrdiff_t len) {
        ptrdiff_t dist;
        for (dist = 0; dist + 16 < len; dist += 16)
            OptimizedLazyStableSortInsertionSort(data_, pos + dist, pos + dist + 16);
        if (dist < len)
            OptimizedLazyStableSortInsertionSort(data_, pos + dist, pos + len);

        for (ptrdiff_t part = 16; part < len; part *= 2) {
            ptrdiff_t left = 0;
            ptrdiff_t right = len - 2 * part;

            while (left <= right) {
                NSortHelpers::grailMergeWithoutBuffer(data_, pos + left, part, part);
                left += 2 * part;
            }

            ptrdiff_t rest = len - left;
            if (rest > part) {
                NSortHelpers::grailMergeWithoutBuffer(data_, pos + left, part, rest - part);
            }
        }
    }

    // port of OptimizedLazyStableSort.runSort
    template<class T = int>
    void OptimizedLazyStableSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        OptimizedLazyStableSortLazyStableSort(data_, 0, length);
    }

    // MergeInsertionSort.blockSwap
    template<class T>
    void MergeInsertionSortBlockSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t s) {
        using std::swap;
        while (s-- > 0) {
            swap(data_[a], data_[b]);
            a--;
            b--;
        }
    }

    // MergeInsertionSort.blockInsert
    template<class T>
    void MergeInsertionSortBlockInsert(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t s) {
        while (a - s >= b) {
            MergeInsertionSortBlockSwap(data_, a - s, a, s);
            a -= s;
        }
    }

    // MergeInsertionSort.blockReversal
    template<class T>
    void MergeInsertionSortBlockReversal(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t s) {
        b -= s;
        while (b > a) {
            MergeInsertionSortBlockSwap(data_, a, b, s);
            a += s;
            b -= s;
        }
    }

    // MergeInsertionSort.blockSearch
    template<class T>
    ptrdiff_t MergeInsertionSortBlockSearch(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t s, const T& val) {
        while (a < b) {
            ptrdiff_t m = a + (((b - a) / s) / 2) * s;

            if (NSortHelpers::CompareValues(val, data_[m]) < 0)
                b = m;
            else
                a = m + s;
        }

        return a;
    }

    // MergeInsertionSort.order
    template<class T>
    void MergeInsertionSortOrder(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t s) {
        for (ptrdiff_t i = a, j = i + s; j < b; i += s, j += 2 * s)
            MergeInsertionSortBlockInsert(data_, j, i, s);

        ptrdiff_t m = a + (((b - a) / s) / 2) * s;
        MergeInsertionSortBlockReversal(data_, m, b, s);
    }

    // port of MergeInsertionSort.runSort
    template<class T = int>
    void MergeInsertionSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        ptrdiff_t k = 1;
        while (2 * k <= length) {
            for (ptrdiff_t i = 2 * k - 1; i < length; i += 2 * k)
                if (NSortHelpers::CompareValues(data_[i - k], data_[i]) > 0)
                    MergeInsertionSortBlockSwap(data_, i - k, i, k);

            k *= 2;
        }

        while (k > 0) {
            ptrdiff_t a = k - 1, i = a + 2 * k, g = 2, p = 4;

            while (i + 2 * k * g - k <= length) {
                MergeInsertionSortOrder(data_, i, i + 2 * k * g - k, k);
                ptrdiff_t b = a + k * (p - 1);

                i += k * g - k;
                for (ptrdiff_t j = i; j < i + k * g; j += k) {
                    // Java evaluates array[j] before blockInsert moves the blocks.
                    ptrdiff_t target = MergeInsertionSortBlockSearch(data_, a, b, k, data_[j]);
                    MergeInsertionSortBlockInsert(data_, j, target, k);
                }

                i += k * g + k;
                g = p - g;
                p *= 2;
            }
            while (i < length) {
                ptrdiff_t target = MergeInsertionSortBlockSearch(data_, a, i, k, data_[i]);
                MergeInsertionSortBlockInsert(data_, i, target, k);
                i += 2 * k;
            }

            k /= 2;
        }
    }

    // ImprovedBlockSelectionSort.sqrt (the smallest power of two >= ceil(sqrt(n)))
    inline ptrdiff_t ImprovedBlockSelectionSortSqrt(ptrdiff_t n) {
        ptrdiff_t i = 1;
        for (; i * i < n; i *= 2);
        return i;
    }

    // ImprovedBlockSelectionSort.multiSwap
    template<class T>
    void ImprovedBlockSelectionSortMultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
        using std::swap;
        for (ptrdiff_t i = 0; i < len; i++)
            swap(data_[a + i], data_[b + i]);
    }

    // ImprovedBlockSelectionSort.rotate
    template<class T>
    void ImprovedBlockSelectionSortRotate(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t l = m - a, r = b - m;

        while (l > 0 && r > 0) {
            if (r < l) {
                ImprovedBlockSelectionSortMultiSwap(data_, m - r, m, r);
                b -= r;
                m -= r;
                l -= r;
            }
            else {
                ImprovedBlockSelectionSortMultiSwap(data_, a, m, l);
                a += l;
                m += l;
                r -= l;
            }
        }
    }

    // ImprovedBlockSelectionSort.inPlaceMerge
    template<class T>
    ptrdiff_t ImprovedBlockSelectionSortInPlaceMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = a, j = m, k;

        while (i < j && j < b) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) > 0) {
                k = j;
                do k++;
                while (k < b && NSortHelpers::CompareValues(data_[i], data_[k]) > 0);

                ImprovedBlockSelectionSortRotate(data_, i, j, k);

                i += k - j;
                j = k;
            }
            else i++;
        }

        return i;
    }

    // ImprovedBlockSelectionSort.inPlaceMergeBW
    template<class T>
    void ImprovedBlockSelectionSortInPlaceMergeBW(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b) {
        ptrdiff_t i = m - 1, j = b - 1, k;

        while (j > i && i >= a) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) > 0) {
                k = i;
                do k--;
                while (k >= a && NSortHelpers::CompareValues(data_[k], data_[j]) > 0);

                ImprovedBlockSelectionSortRotate(data_, k + 1, i + 1, j + 1);

                j -= i - k;
                i = k;
            }
            else j--;
        }
    }

    // ImprovedBlockSelectionSort.selectRange
    template<class T>
    ptrdiff_t ImprovedBlockSelectionSortSelectRange(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t bLen) {
        ptrdiff_t min = a;
        a += bLen;

        while (a < b) {
            int comp = NSortHelpers::CompareValues(data_[a], data_[min]);

            if (comp == -1 || (comp == 0 && NSortHelpers::CompareValues(data_[a + bLen - 1], data_[min + bLen - 1]) == -1))
                min = a;

            a += bLen;
        }
        return min;
    }

    // ImprovedBlockSelectionSort.blockSelect
    template<class T>
    void ImprovedBlockSelectionSortBlockSelect(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t bLen) {
        ptrdiff_t k = a, j = m;

        while (k < m && NSortHelpers::CompareValues(data_[k], data_[m]) <= 0) k += bLen;
        if (k == m) return;

        ptrdiff_t i = m;
        ImprovedBlockSelectionSortMultiSwap(data_, k, j, bLen);
        k += bLen;
        j += bLen;

        while (k < j && j < b) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) <= 0) {
                if (k != i) ImprovedBlockSelectionSortMultiSwap(data_, k, i, bLen);
                k += bLen;
                i = ImprovedBlockSelectionSortSelectRange(data_, (std::max)(m, k), j, bLen);
            }
            else {
                if (i == k) i = j;
                if (k != j) ImprovedBlockSelectionSortMultiSwap(data_, k, j, bLen);
                k += bLen;
                j += bLen;
            }
        }
        while (k < j) {
            i = ImprovedBlockSelectionSortSelectRange(data_, k, b, bLen);
            if (k != i) ImprovedBlockSelectionSortMultiSwap(data_, k, i, bLen);
            k += bLen;
        }
    }

    // port of ImprovedBlockSelectionSort.runSort
    template<class T = int>
    void ImprovedBlockSelectionSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        for (ptrdiff_t i = 0, j = 1; j < length; j *= 2) {
            ptrdiff_t bLen = ImprovedBlockSelectionSortSqrt(j), n = j;
            ptrdiff_t b = length - length % bLen;

            while (n > 16) {
                for (i = 0; i + j < b; i += 2 * j)
                    for (ptrdiff_t k = i; k + n < (std::min)(i + 2 * j, b); k += n)
                        ImprovedBlockSelectionSortBlockSelect(data_, k, k + n, (std::min)(k + 2 * n, b), bLen);

                n = bLen;
                bLen = ImprovedBlockSelectionSortSqrt(bLen);
            }

            for (i = 0; i + j < b; i += 2 * j)
                for (ptrdiff_t k = i, f = i; k + n < (std::min)(i + 2 * j, b); k += n)
                    f = ImprovedBlockSelectionSortInPlaceMerge(data_, f, k + n, (std::min)(k + 2 * n, b));

            ImprovedBlockSelectionSortInPlaceMergeBW(data_, length - length % (2 * j), b, length);
        }
    }

    // YujisBufferedMergeSort2.ceilLog (ceil(log2(n)), 0 for n <= 1)
    inline ptrdiff_t YujisBufferedMergeSort2CeilLog(ptrdiff_t n) {
        ptrdiff_t i;
        for (i = 0; (static_cast<ptrdiff_t>(1) << i) < n; i++);
        return i;
    }

    // YujisBufferedMergeSort2.multiSwap
    template<class T>
    void YujisBufferedMergeSort2MultiSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t len) {
        using std::swap;
        for (ptrdiff_t i = 0; i < len; i++)
            swap(data_[a + i], data_[b + i]);
    }

    // YujisBufferedMergeSort2.insertTo
    template<class T>
    void YujisBufferedMergeSort2InsertTo(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        T temp = data_[a];
        while (a > b) {
            // Java: Writes.write(array, a, array[(a--)-1]) - the target index is
            // evaluated before the post-decrement of the source.
            data_[a] = data_[a - 1];
            --a;
        }
        data_[b] = temp;
    }

    // YujisBufferedMergeSort2.binarySearch
    template<class T>
    ptrdiff_t YujisBufferedMergeSort2BinarySearch(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end,
                                                  const T& value, bool left) {
        ptrdiff_t a = start, b = end;

        while (a < b) {
            ptrdiff_t m = a + (b - a) / 2;
            bool comp;

            if (left) comp = NSortHelpers::CompareValues(value, data_[m]) <= 0;
            else      comp = NSortHelpers::CompareValues(value, data_[m]) < 0;

            if (comp) b = m;
            else      a = m + 1;
        }

        return a;
    }

    // YujisBufferedMergeSort2.binaryInsertion
    template<class T>
    void YujisBufferedMergeSort2BinaryInsertion(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        for (ptrdiff_t i = a + 1; i < b; i++) {
            // Java evaluates array[i] inside the binarySearch call, before insertTo runs.
            ptrdiff_t target = YujisBufferedMergeSort2BinarySearch(data_, a, i, data_[i], false);
            YujisBufferedMergeSort2InsertTo(data_, i, target);
        }
    }

    // YujisBufferedMergeSort2.mergeWithBufStatic
    template<class T>
    void YujisBufferedMergeSort2MergeWithBufStatic(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b,
                                                   ptrdiff_t p, bool useBinarySearch) {
        using std::swap;

        ptrdiff_t i = 0, j = m, k = a;

        if (useBinarySearch) {
            while (i < m - a && j < b) {
                if (NSortHelpers::CompareValues(data_[j], data_[p + i]) == -1) {
                    ptrdiff_t q = YujisBufferedMergeSort2BinarySearch(data_, j, b, data_[p + i], true);
                    while (j < q) {
                        swap(data_[k], data_[j]);
                        k++;
                        j++;
                    }
                }
                swap(data_[k], data_[p + i]);
                k++;
                i++;
            }
            while (i < m - a) {
                swap(data_[k], data_[p + i]);
                k++;
                i++;
            }
        }
        else {
            while (i < m - a && j < b) {
                if (NSortHelpers::CompareValues(data_[p + i], data_[j]) <= 0) {
                    swap(data_[k], data_[p + i]);
                    k++;
                    i++;
                }
                else {
                    swap(data_[k], data_[j]);
                    k++;
                    j++;
                }
            }
            while (i < m - a) {
                swap(data_[k], data_[p + i]);
                k++;
                i++;
            }
        }
    }

    // YujisBufferedMergeSort2.merge (writes the merged run to p, returns the
    // number of leftover elements taken from the right run)
    template<class T>
    ptrdiff_t YujisBufferedMergeSort2Merge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t m, ptrdiff_t b, ptrdiff_t p) {
        using std::swap;

        ptrdiff_t i = a, j = m;
        while (i < m && j < b) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) <= 0) {
                swap(data_[p], data_[i]);
                p++;
                i++;
            }
            else {
                swap(data_[p], data_[j]);
                p++;
                j++;
            }
        }
        ptrdiff_t leftover = 0;
        while (i < m) {
            swap(data_[p], data_[i]);
            p++;
            i++;
        }
        while (j < b) {
            swap(data_[p], data_[j]);
            p++;
            j++;
            leftover++;
        }
        return leftover;
    }

    // YujisBufferedMergeSort2.mergeSort (ping-pongs the runs between the regions
    // starting at a and at p)
    template<class T>
    void YujisBufferedMergeSort2MergeSort(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t p, ptrdiff_t length) {
        using std::swap;

        ptrdiff_t i, j = 16;
        ptrdiff_t cl = YujisBufferedMergeSort2CeilLog(length); // Java local named 'ceilLog'
        ptrdiff_t pos;

        if (length > 16 && (cl & 1) == 1)
            pos = p;
        else
            pos = a;

        for (i = pos; i + 16 <= pos + length; i += 16)
            YujisBufferedMergeSort2BinaryInsertion(data_, i, i + 16);
        YujisBufferedMergeSort2BinaryInsertion(data_, i, pos + length);

        ptrdiff_t next = pos, posNext;
        while (j < length) {
            pos = next;
            next ^= a ^ p;
            posNext = next;

            for (i = pos; i + 2 * j <= pos + length; i += 2 * j, posNext += 2 * j)
                YujisBufferedMergeSort2Merge(data_, i, i + j, i + 2 * j, posNext);
            if (i + j < pos + length)
                YujisBufferedMergeSort2Merge(data_, i, i + j, pos + length, posNext);
            else
                while (i < pos + length) {
                    swap(data_[i], data_[posNext]);
                    i++;
                    posNext++;
                }

            j *= 2;
        }
    }

    // YujisBufferedMergeSort2.bufferedMerge (recursion depth is O(log n))
    template<class T>
    void YujisBufferedMergeSort2BufferedMerge(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        if (b - a <= 16) {
            YujisBufferedMergeSort2BinaryInsertion(data_, a, b);
            return;
        }

        ptrdiff_t m = (a + b + 1) / 2;
        YujisBufferedMergeSort2MergeSort(data_, m, 2 * m - b, b - m);

        ptrdiff_t n = (a + m + 1) / 2;
        // The 16 means that it will recursively sort 1/16 of the array.
        ptrdiff_t limit = (b - a) / 16;
        while (m - a > limit) {
            YujisBufferedMergeSort2MergeSort(data_, 2 * n - m, n, m - n);
            YujisBufferedMergeSort2MergeWithBufStatic(data_, n, m, b, 2 * n - m,
                                                      (b - m) / (m - n) >= YujisBufferedMergeSort2CeilLog(n - a));
            m = n;
            n = (a + m + 1) / 2;
        }

        // the same as Andreysort's buffer redistribution
        YujisBufferedMergeSort2BufferedMerge(data_, a, m);
        YujisBufferedMergeSort2MultiSwap(data_, a, b - (m - a), m - a);
        ptrdiff_t s = YujisBufferedMergeSort2Merge(data_, m, b - (m - a), b, a);
        YujisBufferedMergeSort2BufferedMerge(data_, b - (m - a) - s, b);
    }

    // port of YujisBufferedMergeSort2.runSort
    template<class T = int>
    void YujisBufferedMergeSort2(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        YujisBufferedMergeSort2BufferedMerge(data_, 0, length);
    }

} // namespace NVisualSort::NSortAlgorithms
