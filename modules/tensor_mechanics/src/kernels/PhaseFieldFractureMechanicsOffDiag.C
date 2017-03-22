/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PhaseFieldFractureMechanicsOffDiag.h"

template <>
InputParameters
validParams<PhaseFieldFractureMechanicsOffDiag>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Stress divergence kernel for phase-field fracture: Computes off "
                             "diagonal damage dependent Jacobian components. To be used with "
                             "StressDivergenceTensors or DynamicStressDivergenceTensors.");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction "
                                        "the variable this kernel acts in. (0 for x, "
                                        "1 for y, 2 for z)");
  params.addCoupledVar(
      "c",
      "Phase field damage variable: Used to indicate calculation of Off Diagonal Jacobian term");
  return params;
}

PhaseFieldFractureMechanicsOffDiag::PhaseFieldFractureMechanicsOffDiag(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _component(getParam<unsigned int>("component")),
    _c_coupled(isCoupled("c")),
    _c_var(_c_coupled ? coupled("c") : 0),
    _d_stress_dc(
        getMaterialPropertyDerivative<RankTwoTensor>(_base_name + "stress", getVar("c", 0)->name()))
{
}

Real
PhaseFieldFractureMechanicsOffDiag::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_c_coupled && jvar == _c_var)
  {
    Real val = 0.0;
    for (unsigned int k = 0; k < 3; ++k)
      val += _d_stress_dc[_qp](_component, k) * _grad_test[_i][_qp](k);
    return val * _phi[_j][_qp];
  }

  // Returns if coupled variable is not c (damage variable)
  return 0.0;
}
