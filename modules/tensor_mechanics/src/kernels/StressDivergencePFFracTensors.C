/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StressDivergencePFFracTensors.h"

template <>
InputParameters
validParams<StressDivergencePFFracTensors>()
{
  InputParameters params = validParams<StressDivergenceTensors>();
  params.addClassDescription("Stress divergence kernel for phase-field fracture: Additionally "
                             "computes off diagonal damage dependent Jacobian components");
  params.addCoupledVar(
      "c",
      "Phase field damage variable: Used to indicate calculation of Off Diagonal Jacobian term");
  return params;
}

StressDivergencePFFracTensors::StressDivergencePFFracTensors(const InputParameters & parameters)
  : DerivativeMaterialInterface<StressDivergenceTensors>(parameters),
    _c_coupled(isCoupled("c")),
    _c_var(_c_coupled ? coupled("c") : 0),
    _d_stress_dc(
        getMaterialPropertyDerivative<RankTwoTensor>(_base_name + "stress", getVar("c", 0)->name()))
{
  mooseDeprecated("StressDivergencePFFracTensors is deprecated. Please use "
                  "PhaseFieldFractureMechanicsOffDiag and StressDivergenceTensors instead.");
}

Real
StressDivergencePFFracTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_c_coupled && jvar == _c_var)
  {
    Real val = 0.0;
    for (unsigned int k = 0; k < 3; ++k)
      val += _d_stress_dc[_qp](_component, k) * _grad_test[_i][_qp](k);
    return val * _phi[_j][_qp];
  }

  // Returns if coupled variable is not c (damage variable)
  return StressDivergenceTensors::computeQpOffDiagJacobian(jvar);
}
