/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterialDensityConstBulk.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowMaterialDensityConstBulk>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<Real>("density0", "The density of each phase at zero porepressure");
  params.addRequiredParam<Real>("bulk_modulus", "The constant bulk modulus of each phase");
  params.addRequiredParam<unsigned int>("phase", "The phase number");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates a fluid density from its porepressure, assuming constant bulk modulus for the fluid");
  return params;
}

PorousFlowMaterialDensityConstBulk::PorousFlowMaterialDensityConstBulk(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _dens0(getParam<Real>("density0")),
    _bulk(getParam<Real>("bulk_modulus")),
    _phase_num(getParam<unsigned int>("phase")),
    _porflow_name_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),

    _porepressure(getMaterialProperty<std::vector<Real> >("PorousFlow_porepressure")),
    _dporepressure_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_dvar")),
    _density(declareProperty<Real>("PorousFlow_fluid_phase_density" + Moose::stringify(_phase_num))),
    _density_old(declarePropertyOld<Real>("PorousFlow_fluid_phase_density" + Moose::stringify(_phase_num))),
    _ddensity_dvar(declareProperty<std::vector<Real> >("dPorousFlow_fluid_phase_density" + Moose::stringify(_phase_num) + "_dvar")),

    _porepressure_qp(getMaterialProperty<std::vector<Real> >("PorousFlow_porepressure_qp")),
    _dporepressure_qp_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_qp_dvar")),
    _density_qp(declareProperty<Real>("PorousFlow_fluid_phase_density_qp" + Moose::stringify(_phase_num))),
    _ddensity_qp_dvar(declareProperty<std::vector<Real> >("dPorousFlow_fluid_phase_density_qp" + Moose::stringify(_phase_num) + "_dvar"))
{
  if (_phase_num >= _porflow_name_UO.num_phases())
    mooseError("PorousFlowMaterialDensityConstBulk: The Dictator proclaims that the number of fluid phases is " << _porflow_name_UO.num_phases() << " while you have foolishly entered phase = " << _phase_num << ".  Be aware that the Dictator does not tolerate mistakes.");
}

void
PorousFlowMaterialDensityConstBulk::initQpStatefulProperties()
{
  _ddensity_dvar[_qp].resize(_porflow_name_UO.num_v());
  _ddensity_qp_dvar[_qp].resize(_porflow_name_UO.num_v());
  //
  _density[_qp] = _dens0*std::exp(_porepressure[_qp][_phase_num]/_bulk);
}

void
PorousFlowMaterialDensityConstBulk::computeQpProperties()
{
  mooseAssert(_phase_num < _porepressure[_qp].size(), "PorousFlowMaterialDensityConstBulk: phase number is " << _phase_num << " but size of porepressure is " << _porepressure[_qp].size() << ".  These must be equal");

  _density[_qp] = _dens0*std::exp(_porepressure[_qp][_phase_num]/_bulk);
  _density_qp[_qp] = _dens0*std::exp(_porepressure_qp[_qp][_phase_num]/_bulk);

  for (unsigned v = 0; v < _porflow_name_UO.num_v() ; ++v)
  {
    _ddensity_dvar[_qp][v] = (1.0/_bulk)*_density[_qp]*_dporepressure_dvar[_qp][_phase_num][v];
    _ddensity_qp_dvar[_qp][v] = (1.0/_bulk)*_density_qp[_qp]*_dporepressure_qp_dvar[_qp][_phase_num][v];
  }

  /*
   *  YAQI HACK !!
   *
   * I really just want to simply set
   * _density[_qp] = _dens0*std::exp(_porepressure[_qp][_phase_num]/_bulk);
   * in initQpStatefulProperties, but the Variables
   * aren't initialised at that point so moose crashes
   */
  /*if (_t_step == 1)
    _density_old[_qp] = _density[_qp];
  */
}

