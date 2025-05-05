#ifndef SPARSE_SET_H
#define SPARSE_SET_H

#include <climits>
#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace SparseSet {

static constexpr int INITIAL_DENSE_SIZE = 10;
static constexpr int INITIAL_ENTITIES = 10;
static constexpr size_t EMPTY = ULONG_LONG_MAX - 1;

template <typename T> class SparseSet {
public:
  std::vector<size_t> sparse;
  std::vector<T> dense;

  SparseSet(size_t initialNumOfEntities = INITIAL_ENTITIES) {
    if (initialNumOfEntities > sparse.capacity()) {
      sparse.resize(initialNumOfEntities, EMPTY);
    }
    dense.reserve(INITIAL_DENSE_SIZE);
  }

  ~SparseSet() {
    sparse.clear();
    dense.clear();
  }

  bool contains(size_t id) const {
    return id < sparse.size() && dense[sparse[id]].entity == id;
  }

  // TODO: reuse and/or update
  bool Add(size_t id, T &denseItem) {
    if (!contains(id)) {
      denseItem.entity = id;
      sparse[id] = dense.size();
      dense.push_back(denseItem);
      return true;
    }
    return false;
  }

  std::unique_ptr<T> Get(size_t id) const {
    if (id < sparse.size()) {
      size_t denseIndex = sparse[id];
      if (denseIndex != EMPTY && dense[denseIndex].entity == id) {
        return std::make_unique<T>(dense[denseIndex]);
      }
    }
    return nullptr;
  }

  // TODO: return bool for success removal
  void Remove(size_t entity) {
    if (contains(entity)) {
      std::cout << "REMOVING: " << entity << "\n";
      size_t index = sparse[entity];
      std::swap(dense[dense.size() - 1], dense[index]);
      sparse[entity] = EMPTY;
      dense.pop_back();
    }
  }

  void Reset() {
    sparse.clear();
    dense.clear();
    // size = 0;
  }

  std::vector<T> &GetDense() { return dense; }
};

} // namespace SparseSet

#endif
