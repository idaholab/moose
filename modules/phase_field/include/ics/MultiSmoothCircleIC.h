#ifndef MULTISMOOTHCIRCLEIC_H
#define MULTISMOOTHCIRCLEIC_H

#include "Kernel.h"
#include "SmoothCircleIC.h"

// System includes
#include <string>

// Forward Declarations
class MultiSmoothCircleIC;

template<>
InputParameters validParams<MultiSmoothCircleIC>();

/**
 * MultismoothCircleIC creates multiple SmoothCircles (number = numbub) that are randomly
 * positioned around the domain, with a minimum spacing equal to bubspac
 **/
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

  virtual void initialSetup();

  virtual Real value(const Point & p);
  virtual RealGradient gradient(const Point & p);

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
};

#endif //MULTISMOOTHCIRCLEIC_H
