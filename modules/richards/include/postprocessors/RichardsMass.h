/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSMASS_H
#define RICHARDSMASS_H

#include "ElementIntegralVariablePostprocessor.h"
#include "RichardsPorepressureNames.h"

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

  /// userobject that holds porepressure names
  const RichardsPorepressureNames & _pp_name_UO;

  /// pressure variable number that we want the mass for
  unsigned int _pvar;

  /// material porosity
  MaterialProperty<Real> &_porosity;

  /// fluid saturation
  MaterialProperty<std::vector<Real> > &_sat;

  /// fluid density
  MaterialProperty<std::vector<Real> > &_density;
};

#endif
