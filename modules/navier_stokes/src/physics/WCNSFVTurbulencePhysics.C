//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVTurbulencePhysics.h"
#include "WCNSFVFlowPhysics.h"
#include "WCNSFVFluidHeatTransferPhysics.h"
#include "WCNSFVScalarTransportPhysics.h"
#include "WCNSFVCoupledAdvectionPhysicsHelper.h"
#include "NSFVBase.h"

registerNavierStokesPhysicsBaseTasks("NavierStokesApp", WCNSFVTurbulencePhysics);
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_variable");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_fv_kernel");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_aux_variable");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_aux_kernel");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_material");

InputParameters
WCNSFVTurbulencePhysics::validParams()
{
  InputParameters params = NavierStokesPhysicsBase::validParams();
  params += WCNSFVCoupledAdvectionPhysicsHelper::validParams();
  params.addClassDescription(
      "Define a turbulence model for a incompressible or weakly-compressible Navier Stokes "
      "flow with a finite volume discretization");

  MooseEnum turbulence_type("mixing-length none", "none");
  params.addParam<MooseEnum>(
      "turbulence_handling",
      turbulence_type,
      "The way turbulent diffusivities are determined in the turbulent regime.");
  params += NSFVBase::commonTurbulenceParams();
  params.transferParam<bool>(NSFVBase::validParams(), "mixing_length_two_term_bc_expansion");

  // TODO Added to facilitate transition, remove default once NavierStokesFV action is removed
  params.addParam<AuxVariableName>(
      "mixing_length_name", "mixing_length", "Name of the mixing length auxiliary variable");
  params.deprecateParam("mixing_length_walls", "turbulence_walls", "");

  // Not implemented, re-enable with k-epsilon
  params.suppressParameter<MooseEnum>("preconditioning");

  // Add the coupled physics
  // TODO Remove the defaults once NavierStokesFV action is removed
  // It is a little risky right now because the user could forget to pass the parameter and
  // be missing the influence of turbulence on either of these physics. There is a check in the
  // constructor to present this from happening
  params.addParam<PhysicsName>(
      "fluid_heat_transfer_physics",
      "NavierStokesFV",
      "WCNSFVFluidHeatTransferPhysics generating the heat advection equations");
  params.addParam<PhysicsName>(
      "scalar_transport_physics",
      "NavierStokesFV",
      "WCNSFVScalarTransportPhysics generating the scalar advection equations");

  // Parameter groups
  params.addParamNamesToGroup("mixing_length_name mixing_length_two_term_bc_expansion",
                              "Mixing length model");
  params.addParamNamesToGroup("fluid_heat_transfer_physics turbulent_prandtl "
                              "scalar_transport_physics passive_scalar_schmidt_number",
                              "Coupled Physics");

  return params;
}

