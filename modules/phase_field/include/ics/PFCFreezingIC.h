#ifndef PFCFREEZINGIC_H
#define PFCFREEZINGIC_H

#include "InitialCondition.h"

// Forward Declarations
class PFCFreezingIC;

template<>
InputParameters validParams<PFCFreezingIC>();

/**
 * PFCFreezingIC creates an intial density for a PFC model that has one area of a set
 * crystal structure (initialized using sinusoids) and all the rest with a random structure.
 * The random values will fall between 0 and 1.
 * \todo For the FCC this returns 0. This cannot be right, yet it satisfies the (probably bogus) test.
 */
class PFCFreezingIC : public InitialCondition
{
public:
  PFCFreezingIC(const std::string & name,
                InputParameters parameters);

  virtual Real value(const Point & p);

private:
  Real _x1;
  Real _y1;
  Real _z1;

  Real _x2;
  Real _y2;
  Real _z2;

  Real _lc;
  MooseEnum _crystal_structure;

  Point _bottom_left;
  Point _top_right;
  Point _range;

  Real _min, _max, _val_range;
  Real _inside, _outside;

  unsigned int _icdim;
};

#endif //PFCFREEZINGIC_H
