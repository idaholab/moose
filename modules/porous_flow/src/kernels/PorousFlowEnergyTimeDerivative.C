//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowEnergyTimeDerivative.h"

#include "MooseVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowEnergyTimeDerivative);

InputParameters
PorousFlowEnergyTimeDerivative::validParams()
{
  InputParameters params = TimeKernel::validParams();
  params.addParam<bool>("strain_at_nearest_qp",
                        false,
                        "When calculating nodal porosity that depends on strain, use the strain at "
                        "the nearest quadpoint.  This adds a small extra computational burden, and "
                        "is not necessary for simulations involving only linear lagrange elements. "
                        " If you set this to true, you will also want to set the same parameter to "
                        "true for related Kernels and Materials");
  params.addParam<std::string>(
      "base_name",
      "For mechanically-coupled systems, this Kernel will depend on the volumetric strain.  "
      "base_name should almost always be the same base_name as given to the TensorMechanics object "
      "that computes strain.  Supplying a base_name to this Kernel but not defining an associated "
      "TensorMechanics strain calculator means that this Kernel will not depend on volumetric "
      "strain.  That could be useful when models contain solid mechanics that is not coupled to "
      "porous flow, for example");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.set<bool>("use_displaced_mesh") = false;
  params.suppressParameter<bool>("use_displaced_mesh");
  params.addClassDescription("Derivative of heat-energy-density wrt time");
  return params;
}

PorousFlowEnergyTimeDerivative::PorousFlowEnergyTimeDerivative(const InputParameters & parameters)
  : TimeKernel(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _var_is_porflow_var(_dictator.isPorousFlowVariable(_var.number())),
    _num_phases(_dictator.numPhases()),
    _fluid_present(_num_phases > 0),
    _strain_at_nearest_qp(getParam<bool>("strain_at_nearest_qp")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _has_total_strain(hasMaterialProperty<RankTwoTensor>(_base_name + "total_strain")),
    _total_strain_old(_has_total_strain
                          ? &getMaterialPropertyOld<RankTwoTensor>(_base_name + "total_strain")
                          : nullptr),
    _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
    _porosity_old(getMaterialPropertyOld<Real>("PorousFlow_porosity_nodal")),
    _dporosity_dvar(getMaterialProperty<std::vector<Real>>("dPorousFlow_porosity_nodal_dvar")),
    _dporosity_dgradvar(
        getMaterialProperty<std::vector<RealGradient>>("dPorousFlow_porosity_nodal_dgradvar")),
    _nearest_qp(_strain_at_nearest_qp
                    ? &getMaterialProperty<unsigned int>("PorousFlow_nearestqp_nodal")
                    : nullptr),
    _rock_energy_nodal(getMaterialProperty<Real>("PorousFlow_matrix_internal_energy_nodal")),
    _rock_energy_nodal_old(getMaterialPropertyOld<Real>("PorousFlow_matrix_internal_energy_nodal")),
    _drock_energy_nodal_dvar(
        getMaterialProperty<std::vector<Real>>("dPorousFlow_matrix_internal_energy_nodal_dvar")),
    _fluid_density(_fluid_present ? &getMaterialProperty<std::vector<Real>>(
                                        "PorousFlow_fluid_phase_density_nodal")
                                  : nullptr),
    _fluid_density_old(_fluid_present ? &getMaterialPropertyOld<std::vector<Real>>(
                                            "PorousFlow_fluid_phase_density_nodal")
                                      : nullptr),
    _dfluid_density_dvar(_fluid_present ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                              "dPorousFlow_fluid_phase_density_nodal_dvar")
                                        : nullptr),
    _fluid_saturation_nodal(
        _fluid_present ? &getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_nodal")
                       : nullptr),
    _fluid_saturation_nodal_old(
        _fluid_present ? &getMaterialPropertyOld<std::vector<Real>>("PorousFlow_saturation_nodal")
                       : nullptr),
    _dfluid_saturation_nodal_dvar(_fluid_present
                                      ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                            "dPorousFlow_saturation_nodal_dvar")
                                      : nullptr),
    _energy_nodal(_fluid_present ? &getMaterialProperty<std::vector<Real>>(
                                       "PorousFlow_fluid_phase_internal_energy_nodal")
                                 : nullptr),
    _energy_nodal_old(_fluid_present ? &getMaterialPropertyOld<std::vector<Real>>(
                                           "PorousFlow_fluid_phase_internal_energy_nodal")
                                     : nullptr),
    _denergy_nodal_dvar(_fluid_present ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                             "dPorousFlow_fluid_phase_internal_energy_nodal_dvar")
                                       : nullptr)
{
}

