/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
// See
// Homogenization of Temperature-Dependent Thermal Conductivity in Composite
// Materials, Journal of Thermophysics and Heat Transfer, Vol. 15, No. 1,
// January-March 2001.


#ifndef HOMOGENIZATIONHEATCONDUCTION_H
#define HOMOGENIZATIONHEATCONDUCTION_H

#include "Kernel.h"


class HomogenizationHeatConduction : public Kernel
{
public:

  HomogenizationHeatConduction(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  const MaterialProperty<Real> & _diffusion_coefficient;

private:
  const unsigned int _component;

};

template<>
InputParameters validParams<HomogenizationHeatConduction>();

#endif //HOMOGENIZATIONHEATCONDUCTION_H
