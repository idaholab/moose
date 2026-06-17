//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPreDis.h"

registerMooseObject("PorousFlowApp", PorousFlowPreDis);
registerMooseObject("PorousFlowApp", ADPorousFlowPreDis);

template <bool is_ad>
InputParameters
PorousFlowPreDisTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";
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

template <bool is_ad>
PorousFlowPreDisTempl<is_ad>::PorousFlowPreDisTempl(const InputParameters & parameters)
  : PorousFlowLumpedKernelBaseTempl<is_ad>(parameters),
    _mineral_density(this->template getParam<std::vector<Real>>("mineral_density")),
    _dictator(this->template getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _aq_ph(_dictator.aqueousPhaseNumber()),
    _porosity_old(this->template getMaterialPropertyOld<Real>("PorousFlow_porosity_nodal")),
    _saturation(this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_saturation_nodal")),
    _dsaturation_dvar(is_ad ? nullptr
                            : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                                  "dPorousFlow_saturation_nodal_dvar")),
    _reaction_rate(this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_mineral_reaction_rate_nodal")),
    _dreaction_rate_dvar(is_ad
                             ? nullptr
                             : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                                   "dPorousFlow_mineral_reaction_rate_nodal_dvar")),
    _stoichiometry(this->template getParam<std::vector<Real>>("stoichiometry"))
{
  /* Not needed due to PorousFlow_mineral_reaction_rate already checking this condition
  if (_dictator.numPhases() < 1)
    mooseError("PorousFlowPreDis: The number of fluid phases must not be zero");
  */

  if (_mineral_density.size() != _dictator.numAqueousKinetic())
    this->paramError(
        "mineral_density",
        "The Dictator proclaims that the number of precipitation-dissolution secondary "
        "species in this simulation is ",
        _dictator.numAqueousKinetic(),
        " whereas you have provided ",
        _mineral_density.size(),
        ". The Dictator does not take such mistakes lightly");

  if (_stoichiometry.size() != _dictator.numAqueousKinetic())
    this->paramError(
        "stoichiometry",
        "The Dictator proclaims that the number of precipitation-dissolution secondary "
        "species in this simulation is ",
        _dictator.numAqueousKinetic(),
        " whereas you have provided ",
        _stoichiometry.size(),
        ". The Dictator does not take such mistakes lightly");
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowPreDisTempl<is_ad>::computeQpResidual()
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
  GenericReal<is_ad> res = 0.0;
  for (unsigned r = 0; r < _dictator.numAqueousKinetic(); ++r)
    res += _stoichiometry[r] * _mineral_density[r] * _reaction_rate[_i][r];
  return _test[_i][_qp] * res * _porosity_old[_i] * _saturation[_i][_aq_ph];
}

template <bool is_ad>
Real
PorousFlowPreDisTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
  {
    /// If the variable is not a PorousFlow variable (very unusual), the diag Jacobian terms are 0
    if (_dictator.notPorousFlowVariable(_var.number()))
      return 0.0;
    return computeQpJac(_dictator.porousFlowVariableNum(_var.number()));
  }
  return 0.0;
}

template <bool is_ad>
Real
PorousFlowPreDisTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  if constexpr (!is_ad)
  {
    /// If the variable is not a PorousFlow variable, the OffDiag Jacobian terms are 0
    if (_dictator.notPorousFlowVariable(jvar))
      return 0.0;
    return computeQpJac(_dictator.porousFlowVariableNum(jvar));
  }
  return 0.0;
}

template <bool is_ad>
Real
PorousFlowPreDisTempl<is_ad>::computeQpJac(unsigned int pvar)
{
  if constexpr (!is_ad)
  {
    if (_i != _j)
      return 0.0;

    Real res = 0.0;
    Real dres = 0.0;
    for (unsigned r = 0; r < _dictator.numAqueousKinetic(); ++r)
    {
      dres += _stoichiometry[r] * _mineral_density[r] * (*_dreaction_rate_dvar)[_i][r][pvar];
      res += _stoichiometry[r] * _mineral_density[r] * _reaction_rate[_i][r];
    }

    return _test[_i][_qp] *
           (dres * _saturation[_i][_aq_ph] + res * (*_dsaturation_dvar)[_i][_aq_ph][pvar]) *
           _porosity_old[_i];
  }
  return 0.0;
}

template class PorousFlowPreDisTempl<false>;
template class PorousFlowPreDisTempl<true>;