WCNSFVTurbulencePhysics::WCNSFVTurbulencePhysics(const InputParameters & parameters)
  : NavierStokesPhysicsBase(parameters),
    WCNSFVCoupledAdvectionPhysicsHelper(parameters, this),
    _turbulence_model(getParam<MooseEnum>("turbulence_handling")),
    _mixing_length_name(getParam<AuxVariableName>("mixing_length_name")),
    _turbulence_walls(getParam<std::vector<BoundaryName>>("turbulence_walls"))
{
  if (_verbose && _turbulence_model != "none")
    _console << "Creating a " << std::string(_turbulence_model) << " turbulence model."
             << std::endl;

  if (_flow_equations_physics && _flow_equations_physics->hasFlowEquations())
    _has_flow_equations = true;
  else
    _has_flow_equations = false;

  if (isParamValid("fluid_heat_transfer_physics") && _turbulence_model != "none")
  {
    _fluid_energy_physics = getCoupledPhysics<WCNSFVFluidHeatTransferPhysics>(
        getParam<PhysicsName>("fluid_heat_transfer_physics"), true);
    // Check for a missing parameter / do not support isolated physics for now
    if (!_fluid_energy_physics &&
        !getCoupledPhysics<const WCNSFVFluidHeatTransferPhysics>(true).empty())
      paramError("fluid_heat_transfer_physics",
                 "We currently do not support creating both turbulence physics and fluid heat "
                 "transfer physics that are not coupled together");
    if (_fluid_energy_physics && _fluid_energy_physics->hasEnergyEquation())
      _has_energy_equation = true;
    else
      _has_energy_equation = false;
  }
  else
  {
    _has_energy_equation = false;
    _fluid_energy_physics = nullptr;
  }

  if (isParamValid("scalar_transport_physics") && _turbulence_model != "none")
  {
    _scalar_transport_physics = getCoupledPhysics<WCNSFVScalarTransportPhysics>(
        getParam<PhysicsName>("scalar_transport_physics"), true);
    if (!_scalar_transport_physics &&
        !getCoupledPhysics<const WCNSFVScalarTransportPhysics>(true).empty())
      paramError(
          "scalar_transport_physics",
          "We currently do not support creating both turbulence physics and scalar transport "
          "physics that are not coupled together");
    if (_scalar_transport_physics && _scalar_transport_physics->hasScalarEquations())
      _has_scalar_equations = true;
    else
      _has_scalar_equations = false;
  }
  else
  {
    _has_scalar_equations = false;
    _scalar_transport_physics = nullptr;
  }

  // To help remediate the danger of the parameter setup
  if (_verbose)
  {
    if (_has_energy_equation)
      mooseInfoRepeated("Coupling turbulence physics with fluid heat transfer physics " +
                        _fluid_energy_physics->name());
    else
      mooseInfoRepeated("No fluid heat transfer equation considered by this turbulence "
                        "physics.");
    if (_has_scalar_equations)
      mooseInfoRepeated("Coupling turbulence physics with scalar transport physics " +
                        _scalar_transport_physics->name());
    else
      mooseInfoRepeated("No scalar transport equations considered by this turbulence physics.");
  }

  // Parameter checks
  if (_turbulence_model != "mixing-length")
    errorDependentParameter("turbulence_handling",
                            "mixing-length",
                            {"mixing_length_delta",
                             "mixing_length_aux_execute_on",
                             "turbulence_walls",
                             "von_karman_const",
                             "von_karman_const_0",
                             "mixing_length_two_term_bc_expansion"});
}

void
WCNSFVTurbulencePhysics::addNonlinearVariables()
{
  if (_turbulence_model == "mixing-length")
    return;
}

void
WCNSFVTurbulencePhysics::addAuxiliaryVariables()
{
  if (_turbulence_model == "mixing-length" && _define_variables)
  {
    auto params = getFactory().getValidParams("MooseVariableFVReal");
    assignBlocks(params, _blocks);
    if (isParamValid("mixing_length_two_term_bc_expansion"))
      params.set<bool>("two_term_boundary_expansion") =
          getParam<bool>("mixing_length_two_term_bc_expansion");
    getProblem().addAuxVariable("MooseVariableFVReal", _mixing_length_name, params);
  }
}

void
WCNSFVTurbulencePhysics::addFVKernels()
{
  if (_has_flow_equations)
    addFlowTurbulenceKernels();
  if (_has_energy_equation)
    addFluidEnergyTurbulenceKernels();
  if (_has_scalar_equations)
    addScalarAdvectionTurbulenceKernels();
}

void
WCNSFVTurbulencePhysics::addFlowTurbulenceKernels()
{
  if (_turbulence_model == "mixing-length")
  {
    const std::string u_names[3] = {"u", "v", "w"};
    const std::string kernel_type = "INSFVMixingLengthReynoldsStress";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseFunctorName>(NS::mixing_length) = _mixing_length_name;

    std::string kernel_name = prefix() + "ins_momentum_mixing_length_reynolds_stress_";
    if (_porous_medium_treatment)
      kernel_name = prefix() + "pins_momentum_mixing_length_reynolds_stress_";

    params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
    for (const auto dim_i : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[dim_i]) = _velocity_names[dim_i];

    for (const auto d : make_range(dimension()))
    {
      params.set<NonlinearVariableName>("variable") = _velocity_names[d];
      params.set<MooseEnum>("momentum_component") = NS::directions[d];

      getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
    }
  }
}

