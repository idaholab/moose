//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CHSplitFlux.h"

registerMooseObject("PhaseFieldApp", CHSplitFlux);

InputParameters
CHSplitFlux::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Computes flux $j$ as nodal variable $j = -M\\nabla\\mu$");
  params.addRequiredParam<unsigned int>("component", "Flux component");
  params.addRequiredParam<MaterialPropertyName>("mobility_name", "Mobility property name");
  params.addRequiredCoupledVar("mu", "Chemical Potential");
  params.addCoupledVar("c", "Concentration");
  return params;
}

CHSplitFlux::CHSplitFlux(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _component(getParam<unsigned int>("component")),
    _mu_var(coupled("mu")),
    _grad_mu(coupledGradient("mu")),
    _mobility(getMaterialProperty<RealTensorValue>("mobility_name")),
    _has_coupled_c(isCoupled("c")),
    _c_var(_has_coupled_c ? coupled("c") : 0),
    _dmobility_dc(_has_coupled_c ? &getMaterialPropertyDerivative<RealTensorValue>(
                                       "mobility_name", coupledName("c", 0))
                                 : NULL)
{
}

Real
CHSplitFlux::computeQpResidual()
{
  return _test[_i][_qp] * (_u[_qp] + _mobility[_qp].row(_component) * _grad_mu[_qp]);
}

Real
CHSplitFlux::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp];
}

Real
CHSplitFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _mu_var)
    return _test[_i][_qp] * _mobility[_qp].row(_component) * _grad_phi[_j][_qp];
  else if (_has_coupled_c && jvar == _c_var)
    return _test[_i][_qp] * (*_dmobility_dc)[_qp].row(_component) * _grad_mu[_qp] * _phi[_j][_qp];
  else
    return 0.0;
}
