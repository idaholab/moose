#ifndef MASS_H
#define MASS_H

#include "ElementIntegralVariablePostprocessor.h"

//Forward Declarations
class Mass;

template<>
InputParameters validParams<Mass>();

/**
 * This postprocessor computes the mass by integrating the density over the volume
 *
 */
class Mass: public ElementIntegralVariablePostprocessor
{
public:
  Mass(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();
  MaterialProperty<Real> & _density;
};

#endif
