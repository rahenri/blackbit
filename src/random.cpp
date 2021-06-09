#include "random.hh"

#include <random>

std::default_random_engine rng;
std::uniform_int_distribution<uint32_t> uint32_interval(0, 0xffffffffu);
std::uniform_int_distribution<uint64_t> uint64_interval(0,
                                                        0xffffffffffffffffllu);

uint32_t rand32() { return uint32_interval(rng); }

uint64_t rand64() { return uint64_interval(rng); }
