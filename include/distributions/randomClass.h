
namespace boost {
namespace random {
class mt19937;
}
}

class RandomClass {
  boost::random::mt19937 *rng;
  const double range;
public:
  RandomClass();
  ~RandomClass();
  void seed(unsigned int seed);
  double random();
};
