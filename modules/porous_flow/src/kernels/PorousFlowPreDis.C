//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPreDis.h"

registerMooseObject("PorousFlowApp", PorousFlowPreDis);

InputParameters
PorousFlowPreDis::validParams()
{
  InputParameters params = TimeKernel::validParams();
  params.addRequiredParam<std::vector<Real>>(
      "mineral_density",
      "Density (kg(precipitate)/m^3(precipitate)) of each secondary species in the "
      "aqueous precipitation-dissolution reaction system");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
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
    _aq_ph(_dictator.aqueousPhaseNumber()),
    _porosity_old(getMaterialPropertyOld<Real>("PorousFlow_porosity_nodal")),
    _saturation(getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_nodal")),
    _dsaturation_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_saturation_nodal_dvar")),
    _reaction_rate(
        getMaterialProperty<std::vector<Real>>("PorousFlow_mineral_reaction_rate_nodal")),
    _dreaction_rate_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_mineral_reaction_rate_nodal_dvar")),
    _stoichiometry(getParam<std::vector<Real>>("stoichiometry"))
{
  /* Not needed due to PorousFlow_mineral_reaction_rate already checking this condition
  if (_dictator.numPhases() < 1)
    mooseError("PorousFlowPreDis: The number of fluid phases must not be zero");
  */

  if (_mineral_density.size() != _dictator.numAqueousKinetic())
    paramError("mineral_density",
               "The Dictator proclaims that the number of precipitation-dissolution secondary "
               "species in this simulation is ",
               _dictator.numAqueousKinetic(),
               " whereas you have provided ",
               _mineral_density.size(),
               ". The Dictator does not take such mistakes lightly");

  if (_stoichiometry.size() != _dictator.numAqueousKinetic())
    paramError("stoichiometry",
               "The Dictator proclaims that the number of precipitation-dissolution secondary "
               "species in this simulation is ",
               _dictator.numAqueousKinetic(),
               " whereas you have provided ",
               _stoichiometry.size(),
               ". The Dictator does not take such mistakes lightly");
}

Real
PorousFlowPreDis::computeQpResidual()
{
  /*
   *
   * Note the use of the OLD value of porosity here.
   * This strategy, which breaks the cyclic dependency between porosity
   * and mineral concentration, is used in
   * Kernel: PorousFlowPreDis
   * Material: PorousFlowPorosity
   * Material: PorousFlowAqueousPreDisChemistry
   * Material: PorousFlowAqueousPreDisMineral
   *
   */
  Real res = 0.0;
  for (unsigned r = 0; r < _dictator.numAqueousKinetic(); ++r)
    res += _stoichiometry[r] * _mineral_density[r] * _reaction_rate[_i][r];
  return _test[_i][_qp] * res * _porosity_old[_i] * _saturation[_i][_aq_ph];
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
  if (_i != _j)
    return 0.0;

  Real res = 0.0;
  Real dres = 0.0;
  for (unsigned r = 0; r < _dictator.numAqueousKinetic(); ++r)
  {
    dres += _stoichiometry[r] * _mineral_density[r] * _dreaction_rate_dvar[_i][r][pvar];
    res += _stoichiometry[r] * _mineral_density[r] * _reaction_rate[_i][r];
  }

  return _test[_i][_qp] *
         (dres * _saturation[_i][_aq_ph] + res * _dsaturation_dvar[_i][_aq_ph][pvar]) *
         _porosity_old[_i];
}
