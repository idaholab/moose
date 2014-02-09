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

  const RichardsPorepressureNames & _pp_name_UO;
  unsigned int _pvar;

  MaterialProperty<Real> &_porosity;
  MaterialProperty<std::vector<Real> > &_sat;
  MaterialProperty<std::vector<Real> > &_density;
};

#endif
