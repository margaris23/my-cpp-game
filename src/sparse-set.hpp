#ifndef SPARSE_SET_H
#define SPARSE_SET_H

#include "fmt/core.h"
#include <climits>
#include <cstddef>
#include <utility>
#include <vector>

static constexpr int INITIAL_ELEMENTS = 20;
static constexpr size_t EMPTY = ULONG_LONG_MAX - 1;

template <typename T> class SparseSet {
public:
  std::vector<size_t> sparse;
  std::vector<T> dense;

  SparseSet(size_t initialNumOfEntities = INITIAL_ELEMENTS) {
    if (initialNumOfEntities > sparse.capacity()) {
      sparse.resize(initialNumOfEntities, EMPTY);
    }
    dense.reserve(INITIAL_ELEMENTS);
  }

  ~SparseSet() {
    sparse.clear();
    dense.clear();
  }

  bool contains(size_t id) const {
    return id >= sparse.size() || sparse[id] != EMPTY; // && dense[sparse[id]].m_entity == id;
  }

  // TODO: reuse and/or update
  // TODO: handle resize
  bool Add(size_t id, T &&denseItem) {
    if (!contains(id)) {
      denseItem.m_entity = id;
      sparse[id] = dense.size();
      dense.push_back(std::move(denseItem));
      return true;
    }
    return false;
  }

  T *Get(size_t id) {
    if (id < sparse.size()) {
      size_t denseIndex = sparse[id];
      if (denseIndex != EMPTY && dense[denseIndex].m_entity == id) {
        return &(dense[denseIndex]);
      }
    }
    return nullptr;
  }

  // TODO: return bool for success removal
  void Remove(size_t entity) {
    // fmt::println("REMOVING {} for {}", typeid(T).name(), entity);
    if (contains(entity)) {
      size_t index = sparse[entity];
      size_t last_items_entity = dense[dense.size() - 1].m_entity;
      std::swap(dense[dense.size() - 1], dense[index]);
      sparse[last_items_entity] = index;
      sparse[entity] = EMPTY;
      dense.pop_back();
      fmt::println("\t{} -> {} REMOVED", typeid(T).name(), dense.size());
    }
  }

  void Reset() {
    sparse.clear();
    dense.clear();
    // size = 0;
  }
};

#endif
