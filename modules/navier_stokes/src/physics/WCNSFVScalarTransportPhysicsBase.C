//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVScalarTransportPhysicsBase.h"
#include "WCNSFVCoupledAdvectionPhysicsHelper.h"
#include "WCNSFVFlowPhysics.h"
#include "NSFVBase.h"
#include "NS.h"

InputParameters
WCNSFVScalarTransportPhysicsBase::validParams()
{
  InputParameters params = NavierStokesPhysicsBase::validParams();
  params += WCNSFVCoupledAdvectionPhysicsHelper::validParams();
  params.addClassDescription(
      "Define the Navier Stokes weakly-compressible scalar field transport equation(s)");

  params += NSFVBase::commonScalarFieldAdvectionParams();

  // TODO Remove the parameter once NavierStokesFV syntax has been removed
  params.addParam<bool>(
      "add_scalar_equation",
      "Whether to add the scalar transport equation. This parameter is not necessary if "
      "using the Physics syntax");

  // These parameters are not shared because the NSFVPhysics use functors
  params.addParam<std::vector<std::vector<MooseFunctorName>>>(
      "passive_scalar_inlet_function",
      std::vector<std::vector<MooseFunctorName>>(),
      "Functors for inlet boundaries in the passive scalar equations.");

  // New functor boundary conditions
  params.deprecateParam(
      "passive_scalar_inlet_function", "passive_scalar_inlet_functors", "01/01/2025");

  // No need for the duplication
  params.addParam<std::vector<MooseFunctorName>>("passive_scalar_source", "Passive scalar sources");

  // Spatial finite volume discretization scheme
  params.transferParam<MooseEnum>(NSFVBase::validParams(),
                                  "passive_scalar_advection_interpolation");
  params.transferParam<bool>(NSFVBase::validParams(), "passive_scalar_two_term_bc_expansion");

  // Nonlinear equation solver scaling
  params.addRangeCheckedParam<std::vector<Real>>(
      "passive_scalar_scaling",
      "passive_scalar_scaling > 0.0",
      "The scaling factor for the passive scalar field variables.");

  // Parameter groups
  params.addParamNamesToGroup("passive_scalar_names initial_scalar_variables", "Variable");
  params.addParamNamesToGroup("passive_scalar_advection_interpolation passive_scalar_scaling "
                              "passive_scalar_two_term_bc_expansion",
                              "Numerical scheme");
  params.addParamNamesToGroup("passive_scalar_inlet_types passive_scalar_inlet_functors",
                              "Inlet boundary");

  return params;
}

