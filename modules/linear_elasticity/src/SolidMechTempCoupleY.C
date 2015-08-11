/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SolidMechTempCoupleY.h"

template<>
InputParameters validParams<SolidMechTempCoupleY>()
{
  InputParameters params = validParams<SolidMechTempCouple>();
  return params;
}


SolidMechTempCoupleY::SolidMechTempCoupleY(const InputParameters & parameters)
  :SolidMechTempCouple(parameters)
{}

Real
SolidMechTempCoupleY::computeQpResidual()
{
  if (!_constant_properties)
    recomputeCouplingConstants();

  return -(_c1*(1+2*_c2)*_grad_test[_i][_qp](1)*_thermal_strain[_qp]);
}

Real
SolidMechTempCoupleY::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (!_constant_properties)
    recomputeCouplingConstants();

  if (jvar == _temp_var)
    return -(_c1*(1+2*_c2)*_grad_test[_i][_qp](1)*_alpha[_qp]*_phi[_j][_qp]);

  return 0.0;
}

