#pragma once
// Ports of the ArrayV (Java, MIT, https://github.com/Gaming32/ArrayV) "Selection Sorts"
// category. VisualSort - selection / cycle / tournament / heap-family sorts.
//
// ASCII ONLY: do not put non-ASCII characters in this file. MSVC parses sources in the
// system code page (GBK); Chinese text appears only as \uXXXX escapes inside wide strings
// (WideError messages).
//
// This header is #included from Sort.h right after SortHelpers.h, so Strip.h / Counter.h /
// ConfigManager.h / WideError.h and the common std headers are already available; the
// includes below only keep the file self-documenting.
//
// Porting conventions (see helper_spec.md):
//   Reads.compareValues(a, b)        -> plain operators / NSortHelpers::CompareValues
//   Reads.compareIndices(array, i, j)-> data_[i] < data_[j]
//   Writes.swap(array, i, j, ...)    -> using std::swap; swap(data_[i], data_[j]);
//   Writes.write(array, i, v, ...)   -> data_[i] = v;
//   Writes.createExternalArray(n)    -> std::vector<T> name(n);  (std::vector<int> for
//                                       flag/index arrays; std::vector<ptrdiff_t> for indices)
//   Highlights.markArray(k, i)       -> NSortHelpers::MarkArray(k, data_, i);
//   Delays.sleep(x) / clearMark      -> omitted
//   ThreadLocalRandom                -> std::mt19937 (not used by this category)
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

namespace NVisualSort::NSortAlgorithms {

    // Asynchronous Sort: port of select.AsynchronousSort.runSort.
    // Distribution-like selection sort; the outer loop runs once per distinct value, so a
    // cheap range guard is added for raw int data (Java would simply run for a very long
    // time). The trailing InsertionSort.customInsertSort(array, 0, cur) uses the value
    // counter "cur" as an end index in Java (the Java array is padded); the range is clamped
    // to the real array size here.
    template<class T = int>
    void AsynchronousSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        std::vector<T> ext(static_cast<size_t>(dataSize));
        int minValue = static_cast<int>(data_[0]);
        int maxValue = static_cast<int>(data_[0]);
        for (ptrdiff_t i = 0; i < dataSize; ++i) {
            ext[i] = data_[i];
            if (static_cast<int>(data_[i]) < minValue) minValue = static_cast<int>(data_[i]);
            if (static_cast<int>(data_[i]) > maxValue) maxValue = static_cast<int>(data_[i]);
        }

        if constexpr (std::is_same_v<T, int>) {
            if (static_cast<long long>(maxValue) - minValue + 1 > static_cast<long long>(dataSize)) {
                // "data value range is too large for Asynchronous Sort"
                throw WideError(L"\u6570\u636e\u53d6\u503c\u8303\u56f4\u8fc7\u5927\uff0c\u4e0d\u9002\u5408\u4f7f\u7528\u5f02\u6b65\u6392\u5e8f\uff01");
            }
        }
        ++maxValue;

        int cur = minValue;
        ptrdiff_t i = 0;
        while (i < dataSize) {
            for (ptrdiff_t j = 0; j < dataSize; ++j) {
                NSortHelpers::MarkArray(2, data_, j);
                if (static_cast<int>(ext[j]) <= cur) {
                    data_[i] = ext[j];
                    ext[j] = maxValue;
                    ++i;
                }
            }
            ++cur;
        }

