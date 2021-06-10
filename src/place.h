#pragma once

#include <numeric>

struct __attribute__((__packed__)) Place {
public:
  inline Place() : _place(0) {}
  /* not BB */
  inline int8_t line() const { return _place >> 3; }

  /* not BB */
  inline int8_t col() const { return _place & 7; }

  /* not BB */
  static inline Place of_line_of_col(int8_t lin, int8_t col) {
    return Place(col | (lin << 3));
  }

  inline Place down() const { return Place(_place - 8); }

  inline Place up() const { return Place(_place + 8); }

  inline Place right() const { return Place(_place + 1); }

  inline Place left() const { return Place(_place - 1); }

  static inline Place of_int(int8_t place) { return Place(place); }

  inline int8_t to_int() const { return _place; }

  inline bool operator==(const Place &other) const {
    return _place == other._place;
  }
  inline bool operator!=(const Place &other) const {
    return _place != other._place;
  }

  inline bool is_valid() const { return _place >= 0 && _place < 64; }

  static inline Place invalid() { return of_int(64); }

private:
  int8_t _place;

  inline Place(int8_t place) : _place(place) {}
};
