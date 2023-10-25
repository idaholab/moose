//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesFlowPhysicsBase.h"
#include "NSFVAction.h"

InputParameters
NavierStokesFlowPhysicsBase::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params.addClassDescription("Define the Navier Stokes equation");

  // We pull in parameters from various flow objects. This helps make sure the parameters are
  // spelled the same way and match the evolution of other objects.
  // If we remove these objects, or change their parameters, these parameters should be updated
  // Downstream actions must either implement all these options, or redefine the parameter with
  // a restricted MooseEnum, or place an error in the constructor for unsupported configurations
  // We mostly pull the boundary parameters from NSFV Action

  params += NSFVAction::commonNavierStokesFlowParams();

  // Most downstream physics implementations are valid for porous media too
  // If yours is not, please remember to disable the 'porous_medium_treatment' parameter
  params.transferParam<bool>(NSFVAction::validParams(), "porous_medium_treatment");
  params.transferParam<MooseFunctorName>(NSFVAction::validParams(), "porosity");
  params.transferParam<unsigned short>(NSFVAction::validParams(), "porosity_smoothing_layers");

  // Momentum boundary conditions are important for advection problems as well
  params += NSFVAction::commonMomentumBoundaryTypesParams();

  // Material properties
  params.transferParam<MooseFunctorName>(NSFVAction::validParams(), "dynamic_viscosity");
  params.transferParam<MooseFunctorName>(NSFVAction::validParams(), "density");

  params.addParam<bool>(
      "boundary_conditions_all_set",
      true,
      "Whether all boundary conditions have been set using the parameters or "
      "whether additional information may be passed by other objects (THM junctions)");

  return params;
}

NavierStokesFlowPhysicsBase::NavierStokesFlowPhysicsBase(const InputParameters & parameters)
  : PhysicsBase(parameters),
    _compressibility(getParam<MooseEnum>("compressibility")),
    _porous_medium_treatment(getParam<bool>("porous_medium_treatment")),
    _porosity_name(getParam<MooseFunctorName>("porosity")),
    _flow_porosity_functor_name(isParamValid("porosity_smoothing_layers") &&
                                        getParam<unsigned short>("porosity_smoothing_layers")
                                    ? NS::smoothed_porosity
                                    : _porosity_name),
    _porosity_smoothing_layers(isParamValid("porosity_smoothing_layers")
                                   ? getParam<unsigned short>("porosity_smoothing_layers")
                                   : 0),
    _velocity_names(
        isParamValid("velocity_variable")
            ? getParam<std::vector<std::string>>("velocity_variable")
            : (_porous_medium_treatment
                   ? std::vector<std::string>(NS::superficial_velocity_vector,
                                              NS::superficial_velocity_vector + 3)
                   : std::vector<std::string>(NS::velocity_vector, NS::velocity_vector + 3))),
    _pressure_name(isParamValid("pressure_variable")
                       ? getParam<NonlinearVariableName>("pressure_variable")
                       : NS::pressure),
    _fluid_temperature_name(isParamValid("fluid_temperature_variable")
                                ? getParam<NonlinearVariableName>("fluid_temperature_variable")
                                : NS::T_fluid),
    _inlet_boundaries(getParam<std::vector<BoundaryName>>("inlet_boundaries")),
    _outlet_boundaries(getParam<std::vector<BoundaryName>>("outlet_boundaries")),
    _wall_boundaries(getParam<std::vector<BoundaryName>>("wall_boundaries")),
    _momentum_inlet_types(createMapFromVectorAndMultiMooseEnum<BoundaryName>(
        _inlet_boundaries, getParam<MultiMooseEnum>("momentum_inlet_types"))),
    _momentum_outlet_types(createMapFromVectorAndMultiMooseEnum<BoundaryName>(
        _outlet_boundaries, getParam<MultiMooseEnum>("momentum_outlet_types"))),
    _momentum_wall_types(getParam<MultiMooseEnum>("momentum_wall_types")),
    _density_name(getParam<MooseFunctorName>("density")),
    _dynamic_viscosity_name(getParam<MooseFunctorName>("dynamic_viscosity")),
    _boundary_condition_information_complete(getParam<bool>("boundary_conditions_all_set"))
{
  // Parameter checking
  if (_boundary_condition_information_complete)
  {
    checkVectorParamAndMultiMooseEnumLength<BoundaryName>("inlet_boundaries",
                                                          "momentum_inlet_types");
    checkVectorParamAndMultiMooseEnumLength<BoundaryName>("outlet_boundaries",
                                                          "momentum_outlet_types");
    checkVectorParamAndMultiMooseEnumLength<BoundaryName>("wall_boundaries", "momentum_wall_types");
  }
  checkVectorParamsNoOverlap<BoundaryName>(
      {"inlet_boundaries", "outlet_boundaries", "wall_boundaries"});
  checkSecondParamSetOnlyIfFirstOneTrue("porous_medium_treatment", "porosity");
  checkSecondParamSetOnlyIfFirstOneTrue("porous_medium_treatment", "porosity_smoothing_layers");
}

MooseFunctorName
NavierStokesFlowPhysicsBase::getPorosityFunctorName(bool smoothed) const
{
  mooseAssert(!smoothed || !_porosity_smoothing_layers,
              "Smooth porosity cannot be used without the smoothing treatment turned on");
  if (smoothed)
    return _flow_porosity_functor_name;
  else
    return _porosity_name;
}

void
NavierStokesFlowPhysicsBase::checkCommonParametersConsistent(
    const InputParameters & other_params) const
{
  // TODO C++20: make warnInconsistent a templated lambda
  warnInconsistent<MooseEnum>(other_params, "compressibility");
  warnInconsistent<std::vector<std::string>>(other_params, "velocity_variable");
  warnInconsistent<NonlinearVariableName>(other_params, "pressure_variable");
  warnInconsistent<NonlinearVariableName>(other_params, "fluid_temperature_variable");
  warnInconsistent<bool>(other_params, "porous_medium_treatment");
  warnInconsistent<MooseFunctorName>(other_params, "porosity");
  warnInconsistent<unsigned short>(other_params, "porosity_smoothing_layers");
  warnInconsistent<std::vector<BoundaryName>>(other_params, "inlet_boundaries");
  warnInconsistent<std::vector<BoundaryName>>(other_params, "outlet_boundaries");
  warnInconsistent<std::vector<BoundaryName>>(other_params, "wall_boundaries");

  warnInconsistent<MultiMooseEnum>(other_params, "momentum_inlet_types");
  warnInconsistent<MultiMooseEnum>(other_params, "momentum_outlet_types");
  warnInconsistent<MultiMooseEnum>(other_params, "momentum_wall_types");

  warnInconsistent<MooseFunctorName>(other_params, "dynamic_viscosity");
  warnInconsistent<MooseFunctorName>(other_params, "density");
}
