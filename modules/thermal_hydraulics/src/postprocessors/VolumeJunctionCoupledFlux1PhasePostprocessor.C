//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeJunctionCoupledFlux1PhasePostprocessor.h"
#include "ADVolumeJunction1PhaseUserObject.h"
#include "ADNumericalFlux3EqnBase.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndicesVACE.h"

registerMooseObject("ThermalHydraulicsApp", VolumeJunctionCoupledFlux1PhasePostprocessor);

InputParameters
VolumeJunctionCoupledFlux1PhasePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addRequiredParam<unsigned int>("equation_index", "Equation index");
  params.addRequiredParam<PostprocessorName>("pressure", "Pressure post-processor");
  params.addRequiredParam<PostprocessorName>("temperature", "Temperature post-processor");
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "passives", "Passive transport post-processors, if any");
  params.addRequiredParam<Real>("A_coupled", "Area of the flux coupling");
  params.addRequiredParam<RealVectorValue>("normal_from_junction",
                                           "Unit normal vector from the junction");
  params.addRequiredParam<UserObjectName>(
      "volume_junction_uo",
      "ADVolumeJunction1PhaseUserObject object corresponding to the volume junction");
  params.addRequiredParam<UserObjectName>("numerical_flux_uo", "ADNumericalFlux3EqnBase object");
  params.addRequiredParam<UserObjectName>("fluid_properties", "SinglePhaseFluidProperties object");

  params.addClassDescription("Computes a flux for VolumeJunctionCoupledFlux1Phase.");

  return params;
}

VolumeJunctionCoupledFlux1PhasePostprocessor::VolumeJunctionCoupledFlux1PhasePostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _equation_index(getParam<unsigned int>("equation_index")),
    _p(getPostprocessorValue("pressure")),
    _T(getPostprocessorValue("temperature")),
    _A_coupled(getParam<Real>("A_coupled")),
    _normal_from_junction(getParam<RealVectorValue>("normal_from_junction")),
    _normal_to_junction(-_normal_from_junction),
    _volume_junction_uo(getUserObject<ADVolumeJunction1PhaseUserObject>("volume_junction_uo")),
    _numerical_flux_uo(getUserObject<ADNumericalFlux3EqnBase>("numerical_flux_uo")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
  const auto & passive_names = getParam<std::vector<PostprocessorName>>("passives");
  for (const auto & passive : passive_names)
    _passives.push_back(&getPostprocessorValueByName(passive));
}

void
VolumeJunctionCoupledFlux1PhasePostprocessor::finalize()
{
  const auto rho = _fp.rho_from_p_T(_p, _T);
  const auto E = _fp.e_from_p_T(_p, _T);

  std::vector<ADReal> U(THMVACE3D::N_FLUX_INPUTS + _passives.size(), 0.0);
  U[THMVACE3D::RHOA] = rho * _A_coupled;
  U[THMVACE3D::RHOEA] = rho * E * _A_coupled;
  U[THMVACE3D::AREA] = _A_coupled;
  for (const auto i : make_range(_passives.size()))
    U[THMVACE3D::N_FLUX_INPUTS + i] = *(_passives[i]) * _A_coupled;

  const auto flux_3d =
      _volume_junction_uo.compute3DFlux(_numerical_flux_uo, U, _normal_to_junction);

  _value = raw_value(flux_3d[_equation_index]);
}

PostprocessorValue
VolumeJunctionCoupledFlux1PhasePostprocessor::getValue() const
{
  return _value;
}
