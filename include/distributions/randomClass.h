#ifndef RANDOMCLASS_H
#define RANDOMCLASS_H

class RandomClassImpl;


/**
 * The RandomClass class allows to create a random number generator instance
 * anywhere in crow
 */

class RandomClass {
  RandomClassImpl *_rng;
  const double _range;
public:
  RandomClass();
  ~RandomClass();
  void seed(unsigned int seed);
  double random();
};

#endif /* RANDOMCLASS_H */
