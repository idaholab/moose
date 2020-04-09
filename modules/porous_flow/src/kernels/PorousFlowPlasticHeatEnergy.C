//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPlasticHeatEnergy.h"

#include "MooseMesh.h"
#include "MooseVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowPlasticHeatEnergy);

InputParameters
PorousFlowPlasticHeatEnergy::validParams()
{
  InputParameters params = PlasticHeatEnergy::validParams();
  params.addParam<bool>("strain_at_nearest_qp",
                        false,
                        "When calculating nodal porosity that depends on strain, use the strain at "
                        "the nearest quadpoint.  This adds a small extra computational burden, and "
                        "is not necessary for simulations involving only linear lagrange elements. "
                        " If you set this to true, you will also want to set the same parameter to "
                        "true for related Kernels and Materials");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.addClassDescription(
      "Plastic heat energy density source = (1 - porosity) * coeff * stress * plastic_strain_rate");
  return params;
}

PorousFlowPlasticHeatEnergy::PorousFlowPlasticHeatEnergy(const InputParameters & parameters)
  : PlasticHeatEnergy(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _strain_at_nearest_qp(getParam<bool>("strain_at_nearest_qp")),
    _nearest_qp(_strain_at_nearest_qp
                    ? &getMaterialProperty<unsigned int>("PorousFlow_nearestqp_nodal")
                    : nullptr),
    _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
    _dporosity_dvar(getMaterialProperty<std::vector<Real>>("dPorousFlow_porosity_nodal_dvar")),
    _dporosity_dgradvar(
        getMaterialProperty<std::vector<RealGradient>>("dPorousFlow_porosity_nodal_dgradvar"))
{
}

Real
PorousFlowPlasticHeatEnergy::computeQpResidual()
{
  return (1.0 - _porosity[_i]) * PlasticHeatEnergy::computeQpResidual();
}

Real
PorousFlowPlasticHeatEnergy::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
PorousFlowPlasticHeatEnergy::computeQpOffDiagJacobian(unsigned int jvar)
{
  // If the variable is not a PorousFlow variable, the Jacobian terms are 0
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;

  const Real res_no_porosity = PlasticHeatEnergy::computeQpResidual();
  const Real jac_no_porosity = PlasticHeatEnergy::computeQpOffDiagJacobian(jvar);

  const unsigned pvar = _dictator.porousFlowVariableNum(jvar);
  const unsigned nearest_qp = (_strain_at_nearest_qp ? (*_nearest_qp)[_i] : _i);

  Real jac = (1.0 - _porosity[_i]) * jac_no_porosity -
             _dporosity_dgradvar[_i][pvar] * _grad_phi[_j][nearest_qp] * res_no_porosity;
  if (_i != _j)
    return jac;

  return jac - _dporosity_dvar[_i][pvar] * res_no_porosity;
}
