//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SLKKSChemicalPotential.h"
#include "MathUtils.h"

using namespace MathUtils;

registerMooseObject("PhaseFieldApp", SLKKSChemicalPotential);

InputParameters
SLKKSChemicalPotential::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "SLKKS model kernel to enforce the pointwise equality of sublattice chemical "
      "potentials in the same phase.");
  params.addRequiredCoupledVar("cs", "other sublattice concentration in the same phase");
  params.addRequiredParam<Real>("a", "sublattice site fraction for the kernel variable");
  params.addRequiredParam<Real>("as", "other sublattice site fraction in the same phase");
  params.addRequiredParam<MaterialPropertyName>("F", "Base name of the free energy function");
  params.addCoupledVar("args", "Vector of variable arguments to the free energy function");
  params.deprecateCoupledVar("args", "coupled_variables", "02/27/2024");

  return params;
}

SLKKSChemicalPotential::SLKKSChemicalPotential(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _cs_var(coupled("cs")),
    _cs_name(coupledName("cs", 0)),
    // first derivatives
    _dFdu(getMaterialPropertyDerivative<Real>("F", _var.name())),
    _dFdcs(getMaterialPropertyDerivative<Real>("F", _cs_name)),
    // second derivatives d2F/dx*dca for jacobian diagonal elements
    _d2Fdu2(getMaterialPropertyDerivative<Real>("F", _var.name(), _var.name())),
    _d2Fdcsu(getMaterialPropertyDerivative<Real>("F", _cs_name, _var.name())),
    // site fractions
    _a_u(getParam<Real>("a")),
    _a_cs(getParam<Real>("as"))
{
  const auto nvar = _coupled_moose_vars.size();
  _d2Fdudarg.resize(nvar);
  _d2Fdcsdarg.resize(nvar);

  for (std::size_t i = 0; i < nvar; ++i)
  {
    // get the moose variable
    const auto & arg_name = _coupled_moose_vars[i]->name();

    // lookup table for the material properties representing the derivatives
    // needed for the off-diagonal Jacobian
    _d2Fdudarg[i] = &getMaterialPropertyDerivative<Real>("F", _var.name(), arg_name);
    _d2Fdcsdarg[i] = &getMaterialPropertyDerivative<Real>("F", _cs_name, arg_name);
  }
}

void
SLKKSChemicalPotential::initialSetup()
{
  validateNonlinearCoupling<Real>("F");
}

Real
SLKKSChemicalPotential::computeQpResidual()
{
  return _test[_i][_qp] * (_dFdu[_qp] / _a_u - _dFdcs[_qp] / _a_cs);
}

Real
SLKKSChemicalPotential::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp] * (_d2Fdu2[_qp] / _a_u - _d2Fdcsu[_qp] / _a_cs);
}

Real
SLKKSChemicalPotential::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return _test[_i][_qp] * _phi[_j][_qp] *
         ((*_d2Fdudarg[cvar])[_qp] / _a_u - (*_d2Fdcsdarg[cvar])[_qp] / _a_cs);
}