void
WCNSFVTurbulencePhysics::addFluidEnergyTurbulenceKernels()
{
  if (_turbulence_model == "mixing-length")
  {
    const std::string u_names[3] = {"u", "v", "w"};
    const std::string kernel_type = "WCNSFVMixingLengthEnergyDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseFunctorName>(NS::cp) = _fluid_energy_physics->getSpecificHeatName();
    params.set<MooseFunctorName>(NS::mixing_length) = _mixing_length_name;
    params.set<Real>("schmidt_number") = getParam<Real>("turbulent_prandtl");
    params.set<NonlinearVariableName>("variable") =
        _flow_equations_physics->getFluidTemperatureName();

    for (const auto dim_i : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[dim_i]) = _velocity_names[dim_i];

    if (_porous_medium_treatment)
      getProblem().addFVKernel(
          kernel_type, prefix() + "pins_energy_mixing_length_diffusion", params);
    else
      getProblem().addFVKernel(
          kernel_type, prefix() + "ins_energy_mixing_length_diffusion", params);
  }
}

void
WCNSFVTurbulencePhysics::addScalarAdvectionTurbulenceKernels()
{
  if (_turbulence_model == "mixing-length")
  {
    const std::string u_names[3] = {"u", "v", "w"};
    const std::string kernel_type = "INSFVMixingLengthScalarDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>(NS::mixing_length) = _mixing_length_name;

    for (const auto dim_i : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[dim_i]) = _velocity_names[dim_i];

    const auto & passive_scalar_names = _scalar_transport_physics->getAdvectedScalarNames();
    const auto & passive_scalar_schmidt_number =
        getParam<std::vector<Real>>("passive_scalar_schmidt_number");
    if (passive_scalar_schmidt_number.size() != passive_scalar_names.size() &&
        passive_scalar_schmidt_number.size() != 1)
      paramError("passive_scalar_schmidt_number",
                 "The number of Schmidt numbers defined is not equal to the number of passive "
                 "scalar fields!");

    for (const auto & name_i : index_range(passive_scalar_names))
    {
      params.set<NonlinearVariableName>("variable") = passive_scalar_names[name_i];
      if (passive_scalar_schmidt_number.size() > 1)
        params.set<Real>("schmidt_number") = passive_scalar_schmidt_number[name_i];
      else if (passive_scalar_schmidt_number.size() == 1)
        params.set<Real>("schmidt_number") = passive_scalar_schmidt_number[0];
      else
        params.set<Real>("schmidt_number") = 1.0;

      getProblem().addFVKernel(
          kernel_type, prefix() + passive_scalar_names[name_i] + "_mixing_length", params);
    }
  }
}

void
WCNSFVTurbulencePhysics::addAuxiliaryKernels()
{
  // Note that if we are restarting this will overwrite the restarted mixing-length
  if (_turbulence_model == "mixing-length")
  {
    const std::string ml_kernel_type = "WallDistanceMixingLengthAux";
    InputParameters ml_params = getFactory().getValidParams(ml_kernel_type);
    assignBlocks(ml_params, _blocks);
    ml_params.set<AuxVariableName>("variable") = _mixing_length_name;
    ml_params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    if (parameters().isParamValid("mixing_length_aux_execute_on"))
      ml_params.set<ExecFlagEnum>("execute_on") =
          getParam<ExecFlagEnum>("mixing_length_aux_execute_on");
    else
      ml_params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    ml_params.set<MooseFunctorName>("von_karman_const") =
        getParam<MooseFunctorName>("von_karman_const");
    ml_params.set<MooseFunctorName>("von_karman_const_0") =
        getParam<MooseFunctorName>("von_karman_const_0");
    ml_params.set<MooseFunctorName>("delta") = getParam<MooseFunctorName>("mixing_length_delta");

    getProblem().addAuxKernel(ml_kernel_type, prefix() + "mixing_length_aux ", ml_params);
  }
}

void
WCNSFVTurbulencePhysics::addMaterials()
{
  if (_turbulence_model == "mixing-length")
  {
    const std::string u_names[3] = {"u", "v", "w"};
    InputParameters params =
        getFactory().getValidParams("MixingLengthTurbulentViscosityFunctorMaterial");
    assignBlocks(params, _blocks);

    for (const auto d : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];

    params.set<MooseFunctorName>(NS::mixing_length) = _mixing_length_name;
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;

    getProblem().addMaterial("MixingLengthTurbulentViscosityFunctorMaterial",
                             prefix() + "mixing_length_material",
                             params);
  }
}

unsigned short
WCNSFVTurbulencePhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  unsigned short ghost_layers = _flow_equations_physics->getNumberAlgebraicGhostingLayersNeeded();
  if (_turbulence_model == "mixing-length")
    ghost_layers = std::max(ghost_layers, (unsigned short)3);
  return ghost_layers;
}
