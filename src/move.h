#ifndef MOVE_HH
#define MOVE_HH

#include "place.h"

#include <cstdio>
#include <iostream>

struct Move {
  Move(int8_t ol, int8_t oc, int8_t dl, int8_t dc)
      : o(Place::of_line_of_col(ol, oc)), d(Place::of_line_of_col(dl, dc)) {}
  Move(Place o, Place d) : o(o), d(d) {}
  Move() {}
  int8_t ol() const { return o.line(); }
  int8_t oc() const { return o.col(); }
  int8_t dl() const { return d.line(); }
  int8_t dc() const { return d.col(); }
  inline bool operator==(const Move &m) const { return o == m.o and d == m.d; }
  inline bool operator!=(const Move &m) const { return o != m.o or d != m.d; }
  inline bool is_valid() const {
    return o.is_valid() && d.is_valid() && (o != d);
  }
  Place o, d;
};

inline std::ostream &operator<<(std::ostream &stream, const Move &m) {
  return stream << m.oc() + 'a' << m.ol() + '1' << m.dc() + 'a' << m.dl() + '1';
}

inline void print_move(FILE *file, const Move &m) {
  fprintf(file, "%c%c%c%c", m.oc() + 'a', m.ol() + '1', m.dc() + 'a',
          m.dl() + '1');
}

#endif
