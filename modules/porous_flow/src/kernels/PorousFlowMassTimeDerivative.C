//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowMassTimeDerivative.h"

#include "MooseVariable.h"

#include "libmesh/quadrature.h"

#include <limits>

registerMooseObject("PorousFlowApp", PorousFlowMassTimeDerivative);

InputParameters
PorousFlowMassTimeDerivative::validParams()
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
  params.addParam<bool>(
      "multiply_by_density",
      true,
      "If true, then this Kernel represents the time derivative of the fluid mass.  If false, then "
      "this Kernel represents the time derivative of the fluid volume (care must then be taken "
      "when using other PorousFlow objects, such as the PorousFlowFluidMass postprocessor).");
  params.addParam<unsigned int>(
      "fluid_component", 0, "The index corresponding to the component for this kernel");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.set<bool>("use_displaced_mesh") = false;
  params.suppressParameter<bool>("use_displaced_mesh");
  params.addClassDescription("Derivative of fluid-component mass with respect to time.  Mass "
                             "lumping to the nodes is used.");
  return params;
}

PorousFlowMassTimeDerivative::PorousFlowMassTimeDerivative(const InputParameters & parameters)
  : TimeKernel(parameters),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _var_is_porflow_var(_dictator.isPorousFlowVariable(_var.number())),
    _num_phases(_dictator.numPhases()),
    _strain_at_nearest_qp(getParam<bool>("strain_at_nearest_qp")),
    _multiply_by_density(getParam<bool>("multiply_by_density")),
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
    _fluid_density(_multiply_by_density ? &getMaterialProperty<std::vector<Real>>(
                                              "PorousFlow_fluid_phase_density_nodal")
                                        : nullptr),
    _fluid_density_old(_multiply_by_density ? &getMaterialPropertyOld<std::vector<Real>>(
                                                  "PorousFlow_fluid_phase_density_nodal")
                                            : nullptr),
    _dfluid_density_dvar(_multiply_by_density
                             ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                   "dPorousFlow_fluid_phase_density_nodal_dvar")
                             : nullptr),
    _fluid_saturation_nodal(getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_nodal")),
    _fluid_saturation_nodal_old(
        getMaterialPropertyOld<std::vector<Real>>("PorousFlow_saturation_nodal")),
    _dfluid_saturation_nodal_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_saturation_nodal_dvar")),
    _mass_frac(getMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal")),
    _mass_frac_old(
        getMaterialPropertyOld<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal")),
    _dmass_frac_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
        "dPorousFlow_mass_frac_nodal_dvar"))
{
  if (_fluid_component >= _dictator.numComponents())
    paramError(
        "fluid_component",
        "The Dictator proclaims that the maximum fluid component index in this simulation is ",
        _dictator.numComponents() - 1,
        " whereas you have used ",
        _fluid_component,
        ". Remember that indexing starts at 0. The Dictator does not take such mistakes lightly.");
}

Real
PorousFlowMassTimeDerivative::computeQpResidual()
{
  Real mass = 0.0;
  Real mass_old = 0.0;
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    const Real dens = (_multiply_by_density ? (*_fluid_density)[_i][ph] : 1.0);
    mass += dens * _fluid_saturation_nodal[_i][ph] * _mass_frac[_i][ph][_fluid_component];
    const Real dens_old = (_multiply_by_density ? (*_fluid_density_old)[_i][ph] : 1.0);
    mass_old +=
        dens_old * _fluid_saturation_nodal_old[_i][ph] * _mass_frac_old[_i][ph][_fluid_component];
  }
  const Real strain = (_has_total_strain ? (*_total_strain_old)[_qp].trace() : 0.0);

  return _test[_i][_qp] * (1.0 + strain) * (_porosity[_i] * mass - _porosity_old[_i] * mass_old) /
         _dt;
}

Real
PorousFlowMassTimeDerivative::computeQpJacobian()
{
  // If the variable is not a PorousFlow variable (very unusual), the diag Jacobian terms are 0
  if (!_var_is_porflow_var)
    return 0.0;
  return computeQpJac(_dictator.porousFlowVariableNum(_var.number()));
}

Real
PorousFlowMassTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  // If the variable is not a PorousFlow variable, the OffDiag Jacobian terms are 0
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;
  return computeQpJac(_dictator.porousFlowVariableNum(jvar));
}

Real
PorousFlowMassTimeDerivative::computeQpJac(unsigned int pvar)
{
  const unsigned nearest_qp = (_strain_at_nearest_qp ? (*_nearest_qp)[_i] : _i);

  const Real strain = (_has_total_strain ? (*_total_strain_old)[_qp].trace() : 0.0);

  // porosity is dependent on variables that are lumped to the nodes,
  // but it can depend on the gradient
  // of variables, which are NOT lumped to the nodes, hence:
  Real dmass = 0.0;
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    const Real dens = (_multiply_by_density ? (*_fluid_density)[_i][ph] : 1.0);
    dmass += dens * _fluid_saturation_nodal[_i][ph] * _mass_frac[_i][ph][_fluid_component] *
             _dporosity_dgradvar[_i][pvar] * _grad_phi[_j][nearest_qp];
  }

  if (_i != _j)
    return _test[_i][_qp] * (1.0 + strain) * dmass / _dt;

  // As the fluid mass is lumped to the nodes, only non-zero terms are for _i==_j
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    if (_multiply_by_density)
      dmass += (*_dfluid_density_dvar)[_i][ph][pvar] * _fluid_saturation_nodal[_i][ph] *
               _mass_frac[_i][ph][_fluid_component] * _porosity[_i];
    const Real dens = (_multiply_by_density ? (*_fluid_density)[_i][ph] : 1.0);
    dmass += dens * _dfluid_saturation_nodal_dvar[_i][ph][pvar] *
             _mass_frac[_i][ph][_fluid_component] * _porosity[_i];
    dmass += dens * _fluid_saturation_nodal[_i][ph] *
             _dmass_frac_dvar[_i][ph][_fluid_component][pvar] * _porosity[_i];
    dmass += dens * _fluid_saturation_nodal[_i][ph] * _mass_frac[_i][ph][_fluid_component] *
             _dporosity_dvar[_i][pvar];
  }
  return _test[_i][_qp] * (1.0 + strain) * dmass / _dt;
}
