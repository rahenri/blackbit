#ifndef RANDOM_HH
#define RANDOM_HH

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>


boost::mt19937 rng;
boost::uniform_int<uint32_t> uint32_interval(0,0xffffffffu);
boost::uniform_int<uint64_t> uint64_interval(0,0xffffffffffffffffllu);

boost::variate_generator<boost::mt19937&, boost::uniform_int<uint32_t> > rand32(rng, uint32_interval);
boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t> > rand64(rng, uint64_interval);

#endif
