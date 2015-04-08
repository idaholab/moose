#include "randomClass.h"
#include <boost/random/mersenne_twister.hpp>

class RandomClassImpl {
public:
  boost::random::mt19937 backend;
};

RandomClass::RandomClass() : rng(new RandomClassImpl()), range(rng->backend.max() - rng->backend.min()) {
};

void RandomClass::seed(unsigned int seed) {
    rng->backend.seed(seed);
  }

double RandomClass::random() {
    return (rng->backend()-rng->backend.min())/range;
  }

RandomClass::~RandomClass(){
  delete rng;
}
