#pragma once

#include "place.h"

#include <array>

template <class T> struct BoardArray {
public:
  BoardArray() {}

  BoardArray(const std::array<T, 64> &values) : _array(values) {}

  T &operator[](Place place) { return _array[place.to_int()]; }
  const T &operator[](Place place) const { return _array[place.to_int()]; }

  void clear(const T &value) {
    for (int i = 0; i < 64; i++) {
      _array[i] = value;
    }
  }

private:
  std::array<T, 64> _array;
};
