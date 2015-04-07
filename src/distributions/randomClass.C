#include "randomClass.h"

void RandomClass::seed(unsigned int seed) {
    rng->seed(seed);
  }

double RandomClass::random() {
    return ((*rng)()-rng->min())/range;
  }

RandomClass::~RandomClass(){

}
