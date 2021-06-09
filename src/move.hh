#ifndef MOVE_HH
#define MOVE_HH

#include <cstdio>
#include <iostream>

/* check if the position is valid */
#define VALID_POS(p) ((p) >= 0 and (p) < 64)

struct Move {
  Move(int8_t ol, int8_t oc, int8_t dl, int8_t dc)
      : o(oc | (ol << 3)), d(dc | (dl << 3)) {}
  Move(int16_t o, int16_t d) : o(o), d(d) {}
  Move() {}
  int8_t ol() const { return o / 8; }
  int8_t oc() const { return o % 8; }
  int8_t dl() const { return d / 8; }
  int8_t dc() const { return d % 8; }
  inline bool operator==(const Move &m) const { return o == m.o and d == m.d; }
  inline bool operator!=(const Move &m) const { return o != m.o or d != m.d; }
  inline bool is_valid() const {
    return VALID_POS(o) and VALID_POS(d) and (o != d);
  }
  int16_t o, d;
};

inline std::ostream &operator<<(std::ostream &stream, const Move &m) {
  return stream << m.oc() + 'a' << m.ol() + '1' << m.dc() + 'a' << m.dl() + '1';
}

inline void print_move(FILE *file, const Move &m) {
  fprintf(file, "%c%c%c%c", m.oc() + 'a', m.ol() + '1', m.dc() + 'a',
          m.dl() + '1');
}

#endif
