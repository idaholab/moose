//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAqueousPreDisMineral.h"

registerMooseObject("PorousFlowApp", PorousFlowAqueousPreDisMineral);

InputParameters
PorousFlowAqueousPreDisMineral::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.addCoupledVar("initial_concentrations",
                       "Initial concentrations for the mineral species "
                       "(m^{3}(precipitate)/m^{3}(porous material)).  Default = 0");
  params.addPrivateParam<std::string>("pf_material_type", "mineral");
  params.addClassDescription("This Material forms a std::vector of mineral concentrations "
                             "(volume-of-mineral/volume-of-material) appropriate to the aqueous "
                             "precipitation-dissolution system provided.");
  return params;
}

PorousFlowAqueousPreDisMineral::PorousFlowAqueousPreDisMineral(const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _num_reactions(_dictator.numAqueousKinetic()),
    _aq_ph(_dictator.aqueousPhaseNumber()),
    _saturation(_nodal_material
                    ? getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_nodal")
                    : getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_qp")),
    _sec_conc(_nodal_material
                  ? declareProperty<std::vector<Real>>("PorousFlow_mineral_concentration_nodal")
                  : declareProperty<std::vector<Real>>("PorousFlow_mineral_concentration_qp")),

    _porosity_old(_nodal_material ? getMaterialPropertyOld<Real>("PorousFlow_porosity_nodal")
                                  : getMaterialPropertyOld<Real>("PorousFlow_porosity_qp")),
    _sec_conc_old(
        _nodal_material
            ? getMaterialPropertyOld<std::vector<Real>>("PorousFlow_mineral_concentration_nodal")
            : getMaterialPropertyOld<std::vector<Real>>("PorousFlow_mineral_concentration_qp")),
    _reaction_rate(
        _nodal_material
            ? getMaterialProperty<std::vector<Real>>("PorousFlow_mineral_reaction_rate_nodal")
            : getMaterialProperty<std::vector<Real>>("PorousFlow_mineral_reaction_rate_qp")),

    _initial_conc_supplied(isParamValid("initial_concentrations")),
    _num_initial_conc(_initial_conc_supplied ? coupledComponents("initial_concentrations")
                                             : _num_reactions)
{
  /* Not needed due to PorousFlow_mineral_reaction_rate already checking this condition
  if (_dictator.numPhases() < 1)
    mooseError("PorousFlowAqueousPreDisMineral: The number of fluid phases must not be zero");
  */

  if (_num_initial_conc != _dictator.numAqueousKinetic())
    mooseError("PorousFlowAqueousPreDisMineral: The number of initial concentrations is ",
               _num_initial_conc,
               " but the Dictator knows that the number of aqueous kinetic "
               "(precipitation-dissolution) reactions is ",
               _dictator.numAqueousKinetic());

  _initial_conc.resize(_num_initial_conc);
  if (_initial_conc_supplied)
    for (unsigned r = 0; r < _num_reactions; ++r)
    {
      // If initial_concentrations are elemental AuxVariables (or constants), we want to use
      // coupledGenericValue() rather than coupledGenericDofValue()
      const bool is_nodal = isCoupled("initial_concentrations")
                                ? getVar("initial_concentrations", r)->isNodal()
                                : false;

      _initial_conc[r] =
          (_nodal_material && is_nodal ? &coupledDofValues("initial_concentrations", r)
                                       : &coupledValue("initial_concentrations", r));
    }
}

void
PorousFlowAqueousPreDisMineral::initQpStatefulProperties()
{
  _sec_conc[_qp].assign(_num_reactions, 0.0);
  if (_initial_conc_supplied)
    for (unsigned r = 0; r < _num_reactions; ++r)
      _sec_conc[_qp][r] = (*_initial_conc[r])[_qp];
}

void
PorousFlowAqueousPreDisMineral::computeQpProperties()
{
  _sec_conc[_qp].resize(_num_reactions);

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
  for (unsigned r = 0; r < _num_reactions; ++r)
    _sec_conc[_qp][r] = _sec_conc_old[_qp][r] + _porosity_old[_qp] * _reaction_rate[_qp][r] *
                                                    _saturation[_qp][_aq_ph] * _dt;
}
