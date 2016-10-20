#include "randomClass.h"
#include <boost/random/mersenne_twister.hpp>

class RandomClassImpl {
  /**
   * This class create the instance of a mersenne-twister random number generator (BOOST)
   */
public:
  boost::random::mt19937 backend;
};

RandomClass::RandomClass() : rng(new RandomClassImpl()), range(rng->backend.max() - rng->backend.min()) {
}

void RandomClass::seed(unsigned int seed) {
    rng->backend.seed(seed);
  }

double RandomClass::random() {
    return (rng->backend()-rng->backend.min())/range;
  }

RandomClass::~RandomClass(){
  delete rng;
}
