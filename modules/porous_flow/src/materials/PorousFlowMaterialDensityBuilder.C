/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMaterialDensityBuilder.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowMaterialDensityBuilder>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material forms a std::vector of density out of the individual phase densities");
  return params;
}

PorousFlowMaterialDensityBuilder::PorousFlowMaterialDensityBuilder(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _porflow_name_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
    _num_phases(_porflow_name_UO.numPhases()),

    _density(declareProperty<std::vector<Real> >("PorousFlow_fluid_phase_density")),
    _density_qp(declareProperty<std::vector<Real> >("PorousFlow_fluid_phase_density_qp")),
    _density_old(declarePropertyOld<std::vector<Real> >("PorousFlow_fluid_phase_density")),
    _ddensity_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_fluid_phase_density_dvar")),
    _ddensity_qp_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_fluid_phase_density_qp_dvar"))
{
  _phase_density.resize(_num_phases);
  _phase_density_qp.resize(_num_phases);
  _dphase_density_dvar.resize(_num_phases);
  _dphase_density_qp_dvar.resize(_num_phases);
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _phase_density[ph] = &getMaterialProperty<Real>("PorousFlow_fluid_phase_density" + Moose::stringify(ph));
    _phase_density_qp[ph] = &getMaterialProperty<Real>("PorousFlow_fluid_phase_density_qp" + Moose::stringify(ph));
    _dphase_density_dvar[ph] = &getMaterialProperty<std::vector<Real> >("dPorousFlow_fluid_phase_density" + Moose::stringify(ph) + "_dvar");
    _dphase_density_qp_dvar[ph] = &getMaterialProperty<std::vector<Real> >("dPorousFlow_fluid_phase_density_qp" + Moose::stringify(ph) + "_dvar");
  }
}

void
PorousFlowMaterialDensityBuilder::initQpStatefulProperties()
{
  _density[_qp].resize(_num_phases);
  _density_qp[_qp].resize(_num_phases);
  _density_old[_qp].resize(_num_phases);
  _ddensity_dvar[_qp].resize(_num_phases);
  _ddensity_qp_dvar[_qp].resize(_num_phases);
  const unsigned int num_var = _porflow_name_UO.numVariables();
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _ddensity_dvar[_qp][ph].resize(num_var);
    _ddensity_qp_dvar[_qp][ph].resize(num_var);
  }
}

void
PorousFlowMaterialDensityBuilder::computeQpProperties()
{
  const unsigned int num_var = _porflow_name_UO.numVariables();

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _density[_qp][ph] = (*_phase_density[ph])[_qp];
    _density_qp[_qp][ph] = (*_phase_density_qp[ph])[_qp];
    for (unsigned v = 0; v < num_var; ++v)
    {
      _ddensity_dvar[_qp][ph][v] = (*_dphase_density_dvar[ph])[_qp][v];
      _ddensity_qp_dvar[_qp][ph][v] = (*_dphase_density_qp_dvar[ph])[_qp][v];
    }
  }

  /*
   *  YAQI HACK !!
   *
   * I really just want to simply set
   * _density[_qp][ph] = (*_phase_density[ph])[_qp];
   * in initQpStatefulProperties, but the Variables
   * aren't initialised at that point so moose crashes
   */
  if (_t_step == 1)
    for (unsigned int ph = 0; ph < _num_phases; ++ph)
      _density_old[_qp][ph] = _density[_qp][ph];
}