        NSortHelpers::insertionSort(data_, 0, (std::min)(static_cast<ptrdiff_t>(cur), dataSize));
    }

    // Bad Sort: port of select.BadSort.runSort (O(n^3) joke sort by James Jensen).
    template<class T = int>
    void BadSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t currentLen = static_cast<ptrdiff_t>(data_.size());

        for (ptrdiff_t i = 0; i < currentLen; ++i) {
            ptrdiff_t shortest = i;

            for (ptrdiff_t j = i; j < currentLen; ++j) {
                NSortHelpers::MarkArray(1, data_, j);

                bool isShortest = true;
                for (ptrdiff_t k = j + 1; k < currentLen; ++k) {
                    NSortHelpers::MarkArray(2, data_, k);
                    if (NSortHelpers::CompareValues(data_[j], data_[k]) == 1) {
                        isShortest = false;
                        break;
                    }
                }
                if (isShortest) {
                    shortest = j;
                    break;
                }
            }
            using std::swap;
            swap(data_[i], data_[shortest]);
        }
    }

    // Base-N Max Heap Sort: port of select.BaseNMaxHeapSort.runSort.
    // The Java sort asks the user for a base (@SortMeta defaultAnswer = 4); the default
    // base 4 is used here.
    template<class T = int>
    void BaseNMaxHeapSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());
        constexpr ptrdiff_t base = 4;

        // BaseNMaxHeapSort.siftDown (recursion follows one child chain: O(log n) deep).
        auto siftDown = [&](auto&& self, ptrdiff_t node, ptrdiff_t stop) -> void {
            ptrdiff_t left = node * base + 1;
            if (left < stop) {
                ptrdiff_t maxIndex = left;
                for (ptrdiff_t i = left + 1; i < left + base; ++i) {
                    if (i >= stop) break;
                    if (data_[maxIndex] < data_[i]) maxIndex = i;
                }
                if (data_[node] < data_[maxIndex]) {
                    using std::swap;
                    swap(data_[node], data_[maxIndex]);
                    self(self, maxIndex, stop);
                }
            }
        };

        for (ptrdiff_t i = dataSize - 1; i > -1; --i) {
            siftDown(siftDown, i, dataSize);
        }
        for (ptrdiff_t i = dataSize - 1; i > 0; --i) {
            using std::swap;
            swap(data_[0], data_[i]);
            siftDown(siftDown, 0, i);
        }
    }

    // Bingo Sort: port of select.BingoSort.runSort. Java feeds element VALUES to
    // Highlights.markArray as indices; MarkArray is bounds-guarded, so that is kept.
    template<class T = int>
    void BingoSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        ptrdiff_t maximum = dataSize - 1;
        T next = data_[maximum];

        for (ptrdiff_t i = maximum - 1; i >= 0; --i) {
            if (data_[i] > next) {
                next = data_[i];
            }
        }
        while (maximum > 0 && data_[maximum] == next) {
            --maximum;
        }
        while (maximum > 0) {
            T val = next;
            next = data_[maximum];

            for (ptrdiff_t j = maximum - 1; j >= 0; --j) {
                NSortHelpers::MarkArray(1, data_, static_cast<ptrdiff_t>(static_cast<int>(data_[j])));
                NSortHelpers::MarkArray(2, data_, static_cast<ptrdiff_t>(static_cast<int>(val)));

                if (NSortHelpers::CompareValues(data_[j], val) == 0) {
                    using std::swap;
                    swap(data_[j], data_[maximum]);
                    --maximum;
                }
                else {
                    if (data_[j] > next) {
                        next = data_[j];
                    }
                }
            }
            while (maximum > 0 && data_[maximum] == next) {
                --maximum;
            }
        }
    }

    // Binomial Heap Sort: port of select.BinomialHeapSort.runSort (implicit binomial heap).
    template<class T = int>
    void BinomialHeapSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        ptrdiff_t maxNode, focus, index, depth;
        for (index = 2; index <= dataSize; index += 2) {
            maxNode = index;
            do {
                focus = maxNode;
                for (depth = 1; (focus & depth) == 0; depth *= 2) {
                    if (data_[focus - depth - 1] > data_[maxNode - 1])
                        maxNode = focus - depth;
                }
                if (focus != maxNode) {
                    using std::swap;
                    swap(data_[focus - 1], data_[maxNode - 1]);
                }
            } while (focus != maxNode);
        }
        for (index = dataSize; index > 2; --index) {
            maxNode = index;
            focus = index;
            for (depth = 1; focus != 0; depth *= 2) {
                if ((focus & depth) != 0) {
                    if (data_[focus - 1] > data_[maxNode - 1])
                        maxNode = focus;
                    focus -= depth;
                }
            }
            if (maxNode != index) {
                focus = index;
                do {
                    using std::swap;
                    swap(data_[focus - 1], data_[maxNode - 1]);
                    focus = maxNode;
                    for (depth = 1; (focus & depth) == 0; depth *= 2) {
                        if (data_[focus - depth - 1] > data_[maxNode - 1])
                            maxNode = focus - depth;
                    }
                } while (focus != maxNode);
            }
        }
    }

    // Binomial Smooth Sort: port of select.BinomialSmoothSort.runSort.
    // thrift() recurses along one heap path (O(log n) deep), so recursion is kept.
    template<class T = int>
    void BinomialSmoothSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        // BinomialSmoothSort.height
        auto height = [](ptrdiff_t node) {
            ptrdiff_t count = 0;
            while (((node >> count) & 1) == 1) ++count;
            return count;
        };

        // BinomialSmoothSort.thrift
        auto thrift = [&](auto&& self, ptrdiff_t node, bool parent, bool root) -> void {
            root = root && (node >= (static_cast<ptrdiff_t>(1) << height(node)));
            if (!root && !parent) return;

            ptrdiff_t choice = height(node) - (root ? 0 : 1);
            if (parent) {
                for (ptrdiff_t child = choice - 1; child >= 0; --child) {
                    if (data_[node - (static_cast<ptrdiff_t>(1) << choice)] <= data_[node - (static_cast<ptrdiff_t>(1) << child)])
                        choice = child;
                }
            }
            if (data_[node - (static_cast<ptrdiff_t>(1) << choice)] <= data_[node]) return;

            using std::swap;
            swap(data_[node], data_[node - (static_cast<ptrdiff_t>(1) << choice)]);
            ptrdiff_t child = node - (static_cast<ptrdiff_t>(1) << choice);
            self(self, child, child % 2 == 1, choice == height(node));
        };

        ptrdiff_t node;
        for (node = 1; node < dataSize; ++node) {
            thrift(thrift, node, node % 2 == 1, (node + (static_cast<ptrdiff_t>(1) << height(node)) >= dataSize));
        }
        for (node -= (node - 1) % 2; node > 2; node -= 2) {
            for (ptrdiff_t child = height(node) - 1; child >= 0; --child) {
                thrift(thrift, node - (static_cast<ptrdiff_t>(1) << child), false, true);
            }
        }
    }

    // Bottom-up Heap Sort: port of select.BottomUpHeapSort.runSort.
    template<class T = int>
    void BottomUpHeapSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        // BottomUpHeapSort.siftDown
        auto siftDown = [&](ptrdiff_t i, ptrdiff_t b) {
            ptrdiff_t j = i;
            while (2 * j + 1 < b) {
                if (2 * j + 2 < b) {
                    j = data_[2 * j + 2] > data_[2 * j + 1] ? 2 * j + 2 : 2 * j + 1;
                }
                else {
                    j = 2 * j + 1;
                }
            }
            while (data_[i] > data_[j]) {
                j = (j - 1) / 2;
            }
            while (j > i) {
                using std::swap;
                swap(data_[i], data_[j]);
                j = (j - 1) / 2;
            }
        };

        for (ptrdiff_t i = (dataSize - 1) / 2; i >= 0; --i) {
            siftDown(i, dataSize);
        }
        for (ptrdiff_t i = dataSize - 1; i > 0; --i) {
            using std::swap;
            swap(data_[0], data_[i]);
            siftDown(0, i);
        }
    }

    // Classic Tournament Sort: port of select.ClassicTournamentSort.runSort.
    // The tree stores index-values; -1 means "no player". Java encodes the child state with
    // (tree[i] >> 31) masks - ported with explicit sign masks since ptrdiff_t is 64-bit.
    template<class T = int>
    void ClassicTournamentSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        ptrdiff_t pow2 = 1;
        while (pow2 < dataSize) pow2 *= 2;
        ptrdiff_t size = pow2 - 1;
        ptrdiff_t mod = dataSize & 1;
        ptrdiff_t treeSize = dataSize + size + mod;

        std::vector<ptrdiff_t> tree(static_cast<size_t>(treeSize), -1);

        // ClassicTournamentSort.treeCompare
        auto treeCompare = [&](ptrdiff_t a, ptrdiff_t b) {
            return data_[tree[a]] <= data_[tree[b]];
        };

        // ClassicTournamentSort.buildTree
        for (ptrdiff_t i = size; i < treeSize - mod; ++i) {
            NSortHelpers::MarkArray(1, data_, i - size);
            tree[i] = i - size;
        }
        for (ptrdiff_t j = size, k = treeSize - mod; j > 0; j /= 2, k /= 2) {
            ptrdiff_t i;
            for (i = j; i + 1 < k; i += 2) {
                ptrdiff_t val = treeCompare(i, i + 1) ? tree[i] : tree[i + 1];
                tree[i / 2] = val;
            }
            if (i < k) tree[i / 2] = tree[i];
        }

        // ClassicTournamentSort.peek
        auto peek = [&]() -> T { return data_[tree[0]]; };

        // ClassicTournamentSort.findNext
        auto findNext = [&]() -> T {
            ptrdiff_t root = tree[0] + size;

            for (ptrdiff_t i = root; i > 0; i = (i - 1) / 2) {
                tree[i] = -1;
            }

            for (ptrdiff_t i = root; i > 0;) {
                ptrdiff_t j = i + ((i & 1) << 1) - 1;

                ptrdiff_t c1 = tree[i] < 0 ? -1 : 0; // Java: this.tree[i] >> 31
                ptrdiff_t c2 = tree[j] < 0 ? -1 : 0; // Java: this.tree[j] >> 31

                ptrdiff_t nVal = (c1 & ((c2 & -1) + (~c2 & tree[j]))) + (~c1 & ((c2 & tree[i]) + (~c2 & -2)));

                if (nVal == -2) {
                    if (i < j) nVal = treeCompare(i, j) ? tree[i] : tree[j];
                    else       nVal = treeCompare(j, i) ? tree[j] : tree[i];
                }

                i = (i - 1) / 2;
                if (nVal != -1) tree[i] = nVal;
            }

            return peek();
        };

        std::vector<T> tmp(static_cast<size_t>(dataSize));

        NSortHelpers::MarkArray(3, data_, 0);
        tmp[0] = peek();

        for (ptrdiff_t i = 1; i < dataSize; ++i) {
            T val = findNext();

            NSortHelpers::MarkArray(3, data_, i);
            tmp[i] = val;
        }
        for (ptrdiff_t i = 0; i < dataSize; ++i) {
            data_[i] = tmp[i];
        }
    }

    // Cycle Sort (ArrayV): port of select.CycleSort.runSort.
    // Renamed to CycleSortJava: the C++ namespace already has a CycleSort from Sort.h.
    template<class T = int>
    void CycleSortJava(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        // CycleSort.countLesser
        auto countLesser = [&](ptrdiff_t a, ptrdiff_t b, const T& t) {
            ptrdiff_t r = a;

            for (ptrdiff_t i = a + 1; i < b; ++i) {
                NSortHelpers::MarkArray(1, data_, r);
                NSortHelpers::MarkArray(2, data_, i);

                r += NSortHelpers::CompareValues(data_[i], t) < 0 ? 1 : 0;
            }
            return r;
        };

        for (ptrdiff_t i = 0; i < dataSize - 1; ++i) {
            NSortHelpers::MarkArray(3, data_, i);

            T t = data_[i];
            ptrdiff_t r = countLesser(i, dataSize, t);

            if (r != i) {
                do {
                    while (NSortHelpers::CompareValues(data_[r], t) == 0) ++r;

                    T t1 = data_[r];
                    data_[r] = t;
                    t = t1;

                    r = countLesser(i, dataSize, t);
                } while (r != i);

                data_[i] = t;
            }
        }
    }

    // Double Selection Sort: port of select.DoubleSelectionSort.runSort.
    template<class T = int>
    void DoubleSelectionSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        ptrdiff_t left = 0;
        ptrdiff_t right = dataSize - 1;
        ptrdiff_t smallest = 0;
        ptrdiff_t biggest = 0;

        while (left <= right) {
            for (ptrdiff_t i = left; i <= right; ++i) {
                NSortHelpers::MarkArray(3, data_, i);

                if (NSortHelpers::CompareValues(data_[i], data_[biggest]) == 1) {
                    biggest = i;
                    NSortHelpers::MarkArray(1, data_, biggest);
                }
                if (NSortHelpers::CompareValues(data_[i], data_[smallest]) == -1) {
                    smallest = i;
                    NSortHelpers::MarkArray(2, data_, smallest);
                }
            }
            if (biggest == left)
                biggest = smallest;

            using std::swap;
            swap(data_[left], data_[smallest]);
            swap(data_[right], data_[biggest]);

            ++left;
            --right;

            smallest = left;
            biggest = right;
        }
    }

    // Flipped Min Heap Sort: port of select.FlippedMinHeapSort.runSort.
    // The heap is built on the reversed index space (index -> length - index).
    template<class T = int>
    void FlippedMinHeapSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        // FlippedMinHeapSort.siftDown
        auto siftDown = [&](ptrdiff_t root, ptrdiff_t dist) {
            while (root <= dist / 2) {
                ptrdiff_t leaf = 2 * root;
                if (leaf < dist && data_[length - leaf] > data_[length - leaf - 1]) {
                    ++leaf;
                }
                NSortHelpers::MarkArray(1, data_, length - root);
                NSortHelpers::MarkArray(2, data_, length - leaf);
                if (data_[length - root] > data_[length - leaf]) {
                    using std::swap;
                    swap(data_[length - root], data_[length - leaf]);
                    root = leaf;
                }
                else break;
            }
        };

        for (ptrdiff_t i = length / 2; i >= 1; --i) {
            siftDown(i, length);
        }
        for (ptrdiff_t i = length; i > 1; --i) {
            using std::swap;
            swap(data_[length - 1], data_[length - i]);
            siftDown(1, i - 1);
        }
    }

    // Lazy Heap Sort: port of select.LazyHeapSort.runSort (sqrt-blocked lazy heap).
    template<class T = int>
    void LazyHeapSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        // LazyHeapSort.maxToFront
        auto maxToFront = [&](ptrdiff_t a, ptrdiff_t b) {
            ptrdiff_t max = a;

            for (ptrdiff_t i = a + 1; i < b; ++i)
                if (data_[i] > data_[max])
                    max = i;

            using std::swap;
            swap(data_[max], data_[a]);
        };

        ptrdiff_t s = static_cast<ptrdiff_t>(std::sqrt(static_cast<double>(length - 1))) + 1;

        for (ptrdiff_t i = 0; i < length; i += s) {
            maxToFront(i, (std::min)(i + s, length));
        }

        for (ptrdiff_t j = length; j > 0;) {
            ptrdiff_t max = 0;

            for (ptrdiff_t i = max + s; i < j; i += s)
                if (data_[i] >= data_[max])
                    max = i;

            --j;
            using std::swap;
            swap(data_[max], data_[j]);
            maxToFront(max, (std::min)(max + s, j));
        }
    }

    // Max Heap Sort: port of select.MaxHeapSort.runSort (HeapSorting.heapSort, max heap).
    template<class T = int>
    void MaxHeapSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        NSortHelpers::heapSort(data_, 0, dataSize, true);
    }

    // Min Heap Sort: port of select.MinHeapSort.runSort (HeapSorting.heapSort, min heap).
    template<class T = int>
    void MinHeapSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        NSortHelpers::heapSort(data_, 0, dataSize, false);
    }

    // Min-Max Heap Sort: port of select.MinMaxHeapSort.runSort
    // (translated by BartMassey, https://github.com/BartMassey/minmaxheap).
    // Java's is_min_level uses Integer.numberOfLeadingZeros; the equivalent bit length of
    // (index - start + 1) is used here.
    template<class T = int>
    void MinMaxHeapSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t start = 0;
        ptrdiff_t end = static_cast<ptrdiff_t>(data_.size());

        // MinMaxHeapSort.compare
        auto compare = [&](const T& x_, const T& y_, bool isGt) {
            if (isGt) return y_ < x_;
            return x_ < y_;
        };
        auto swapAt = [&](ptrdiff_t i, ptrdiff_t j) {
            using std::swap;
            swap(data_[i], data_[j]);
        };
        // MinMaxHeapSort.is_min_level
        auto is_min_level = [&](ptrdiff_t index) {
            index = index - start + 1;
            int bitLen = 0;
            for (ptrdiff_t v = index; v > 0; v >>= 1) ++bitLen;
            return (bitLen & 1) == 1;
        };
        // MinMaxHeapSort.downheap
        auto downheap = [&](auto&& self, ptrdiff_t i) -> void {
            bool cf = !is_min_level(i);
            ptrdiff_t left = 2 * i + 1;

            while (left < end) {
                ptrdiff_t right = left + 1;
                ptrdiff_t nexti = left;
                const ptrdiff_t candidates[5] = { right, 2 * left + 1, 2 * left + 2, 2 * right + 1, 2 * right + 2 };
                for (int ci = 0; ci < 5; ++ci) {
                    ptrdiff_t c = candidates[ci];
                    if (c >= end) {
                        break;
                    }
                    if (compare(data_[c], data_[nexti], cf)) {
                        nexti = c;
                    }
                }
                if (nexti <= right) {
                    if (compare(data_[nexti], data_[i], cf)) {
                        swapAt(nexti, i);
                    }
                    return;
                }
                else {
                    if (compare(data_[nexti], data_[i], cf)) {
                        swapAt(nexti, i);
                        ptrdiff_t parent = (nexti - 1) / 2;
                        if (compare(data_[parent], data_[nexti], cf)) {
                            swapAt(nexti, parent);
                        }
                    }
                    else {
                        return;
                    }
                }
                i = nexti;
                left = 2 * i + 1;
            }
        };
        // MinMaxHeapSort.heapify
        auto heapify = [&]() {
            for (ptrdiff_t i = (end - 1) / 2; i >= start; --i) {
                downheap(downheap, i);
            }
        };
        // MinMaxHeapSort.store_max
        auto store_max = [&]() {
            if (end <= start + 1) {
                return;
            }
            ptrdiff_t imax = start + 1;
            if (end > imax + 1 && compare(data_[imax], data_[imax + 1], false)) {
                ++imax;
            }
            --end;
            swapAt(imax, end);
            if (imax < end) {
                downheap(downheap, imax);
            }
        };

        heapify();
        for (ptrdiff_t i = end - 1; i > start; --i) {
            store_max();
        }
    }

    // Out-of-Place Heap Sort: port of select.OutOfPlaceHeapSort.runSort.
    // Uses -1 as an "already extracted" sentinel in the array (as Java does).
    template<class T = int>
    void OutOfPlaceHeapSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t length = static_cast<ptrdiff_t>(data_.size());

        // OutOfPlaceHeapSort.siftDown (bottom-up heapify path)
        auto siftDown = [&](ptrdiff_t i, ptrdiff_t b) {
            ptrdiff_t j = i;
            while (2 * j + 1 < b) {
                if (2 * j + 2 < b) {
                    j = data_[2 * j + 2] > data_[2 * j + 1] ? 2 * j + 2 : 2 * j + 1;
                }
                else {
                    j = 2 * j + 1;
                }
            }
            while (data_[i] > data_[j]) {
                j = (j - 1) / 2;
            }
            while (j > i) {
                using std::swap;
                swap(data_[i], data_[j]);
                j = (j - 1) / 2;
            }
        };
        // OutOfPlaceHeapSort.findNext
        auto findNext = [&](ptrdiff_t b) {
            ptrdiff_t i = 0;
            ptrdiff_t l = 1;
            ptrdiff_t r = 2;

            while (r < b && !(data_[l] == -1 && data_[r] == -1)) {
                using std::swap;
                if (data_[l] == -1) {
                    swap(data_[i], data_[r]);
                    i = r;
                }
                else if (data_[r] == -1) {
                    swap(data_[i], data_[l]);
                    i = l;
                }
                else if (data_[r] > data_[l]) {
                    swap(data_[i], data_[r]);
                    i = r;
                }
                else {
                    swap(data_[i], data_[l]);
                    i = l;
                }
                l = 2 * i + 1;
                r = l + 1;
            }
            if (l < b && data_[l] != -1) {
                using std::swap;
                swap(data_[i], data_[l]);
            }
        };

        for (ptrdiff_t i = (length - 1) / 2; i >= 0; --i) {
            siftDown(i, length);
        }

        std::vector<T> tmp(static_cast<size_t>(length));

        for (ptrdiff_t i = length - 1; i >= 0; --i) {
            tmp[i] = data_[0];
            data_[0] = -1;

            findNext(length);
        }
        for (ptrdiff_t i = 0; i < length; ++i) {
            data_[i] = tmp[i];
        }
    }

    // Poplar Heap Sort: port of select.PoplarHeapSort.runSort (Morwenn's poplar heap).
    // hyperfloor is implemented with integer arithmetic (Java used Math.pow/Math.log).
    template<class T = int>
    void PoplarHeapSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        // PoplarHeapSort.hyperfloor: returns 2^floor(log2(n)), assumes n > 0
        auto hyperfloor = [](ptrdiff_t n) {
            ptrdiff_t p = 1;
            while (p <= n / 2) p *= 2;
            return p;
        };
        // PoplarHeapSort.unchecked_insertion_sort
        auto unchecked_insertion_sort = [&](ptrdiff_t first, ptrdiff_t last) {
            for (ptrdiff_t cur = first + 1; cur != last; ++cur) {
                ptrdiff_t sift = cur;
                ptrdiff_t sift_1 = cur - 1;

                if (data_[sift] < data_[sift_1]) {
                    T tmp = data_[sift];
                    while (true) {
                        data_[sift] = data_[sift_1];
                        --sift;
                        if (sift == first) break;
                        --sift_1;
                        if (!(tmp < data_[sift_1])) break;
                    }
                    data_[sift] = tmp;
                }
            }
        };
        // PoplarHeapSort.insertion_sort
        auto insertion_sort = [&](ptrdiff_t first, ptrdiff_t last) {
            if (first == last) return;
            unchecked_insertion_sort(first, last);
        };
        // PoplarHeapSort.sift
        auto sift = [&](ptrdiff_t first, ptrdiff_t size) {
            if (size < 2) return;

            ptrdiff_t root = first + (size - 1);
            ptrdiff_t child_root1 = root - 1;
            ptrdiff_t child_root2 = first + (size / 2 - 1);

            while (true) {
                ptrdiff_t max_root = root;
                if (data_[max_root] < data_[child_root1]) {
                    max_root = child_root1;
                }
                if (data_[max_root] < data_[child_root2]) {
                    max_root = child_root2;
                }
                if (max_root == root) return;

                using std::swap;
                swap(data_[root], data_[max_root]);

                size /= 2;
                if (size < 2) return;

                root = max_root;
                child_root1 = root - 1;
                child_root2 = max_root - (size - size / 2);
            }
        };
        // PoplarHeapSort.pop_heap_with_size
        auto pop_heap_with_size = [&](ptrdiff_t first, ptrdiff_t last, ptrdiff_t size) {
            ptrdiff_t poplar_size = hyperfloor(size + 1) - 1;
            ptrdiff_t last_root = last - 1;
            ptrdiff_t bigger = last_root;
            ptrdiff_t bigger_size = poplar_size;

            ptrdiff_t it = first;
            while (true) {
                ptrdiff_t root = it + poplar_size - 1;
                if (root == last_root) break;
                if (data_[bigger] < data_[root]) {
                    bigger = root;
                    bigger_size = poplar_size;
                }
                it = root + 1;

                size -= poplar_size;
                poplar_size = hyperfloor(size + 1) - 1;
            }

            if (bigger != last_root) {
                using std::swap;
                swap(data_[bigger], data_[last_root]);
                sift(bigger - (bigger_size - 1), bigger_size);
            }
        };
        // PoplarHeapSort.make_heap
        auto make_heap = [&](ptrdiff_t first, ptrdiff_t last) {
            ptrdiff_t size = last - first;
            if (size < 2) return;

            const ptrdiff_t small_poplar_size = 15;
            if (size <= small_poplar_size) {
                unchecked_insertion_sort(first, last);
                return;
            }

            ptrdiff_t poplar_level = 1;

            ptrdiff_t it = first;
            ptrdiff_t next = it + small_poplar_size;
            while (true) {
                unchecked_insertion_sort(it, next);

                ptrdiff_t poplar_size = small_poplar_size;

                for (ptrdiff_t i = (poplar_level & (0 - poplar_level)) >> 1; i != 0; i >>= 1) {
                    it -= poplar_size;
                    poplar_size = 2 * poplar_size + 1;
                    sift(it, poplar_size);
                    ++next;
                }

                if ((last - next) <= small_poplar_size) {
                    insertion_sort(next, last);
                    return;
                }

                it = next;
                next += small_poplar_size;
                ++poplar_level;
            }
        };
        // PoplarHeapSort.sort_heap
        auto sort_heap = [&](ptrdiff_t first, ptrdiff_t last) {
            ptrdiff_t size = last - first;
            if (size < 2) return;

            do {
                pop_heap_with_size(first, last, size);
                --last;
                --size;
            } while (size > 1);
        };

        make_heap(0, dataSize);
        sort_heap(0, dataSize);
    }

    // Selection Sort (ArrayV): port of select.SelectionSort.runSort.
    // Renamed to SelectionSortJava: the C++ namespace already has a SelectionSort from Sort.h.
    template<class T = int>
    void SelectionSortJava(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        for (ptrdiff_t i = 0; i < dataSize - 1; ++i) {
            ptrdiff_t lowestindex = i;

            for (ptrdiff_t j = i + 1; j < dataSize; ++j) {
                NSortHelpers::MarkArray(2, data_, j);

                if (NSortHelpers::CompareValues(data_[j], data_[lowestindex]) == -1) {
                    lowestindex = j;
                    NSortHelpers::MarkArray(1, data_, lowestindex);
                }
            }
            using std::swap;
            swap(data_[i], data_[lowestindex]);
        }
    }

    // Smooth Sort (Leonardo heaps): port of select.SmoothSort.runSort, fullSort = true.
    // LP holds the Leonardo numbers used by the string-of-heaps encoding (same table as Java).
    template<class T = int>
    void SmoothSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        static constexpr ptrdiff_t LP[21] = { 1, 1, 3, 5, 9, 15, 25, 41, 67, 109,
                                              177, 287, 465, 753, 1219, 1973, 3193, 5167, 8361, 13529, 21891 };

        // Integer.numberOfTrailingZeros
        auto trailingZeros = [](ptrdiff_t v) {
            int count = 0;
            while ((v & 1) == 0) {
                v >>= 1;
                ++count;
            }
            return count;
        };

        // SmoothSort.sift
        auto sift = [&](ptrdiff_t pshift, ptrdiff_t head) {
            T val = data_[head];

            while (pshift > 1) {
                ptrdiff_t rt = head - 1;
                ptrdiff_t lf = head - 1 - LP[pshift - 2];

                NSortHelpers::MarkArray(2, data_, rt);
                NSortHelpers::MarkArray(3, data_, lf);

                if (data_[lf] <= val && data_[rt] <= val) break;

                if (data_[lf] >= data_[rt]) {
                    data_[head] = data_[lf];
                    head = lf;
                    pshift -= 1;
                }
                else {
                    data_[head] = data_[rt];
                    head = rt;
                    pshift -= 2;
                }
            }
            data_[head] = val;
        };

        // SmoothSort.trinkle
        auto trinkle = [&](ptrdiff_t p, ptrdiff_t pshift, ptrdiff_t head, bool isTrusty) {
            T val = data_[head];

            while (p != 1) {
                ptrdiff_t stepson = head - LP[pshift];

                if (data_[stepson] <= val) break;

                if (!isTrusty && pshift > 1) {
                    ptrdiff_t rt = head - 1;
                    ptrdiff_t lf = head - 1 - LP[pshift - 2];

                    NSortHelpers::MarkArray(2, data_, rt);
                    NSortHelpers::MarkArray(3, data_, lf);

                    if (data_[rt] >= data_[stepson] || data_[lf] >= data_[stepson]) break;
                }
                data_[head] = data_[stepson];

                head = stepson;
                int trail = trailingZeros(p & ~static_cast<ptrdiff_t>(1));
                p >>= trail;
                pshift += trail;
                isTrusty = false;
            }

            if (!isTrusty) {
                data_[head] = val;
                sift(pshift, head);
            }
        };

        // SmoothSort.smoothSort (lo = 0, hi = length - 1, fullSort = true)
        ptrdiff_t head = 0;
        ptrdiff_t hi = dataSize - 1;
        ptrdiff_t p = 1;
        ptrdiff_t pshift = 1;

        while (head < hi) {
            if ((p & 3) == 3) {
                sift(pshift, head);
                p >>= 2;
                pshift += 2;
            }
            else {
                if (LP[pshift - 1] >= hi - head) {
                    trinkle(p, pshift, head, false);
                }
                else {
                    sift(pshift, head);
                }

                if (pshift == 1) {
                    p <<= 1;
                    pshift--;
                }
                else {
                    p <<= (pshift - 1);
                    pshift = 1;
                }
            }
            p |= 1;
            ++head;
        }

        trinkle(p, pshift, head, false);

        while (pshift != 1 || p != 1) {
            if (pshift <= 1) {
                int trail = trailingZeros(p & ~static_cast<ptrdiff_t>(1));
                p >>= trail;
                pshift += trail;
            }
            else {
                p <<= 2;
                p ^= 7;
                pshift -= 2;

                trinkle(p >> 1, pshift + 1, head - LP[pshift] - 1, true);
                trinkle(p, pshift, head - 1, true);
            }
            --head;
        }
    }

    // Stable Cycle Sort: port of select.StableCycleSort.runSort.
    // The Java int[] "bits" bitmap becomes a std::vector<int> (flags are not element values).
    template<class T = int>
    void StableCycleSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        constexpr ptrdiff_t WLEN = 3;
        constexpr ptrdiff_t WMASK = (static_cast<ptrdiff_t>(1) << WLEN) - 1;

        std::vector<int> bits(static_cast<size_t>(((dataSize - 1) >> WLEN) + 1), 0);

        // StableCycleSort.getBit
        auto getBit = [&](ptrdiff_t idx) {
            int b = (bits[idx >> WLEN] >> (idx & WMASK)) & 1;
            return b == 1;
        };
        // StableCycleSort.flag
        auto flag = [&](ptrdiff_t idx) {
            NSortHelpers::MarkArray(4, data_, idx >> WLEN);
            bits[idx >> WLEN] = bits[idx >> WLEN] | (1 << (idx & WMASK));
        };
        // StableCycleSort.destination1
        auto destination1 = [&](ptrdiff_t a, ptrdiff_t b1, ptrdiff_t b) {
            ptrdiff_t d = a;
            ptrdiff_t e = 0;

            for (ptrdiff_t i = a + 1; i < b; ++i) {
                NSortHelpers::MarkArray(2, data_, i);
                int cmp = NSortHelpers::CompareValues(data_[i], data_[a]);

                if (cmp < 0) ++d;
                else if (i < b1 && !getBit(i) && cmp == 0) ++e;

                NSortHelpers::MarkArray(3, data_, d);
            }
            while (getBit(d) || e-- > 0) {
                ++d;

                NSortHelpers::MarkArray(3, data_, d);
            }

            return d;
        };

        for (ptrdiff_t i = 0; i < dataSize - 1; ++i) {
            if (!getBit(i)) {
                NSortHelpers::MarkArray(1, data_, i);
                ptrdiff_t j = i;

                do {
                    ptrdiff_t k = destination1(i, j, dataSize);
                    using std::swap;
                    swap(data_[i], data_[k]);
                    flag(k);
                    j = k;
                } while (j != i);
            }
        }
    }

    // Stable Selection Sort: port of select.StableSelectionSort.runSort.
    template<class T = int>
    void StableSelectionSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        for (ptrdiff_t i = 0; i < dataSize - 1; ++i) {
            ptrdiff_t minPos = i;
            for (ptrdiff_t j = i + 1; j < dataSize; ++j) {
                NSortHelpers::MarkArray(1, data_, j);
                if (NSortHelpers::CompareValues(data_[j], data_[minPos]) == -1) {
                    minPos = j;
                    NSortHelpers::MarkArray(2, data_, j);
                }
            }
            T tmp = data_[minPos];
            ptrdiff_t pos = minPos;
            while (pos > i) {
                data_[pos] = data_[pos - 1];
                --pos;
            }
            data_[pos] = tmp;
        }
    }

    // Ternary Heap Sort: port of select.TernaryHeapSort.runSort (by qbit).
    // buildMaxTernaryHeap keeps Java's `length - 1 / 3` (== length, since 1 / 3 == 0).
    template<class T = int>
    void TernaryHeapSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());
        ptrdiff_t heapSize = dataSize - 1;

        // TernaryHeapSort.maxHeapify (recursion follows one child chain: O(log n) deep)
        auto maxHeapify = [&](auto&& self, ptrdiff_t i) -> void {
            ptrdiff_t leftChild = 3 * i + 1;
            ptrdiff_t rightChild = 3 * i + 3;
            ptrdiff_t middleChild = 3 * i + 2;
            ptrdiff_t largest;

            largest = leftChild <= heapSize && data_[leftChild] > data_[i] ? leftChild : i;

            if (rightChild <= heapSize && data_[rightChild] > data_[largest]) {
                largest = rightChild;
            }
            if (middleChild <= heapSize && data_[middleChild] > data_[largest]) {
                largest = middleChild;
            }

            if (largest != i) {
                using std::swap;
                swap(data_[i], data_[largest]);
                self(self, largest);
            }
        };

        heapSize = dataSize - 1;
        for (ptrdiff_t i = dataSize - (1 / 3); i >= 0; --i) {
            maxHeapify(maxHeapify, i);
        }

        for (ptrdiff_t i = dataSize - 1; i >= 0; --i) {
            using std::swap;
            swap(data_[0], data_[i]);

            heapSize = heapSize - 1;
            maxHeapify(maxHeapify, 0);
        }
    }

    // Tournament Sort: port of select.TournamentSort.runSort (Guy Argo's tournament tree).
    // "matches" holds player indices (negative = player at position -i), so it is
    // std::vector<ptrdiff_t>. knockout/rebuild recurse O(log n) deep.
    template<class T = int>
    void TournamentSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t currentLen = static_cast<ptrdiff_t>(data_.size());

        std::vector<ptrdiff_t> matches(static_cast<size_t>(6 * currentLen), 0);
        ptrdiff_t tourney = 0;

        // TournamentSort.isPlayer / makePlayer
        auto isPlayer = [](ptrdiff_t i) { return i <= 0; };

        // TournamentSort.setWinner / setWinners / setLosers / getWinner / getWinners / getLosers
        auto setWinner = [&](ptrdiff_t root, ptrdiff_t winner) { matches[root] = winner; };
        auto setWinners = [&](ptrdiff_t root, ptrdiff_t winners) { matches[root + 1] = winners; };
        auto setLosers = [&](ptrdiff_t root, ptrdiff_t losers) { matches[root + 2] = losers; };
        auto getWinners = [&](ptrdiff_t root) { return matches[root + 1]; };
        auto getLosers = [&](ptrdiff_t root) { return matches[root + 2]; };
        // TournamentSort.getPlayer
        auto getPlayer = [&](ptrdiff_t i) -> ptrdiff_t {
            return i <= 0 ? -i : matches[i];
        };
        // TournamentSort.tourneyCompare (Java marks with the player VALUES, as indices)
        auto tourneyCompare = [&](const T& a, const T& b) {
            NSortHelpers::MarkArray(2, data_, static_cast<ptrdiff_t>(static_cast<int>(a)));
            NSortHelpers::MarkArray(3, data_, static_cast<ptrdiff_t>(static_cast<int>(b)));
            return NSortHelpers::CompareValues(a, b);
        };

        // TournamentSort.knockout
        auto knockout = [&](auto&& self, ptrdiff_t i, ptrdiff_t k, ptrdiff_t root) -> ptrdiff_t {
            if (i == k) return -i;

            ptrdiff_t j = (i + k) / 2;
            ptrdiff_t top = self(self, i, j, 2 * root);
            ptrdiff_t bot = self(self, j + 1, k, (2 * root) + 3);

            // TournamentSort.makeMatch
            ptrdiff_t top_w = getPlayer(top);
            ptrdiff_t bot_w = getPlayer(bot);

            if (tourneyCompare(data_[top_w], data_[bot_w]) <= 0)
                setWinner(root, top_w), setWinners(root, top), setLosers(root, bot);
            else
                setWinner(root, bot_w), setWinners(root, bot), setLosers(root, top);

            return root;
        };

        // TournamentSort.rebuild
        auto rebuild = [&](auto&& self, ptrdiff_t root) -> ptrdiff_t {
            if (isPlayer(getWinners(root)))
                return getLosers(root);

            ptrdiff_t newWinners = self(self, getWinners(root));
            setWinners(root, newWinners);

            if (tourneyCompare(data_[getPlayer(getLosers(root))], data_[getPlayer(getWinners(root))]) < 0) {
                setWinner(root, getPlayer(getLosers(root)));

                ptrdiff_t temp = getLosers(root);

                setLosers(root, getWinners(root));
                setWinners(root, temp);
            }
            else {
                setWinner(root, getPlayer(getWinners(root)));
            }

            return root;
        };

        // TournamentSort.pop
        auto pop = [&]() -> T {
            T result = data_[getPlayer(tourney)];
            tourney = isPlayer(tourney) ? 0 : rebuild(rebuild, tourney);
            return result;
        };

        tourney = knockout(knockout, 0, currentLen - 1, 3);

        std::vector<T> copy(static_cast<size_t>(currentLen));

        for (ptrdiff_t i = 0; i < currentLen; ++i) {
            copy[i] = pop();
        }

        for (ptrdiff_t i = 0; i < currentLen; ++i) {
            data_[i] = copy[i];
        }
    }

    // Triangular Heap Sort: port of select.TriangularHeapSort.runSort (aphitorite).
    template<class T = int>
    void TriangularHeapSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

        // TriangularHeapSort.triangularRoot
        auto triangularRoot = [](ptrdiff_t val) {
            ptrdiff_t r = static_cast<ptrdiff_t>(std::sqrt(static_cast<double>(8 * val + 1)));
            return (r - 1) / 2;
        };
        // TriangularHeapSort.siftDown
        auto siftDown = [&](ptrdiff_t end, ptrdiff_t root) {
            T temp = data_[root];
            ptrdiff_t len = triangularRoot(root);
            ptrdiff_t left = root + len + 1;
            ptrdiff_t right = left + 1;

            while (left < end) {
                if (right >= end) {
                    NSortHelpers::MarkArray(2, data_, left);
                    if (data_[left] > temp) {
                        data_[root] = data_[left];
                    }

                    break;
                }
                ptrdiff_t max = data_[left] >= data_[right] ? left : right;

                NSortHelpers::MarkArray(2, data_, max);
                if (data_[max] > temp) {
                    data_[root] = data_[max];

                    root = max;
                    len = triangularRoot(root);
                    left = root + len + 1;
                    right = left + 1;
                    continue;
                }
                break;
            }
            data_[root] = temp;
        };

        for (ptrdiff_t i = dataSize - 1; i >= 0; --i) {
            siftDown(dataSize, i);
        }
        for (ptrdiff_t i = 1; i < dataSize - 1; ++i) {
            using std::swap;
            swap(data_[0], data_[dataSize - i]);
            siftDown(dataSize - i, 0);
        }
        if (data_[0] > data_[1]) {
            using std::swap;
            swap(data_[0], data_[1]);
        }
    }

    // Weak Heap Sort: port of select.WeakHeapSort.runSort (Manish Bhojasia's weak heap).
    // The Java int[] "bits" flag array becomes a std::vector<int>.
    template<class T = int>
    void WeakHeapSort(std::vector<T>& data_) {
        if (data_.size() < 2) return;
        ptrdiff_t n = static_cast<ptrdiff_t>(data_.size());

        std::vector<int> bits(static_cast<size_t>((n + 7) / 8), 0);

        // WeakHeapSort.getBitwiseFlag
        auto getBitwiseFlag = [&](ptrdiff_t x) {
            return (bits[x >> 3] >> (x & 7)) & 1;
        };
        // WeakHeapSort.toggleBitwiseFlag
        auto toggleBitwiseFlag = [&](ptrdiff_t x) {
            bits[x >> 3] ^= 1 << (x & 7);
        };
        // WeakHeapSort.weakHeapMerge
        auto weakHeapMerge = [&](ptrdiff_t i, ptrdiff_t j) {
            if (NSortHelpers::CompareValues(data_[i], data_[j]) == -1) {
                toggleBitwiseFlag(j);
                using std::swap;
                swap(data_[i], data_[j]);
            }
        };

        for (ptrdiff_t i = 0; i < n / 8; ++i) {
            bits[i] = 0;
        }

        for (ptrdiff_t i = n - 1; i > 0; --i) {
            ptrdiff_t j = i;

            while ((j & 1) == static_cast<ptrdiff_t>(getBitwiseFlag(j >> 1)))
                j >>= 1;
            ptrdiff_t Gparent = j >> 1;

            weakHeapMerge(Gparent, i);
        }

        for (ptrdiff_t i = n - 1; i >= 2; --i) {
            using std::swap;
            swap(data_[0], data_[i]);

            ptrdiff_t x = 1;

            ptrdiff_t y;
            while ((y = 2 * x + static_cast<ptrdiff_t>(getBitwiseFlag(x))) < i)
                x = y;

            while (x > 0) {
                weakHeapMerge(0, x);
                x >>= 1;
            }
        }
        using std::swap;
        swap(data_[0], data_[1]);
    }

} // namespace NVisualSort::NSortAlgorithms
