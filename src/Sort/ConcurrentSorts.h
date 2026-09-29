#pragma once
// Ports of the ArrayV (Java, MIT) "concurrent" category sorting classes,
// https://github.com/Gaming32/ArrayV -- thread based sorts and sorting networks.
//
// ASCII ONLY: do not put non-ASCII characters in this file. MSVC parses sources in the
// system code page (GBK); use \uXXXX escapes if a wide string is ever needed.
//
// Every algorithm is a sorting network ported statement by statement from its Java class.
// The comparator is a compare-and-swap (swap when out of order):
//     if (data_[i] > data_[j]) { using std::swap; swap(data_[i], data_[j]); }
// Networks that only compare with a guard "b < end" (end = the real length) run on a
// zero-padded power-of-two size and therefore work for any length; the remaining ones
// are registered with the power-of-two size constraint. The Parallel variants keep the
// Java thread structure (std::thread + join); the Recursive variants are plain recursion
// because the Java sources do not spawn threads in them.
#include "SortHelpers.h"
#include <cmath>
#include <cstddef>
#include <thread>
#include <utility>
#include <vector>

namespace NVisualSort::NSortAlgorithms {

    // ============================ Bitonic family ============================

    // BitonicSortIterative.runSort (Nikos Pitsianis' version, rewritten by Piotr
    // Grochowski so that it also works for lengths that are not powers of two).
    template<class T = int>
    void BitonicSortIterative(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());