WCNSFVScalarTransportPhysicsBase::WCNSFVScalarTransportPhysicsBase(
    const InputParameters & parameters)
  : NavierStokesPhysicsBase(parameters),
    WCNSFVCoupledAdvectionPhysicsHelper(this),
    _passive_scalar_names(getParam<std::vector<NonlinearVariableName>>("passive_scalar_names")),
    _has_scalar_equation(isParamValid("add_scalar_equation") ? getParam<bool>("add_scalar_equation")
                                                             : !usingNavierStokesFVSyntax()),
    _passive_scalar_inlet_types(getParam<MultiMooseEnum>("passive_scalar_inlet_types")),
    _passive_scalar_inlet_functors(
        getParam<std::vector<std::vector<MooseFunctorName>>>("passive_scalar_inlet_functors")),
    _passive_scalar_sources(getParam<std::vector<MooseFunctorName>>("passive_scalar_source")),
    _passive_scalar_coupled_sources(
        getParam<std::vector<std::vector<MooseFunctorName>>>("passive_scalar_coupled_source")),
    _passive_scalar_sources_coef(
        getParam<std::vector<std::vector<Real>>>("passive_scalar_coupled_source_coeff"))
{
  if (_has_scalar_equation)
    for (const auto & scalar_name : _passive_scalar_names)
      saveSolverVariableName(scalar_name);

  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_scalar_equation)
    return;

  // These parameters must be passed for every passive scalar at a time
  checkVectorParamsSameLengthIfSet<NonlinearVariableName, MooseFunctorName>(
      "passive_scalar_names", "passive_scalar_diffusivity", true);
  checkVectorParamsSameLengthIfSet<NonlinearVariableName, std::vector<MooseFunctorName>>(
      "passive_scalar_names", "passive_scalar_coupled_source", true);
  checkVectorParamsSameLengthIfSet<NonlinearVariableName, MooseFunctorName>(
      "passive_scalar_names", "passive_scalar_source", true);
  checkVectorParamsSameLengthIfSet<NonlinearVariableName, Real>(
      "passive_scalar_names", "passive_scalar_scaling", true);
  checkVectorParamsSameLengthIfSet<NonlinearVariableName, FunctionName>(
      "passive_scalar_names", "initial_scalar_variables", true);
  checkVectorParamsSameLengthIfSet<NonlinearVariableName, std::vector<MooseFunctorName>>(
      "passive_scalar_names", "passive_scalar_inlet_functors", true);
  if (_passive_scalar_inlet_functors.size())
    checkTwoDVectorParamMultiMooseEnumSameLength<MooseFunctorName>(
        "passive_scalar_inlet_functors", "passive_scalar_inlet_types", false);

  if (_passive_scalar_sources_coef.size())
    checkTwoDVectorParamsSameLength<MooseFunctorName, Real>("passive_scalar_coupled_source",
                                                            "passive_scalar_coupled_source_coeff");

  if (_porous_medium_treatment)
    _flow_equations_physics->paramError("porous_medium_treatment",
                                        "Porous media scalar advection is currently unimplemented");
}

void
WCNSFVScalarTransportPhysicsBase::addFVKernels()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_scalar_equation)
    return;

  if (isTransient())
    addScalarTimeKernels();

  addScalarAdvectionKernels();
  addScalarDiffusionKernels();
  if (_passive_scalar_sources.size() || _passive_scalar_coupled_sources.size())
    addScalarSourceKernels();
}

void
WCNSFVScalarTransportPhysicsBase::addFVBCs()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_scalar_equation)
    return;

  addScalarInletBC();
  // There is typically no wall flux of passive scalars, similarly we rarely know
  // their concentrations at the outlet at the beginning of the simulation
  // TODO: we will know the outlet values in case of flow reversal. Implement scalar outlet
  addScalarWallBC();
  addScalarOutletBC();
}

void
WCNSFVScalarTransportPhysicsBase::addInitialConditions()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_scalar_equation)
    return;
  if (!_define_variables && parameters().isParamSetByUser("initial_scalar_variables"))
    paramError("initial_scalar_variables",
               "Scalar variables are defined externally of NavierStokesFV, so should their inital "
               "conditions");
  // do not set initial conditions if we load from file
  if (getParam<bool>("initialize_variables_from_mesh_file"))
    return;
  // do not set initial conditions if we are not defining variables
  if (!_define_variables)
    return;

  InputParameters params = getFactory().getValidParams("FunctionIC");
  assignBlocks(params, _blocks);

  // There are no default initial conditions for passive scalar variables, we however
  // must obey the user-defined initial conditions, even if we are restarting
  if (parameters().isParamSetByUser("initial_scalar_variables"))
  {
    for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
    {
      params.set<VariableName>("variable") = _passive_scalar_names[name_i];
      params.set<FunctionName>("function") =
          getParam<std::vector<FunctionName>>("initial_scalar_variables")[name_i];

      getProblem().addInitialCondition("FunctionIC", _passive_scalar_names[name_i] + "_ic", params);
    }
  }
}

unsigned short
WCNSFVScalarTransportPhysicsBase::getNumberAlgebraicGhostingLayersNeeded() const
{
  unsigned short necessary_layers = getParam<unsigned short>("ghost_layers");
  necessary_layers =
      std::max(necessary_layers, _flow_equations_physics->getNumberAlgebraicGhostingLayersNeeded());
  if (getParam<MooseEnum>("passive_scalar_advection_interpolation") == "skewness-corrected")
    necessary_layers = std::max(necessary_layers, (unsigned short)3);

  return necessary_layers;
}
