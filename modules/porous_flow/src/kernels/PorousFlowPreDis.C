//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPreDis.h"

template <>
InputParameters
validParams<PorousFlowPreDis>()
{
  InputParameters params = validParams<TimeKernel>();
  params.addParam<bool>("strain_at_nearest_qp",
                        false,
                        "When calculating nodal porosity that depends on strain, use the strain at "
                        "the nearest quadpoint.  This adds a small extra computational burden, and "
                        "is not necessary for simulations involving only linear lagrange elements. "
                        " If you set this to true, you will also want to set the same parameter to "
                        "true for related Kernels and Materials");
  params.addRequiredParam<std::vector<Real>>("mineral_density",
                                             "Density (kg/m^3) of each secondary species in the "
                                             "aqueous precipitation-dissolution reaction system");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addRequiredParam<std::vector<Real>>("stoichiometry",
                                             "A vector of stoichiometric coefficients for the "
                                             "primary species that is the Variable of this Kernel: "
                                             "one for each precipitation-dissolution reaction "
                                             "(these are one columns of the 'reactions' matrix)");
  params.addClassDescription("Precipitation-dissolution of chemical species");
  return params;
}

PorousFlowPreDis::PorousFlowPreDis(const InputParameters & parameters)
  : TimeKernel(parameters),
    _mineral_density(getParam<std::vector<Real>>("mineral_density")),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
    _porosity_old(getMaterialPropertyOld<Real>("PorousFlow_porosity_nodal")),
    _dporosity_dvar(getMaterialProperty<std::vector<Real>>("dPorousFlow_porosity_nodal_dvar")),
    _dporosity_dgradvar(
        getMaterialProperty<std::vector<RealGradient>>("dPorousFlow_porosity_nodal_dgradvar")),
    _strain_at_nearest_qp(getParam<bool>("strain_at_nearest_qp")),
    _nearest_qp(_strain_at_nearest_qp
                    ? &getMaterialProperty<unsigned int>("PorousFlow_nearestqp_nodal")
                    : nullptr),
    _sec_conc(getMaterialProperty<std::vector<Real>>("PorousFlow_mineral_concentration_nodal")),
    _sec_conc_old(
        getMaterialPropertyOld<std::vector<Real>>("PorousFlow_mineral_concentration_nodal")),
    _dsec_conc_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_mineral_concentration_nodal_dvar")),
    _stoichiometry(getParam<std::vector<Real>>("stoichiometry"))
{
  if (_mineral_density.size() != _dictator.numAqueousKinetic())
    mooseError("The Dictator proclaims that the number of precipitation-dissolution secondary "
               "species in this simulation is ",
               _dictator.numAqueousKinetic(),
               " whereas you have provided ",
               _mineral_density.size(),
               " densities of the secondary species to PorousFlowPreDis.  The Dictator does not "
               "take such mistakes lightly");
  if (_stoichiometry.size() != _dictator.numAqueousKinetic())
    mooseError("The Dictator proclaims that the number of precipitation-dissolution secondary "
               "species in this simulation is ",
               _dictator.numAqueousKinetic(),
               " whereas you have provided ",
               _stoichiometry.size(),
               " stoichiometric coefficients to PorousFlowPreDis.  The Dictator does not take such "
               "mistakes lightly");
}

Real
PorousFlowPreDis::computeQpResidual()
{
  Real res = 0.0;
  for (unsigned r = 0; r < _dictator.numAqueousKinetic(); ++r)
    res += _stoichiometry[r] * _mineral_density[r] *
           (_porosity[_i] * _sec_conc[_i][r] - _porosity_old[_i] * _sec_conc_old[_i][r]) / _dt;
  return _test[_i][_qp] * res;
}

Real
PorousFlowPreDis::computeQpJacobian()
{
  /// If the variable is not a PorousFlow variable (very unusual), the diag Jacobian terms are 0
  if (_dictator.notPorousFlowVariable(_var.number()))
    return 0.0;
  return computeQpJac(_dictator.porousFlowVariableNum(_var.number()));
}

Real
PorousFlowPreDis::computeQpOffDiagJacobian(unsigned int jvar)
{
  /// If the variable is not a PorousFlow variable, the OffDiag Jacobian terms are 0
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;
  return computeQpJac(_dictator.porousFlowVariableNum(jvar));
}

Real
PorousFlowPreDis::computeQpJac(unsigned int pvar)
{
  const unsigned nearest_qp = (_strain_at_nearest_qp ? (*_nearest_qp)[_i] : _i);

  Real res = 0.0;
  for (unsigned r = 0; r < _dictator.numAqueousKinetic(); ++r)
    res += _stoichiometry[r] * _mineral_density[r] * _sec_conc[_i][r] *
           _dporosity_dgradvar[_i][pvar] * _grad_phi[_j][nearest_qp];

  if (_i != _j)
    return _test[_i][_qp] * res / _dt;

  /// As the mineral density is lumped to the nodes, only non-zero terms are for _i==_j
  for (unsigned r = 0; r < _dictator.numAqueousKinetic(); ++r)
    res += _stoichiometry[r] * _mineral_density[r] *
           (_dsec_conc_dvar[_i][r][pvar] * _porosity[_i] +
            _sec_conc[_i][r] * _dporosity_dvar[_i][pvar]);

  return _test[_i][_qp] * res / _dt;
}
