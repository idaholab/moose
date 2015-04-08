
class RandomClassImpl;

class RandomClass {
  RandomClassImpl *rng;
  const double range;
public:
  RandomClass();
  ~RandomClass();
  void seed(unsigned int seed);
  double random();
};
