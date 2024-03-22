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
#include "INSFVRhieChowInterpolator.h"
#include "RelationshipManager.h"
#include "WCNSFVFlowPhysics.h"

InputParameters
WCNSFVPhysicsBase::validParams()
{
  InputParameters params = NavierStokesFlowPhysicsBase::validParams();
  params.addClassDescription(
      "Base class to define the Navier Stokes incompressible and weakly-compressible equation");

  params.addParam<PhysicsName>("coupled_flow_physics",
                               "WCNSFVFlowPhysics generating the velocities");
  params.addParam<bool>(
      "define_variables",
      true,
      "Whether to define variables if the variables with the specified names do not exist");

  // We pull in parameters from various flow objects. This helps make sure the parameters are
  // spelled the same way and match the evolution of other objects.
  // If we remove these objects, or change their parameters, these parameters should be updated

  // Specify the weakly compressible boundary flux information. They are used for specifying in flux
  // boundary conditions for advection physics in WCNSFV
  params += NSFVAction::commonMomentumBoundaryFluxesParams();

  // Specify the numerical schemes for interpolations of velocity and pressure
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "velocity_interpolation");
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "pressure_face_interpolation");
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "momentum_face_interpolation");
  params.addParam<unsigned short>(
      "ghost_layers", 2, "Number of layers of elements to ghost near process domain boundaries");

  return params;
}

WCNSFVPhysicsBase::WCNSFVPhysicsBase(const InputParameters & parameters)
  : NavierStokesFlowPhysicsBase(parameters),
    _define_variables(getParam<bool>("define_variables")),
    _velocity_interpolation(getParam<MooseEnum>("velocity_interpolation")),
    _flux_inlet_pps(getParam<std::vector<PostprocessorName>>("flux_inlet_pps")),
    _flux_inlet_directions(getParam<std::vector<Point>>("flux_inlet_directions"))
{
  // Parameter checking
  checkSecondParamSetOnlyIfFirstOneSet("flux_inlet_pps", "flux_inlet_directions");
  if (_flux_inlet_directions.size())
    checkVectorParamsSameLengthIfSet<PostprocessorName, Point>("flux_inlet_pps",
                                                               "flux_inlet_directions");

  // Placeholder before work with components
  if (_blocks.empty())
    _blocks.push_back("ANY_BLOCK_ID");

  // Check that flow physics are consistent
  if (isParamValid("coupled_flow_physics"))
    _flow_equations_physics =
        getCoupledPhysics<WCNSFVFlowPhysics>(getParam<PhysicsName>("coupled_flow_physics"));
  else
    _flow_equations_physics = nullptr;
}

void
WCNSFVPhysicsBase::addUserObjects()
{
  addRhieChowUserObjects();
}

void
WCNSFVPhysicsBase::addRhieChowUserObjects()
{
  mooseAssert(dimension(), "0-dimension not supported");

  // This means we are solving for velocity. We dont need external RC coefficients
  bool has_flow_equations = nonlinearVariableExists(_velocity_names[0], false);

  // First make sure that we only add this object once
  // Potential cases:
  // - there is a flow physics, and an advection one (UO should be added by one)
  // - there is only an advection physics (UO should be created)
  // - there are two advection physics on different blocks with set velocities (first one picks)
  // Counting RC UOs defined on the same blocks seems to be the most fool proof option
  std::vector<UserObject *> objs;
  getProblem()
      .theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .queryInto(objs);
  unsigned int num_rc_uo = 0;
  for (const auto & obj : objs)
    if (dynamic_cast<INSFVRhieChowInterpolator *>(obj))
    {
      const auto rc_obj = dynamic_cast<INSFVRhieChowInterpolator *>(obj);
      if (rc_obj->blocks() == _blocks)
        num_rc_uo++;
      // one of the RC user object is defined everywhere
      else if (rc_obj->blocks().size() == 0 || _blocks.size() == 0)
        num_rc_uo++;
    }

  if (num_rc_uo)
    return;

  if (_verbose)
    _console << prefix() << " creating Rhie Chow user-object: " << rhieChowUOName() << std::endl;

  const std::string u_names[3] = {"u", "v", "w"};
  const auto object_type =
      _porous_medium_treatment ? "PINSFVRhieChowInterpolator" : "INSFVRhieChowInterpolator";

  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  for (unsigned int d = 0; d < dimension(); ++d)
    params.set<VariableName>(u_names[d]) = _velocity_names[d];

  params.set<VariableName>("pressure") = _pressure_name;

  if (_porous_medium_treatment)
  {
    params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
    unsigned short smoothing_layers = isParamValid("porosity_smoothing_layers")
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
  getProblem().addUserObject(object_type, rhieChowUOName(), params);
}

void
WCNSFVPhysicsBase::checkRhieChowFunctorsDefined() const
{
  if (!getProblem().hasFunctor("ax", /*thread_id=*/0))
    mooseError("Rhie Chow coefficient ax must be provided for advection by auxiliary velocities");
  if (dimension() >= 2 && !getProblem().hasFunctor("ay", /*thread_id=*/0))
    mooseError("Rhie Chow coefficient ay must be provided for advection by auxiliary velocities");
  if (dimension() == 3 && !getProblem().hasFunctor("az", /*thread_id=*/0))
    mooseError("Rhie Chow coefficient az must be provided for advection by auxiliary velocities");
}

void
WCNSFVPhysicsBase::addPostprocessors()
{
  const auto momentum_inlet_types = getParam<MultiMooseEnum>("momentum_inlet_types");

  for (unsigned int bc_ind = 0; bc_ind < momentum_inlet_types.size(); ++bc_ind)
    if (momentum_inlet_types[bc_ind] == "flux-mass" ||
        momentum_inlet_types[bc_ind] == "flux-velocity")
    {
      const std::string pp_type = "AreaPostprocessor";
      InputParameters params = getFactory().getValidParams(pp_type);
      params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};
      params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;

      const auto name_pp = "area_pp_" + _inlet_boundaries[bc_ind];
      if (!getProblem().hasUserObject(name_pp))
        getProblem().addPostprocessor(pp_type, name_pp, params);
    }
}

