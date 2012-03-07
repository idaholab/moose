#ifndef MULTISMOOTHCIRCLEIC_H
#define MULTISMOOTHCIRCLEIC_H

#include "Kernel.h"
#include "SmoothCircleIC.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

// Forward Declarations
class MultiSmoothCircleIC;

template<>
InputParameters validParams<MultiSmoothCircleIC>();

/**
 * SmoothcircleIC just returns a constant value.
 */
class MultiSmoothCircleIC : public SmoothCircleIC
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  MultiSmoothCircleIC(const std::string & name,
                         InputParameters parameters);
  
  virtual Real value(const Point & p);

protected:
  unsigned int _numbub;
  Real _bubspac;
  Real _Lx;
  Real _Ly;
  Real _Lz;

  unsigned int _rnd_seed, _numtries;
  Real _radius_variation;
  
  std::vector<Point> _bubcent;
  std::vector<Real> _bubradi;
  
  
// private:
  
};

#endif //MULTISMOOTHCIRCLEIC_H
