//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialDerivativeStdVectorRealTestKernel.h"

registerMooseObject("ThermalHydraulicsTestApp", MaterialDerivativeStdVectorRealTestKernel);

InputParameters
MaterialDerivativeStdVectorRealTestKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Class used for testing derivatives of a std::vector<Real> material property.");
  params.addRequiredParam<MaterialPropertyName>(
      "material_property", "Name of material property for which derivatives are to be tested.");
  params.addRequiredCoupledVar("args", "List of variables the material property depends on");
  params.addRequiredParam<unsigned int>("i", "std::vector component");
  return params;
}

MaterialDerivativeStdVectorRealTestKernel::MaterialDerivativeStdVectorRealTestKernel(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _n_vars(_coupled_moose_vars.size()),
    _name(getParam<MaterialPropertyName>("material_property")),
    _p(getMaterialProperty<Real>(_name)),
    _p_off_diag_derivatives(_n_vars),
    _p_diag_derivative(
        getMaterialPropertyDerivative<std::vector<Real>>(_name, VariableName(_var.name()))),
    _component_i(getParam<unsigned int>("i"))
{
  for (unsigned int m = 0; m < _n_vars; ++m)
    _p_off_diag_derivatives[m] = &getMaterialPropertyDerivative<std::vector<Real>>(
        _name, VariableName(_coupled_moose_vars[m]->name()));
}

Real
MaterialDerivativeStdVectorRealTestKernel::computeQpResidual()
{
  // This would be normally _p[_qp] * _test[_i][_qp];
  // But this kernel sits on a variable where another kernel with Real-valued
  // derivatives is. Our residual then must be 0, becuase we want to contribute
  // with just jacobians without modifying the residual
  return 0;
}

Real
MaterialDerivativeStdVectorRealTestKernel::computeQpJacobian()
{
  if (_p_diag_derivative[_qp].size() > 0)
    return _p_diag_derivative[_qp][_component_i] * _phi[_j][_qp] * _test[_i][_qp];
  else
    return 0;
}

Real
MaterialDerivativeStdVectorRealTestKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable number corresponding to jvar
  const unsigned int cvar = mapJvarToCvar(jvar);
  if ((*_p_off_diag_derivatives[cvar])[_qp].size() > 0)
    return (*_p_off_diag_derivatives[cvar])[_qp][_component_i] * _phi[_j][_qp] * _test[_i][_qp];
  else
    return 0;
}
