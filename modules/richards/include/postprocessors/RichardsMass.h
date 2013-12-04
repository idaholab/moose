#ifndef RICHARDSMASS_H
#define RICHARDSMASS_H

#include "ElementIntegralVariablePostprocessor.h"

//Forward Declarations
class RichardsMass;

template<>
InputParameters validParams<RichardsMass>();

/**
 * This postprocessor computes the fluid mass by integrating the density over the volume
 *
 */
class RichardsMass: public ElementIntegralVariablePostprocessor
{
public:
  RichardsMass(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

  MaterialProperty<Real> &_porosity;
  MaterialProperty<Real> &_sat;
  MaterialProperty<Real> &_density;
};

#endif
