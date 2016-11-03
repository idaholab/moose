#include "randomClass.h"
#include <boost/random/mersenne_twister.hpp>

class RandomClassImpl {
  /**
   * This class create the instance of a mersenne-twister random number generator (BOOST)
   */
public:
  boost::random::mt19937 _backend;
};

RandomClass::RandomClass() : _rng(new RandomClassImpl()), _range(_rng->_backend.max() - _rng->_backend.min()) {
}

void RandomClass::seed(unsigned int seed) {
    _rng->_backend.seed(seed);
  }

double RandomClass::random() {
    return (_rng->_backend()-_rng->_backend.min())/_range;
  }

RandomClass::~RandomClass(){
  delete _rng;
}