Real
PorousFlowEnergyTimeDerivative::computeQpResidual()
{
  Real energy = (1.0 - _porosity[_i]) * _rock_energy_nodal[_i];
  Real energy_old = (1.0 - _porosity_old[_i]) * _rock_energy_nodal_old[_i];
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    energy += (*_fluid_density)[_i][ph] * (*_fluid_saturation_nodal)[_i][ph] *
              (*_energy_nodal)[_i][ph] * _porosity[_i];
    energy_old += (*_fluid_density_old)[_i][ph] * (*_fluid_saturation_nodal_old)[_i][ph] *
                  (*_energy_nodal_old)[_i][ph] * _porosity_old[_i];
  }
  const Real strain = (_has_total_strain ? (*_total_strain_old)[_qp].trace() : 0.0);

  return _test[_i][_qp] * (1.0 + strain) * (energy - energy_old) / _dt;
}

Real
PorousFlowEnergyTimeDerivative::computeQpJacobian()
{
  // If the variable is not a PorousFlow variable (very unusual), the diag Jacobian terms are 0
  if (!_var_is_porflow_var)
    return 0.0;
  return computeQpJac(_dictator.porousFlowVariableNum(_var.number()));
}

Real
PorousFlowEnergyTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  // If the variable is not a PorousFlow variable, the OffDiag Jacobian terms are 0
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;
  return computeQpJac(_dictator.porousFlowVariableNum(jvar));
}

Real
PorousFlowEnergyTimeDerivative::computeQpJac(unsigned int pvar) const
{
  const unsigned nearest_qp = (_strain_at_nearest_qp ? (*_nearest_qp)[_i] : _i);

  const Real strain = (_has_total_strain ? (*_total_strain_old)[_qp].trace() : 0.0);

  // porosity is dependent on variables that are lumped to the nodes,
  // but it can depend on the gradient
  // of variables, which are NOT lumped to the nodes, hence:
  Real denergy = -_dporosity_dgradvar[_i][pvar] * _grad_phi[_j][_i] * _rock_energy_nodal[_i];
  for (unsigned ph = 0; ph < _num_phases; ++ph)
    denergy += (*_fluid_density)[_i][ph] * (*_fluid_saturation_nodal)[_i][ph] *
               (*_energy_nodal)[_i][ph] * _dporosity_dgradvar[_i][pvar] * _grad_phi[_j][nearest_qp];

  if (_i != _j)
    return _test[_i][_qp] * (1.0 + strain) * denergy / _dt;

  // As the fluid energy is lumped to the nodes, only non-zero terms are for _i==_j
  denergy += -_dporosity_dvar[_i][pvar] * _rock_energy_nodal[_i];
  denergy += (1.0 - _porosity[_i]) * _drock_energy_nodal_dvar[_i][pvar];
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    denergy += (*_dfluid_density_dvar)[_i][ph][pvar] * (*_fluid_saturation_nodal)[_i][ph] *
               (*_energy_nodal)[_i][ph] * _porosity[_i];
    denergy += (*_fluid_density)[_i][ph] * (*_dfluid_saturation_nodal_dvar)[_i][ph][pvar] *
               (*_energy_nodal)[_i][ph] * _porosity[_i];
    denergy += (*_fluid_density)[_i][ph] * (*_fluid_saturation_nodal)[_i][ph] *
               (*_denergy_nodal_dvar)[_i][ph][pvar] * _porosity[_i];
    denergy += (*_fluid_density)[_i][ph] * (*_fluid_saturation_nodal)[_i][ph] *
               (*_energy_nodal)[_i][ph] * _dporosity_dvar[_i][pvar];
  }
  return _test[_i][_qp] * (1.0 + strain) * denergy / _dt;
}
