//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PNSLinearFVSolidHeatTransferPhysics.h"

registerPhysicsBaseTasks("NavierStokesApp", PNSLinearFVSolidHeatTransferPhysics);
registerMooseAction("NavierStokesApp", PNSLinearFVSolidHeatTransferPhysics, "add_variable");
registerMooseAction("NavierStokesApp", PNSLinearFVSolidHeatTransferPhysics, "add_ic");
registerMooseAction("NavierStokesApp", PNSLinearFVSolidHeatTransferPhysics, "add_linear_fv_kernel");
registerMooseAction("NavierStokesApp", PNSLinearFVSolidHeatTransferPhysics, "add_linear_fv_bc");
registerMooseAction("NavierStokesApp", PNSLinearFVSolidHeatTransferPhysics, "add_material");
registerMooseAction("NavierStokesApp", PNSLinearFVSolidHeatTransferPhysics, "add_preconditioning");

InputParameters
PNSLinearFVSolidHeatTransferPhysics::validParams()
{
  InputParameters params = PNSFVSolidHeatTransferPhysics::validParams();
  params.addClassDescription("Define the Navier Stokes porous media solid energy equation with "
                             "the linear finite volume discretization");

  params.addParam<bool>(
      "use_nonorthogonal_correction",
      true,
      "Whether to use a non-orthogonal correction. This can potentially slow down convergence, "
      "but reduces numerical dispersion on non-orthogonal meshes. Can be safely turned off on "
      "orthogonal meshes.");
  params.addParamNamesToGroup("use_nonorthogonal_correction", "Numerical scheme");
  params.set<std::vector<SolverSystemName>>("system_names") = {"solid_energy_system"};

  // The linear finite volume variable has no face interpolation or two-term boundary options
  params.suppressParameter<MooseEnum>("solid_temperature_face_interpolation");
  params.suppressParameter<bool>("solid_temperature_two_term_bc_expansion");
  params.suppressParameter<Real>("temperature_scaling");
  // The linear solvers do not use the nonlinear enthalpy material, which requires AD
  params.set<bool>("use_external_enthalpy_material") = true;
  params.suppressParameter<bool>("use_external_enthalpy_material");
  params.suppressParameter<MooseEnum>("preconditioning");
  // The solid heat source is specified with 'external_heat_source'
  params.suppressParameter<MooseFunctorName>("heat_source_functor");

  return params;
}

PNSLinearFVSolidHeatTransferPhysics::PNSLinearFVSolidHeatTransferPhysics(
    const InputParameters & parameters)
  : PNSFVSolidHeatTransferPhysics(parameters),
    _non_orthogonal_correction(getParam<bool>("use_nonorthogonal_correction"))
{
}

void
PNSLinearFVSolidHeatTransferPhysics::addSolverVariables()
{
  // Dont add if the user already defined the variable
  if (variableExists(_solid_temperature_name,
                     /*error_if_aux=*/true))
    checkBlockRestrictionIdentical(_solid_temperature_name,
                                   getProblem().getVariable(0, _solid_temperature_name).blocks());
  else
  {
    const std::string var_type = "MooseLinearVariableFVReal";
    auto params = getFactory().getValidParams(var_type);
    addSharedSolverVariable(var_type, _solid_temperature_name, params);
  }
}

void
PNSLinearFVSolidHeatTransferPhysics::addInitialConditions()
{
  // Linear finite volume variables are initialized by finite volume initial conditions
  if (getParam<bool>("initialize_variables_from_mesh_file"))
    return;
  if (shouldCreateIC(_solid_temperature_name,
                     _blocks,
                     /*whether IC is a default*/ !isParamSetByUser("initial_temperature"),
                     /*error if already an IC*/ isParamSetByUser("initial_temperature")))
  {
    InputParameters params = getFactory().getValidParams("FVFunctionIC");
    assignBlocks(params, _blocks);
    params.set<VariableName>("variable") = _solid_temperature_name;
    params.set<FunctionName>("function") = getParam<FunctionName>("initial_temperature");
    getProblem().addFVInitialCondition(
        "FVFunctionIC", prefix() + _solid_temperature_name + "_ic", params);
  }
}