        for (ptrdiff_t k = 2; k < sortLength * 2; k = 2 * k) {
            bool m = (((sortLength + (k - 1)) / k) % 2) != 0;

            for (ptrdiff_t j = k >> 1; j > 0; j = j >> 1) {
                for (ptrdiff_t i = 0; i < sortLength; i++) {
                    ptrdiff_t ij = i ^ j;

                    if (ij > i && ij < sortLength) {
                        if (((i & k) == 0) == m && data_[i] > data_[ij]) {
                            using std::swap;
                            swap(data_[i], data_[ij]);
                        }
                        if (((i & k) != 0) == m && data_[i] < data_[ij]) {
                            using std::swap;
                            swap(data_[i], data_[ij]);
                        }
                    }
                }
            }
        }
    }

    // BitonicSortParallel.bitonicMerge (Java MergeThread body; the two threads become
    // std::thread + join). "flag" is 1 for ascending and -1 for descending runs.
    template<class T>
    void BitonicSortParallelMerge(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t mid, ptrdiff_t stop, ptrdiff_t gap, int flag) {
        if (stop - start >= 2) {
            for (ptrdiff_t i = start; i < mid; i++) {
                if (NSortHelpers::CompareValues(data_[i], data_[i + gap]) == flag) {
                    using std::swap;
                    swap(data_[i], data_[i + gap]);
                }
            }

            ptrdiff_t newGap = gap / 2;
            ptrdiff_t midL = (mid - start) / 2 + start;
            ptrdiff_t midR = (stop - mid) / 2 + mid;

            std::thread left([&data_, start, midL, mid, newGap, flag] {
                BitonicSortParallelMerge(data_, start, midL, mid, newGap, flag);
            });
            std::thread right([&data_, mid, midR, stop, newGap, flag] {
                BitonicSortParallelMerge(data_, mid, midR, stop, newGap, flag);
            });

            left.join();
            right.join();
        }
    }

    // BitonicSortParallel.bitonicSort (Java SortThread body).
    template<class T>
    void BitonicSortParallelSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t stop, bool ascending) {
        ptrdiff_t length = stop - start;

        if (length >= 2) {
            ptrdiff_t gap = length / 2;
            ptrdiff_t mid = gap + start;

            std::thread left([&data_, start, mid, ascending] {
                BitonicSortParallelSort(data_, start, mid, !ascending);
            });
            std::thread right([&data_, mid, stop, ascending] {
                BitonicSortParallelSort(data_, mid, stop, ascending);
            });

            left.join();
            right.join();

            BitonicSortParallelMerge(data_, start, mid, stop, gap, ascending ? 1 : -1);
        }
    }

    // BitonicSortParallel.runSort (parallel bitonic; requires a power-of-two length).
    template<class T = int>
    void BitonicSortParallel(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        BitonicSortParallelSort(data_, 0, static_cast<ptrdiff_t>(data_.size()), true);
    }

    // BitonicSortRecursive.compare
    template<class T>
    void BitonicSortRecursiveCompare(std::vector<T>& data_, ptrdiff_t i, ptrdiff_t j, bool dir) {
        int cmp = NSortHelpers::CompareValues(data_[i], data_[j]);

        if (dir == (cmp == 1)) {
            using std::swap;
            swap(data_[i], data_[j]);
        }
    }

    // BitonicSortRecursive.greatestPowerOfTwoLessThan
    inline ptrdiff_t BitonicSortRecursiveGreatestPowerOfTwoLessThan(ptrdiff_t n) {
        ptrdiff_t k = 1;
        while (k < n) {
            k = k << 1;
        }
        return k >> 1;
    }

    // BitonicSortRecursive.bitonicMerge (H.W. Lang's odd-n variant).
    template<class T>
    void BitonicSortRecursiveMerge(std::vector<T>& data_, ptrdiff_t lo, ptrdiff_t n, bool dir) {
        if (n > 1) {
            ptrdiff_t m = BitonicSortRecursiveGreatestPowerOfTwoLessThan(n);

            for (ptrdiff_t i = lo; i < lo + n - m; i++) {
                BitonicSortRecursiveCompare(data_, i, i + m, dir);
            }

            BitonicSortRecursiveMerge(data_, lo, m, dir);
            BitonicSortRecursiveMerge(data_, lo + m, n - m, dir);
        }
    }

    // BitonicSortRecursive.bitonicSort
    template<class T>
    void BitonicSortRecursiveSort(std::vector<T>& data_, ptrdiff_t lo, ptrdiff_t n, bool dir) {
        if (n > 1) {
            ptrdiff_t m = n / 2;
            BitonicSortRecursiveSort(data_, lo, m, !dir);
            BitonicSortRecursiveSort(data_, lo + m, n - m, dir);
            BitonicSortRecursiveMerge(data_, lo, n, dir);
        }
    }

    // BitonicSortRecursive.runSort (the Java direction field defaults to forward).
    template<class T = int>
    void BitonicSortRecursive(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        BitonicSortRecursiveSort(data_, 0, static_cast<ptrdiff_t>(data_.size()), true);
    }

    // ========================= Bose-Nelson family =========================

    // BoseNelsonSortIterative.compSwap
    template<class T>
    void BoseNelsonSortIterativeCompSwap(std::vector<T>& data_, ptrdiff_t end, ptrdiff_t a, ptrdiff_t b) {
        if (b >= end) return;

        if (NSortHelpers::CompareValues(data_[a], data_[b]) == 1) {
            using std::swap;
            swap(data_[a], data_[b]);
        }
    }

    // BoseNelsonSortIterative.rangeComp
    template<class T>
    void BoseNelsonSortIterativeRangeComp(std::vector<T>& data_, ptrdiff_t end, ptrdiff_t a, ptrdiff_t b, ptrdiff_t offset) {
        ptrdiff_t half = (b - a) / 2;
        ptrdiff_t m = a + half;
        a += offset;

        for (ptrdiff_t i = 0; i < half - offset; i++) {
            if ((i & ~offset) == i) {
                BoseNelsonSortIterativeCompSwap(data_, end, a + i, m + i);
            }
        }
    }

    // BoseNelsonSortIterative.runSort (pads to a power of two and guards comparisons;
    // the trailing padding is never exchanged, so any length works).
    template<class T = int>
    void BoseNelsonSortIterative(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t end = static_cast<ptrdiff_t>(data_.size());

        // currentLength = 1 << (int)ceil(log(length)/log(2))
        ptrdiff_t currentLength = 1;
        for (; currentLength < end; currentLength <<= 1);

        for (ptrdiff_t k = 2; k <= currentLength; k *= 2) {
            for (ptrdiff_t j = 0; j < k / 2; j++) {
                for (ptrdiff_t i = 0; i + j < end; i += k) {
                    BoseNelsonSortIterativeRangeComp(data_, end, i, i + k, j);
                }
            }
        }
    }

    // BoseNelsonSortParallel.compareSwap
    template<class T>
    void BoseNelsonSortParallelCompareSwap(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        if (NSortHelpers::CompareValues(data_[start], data_[end]) == 1) {
            using std::swap;
            swap(data_[start], data_[end]);
        }
    }

    // BoseNelsonSortParallel.boseNelsonMerge (Java BoseNelsonMerge threads).
    template<class T>
    void BoseNelsonSortParallelMerge(std::vector<T>& data_, ptrdiff_t start1, ptrdiff_t len1, ptrdiff_t start2, ptrdiff_t len2) {
        if (len1 == 1 && len2 == 1) {
            BoseNelsonSortParallelCompareSwap(data_, start1, start2);
        }
        else if (len1 == 1 && len2 == 2) {
            BoseNelsonSortParallelCompareSwap(data_, start1, start2 + 1);
            BoseNelsonSortParallelCompareSwap(data_, start1, start2);
        }
        else if (len1 == 2 && len2 == 1) {
            BoseNelsonSortParallelCompareSwap(data_, start1, start2);
            BoseNelsonSortParallelCompareSwap(data_, start1 + 1, start2);
        }
        else {
            ptrdiff_t mid1 = len1 / 2;
            ptrdiff_t mid2 = len1 % 2 == 1 ? len2 / 2 : (len2 + 1) / 2;

            std::thread left([&data_, start1, mid1, start2, mid2] {
                BoseNelsonSortParallelMerge(data_, start1, mid1, start2, mid2);
            });
            std::thread right([&data_, start1, mid1, len1, start2, mid2, len2] {
                BoseNelsonSortParallelMerge(data_, start1 + mid1, len1 - mid1, start2 + mid2, len2 - mid2);
            });

            left.join();
            right.join();

            BoseNelsonSortParallelMerge(data_, start1 + mid1, len1 - mid1, start2, mid2);
        }
    }

    // BoseNelsonSortParallel.boseNelson (Java BoseNelson threads).
    template<class T>
    void BoseNelsonSortParallelSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t length) {
        if (length > 1) {
            ptrdiff_t mid = length / 2;

            std::thread left([&data_, start, mid] {
                BoseNelsonSortParallelSort(data_, start, mid);
            });
            std::thread right([&data_, start, mid, length] {
                BoseNelsonSortParallelSort(data_, start + mid, length - mid);
            });

            left.join();
            right.join();

            BoseNelsonSortParallelMerge(data_, start, mid, start + mid, length - mid);
        }
    }

    // BoseNelsonSortParallel.runSort
    template<class T = int>
    void BoseNelsonSortParallel(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        BoseNelsonSortParallelSort(data_, 0, static_cast<ptrdiff_t>(data_.size()));
    }

    // BoseNelsonSortRecursive.compareSwap
    template<class T>
    void BoseNelsonSortRecursiveCompareSwap(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end) {
        if (NSortHelpers::CompareValues(data_[start], data_[end]) == 1) {
            using std::swap;
            swap(data_[start], data_[end]);
        }
    }

    // BoseNelsonSortRecursive.boseNelsonMerge
    template<class T>
    void BoseNelsonSortRecursiveMerge(std::vector<T>& data_, ptrdiff_t start1, ptrdiff_t len1, ptrdiff_t start2, ptrdiff_t len2) {
        if (len1 == 1 && len2 == 1) {
            BoseNelsonSortRecursiveCompareSwap(data_, start1, start2);
        }
        else if (len1 == 1 && len2 == 2) {
            BoseNelsonSortRecursiveCompareSwap(data_, start1, start2 + 1);
            BoseNelsonSortRecursiveCompareSwap(data_, start1, start2);
        }
        else if (len1 == 2 && len2 == 1) {
            BoseNelsonSortRecursiveCompareSwap(data_, start1, start2);
            BoseNelsonSortRecursiveCompareSwap(data_, start1 + 1, start2);
        }
        else {
            ptrdiff_t mid1 = len1 / 2;
            ptrdiff_t mid2 = len1 % 2 == 1 ? len2 / 2 : (len2 + 1) / 2;

            BoseNelsonSortRecursiveMerge(data_, start1, mid1, start2, mid2);
            BoseNelsonSortRecursiveMerge(data_, start1 + mid1, len1 - mid1, start2 + mid2, len2 - mid2);
            BoseNelsonSortRecursiveMerge(data_, start1 + mid1, len1 - mid1, start2, mid2);
        }
    }

    // BoseNelsonSortRecursive.boseNelson
    template<class T>
    void BoseNelsonSortRecursiveSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t length) {
        if (length > 1) {
            ptrdiff_t mid = length / 2;
            BoseNelsonSortRecursiveSort(data_, start, mid);
            BoseNelsonSortRecursiveSort(data_, start + mid, length - mid);
            BoseNelsonSortRecursiveMerge(data_, start, mid, start + mid, length - mid);
        }
    }

    // BoseNelsonSortRecursive.runSort
    template<class T = int>
    void BoseNelsonSortRecursive(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        BoseNelsonSortRecursiveSort(data_, 0, static_cast<ptrdiff_t>(data_.size()));
    }

    // ============================= Crease sort =============================

    // CreaseSort.runSort (aphitorite's crease sorting network).
    template<class T = int>
    void CreaseSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        auto compSwap = [&data_](ptrdiff_t a, ptrdiff_t b) {
            if (data_[a] > data_[b]) {
                using std::swap;
                swap(data_[a], data_[b]);
            }
        };

        ptrdiff_t max = 1;
        for (; max * 2 < length; max *= 2);

        ptrdiff_t next = max;
        while (next > 0) {
            for (ptrdiff_t i = 0; i + 1 < length; i += 2) {
                compSwap(i, i + 1);
            }

            for (ptrdiff_t j = max; j >= next && j > 1; j /= 2) {
                for (ptrdiff_t i = 1; i + j - 1 < length; i += 2) {
                    compSwap(i, i + j - 1);
                }
            }

            next /= 2;
        }
    }

    // ========================= Diamonds and folds =========================

    // DiamondSortIterative.runSort (_fluffyy / yuji, implemented by aphitorite).
    template<class T = int>
    void DiamondSortIterative(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        auto compSwap = [&data_](ptrdiff_t a, ptrdiff_t b) {
            if (data_[a] > data_[b]) {
                using std::swap;
                swap(data_[a], data_[b]);
            }
        };

        ptrdiff_t n = 1;
        for (; n < length; n *= 2);

        ptrdiff_t m = 4;
        for (; m <= n; m *= 2) {
            for (ptrdiff_t k = 0; k < m / 2; k++) {
                ptrdiff_t cnt = k <= m / 4 ? k : m / 2 - k;

                for (ptrdiff_t j = 0; j < length; j += m) {
                    if (j + cnt + 1 < length) {
                        for (ptrdiff_t i = j + cnt; i + 1 < (std::min)(length, j + m - cnt); i += 2) {
                            compSwap(i, i + 1);
                        }
                    }
                }
            }
        }

        m /= 2;

        for (ptrdiff_t k = 0; k <= m / 2; k++) {
            for (ptrdiff_t i = k; i + 1 < (std::min)(length, m - k); i += 2) {
                compSwap(i, i + 1);
            }
        }
    }

    // DiamondSortRecursive.sort
    template<class T>
    void DiamondSortRecursiveSort(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t stop, bool merge) {
        if (stop - start == 2) {
            if (data_[start] > data_[stop - 1]) {
                using std::swap;
                swap(data_[start], data_[stop - 1]);
            }
        }
        else if (stop - start >= 3) {
            double div = (stop - start) / 4.0;
            ptrdiff_t mid = (stop - start) / 2 + start;
            ptrdiff_t quarter = static_cast<ptrdiff_t>(div);
            ptrdiff_t threeQuarters = static_cast<ptrdiff_t>(div * 3);

            if (merge) {
                DiamondSortRecursiveSort(data_, start, mid, true);
                DiamondSortRecursiveSort(data_, mid, stop, true);
            }

            DiamondSortRecursiveSort(data_, quarter + start, threeQuarters + start, false);
            DiamondSortRecursiveSort(data_, start, mid, false);
            DiamondSortRecursiveSort(data_, mid, stop, false);
            DiamondSortRecursiveSort(data_, quarter + start, threeQuarters + start, false);
        }
    }

    // DiamondSortRecursive.runSort
    template<class T = int>
    void DiamondSortRecursive(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        DiamondSortRecursiveSort(data_, 0, static_cast<ptrdiff_t>(data_.size()), true);
    }

    // FoldSort.compSwap
    template<class T>
    void FoldSortCompSwap(std::vector<T>& data_, ptrdiff_t end, ptrdiff_t a, ptrdiff_t b) {
        if (b < end && data_[a] > data_[b]) {
            using std::swap;
            swap(data_[a], data_[b]);
        }
    }

    // FoldSort.halver
    template<class T>
    void FoldSortHalver(std::vector<T>& data_, ptrdiff_t end, ptrdiff_t low, ptrdiff_t high) {
        while (low < high) {
            FoldSortCompSwap(data_, end, low, high);
            low++;
            high--;
        }
    }

    // FoldSort.runSort (Marcel Pi Nacy's fold sorting network; the size is rounded up
    // to a power of two and comparisons past the real end are skipped).
    template<class T = int>
    void FoldSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t end = static_cast<ptrdiff_t>(data_.size());

        ptrdiff_t ceilLog = 1;
        for (; (static_cast<ptrdiff_t>(1) << ceilLog) < end; ceilLog++);
        ptrdiff_t size = static_cast<ptrdiff_t>(1) << ceilLog;

        for (ptrdiff_t k = size >> 1; k > 0; k >>= 1) {
            for (ptrdiff_t i = size; i >= k; i >>= 1) {
                for (ptrdiff_t j = 0; j < end; j += i) {
                    FoldSortHalver(data_, end, j, j + i - 1);
                }
            }
        }
    }

    // ============================= Matrix sort ============================

    // MatrixSort.MatrixShape
    struct MatrixSortShape {
        ptrdiff_t width;
        bool unbalanced;
        bool insertLast;

        MatrixSortShape(ptrdiff_t width_, ptrdiff_t height, bool insertLast_)
            : width(width_), unbalanced((width_ == 1) ^ (height == 1)), insertLast(unbalanced || insertLast_) {
        }
    };

    // MatrixSort.gapReverse (Writes.changeReversals / Highlights.clearMark are ignored).
    template<class T>
    void MatrixSortGapReverse(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, ptrdiff_t gap) {
        for (ptrdiff_t i = start, j = end; i < j; i += gap, j -= gap) {
            using std::swap;
            swap(data_[i], data_[j - gap]);
        }
    }

    // MatrixSort.dirCompareVal
    template<class T>
    int MatrixSortDirCompareVal(const T& left, const T& right, bool dir) {
        int res = NSortHelpers::CompareValues(left, right);
        return dir ? res : (res * -1);
    }

    // MatrixSort.insertLast (gap-based insertion of the element at b).
    template<class T>
    bool MatrixSortInsertLast(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b, ptrdiff_t gap, bool dir) {
        bool did = false;
        T key = data_[b];
        ptrdiff_t j = b - gap;

        while (j >= a && MatrixSortDirCompareVal(key, data_[j], dir) < 0) {
            data_[j + gap] = data_[j];
            did = true;
            j -= gap;
        }
        data_[j + gap] = key;

        return did;
    }

    // MatrixSort.getMatrixDims
    inline MatrixSortShape MatrixSortGetMatrixDims(ptrdiff_t len) {
        ptrdiff_t dim = static_cast<ptrdiff_t>(std::sqrt(static_cast<double>(len)));
        bool insertLast = false;

        if (dim * dim == len - 1) {
            insertLast = true;
        }
        for (; len % dim != 0; dim--);

        return MatrixSortShape(dim, len / dim, insertLast);
    }

    // MatrixSort.matrixSort (returns whether any insertion moved an element).
    template<class T>
    bool MatrixSortRec(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, ptrdiff_t gap, bool dir) {
        bool did = false;
        ptrdiff_t length = (end - start) / gap;

        if (length < 2) {
            return false;
        }
        else if (length <= 16) {
            did = false;
            for (ptrdiff_t i = start; i < end; i += gap) {
                did = MatrixSortInsertLast(data_, start, i, gap, dir) | did;
            }
        }
        else {
            MatrixSortShape matShape = MatrixSortGetMatrixDims(length);

            if (matShape.insertLast) {
                bool did1 = MatrixSortRec(data_, start, end - gap, gap, dir);
                bool did2 = MatrixSortInsertLast(data_, start, end - gap, gap, dir);
                return did1 || did2;
            }

            for (ptrdiff_t i = start + matShape.width * gap; i < end; i += 2 * matShape.width * gap) {
                MatrixSortGapReverse(data_, i, i + matShape.width * gap, gap);
            }

            did = false;
            bool newdid;
            do {
                newdid = false;

                bool curdir = dir;
                for (ptrdiff_t i = start; i < end; i += matShape.width * gap) {
                    bool res = MatrixSortRec(data_, i, i + matShape.width * gap, gap, curdir);
                    newdid = res || newdid;
                    did = did || newdid;
                    curdir = !curdir;
                }

                newdid = false;

                for (ptrdiff_t i = 0; i < matShape.width; i++) {
                    bool res = MatrixSortRec(data_, start + i * gap, end + i * gap, gap * matShape.width, dir);
                    newdid = res || newdid;
                    did = did || newdid;
                }
            } while (newdid);

            for (ptrdiff_t i = start + matShape.width * gap; i < end; i += 2 * matShape.width * gap) {
                MatrixSortGapReverse(data_, i, i + matShape.width * gap, gap);
            }
        }

        return did;
    }

    // MatrixSort.runSort (matrix based sort, idea by Control#2866).
    template<class T = int>
    void MatrixSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        MatrixSortRec(data_, 0, static_cast<ptrdiff_t>(data_.size()), 1, true);
    }

    // ======================= Merge-exchange (Batcher) =====================

    // MergeExchangeSortIterative.runSort (Batcher's merge-exchange network, Knuth's
    // Algorithm M; works for any length).
    template<class T = int>
    void MergeExchangeSortIterative(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        // t = (int)(log(length - 1) / log(2)) + 1; p0 = 1 << (t - 1):
        // the largest power of two strictly less than length.
        ptrdiff_t p0 = 1;
        for (; p0 * 2 < length; p0 *= 2);

        for (ptrdiff_t p = p0; p > 0; p >>= 1) {
            ptrdiff_t q = p0;
            ptrdiff_t r = 0;
            ptrdiff_t d = p;

            while (true) {
                for (ptrdiff_t i = 0; i < length - d; i++) {
                    if ((i & p) == r && data_[i] > data_[i + d]) {
                        using std::swap;
                        swap(data_[i], data_[i + d]);
                    }
                }
                if (q == p) break;
                d = q - p;
                q >>= 1;
                r = p;
            }
        }
    }

    // ====================== Odd-even merge family =========================

    // OddEvenMergeSortIterative.runSort (wkpark's version, rewritten by Piotr
    // Grochowski so that it also works for lengths that are not powers of two).
    template<class T = int>
    void OddEvenMergeSortIterative(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t sortLength = static_cast<ptrdiff_t>(data_.size());

        for (ptrdiff_t p = 1; p < sortLength; p += p) {
            for (ptrdiff_t k = p; k > 0; k /= 2) {
                for (ptrdiff_t j = k % p; j + k < sortLength; j += k + k) {
                    for (ptrdiff_t i = 0; i < k; i++) {
                        if ((i + j) / (p + p) == (i + j + k) / (p + p)) {
                            if (i + j + k < sortLength) {
                                if (data_[i + j] > data_[i + j + k]) {
                                    using std::swap;
                                    swap(data_[i + j], data_[i + j + k]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // OddEvenMergeSortParallel.compSwap
    template<class T>
    void OddEvenMergeSortParallelCompSwap(std::vector<T>& data_, ptrdiff_t a, ptrdiff_t b) {
        if (data_[a] > data_[b]) {
            using std::swap;
            swap(data_[a], data_[b]);
        }
    }

    // OddEvenMergeSortParallel.oddEvenMerge (Java OddEvenMerge threads).
    template<class T>
    void OddEvenMergeSortParallelMerge(std::vector<T>& data_, ptrdiff_t lo, ptrdiff_t m2, ptrdiff_t n, ptrdiff_t r) {
        ptrdiff_t m = r * 2;

        if (m < n) {
            bool odd = (n / r) % 2 != 0;
            ptrdiff_t nL = odd ? n + r : n;
            ptrdiff_t nR = odd ? n - r : n;

            std::thread left([&data_, lo, m2, nL, m] {
                OddEvenMergeSortParallelMerge(data_, lo, (m2 + 1) / 2, nL, m);
            });
            std::thread right([&data_, lo, r, m2, nR, m] {
                OddEvenMergeSortParallelMerge(data_, lo + r, m2 / 2, nR, m);
            });

            left.join();
            right.join();

            if (m2 % 2 != 0) {
                for (ptrdiff_t i = lo; i + r < lo + n; i += m) {
                    OddEvenMergeSortParallelCompSwap(data_, i, i + r);
                }
            }
            else {
                for (ptrdiff_t i = lo + r; i + r < lo + n; i += m) {
                    OddEvenMergeSortParallelCompSwap(data_, i, i + r);
                }
            }
        }
        else if (n > r) {
            OddEvenMergeSortParallelCompSwap(data_, lo, lo + r);
        }
    }

    // OddEvenMergeSortParallel.oddEvenMergeSort (Java OddEvenMergeSort threads).
    template<class T>
    void OddEvenMergeSortParallelSort(std::vector<T>& data_, ptrdiff_t lo, ptrdiff_t n) {
        if (n > 1) {
            ptrdiff_t m = n / 2;

            std::thread left([&data_, lo, m] {
                OddEvenMergeSortParallelSort(data_, lo, m);
            });
            std::thread right([&data_, lo, m, n] {
                OddEvenMergeSortParallelSort(data_, lo + m, n - m);
            });

            left.join();
            right.join();

            OddEvenMergeSortParallelMerge(data_, lo, m, n, 1);
        }
    }

    // OddEvenMergeSortParallel.runSort
    template<class T = int>
    void OddEvenMergeSortParallel(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        OddEvenMergeSortParallelSort(data_, 0, static_cast<ptrdiff_t>(data_.size()));
    }

    // OddEvenMergeSortRecursive.oddEvenMergeCompare
    template<class T>
    void OddEvenMergeSortRecursiveCompare(std::vector<T>& data_, ptrdiff_t i, ptrdiff_t j) {
        if (data_[i] > data_[j]) {
            using std::swap;
            swap(data_[i], data_[j]);
        }
    }

    // OddEvenMergeSortRecursive.oddEvenMerge (H.W. Lang's version, rewritten by
    // Piotr Grochowski so that it also works for lengths that are not powers of two).
    template<class T>
    void OddEvenMergeSortRecursiveMerge(std::vector<T>& data_, ptrdiff_t lo, ptrdiff_t m2, ptrdiff_t n, ptrdiff_t r) {
        ptrdiff_t m = r * 2;

        if (m < n) {
            if ((n / r) % 2 != 0) {
                OddEvenMergeSortRecursiveMerge(data_, lo, (m2 + 1) / 2, n + r, m);   // even subsequence
                OddEvenMergeSortRecursiveMerge(data_, lo + r, m2 / 2, n - r, m);     // odd subsequence
            }
            else {
                OddEvenMergeSortRecursiveMerge(data_, lo, (m2 + 1) / 2, n, m);
                OddEvenMergeSortRecursiveMerge(data_, lo + r, m2 / 2, n, m);
            }

            if (m2 % 2 != 0) {
                for (ptrdiff_t i = lo; i + r < lo + n; i += m) {
                    OddEvenMergeSortRecursiveCompare(data_, i, i + r);
                }
            }
            else {
                for (ptrdiff_t i = lo + r; i + r < lo + n; i += m) {
                    OddEvenMergeSortRecursiveCompare(data_, i, i + r);
                }
            }
        }
        else if (n > r) {
            OddEvenMergeSortRecursiveCompare(data_, lo, lo + r);
        }
    }

    // OddEvenMergeSortRecursive.oddEvenMergeSort
    template<class T>
    void OddEvenMergeSortRecursiveSort(std::vector<T>& data_, ptrdiff_t lo, ptrdiff_t n) {
        if (n > 1) {
            ptrdiff_t m = n / 2;
            OddEvenMergeSortRecursiveSort(data_, lo, m);
            OddEvenMergeSortRecursiveSort(data_, lo + m, n - m);
            OddEvenMergeSortRecursiveMerge(data_, lo, m, n, 1);
        }
    }

    // OddEvenMergeSortRecursive.runSort
    template<class T = int>
    void OddEvenMergeSortRecursive(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        OddEvenMergeSortRecursiveSort(data_, 0, static_cast<ptrdiff_t>(data_.size()));
    }

    // ========================= Pairwise family ============================

    // PairwiseMergeSortIterative.runSort (aphitorite; pads to a power of two and
    // guards comparisons with b < end, so any length works).
    template<class T = int>
    void PairwiseMergeSortIterative(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t end = static_cast<ptrdiff_t>(data_.size());

        auto compSwap = [&data_, end](ptrdiff_t a, ptrdiff_t b) {
            if (b < end && data_[a] > data_[b]) {
                using std::swap;
                swap(data_[a], data_[b]);
            }
        };

        ptrdiff_t n = 1;
        for (; n < end; n <<= 1);

        for (ptrdiff_t k = n >> 1; k > 0; k >>= 1) {
            for (ptrdiff_t j = 0; j < end; j += k << 1) {
                for (ptrdiff_t i = 0; i < k; i++) {
                    compSwap(j + i, j + k + i);
                }
            }
        }

        for (ptrdiff_t k = 2; k < n; k <<= 1) {
            for (ptrdiff_t m = k >> 1; m > 0; m >>= 1) {
                for (ptrdiff_t j = 0; j < end; j += k << 1) {
                    for (ptrdiff_t p = m; p < ((k - m) << 1); p += m << 1) {
                        for (ptrdiff_t i = 0; i < m; i++) {
                            compSwap(j + p + i, j + p + m + i);
                        }
                    }
                }
            }
        }
    }

    // PairwiseMergeSortRecursive.pairwiseMerge
    template<class T>
    void PairwiseMergeSortRecursiveMerge(std::vector<T>& data_, ptrdiff_t end, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t m = (a + b) / 2;
        ptrdiff_t m1 = (a + m) / 2;
        ptrdiff_t g = m - m1;

        for (ptrdiff_t i = 0; m1 + i < m; i++) {
            for (ptrdiff_t j = m1, k = g; k > 0; k >>= 1, j -= k - (i & k)) {
                if (j + i + k < end && data_[j + i] > data_[j + i + k]) {
                    using std::swap;
                    swap(data_[j + i], data_[j + i + k]);
                }
            }
        }

        if (b - a > 4) {
            PairwiseMergeSortRecursiveMerge(data_, end, m, b);
        }
    }

    // PairwiseMergeSortRecursive.pairwiseMergeSort
    template<class T>
    void PairwiseMergeSortRecursiveSort(std::vector<T>& data_, ptrdiff_t end, ptrdiff_t a, ptrdiff_t b) {
        ptrdiff_t m = (a + b) / 2;

        for (ptrdiff_t i = a, j = m; i < m; i++, j++) {
            if (j < end && data_[i] > data_[j]) {
                using std::swap;
                swap(data_[i], data_[j]);
            }
        }

        if (b - a > 2) {
            PairwiseMergeSortRecursiveSort(data_, end, a, m);
            PairwiseMergeSortRecursiveSort(data_, end, m, b);
            PairwiseMergeSortRecursiveMerge(data_, end, a, b);
        }
    }

    // PairwiseMergeSortRecursive.runSort (sorts the padded power-of-two range, the
    // comparisons past the real end are skipped, so any length works).
    template<class T = int>
    void PairwiseMergeSortRecursive(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t end = static_cast<ptrdiff_t>(data_.size());

        ptrdiff_t n = 1;
        for (; n < end; n <<= 1);

        PairwiseMergeSortRecursiveSort(data_, end, 0, n);
    }

    // PairwiseSortIterative.runSort (Piotr Grochowski's iterative pairwise network;
    // the Java arguments b, c, d, e are only loop state).
    template<class T = int>
    void PairwiseSortIterative(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        auto compSwap = [&data_](ptrdiff_t a, ptrdiff_t b) {
            if (data_[a] > data_[b]) {
                using std::swap;
                swap(data_[a], data_[b]);
            }
        };

        ptrdiff_t a = 1;
        ptrdiff_t b = 0;
        ptrdiff_t c = 0;
        ptrdiff_t d = 0;
        ptrdiff_t e = 0;

        while (a < length) {
            b = a;
            c = 0;
            while (b < length) {
                compSwap(b - a, b);
                c = (c + 1) % a;
                b++;
                if (c == 0) {
                    b += a;
                }
            }
            a *= 2;
        }

        a /= 4;
        e = 1;

        while (a > 0) {
            d = e;
            while (d > 0) {
                b = ((d + 1) * a);
                c = 0;
                while (b < length) {
                    compSwap(b - (d * a), b);
                    c = (c + 1) % a;
                    b++;
                    if (c == 0) {
                        b += a;
                    }
                }
                d /= 2;
            }
            a /= 2;
            e = (e * 2) + 1;
        }
    }

    // PairwiseSortRecursive.pairwiserecursive
    template<class T>
    void PairwiseSortRecursiveRec(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t end, ptrdiff_t gap) {
        if (start == end - gap) {
            return;
        }

        ptrdiff_t b = start + gap;
        while (b < end) {
            if (data_[b - gap] > data_[b]) {
                using std::swap;
                swap(data_[b - gap], data_[b]);
            }
            b += (2 * gap);
        }

        if (((end - start) / gap) % 2 == 0) {
            PairwiseSortRecursiveRec(data_, start, end, gap * 2);
            PairwiseSortRecursiveRec(data_, start + gap, end + gap, gap * 2);
        }
        else {
            PairwiseSortRecursiveRec(data_, start, end + gap, gap * 2);
            PairwiseSortRecursiveRec(data_, start + gap, end, gap * 2);
        }

        ptrdiff_t a = 1;
        while (a < ((end - start) / gap)) {
            a = (a * 2) + 1;
        }

        b = start + gap;
        while (b + gap < end) {
            ptrdiff_t c = a;
            while (c > 1) {
                c /= 2;
                if (b + (c * gap) < end) {
                    if (data_[b] > data_[b + (c * gap)]) {
                        using std::swap;
                        swap(data_[b], data_[b + (c * gap)]);
                    }
                }
            }
            b += (2 * gap);
        }
    }

    // PairwiseSortRecursive.runSort
    template<class T = int>
    void PairwiseSortRecursive(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        PairwiseSortRecursiveRec(data_, 0, static_cast<ptrdiff_t>(data_.size()), 1);
    }

    // ========================== Weave family ==============================

    // WeaveSortIterative.runSort (aphitorite; runs on the padded power-of-two size
    // and skips comparisons past the real end, so any length works).
    template<class T = int>
    void WeaveSortIterative(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t end = static_cast<ptrdiff_t>(data_.size());

        auto compSwap = [&data_, end](ptrdiff_t a, ptrdiff_t b) {
            if (b < end && data_[a] > data_[b]) {
                using std::swap;
                swap(data_[a], data_[b]);
            }
        };

        ptrdiff_t n = 1;
        for (; n < end; n *= 2);

        for (ptrdiff_t i = 1; i < n; i *= 2) {
            for (ptrdiff_t j = 1; j <= i; j *= 2) {
                for (ptrdiff_t k = 0; k < n; k += n / j) {
                    ptrdiff_t d = n / i / 2;
                    ptrdiff_t m = 0;
                    for (ptrdiff_t l = n / j - d; l >= n / j / 2; l -= d) {
                        for (ptrdiff_t p = 0; p < d; p++, m++) {
                            compSwap(k + m, k + l + p);
                        }
                    }
                }
            }
        }
    }

    // WeaveSortParallel.step
    template<class T>
    void WeaveSortParallelStep(std::vector<T>& data_, ptrdiff_t x, ptrdiff_t y) {
        if (data_[x] > data_[y]) {
            using std::swap;
            swap(data_[x], data_[y]);
        }
    }

    // WeaveSortParallel.circle (Java CircleThread).
    template<class T>
    void WeaveSortParallelCircle(std::vector<T>& data_, ptrdiff_t start, ptrdiff_t stop, ptrdiff_t gap) {
        if ((stop - start) / gap >= 1) {
            ptrdiff_t left = start;
            ptrdiff_t right = stop;

            while (left < right) {
                WeaveSortParallelStep(data_, left, right);
                left += gap;
                right -= gap;
            }

            std::thread leftT([&data_, start, right, gap] {
                WeaveSortParallelCircle(data_, start, right, gap);
            });
            std::thread rightT([&data_, left, stop, gap] {
                WeaveSortParallelCircle(data_, left, stop, gap);
            });

            leftT.join();
            rightT.join();
        }
    }

    // WeaveSortParallel.wrapper (Java SortThread).
    template<class T>
    void WeaveSortParallelWrapper(std::vector<T>& data_, ptrdiff_t length, ptrdiff_t start, ptrdiff_t gap) {
        if (gap < length) {
            std::thread left([&data_, length, start, gap] {
                WeaveSortParallelWrapper(data_, length, start, gap * 2);
            });
            std::thread right([&data_, length, start, gap] {
                WeaveSortParallelWrapper(data_, length, start + gap, gap * 2);
            });

            left.join();
            right.join();

            WeaveSortParallelCircle(data_, start, length - gap + start, gap);
        }
    }

    // WeaveSortParallel.runSort (parallel weave; requires a power-of-two length).
    template<class T = int>
    void WeaveSortParallel(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        WeaveSortParallelWrapper(data_, static_cast<ptrdiff_t>(data_.size()), 0, 1);
    }

    // WeaveSortRecursive.circle (compSwap skips indices past the real end).
    template<class T>
    void WeaveSortRecursiveCircle(std::vector<T>& data_, ptrdiff_t end, ptrdiff_t pos, ptrdiff_t len, ptrdiff_t gap) {
        if (len < 2) return;

        for (ptrdiff_t i = 0; 2 * i < (len - 1) * gap; i += gap) {
            ptrdiff_t a = pos + i;
            ptrdiff_t b = pos + (len - 1) * gap - i;

            if (b < end && data_[a] > data_[b]) {
                using std::swap;
                swap(data_[a], data_[b]);
            }
        }

        WeaveSortRecursiveCircle(data_, end, pos, len / 2, gap);
        if (pos + len * gap / 2 < end) {
            WeaveSortRecursiveCircle(data_, end, pos + len * gap / 2, len / 2, gap);
        }
    }

    // WeaveSortRecursive.weaveCircle
    template<class T>
    void WeaveSortRecursiveWeaveCircle(std::vector<T>& data_, ptrdiff_t end, ptrdiff_t pos, ptrdiff_t len, ptrdiff_t gap) {
        if (len < 2) return;

        WeaveSortRecursiveWeaveCircle(data_, end, pos, len / 2, 2 * gap);
        WeaveSortRecursiveWeaveCircle(data_, end, pos + gap, len / 2, 2 * gap);

        WeaveSortRecursiveCircle(data_, end, pos, len, gap);
    }

    // WeaveSortRecursive.runSort (aphitorite; runs on the padded power-of-two size
    // and skips comparisons past the real end, so any length works).
    template<class T = int>
    void WeaveSortRecursive(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t end = static_cast<ptrdiff_t>(data_.size());

        ptrdiff_t n = 1;
        for (; n < end; n *= 2);

        WeaveSortRecursiveWeaveCircle(data_, end, 0, n, 1);
    }

} // namespace NVisualSort::NSortAlgorithms
