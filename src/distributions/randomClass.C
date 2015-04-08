#include "randomClass.h"
#include <boost/random/mersenne_twister.hpp>

RandomClass::RandomClass():range(rng->max() - rng->min()) {};

void RandomClass::seed(unsigned int seed) {
    rng->seed(seed);
  }

double RandomClass::random() {
    return ((*rng)()-rng->min())/range;
  }

RandomClass::~RandomClass(){

}
