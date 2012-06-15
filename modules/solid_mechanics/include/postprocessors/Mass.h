#ifndef MASS_H
#define MASS_H

#include "ElementIntegral.h"

//Forward Declarations
class Mass;

template<>
InputParameters validParams<Mass>();

/**
 * This postprocessor computes the mass by integrating the density over the volume
 *
 */
class Mass: public ElementIntegral
{
public:
  Mass(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();
  MaterialProperty<Real> & _density;
};

#endif