std::string
WCNSFVPhysicsBase::rhieChowUOName() const
{
  // This could still fail if we have 2 advecting physics
  if (_flow_equations_physics)
    return (_porous_medium_treatment ? +"pins_rhie_chow_interpolator"
                                     : "ins_rhie_chow_interpolator");
  else
    return (_porous_medium_treatment ? +"pins_rhie_chow_interpolator"
                                     : "ins_rhie_chow_interpolator");
}

unsigned short
WCNSFVPhysicsBase::getNumberAlgebraicGhostingLayersNeeded() const
{
  unsigned short necessary_layers = getParam<unsigned short>("ghost_layers");
  if (getParam<MooseEnum>("momentum_face_interpolation") == "skewness-corrected" ||
      getParam<MooseEnum>("pressure_face_interpolation") == "skewness-corrected")
    necessary_layers = std::max(necessary_layers, (unsigned short)3);

  if (_porous_medium_treatment && isParamValid("porosity_smoothing_layers"))
    necessary_layers =
        std::max(getParam<unsigned short>("porosity_smoothing_layers"), necessary_layers);

  return necessary_layers;
}

InputParameters
WCNSFVPhysicsBase::getAdditionalRMParams() const
{
  unsigned short necessary_layers = getParam<unsigned short>("ghost_layers");
  necessary_layers = std::max(necessary_layers, getNumberAlgebraicGhostingLayersNeeded());
  if (_porous_medium_treatment && isParamValid("porosity_smoothing_layers"))
    necessary_layers =
        std::max(getParam<unsigned short>("porosity_smoothing_layers"), necessary_layers);

  // Just an object that has a ghost_layers parameter
  const std::string kernel_type = "INSFVMixingLengthReynoldsStress";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.template set<unsigned short>("ghost_layers") = necessary_layers;

  return params;
}

void
WCNSFVPhysicsBase::checkCommonParametersConsistent(const InputParameters & other_params) const
{
  NavierStokesFlowPhysicsBase::checkCommonParametersConsistent(other_params);

  // Check all the parameters in WCNSFVPhysicsBase
  warnInconsistent<std::vector<PostprocessorName>>(other_params, "flux_inlet_pps");
  warnInconsistent<std::vector<Point>>(other_params, "flux_inlet_directions");
  warnInconsistent<MooseEnum>(other_params, "velocity_interpolation");
  warnInconsistent<MooseEnum>(other_params, "pressure_face_interpolation");
  warnInconsistent<MooseEnum>(other_params, "momentum_face_interpolation");
}

VariableName
WCNSFVPhysicsBase::getFlowVariableName(const std::string & short_name) const
{
  if (short_name == NS::pressure)
    return getPressureName();
  else if (short_name == NS::velocity_x && dimension() > 0)
    return getVelocityNames()[0];
  else if (short_name == NS::velocity_y && dimension() > 1)
    return getVelocityNames()[1];
  else if (short_name == NS::velocity_z && dimension() > 2)
    return getVelocityNames()[2];
  else if (short_name == NS::temperature)
    return getTemperatureName();
  else
    mooseError("Short Variable name '", short_name, "' not recognized.");
}