MooseFunctorName
PNSLinearFVSolidHeatTransferPhysics::addSolidFractionFunctor()
{
  const MooseFunctorName name = prefix() + "solid_fraction";
  if (getProblem().hasFunctor(name, /*thread_id=*/0))
    return name;

  const std::string object_type = "ParsedFunctorMaterial";
  InputParameters params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  if (MooseUtils::parsesToReal(_porosity_name))
    params.set<std::string>("expression") = "1 - " + _porosity_name;
  else
  {
    params.set<std::string>("expression") = "1 - eps";
    params.set<std::vector<std::string>>("functor_names") = {_porosity_name};
    params.set<std::vector<std::string>>("functor_symbols") = {"eps"};
  }
  params.set<std::string>("property_name") = name;
  getProblem().addMaterial(object_type, name + "_material", params);
  return name;
}

void
PNSLinearFVSolidHeatTransferPhysics::addFVKernels()
{
  checkFluidAndSolidHeatTransferPhysicsParameters();

  // Time derivative: (1 - eps) rho_s cp_s dT_s/dt
  if (isTransient() &&
      shouldCreateTimeDerivative(_solid_temperature_name, _blocks, /*error if defined*/ false))
  {
    const std::string kernel_type = "LinearFVTimeDerivative";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _solid_temperature_name;

    const std::string object_type = "ParsedFunctorMaterial";
    InputParameters mat_params = getFactory().getValidParams(object_type);
    assignBlocks(mat_params, _blocks);
    const auto solid_fraction = addSolidFractionFunctor();
    std::vector<std::string> names = {solid_fraction};
    std::vector<std::string> symbols = {"f0"};
    std::vector<std::string> factors = {"f0"};
    for (const auto & functor : {_density_name, _specific_heat_name})
      if (MooseUtils::parsesToReal(functor))
        factors.push_back(functor);
      else
      {
        names.push_back(functor);
        symbols.push_back("f" + std::to_string(symbols.size()));
        factors.push_back(symbols.back());
      }
    mat_params.set<std::string>("expression") = MooseUtils::stringJoin(factors, " * ");
    mat_params.set<std::vector<std::string>>("functor_names") = names;
    mat_params.set<std::vector<std::string>>("functor_symbols") = symbols;
    const auto capacity_name = prefix() + "solid_heat_capacity";
    mat_params.set<std::string>("property_name") = capacity_name;
    getProblem().addMaterial(object_type, capacity_name + "_material", mat_params);

    params.set<MooseFunctorName>("factor") = capacity_name;
    // The solid is stationary, so (1 - eps) rho_s is constant in time and the stored energy changes
    // as (1 - eps) rho_s dh_s/dt = (1 - eps) rho_s cp_s dT_s/dt. The specific heat must stay outside
    // the derivative: d(cp_s T_s)/dt is not dh_s/dt when cp_s depends on the temperature.
    params.set<bool>("conservative_form") = false;
    getProblem().addLinearFVKernel(kernel_type, prefix() + "solid_energy_time", params);
  }

  // Conduction. The solid thermal conductivity is an effective conductivity
  {
    if (processThermalConductivity())
      paramError("thermal_conductivity_solid",
                 "Vector thermal conductivities are not supported with the linear finite volume "
                 "discretization");
    const std::string kernel_type = "LinearFVDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<LinearVariableName>("variable") = _solid_temperature_name;
    params.set<bool>("use_nonorthogonal_correction") = _non_orthogonal_correction;
    std::vector<SubdomainName> conduction_blocks;
    for (const auto & block_group : _thermal_conductivity_blocks)
      conduction_blocks.insert(conduction_blocks.end(), block_group.begin(), block_group.end());
    assignBlocks(params, _thermal_conductivity_blocks.size() ? conduction_blocks : _blocks);
    // Multiple conductivities are combined by the material of the base class
    params.set<MooseFunctorName>("diffusion_coeff") =
        _thermal_conductivity_blocks.size() > 1 ? prefix() + "combined_thermal_conductivity"
                                                : _thermal_conductivity_name[0];
    getProblem().addLinearFVKernel(kernel_type, prefix() + "solid_energy_diffusion", params);
  }

  // Volumetric heat exchange with the fluid
  if (_ambient_convection_alpha.size())
  {
    checkAmbientConvectionParameters();
    const std::string kernel_type = "LinearFVVolumetricHeatTransfer";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<LinearVariableName>("variable") = _solid_temperature_name;
    params.set<VariableName>(NS::T_solid) = _solid_temperature_name;
    params.set<bool>("is_solid") = true;
    const auto num_convection_blocks = _ambient_convection_blocks.size();
    for (const auto block_i : make_range(num_convection_blocks ? num_convection_blocks : 1))
    {
      if (num_convection_blocks)
        params.set<std::vector<SubdomainName>>("block") = _ambient_convection_blocks[block_i];
      else
        assignBlocks(params, _blocks);
      params.set<MooseFunctorName>("h_solid_fluid") = _ambient_convection_alpha[block_i];
      params.set<VariableName>(NS::T_fluid) =
          _ambient_temperature.size() > 1 ? _ambient_temperature[block_i] : _ambient_temperature[0];
      getProblem().addLinearFVKernel(
          kernel_type, prefix() + "ambient_convection_" + std::to_string(block_i), params);
    }
  }

  // External heat source, a density per unit total volume
  if (isParamValid("external_heat_source"))
  {
    const std::string kernel_type = "LinearFVSource";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<LinearVariableName>("variable") = _solid_temperature_name;
    const auto & source_blocks =
        getParam<std::vector<SubdomainName>>("external_heat_source_blocks");
    assignBlocks(params, source_blocks.size() ? source_blocks : _blocks);
    params.set<MooseFunctorName>("source_density") =
        getParam<MooseFunctorName>("external_heat_source");
    params.set<MooseFunctorName>("scaling_factor") =
        std::to_string(getParam<Real>("external_heat_source_coeff"));
    getProblem().addLinearFVKernel(kernel_type, prefix() + "external_heat_source", params);
  }
}

