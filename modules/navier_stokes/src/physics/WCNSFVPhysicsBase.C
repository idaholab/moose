//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVPhysicsBase.h"
#include "NSFVAction.h"

InputParameters
WCNSFVPhysicsBase::validParams()
{
  InputParameters params = NavierStokesFlowPhysics::validParams();
  params.addClassDescription(
      "Base class to define the Navier Stokes incompressible and weakly-compressible equation");

  // We pull in parameters from various flow objects. This helps make sure the parameters are
  // spelled the same way and match the evolution of other objects.
  // If we remove these objects, or change their parameters, these parameters should be updated

  // Specify the weakly compressible boundary flux information
  params.transferParam<std::vector<std::vector<FunctionName>>>(NSFVAction::validParams(),
                                                               "momentum_inlet_function");
  params.transferParam<std::vector<PostprocessorName>>(NSFVAction::validParams(), "flux_inlet_pps");
  params.transferParam<std::vector<Point>>(NSFVAction::validParams(), "flux_inlet_directions");
  params.transferParam<std::vector<FunctionName>>(NSFVAction::validParams(), "pressure_function");

  // Specify the numerical schemes for interpolations of velocity and pressure
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "velocity_interpolation");
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "pressure_face_interpolation");
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "velocity_face_interpolation");

  return params;
}

WCNSFVPhysicsBase::WCNSFVPhysicsBase(const InputParameters & parameters)
  : NavierStokesFlowPhysics(parameters),
    _velocity_interpolation(getParam<MooseEnum>("velocity_interpolation"))
{
  // Parameter checking
  // checkVectorParamsSameLengthOrZero<BoundaryName, MooseEnum>("inlet_boundaries",
  // "flux_inlet_pps");
  // checkVectorParamsSameLengthOrZero<BoundaryName,
  // MooseEnum>("inlet_boundaries",
  //                                                            "flux_inlet_directions");
}

void
WCNSFVPhysicsBase::addUserObjects()
{
  addRhieChowUserObjects();
}

void
WCNSFVPhysicsBase::addRhieChowUserObjects()
{
  // This means we are solving for velocity. We dont need external RC coefficients
  bool has_flow_equations = nonLinearVariableExists(_velocity_names[0], false);

  // First make sure that we only add this object once
  // Potential cases:
  // - there is a flow physics, and an advection one (add with flow)
  // - there is only an advection physics
  // - there are two advection physics on different blocks
  if (hasCoupledFlowPhysics())
    return;
  if (!hasCoupledFlowPhysics() && has_flow_equations)
    return;

  const std::string u_names[3] = {"u", "v", "w"};
  const auto object_type =
      _porous_medium_treatment ? "PINSFVRhieChowInterpolator" : "INSFVRhieChowInterpolator";

  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  for (unsigned int d = 0; d < _dim; ++d)
    params.set<VariableName>(u_names[d]) = _velocity_names[d];

  params.set<VariableName>("pressure") = _pressure_name;

  if (_porous_medium_treatment)
  {
    params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
    unsigned short smoothing_layers = parameters().isParamValid("porosity_smoothing_layers")
                                          ? getParam<unsigned short>("porosity_smoothing_layers")
                                          : 0;
    params.set<unsigned short>("smoothing_layers") = smoothing_layers;
  }

  if (!has_flow_equations)
  {
    checkRhieChowFunctorsDefined();
    params.set<MooseFunctorName>("a_u") = "ax";
    params.set<MooseFunctorName>("a_v") = "ay";
    params.set<MooseFunctorName>("a_w") = "az";
  }

  params.applySpecificParameters(parameters(), INSFVRhieChowInterpolator::listOfCommonParams());
  getProblem().addUserObject(object_type, prefix() + "rhie_chow_interpolator", params);
}

void
WCNSFVPhysicsBase::checkRhieChowFunctorsDefined() const
{
  if (!getProblem().hasFunctor("ax", /*thread_id=*/0))
    mooseError("Rhie Chow coefficient ax must be provided for advection by auxiliary velocities");
  if (_dim >= 2 && !getProblem().hasFunctor("ay", /*thread_id=*/0))
    mooseError("Rhie Chow coefficient ay must be provided for advection by auxiliary velocities");
  if (_dim == 3 && !getProblem().hasFunctor("az", /*thread_id=*/0))
    mooseError("Rhie Chow coefficient az must be provided for advection by auxiliary velocities");
}

void
WCNSFVPhysicsBase::addPostprocessors()
{
  const auto momentum_inlet_types = getParam<std::vector<MooseEnum>>("momentum_inlet_types");

  for (unsigned int bc_ind = 0; bc_ind < _inlet_boundaries.size(); ++bc_ind)
    if (momentum_inlet_types[bc_ind] == "flux-mass" ||
        momentum_inlet_types[bc_ind] == "flux-velocity")
    {
      const std::string pp_type = "AreaPostprocessor";
      InputParameters params = getFactory().getValidParams(pp_type);
      params.template set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};
      params.template set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;

      getProblem().addPostprocessor(pp_type, "area_pp_" + _inlet_boundaries[bc_ind], params);
    }
}
