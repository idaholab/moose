/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SwitchingFunctionConstraintLagrange.h"

template <>
InputParameters
validParams<SwitchingFunctionConstraintLagrange>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Lagrange multiplier kernel to constrain the sum of all switching "
                             "functions in a multiphase system. This kernel acts on the lagrange "
                             "multiplier variable.");
  params.addParam<std::vector<MaterialPropertyName>>(
      "h_names", "Switching Function Materials that provide h(eta_i)");
  params.addRequiredCoupledVar("etas", "eta_i order parameters, one for each h");
  params.addParam<Real>("epsilon", 1e-9, "Shift factor to avoid a zero pivot");
  return params;
}

SwitchingFunctionConstraintLagrange::SwitchingFunctionConstraintLagrange(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _h_names(getParam<std::vector<MaterialPropertyName>>("h_names")),
    _num_h(_h_names.size()),
    _h(_num_h),
    _dh(_num_h),
    _number_of_nl_variables(_fe_problem.getNonlinearSystemBase().nVariables()),
    _j_eta(_number_of_nl_variables, -1),
    _epsilon(getParam<Real>("epsilon"))
{
  // parameter check. We need exactly one eta per h
  if (_num_h != coupledComponents("etas"))
    mooseError(
        "Need to pass in as many h_names as etas in SwitchingFunctionConstraintLagrange kernel ",
        name());

  // fetch switching functions (for the residual) and h derivatives (for the Jacobian)
  for (unsigned int i = 0; i < _num_h; ++i)
  {
    _h[i] = &getMaterialPropertyByName<Real>(_h_names[i]);
    _dh[i] = &getMaterialPropertyDerivative<Real>(_h_names[i], getVar("etas", i)->name());

    // generate the lookup table from j_var -> eta index
    unsigned int num = coupled("etas", i);
    if (num < _number_of_nl_variables)
      _j_eta[num] = i;
  }
}

Real
SwitchingFunctionConstraintLagrange::computeQpResidual()
{
  Real g = -_epsilon * _u[_qp] - 1.0;
  for (unsigned int i = 0; i < _num_h; ++i)
    g += (*_h[i])[_qp];

  return _test[_i][_qp] * g;
}

Real
SwitchingFunctionConstraintLagrange::computeQpJacobian()
{
  return _test[_i][_qp] * -_epsilon * _phi[_j][_qp];
}

Real
SwitchingFunctionConstraintLagrange::computeQpOffDiagJacobian(unsigned int j_var)
{
  const int eta = _j_eta[j_var];

  if (eta >= 0)
    return (*_dh[eta])[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  else
    return 0.0;
}