void
PNSLinearFVSolidHeatTransferPhysics::addFVBCs()
{
  // Faces without a boundary condition are insulated
  const auto add_bcs = [this](const std::string & bc_type,
                              const std::string & boundaries_param,
                              const std::string & functors_param,
                              const std::string & functor_param_name)
  {
    const auto & boundaries = getParam<std::vector<BoundaryName>>(boundaries_param);
    const auto & functors = getParam<std::vector<MooseFunctorName>>(functors_param);
    InputParameters params = getFactory().getValidParams(bc_type);
    params.set<LinearVariableName>("variable") = _solid_temperature_name;
    for (const auto i : index_range(boundaries))
    {
      params.set<std::vector<BoundaryName>>("boundary") = {boundaries[i]};
      params.set<MooseFunctorName>(functor_param_name) = functors[i];
      if (bc_type == "LinearFVAdvectionDiffusionFunctorNeumannBC")
        params.set<MooseFunctorName>("diffusion_coeff") =
            _thermal_conductivity_blocks.size() > 1 ? prefix() + "combined_thermal_conductivity"
                                                    : _thermal_conductivity_name[0];
      getProblem().addLinearFVBC(
          bc_type, prefix() + _solid_temperature_name + "_" + boundaries[i], params);
    }
  };

  add_bcs("LinearFVAdvectionDiffusionFunctorDirichletBC",
          "fixed_temperature_boundaries",
          "boundary_temperatures",
          "functor");
  add_bcs("LinearFVAdvectionDiffusionFunctorNeumannBC",
          "heat_flux_boundaries",
          "boundary_heat_fluxes",
          "functor");
}
