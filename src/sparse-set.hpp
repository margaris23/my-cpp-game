#ifndef SPARSE_SET_H
#define SPARSE_SET_H

#include <algorithm>
#include <climits>
#include <cstddef>
#include <utility>
#include <vector>

// allocate enough memory to support large number of particles
static constexpr int INITIAL_ELEMENTS = 5000;
static constexpr size_t EMPTY = ULLONG_MAX - 1;

template <typename T> class SparseSet {
public:
  std::vector<size_t> sparse;
  std::vector<T> dense;

  SparseSet(size_t initialNumOfEntities = INITIAL_ELEMENTS) {
    if (initialNumOfEntities > sparse.capacity()) {
      sparse.resize(initialNumOfEntities, EMPTY);
    }
    dense.reserve(initialNumOfEntities);
  }

  ~SparseSet() {
    sparse.clear();
    dense.clear();
  }

  // throws when out of bounds
  bool contains(size_t id) const {
    return id < sparse.size() && sparse.at(id) != EMPTY;
  }

  // TODO: reuse and/or update
  // TODO: handle resize
  bool Add(size_t id, T &&denseItem) {
    if (!contains(id)) {
      denseItem.entity = id;
      sparse.at(id) = dense.size();
      dense.push_back(std::move(denseItem));
      return true;
    }
    return false;
  }

  T *Get(size_t id) {
    if (id < sparse.size()) {
      size_t denseIndex = sparse.at(id);
      if (denseIndex != EMPTY && dense.at(denseIndex).entity == id) {
        return &(dense.at(denseIndex));
      }
    }
    return nullptr;
  }

  // TODO: return bool for success removal
  void Remove(size_t entity) {
    if (contains(entity)) {
      size_t dense_index = sparse.at(entity);
      size_t last_items_entity = dense.at(dense.size() - 1).entity;
      std::swap(dense.at(dense.size() - 1), dense.at(dense_index));
      sparse.at(last_items_entity) = dense_index;
      sparse.at(entity) = EMPTY;
      dense.pop_back();
    }
  }

  void Reset() {
    sparse.clear();
    dense.clear();
    // size = 0;
  }
};

#endif
