#include <boost/random/mersenne_twister.hpp>

class RandomClass {
  boost::random::mt19937 *rng;
  const double range;
public:
  RandomClass() : range(rng->max() - rng->min()) {};
  ~RandomClass();
  void seed(unsigned int seed);
  double random();
};
