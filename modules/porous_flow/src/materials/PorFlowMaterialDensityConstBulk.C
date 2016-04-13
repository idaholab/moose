/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorFlowMaterialDensityConstBulk.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorFlowMaterialDensityConstBulk>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<Real>("density0", "The density of each phase at zero porepressure");
  params.addRequiredParam<Real>("bulk_modulus", "The constant bulk modulus of each phase");
  params.addRequiredParam<unsigned int>("phase", "The phase number");
  params.addRequiredParam<UserObjectName>("PorFlowVarNames_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates a fluid density from its porepressure, assuming constant bulk modulus for the fluid");
  return params;
}

PorFlowMaterialDensityConstBulk::PorFlowMaterialDensityConstBulk(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _dens0(getParam<Real>("density0")),
    _bulk(getParam<Real>("bulk_modulus")),
    _phase_num(getParam<unsigned int>("phase")),
    _porflow_name_UO(getUserObject<PorFlowVarNames>("PorFlowVarNames_UO")),

    _porepressure(getMaterialProperty<std::vector<Real> >("PorFlow_porepressure")),
    _dporepressure_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorFlow_porepressure_dvar")),
    _density(declareProperty<Real>("PorFlow_fluid_phase_density" + Moose::stringify(_phase_num))),
    _density_old(declarePropertyOld<Real>("PorFlow_fluid_phase_density" + Moose::stringify(_phase_num))),
    _ddensity_dvar(declareProperty<std::vector<Real> >("dPorFlow_fluid_phase_density" + Moose::stringify(_phase_num) + "_dvar"))
{
}

void
PorFlowMaterialDensityConstBulk::initQpStatefulProperties()
{
  _ddensity_dvar[_qp].resize(_porflow_name_UO.num_v());
}

void
PorFlowMaterialDensityConstBulk::computeQpProperties()
{
  mooseAssert(_phase_num < _porepressure[_qp].size(), "PorFlowMaterialDensityConstBulk: phase number is " << _phase_num << " but size of porepressure is " << _porepressure[_qp].size() << ".  These must be equal");

  _density[_qp] = _dens0*std::exp(_porepressure[_qp][_phase_num]/_bulk);

  for (unsigned v = 0; v < _porflow_name_UO.num_v() ; ++v)
    _ddensity_dvar[_qp][v] = (1.0/_bulk)*_density[_qp]*_dporepressure_dvar[_qp][_phase_num][v];

  /*
   *  YAQI HACK !!
   *
   * I really just want to simply set
   * _density[_qp] = _dens0*std::exp(_porepressure[_qp][_phase_num]/_bulk);
   * in initQpStatefulProperties, but the Variables
   * aren't initialised at that point so moose crashes
   */
  if (_t_step == 1)
    _density_old[_qp] = _density[_qp];
}

