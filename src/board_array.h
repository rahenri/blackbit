#pragma once

#include "place.h"

template <class T> struct BoardArray {
public:
  BoardArray() {}

  T &operator[](Place place) { return _array[place.to_int()]; }
  const T &operator[](Place place) const { return _array[place.to_int()]; }

  void clear(const T& value) {
    for (int i = 0; i < 64; i++) {
      _array[i] = value;
    }
  }

private:
  T _array[64];
};
