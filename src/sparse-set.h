#ifndef SPARSE_SET_H
#define SPARSE_SET_H

#include <climits>
#include <cstddef>
#include <utility>
#include <vector>

namespace SparseSet {
static constexpr int INITIAL_DENSE_SIZE = 10;
static constexpr int INITIAL_ENTITIES = 10;
static constexpr size_t EMPTY = ULONG_LONG_MAX - 1;

class SparseSet {
public:
  SparseSet(size_t initialNumOfEntities = INITIAL_ENTITIES) {
    if (initialNumOfEntities > sparse.capacity()) {
      sparse.resize(initialNumOfEntities, EMPTY);
    }
    dense.resize(INITIAL_DENSE_SIZE, EMPTY);
  }

  ~SparseSet() {
    sparse.clear();
    dense.clear();
  }

  void Add(size_t id) {
    if (id < sparse.size()) {
      dense[num_of_components] = id;
      sparse[id] = num_of_components;
      ++num_of_components;
    }
  }

  size_t Get(size_t id) const {
    if (id < sparse.size()) {
      size_t denseIndex = sparse[id];
      if (denseIndex != EMPTY && dense[denseIndex] == id) {
        return id;
      }
    }
    return EMPTY;
  }

  void Remove(size_t id) {
    if (Get(id) != EMPTY) {
      size_t index = sparse[id];
      size_t prev = dense[num_of_components - 1];
      std::swap(dense[num_of_components - 1], dense[index]);
      sparse[id] = num_of_components + 1;
      sparse[prev] = index;
      --num_of_components;
    }
  }

  size_t num_of_components = 0;
  std::vector<size_t> sparse;
  std::vector<size_t> dense;
};

} // namespace SparseSet

#endif
