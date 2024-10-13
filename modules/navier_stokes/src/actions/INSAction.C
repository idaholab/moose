//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "INSAction.h"

#include "NS.h"
#include "AddVariableAction.h"
#include "MooseObject.h"
#include "INSADObjectTracker.h"
#include "NonlinearSystemBase.h"

// MOOSE includes
#include "FEProblem.h"

#include "libmesh/fe.h"
#include "libmesh/vector_value.h"
#include "libmesh/string_to_enum.h"

using namespace libMesh;

registerMooseAction("NavierStokesApp", INSAction, "append_mesh_generator");
registerMooseAction("NavierStokesApp", INSAction, "add_navier_stokes_variables");
registerMooseAction("NavierStokesApp", INSAction, "add_navier_stokes_ics");
registerMooseAction("NavierStokesApp", INSAction, "add_navier_stokes_kernels");
registerMooseAction("NavierStokesApp", INSAction, "add_navier_stokes_bcs");
registerMooseAction("NavierStokesApp", INSAction, "add_material");

InputParameters
INSAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("This class allows us to have a section of the input file for "
                             "setting up incompressible Navier-Stokes equations.");

  MooseEnum type("steady-state transient", "steady-state");
  params.addParam<MooseEnum>("equation_type", type, "Navier-Stokes equation type");

  params.addParam<std::vector<SubdomainName>>(
      "block", {}, "The list of block ids (SubdomainID) on which NS equation is defined on");

  // temperature equation parameters
  params.addParam<bool>("boussinesq_approximation", false, "True to have Boussinesq approximation");
  params.addParam<MaterialPropertyName>(
      "reference_temperature_name", "temp_ref", "Material property name for reference temperature");
  params.addParam<MaterialPropertyName>(
      "thermal_expansion_name", "alpha", "The name of the thermal expansion");

  params.addParam<bool>("add_temperature_equation", false, "True to add temperature equation");
  params.addParam<VariableName>(
      "temperature_variable", NS::temperature, "Temperature variable name");
  params.addParam<Real>("temperature_scaling", 1, "Scaling for the temperature variable");
  params.addParam<Real>(
      "initial_temperature", 0, "The initial temperature, assumed constant everywhere");
  params.addParam<MaterialPropertyName>(
      "thermal_conductivity_name", "k", "The name of the thermal conductivity");
  params.addParam<MaterialPropertyName>(
      "specific_heat_name", "cp", "The name of the specific heat");
  params.addParam<std::vector<BoundaryName>>("natural_temperature_boundary",
                                             std::vector<BoundaryName>(),
                                             "Natural boundaries for temperature equation");
  params.addParam<std::vector<BoundaryName>>("fixed_temperature_boundary",
                                             std::vector<BoundaryName>(),
                                             "Dirichlet boundaries for temperature equation");
  params.addParam<std::vector<FunctionName>>(
      "temperature_function", std::vector<FunctionName>(), "Temperature on Dirichlet boundaries");
  addAmbientConvectionParams(params);
  params.addParam<bool>(
      "has_heat_source", false, "Whether there is a heat source function object in the simulation");
  params.addParam<FunctionName>("heat_source_function", "The function describing the heat source");
  params.addCoupledVar("heat_source_var", "The coupled variable describing the heat source");

  params.addParam<RealVectorValue>(
      "gravity", RealVectorValue(0, 0, 0), "Direction of the gravity vector");

  params.addParam<MaterialPropertyName>(
      "dynamic_viscosity_name", "mu", "The name of the dynamic viscosity");
  params.addParam<MaterialPropertyName>("density_name", "rho", "The name of the density");

  params.addParam<bool>("use_ad", false, "True to use AD");
  params.addParam<bool>(
      "laplace", true, "Whether the viscous term of the momentum equations is in laplace form");
  params.addParam<bool>(
      "integrate_p_by_parts", true, "Whether to integrate the pressure term by parts");
  params.addParam<bool>(
      "convective_term", true, "Whether to include the convective term in Jacobian");
  params.addParam<bool>(
      "supg", false, "Whether to perform SUPG stabilization of the momentum residuals");
  params.addParam<bool>(
      "pspg", false, "Whether to perform PSPG stabilization of the mass equation");
  params.addParam<Real>("alpha", 1, "Multiplicative factor on the stabilization parameter tau");
  params.addParam<bool>("add_standard_velocity_variables_for_ad",
                        true,
                        "True to convert vector velocity variables into standard aux variables");
  params.addParam<bool>(
      "has_coupled_force",
      false,
      "Whether the simulation has a force due to a coupled vector variable/vector function");
  params.addCoupledVar("coupled_force_var", "The variable(s) providing the coupled force(s)");
  params.addParam<std::vector<FunctionName>>("coupled_force_vector_function",
                                             "The function(s) standing in as a coupled force");

  params.addParam<std::vector<BoundaryName>>(
      "velocity_boundary", std::vector<BoundaryName>(), "Boundaries with given velocities");
  params.addParam<std::vector<FunctionName>>(
      "velocity_function", std::vector<FunctionName>(), "Functions for boundary velocities");
  params.addParam<unsigned int>("pressure_pinned_node",
                                "The node where pressure needs to be pinned");
  params.addParam<std::vector<BoundaryName>>(
      "no_bc_boundary", std::vector<BoundaryName>(), "The so-called no-bc Boundaries");
  params.addParam<std::vector<BoundaryName>>(
      "pressure_boundary", std::vector<BoundaryName>(), "Boundaries with given pressures");
  params.addParam<std::vector<FunctionName>>(
      "pressure_function", std::vector<FunctionName>(), "Functions for boundary pressures");

  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies(), "LAGRANGE");
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<MooseEnum>(
      "family", families, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of the FE shape function to use "
                             "for this variable (additional orders not listed are "
                             "allowed)");
  params.addParam<Real>("pressure_scaling", 1, "Scaling for the pressure variable");
  params.addParam<RealVectorValue>(
      "velocity_scaling", RealVectorValue(1, 1, 1), "Scaling for the velocity variables");

  params.addParam<Real>("initial_pressure", 0, "The initial pressure, assumed constant everywhere");

  // We perturb slightly from zero to avoid divide by zero exceptions from stabilization terms
  // involving a velocity norm in the denominator
  params.addParam<RealVectorValue>("initial_velocity",
                                   RealVectorValue(1e-15, 1e-15, 1e-15),
                                   "The initial velocity, assumed constant everywhere");
  params.addParam<std::string>("pressure_variable_name",
                               "A name for the pressure variable. If this is not provided, a "
                               "sensible default will be used.");
  params.addParam<NonlinearSystemName>(
      "nl_sys", "nl0", "The nonlinear system that this action belongs to.");

  params.addParamNamesToGroup(
      "equation_type block gravity dynamic_viscosity_name density_name boussinesq_approximation "
      "reference_temperature_name thermal_expansion_name",
      "Base");
  params.addParamNamesToGroup("use_ad laplace integrate_p_by_parts convective_term supg pspg alpha",
                              "WeakFormControl");
  params.addParamNamesToGroup("velocity_boundary velocity_function pressure_pinned_node "
                              "no_bc_boundary pressure_boundary pressure_function",
                              "BoundaryCondition");
  params.addParamNamesToGroup(
      "family order pressure_scaling velocity_scaling initial_pressure initial_velocity",
      "Variable");
  params.addParamNamesToGroup(
      "add_temperature_equation temperature_variable temperature_scaling initial_temperature "
      "thermal_conductivity_name specific_heat_name natural_temperature_boundary "
      "fixed_temperature_boundary temperature_function",
      "Temperature");
  return params;
}

INSAction::INSAction(const InputParameters & parameters)
  : Action(parameters),
    _type(getParam<MooseEnum>("equation_type")),
    _blocks(getParam<std::vector<SubdomainName>>("block")),
    _velocity_boundary(getParam<std::vector<BoundaryName>>("velocity_boundary")),
    _velocity_function(getParam<std::vector<FunctionName>>("velocity_function")),
    _pressure_boundary(getParam<std::vector<BoundaryName>>("pressure_boundary")),
    _pressure_function(getParam<std::vector<FunctionName>>("pressure_function")),
    _no_bc_boundary(getParam<std::vector<BoundaryName>>("no_bc_boundary")),
    _has_pinned_node(isParamValid("pressure_pinned_node")),
    _pinned_node("ins_pinned_node"),
    _fixed_temperature_boundary(getParam<std::vector<BoundaryName>>("fixed_temperature_boundary")),
    _temperature_function(getParam<std::vector<FunctionName>>("temperature_function")),
    _fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
             Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))),
    _use_ad(getParam<bool>("use_ad")),
    _temperature_variable_name(getParam<VariableName>("temperature_variable")),
    _pressure_variable_name(isParamValid("pressure_variable_name")
                                ? getParam<std::string>("pressure_variable_name")
                                : "p")
{
  if (_pressure_function.size() != _pressure_boundary.size())
    paramError("pressure_function",
               "Size is not the same as the number of boundaries in 'pressure_boundary'");
  if (_temperature_function.size() != _fixed_temperature_boundary.size())
    paramError("temperature_function",
               "Size is not the same as the number of boundaries in 'fixed_temperature_boundary'");
  if (_use_ad)
  {
    if (parameters.isParamSetByUser("convective_term"))
      mooseWarning("'convective_term' is ignored for AD");
  }
  else
  {
    if (getParam<bool>("boussinesq_approximation"))
      mooseError("Boussinesq approximation has not been implemented for non-AD");
  }

  if (getParam<bool>("has_ambient_convection"))
  {
    if (!isParamValid("ambient_convection_alpha"))
      mooseError(
          "If 'has_ambient_convection' is true, then 'ambient_convection_alpha' must be set.");

    if (!isParamValid("ambient_temperature"))
      mooseError("If 'has_ambient_convection' is true, then 'ambient_temperature' must be set.");
  }

  if (getParam<bool>("has_heat_source"))
  {
    bool has_coupled = isParamValid("heat_source_var");
    bool has_function = isParamValid("heat_source_function");
    if (!has_coupled && !has_function)
      mooseError("Either the 'heat_source_var' or 'heat_source_function' param must be "
                 "set for the "
                 "'INSADEnergySource' object");
    else if (has_coupled && has_function)
      mooseError("Both the 'heat_source_var' or 'heat_source_function' param are set for the "
                 "'INSADEnergySource' object. Please use one or the other.");
  }

  if (getParam<bool>("has_coupled_force"))
  {
    bool has_coupled = isParamValid("coupled_force_var");
    bool has_function = isParamValid("coupled_force_vector_function");
    if (!has_coupled && !has_function)
      mooseError("Either the 'coupled_force_var' or 'coupled_force_vector_function' param must be "
                 "set for the "
                 "'INSADMomentumCoupledForce' object");
  }
}

void
INSAction::act()
{
  if (_current_task == "append_mesh_generator")
  {
    if (_has_pinned_node)
    {
      if (_app.getMeshGeneratorNames().size() == 0)
        mooseError("The new mesh generator system is required to pin pressure");

      InputParameters params = _factory.getValidParams("ExtraNodesetGenerator");
      params.set<std::vector<BoundaryName>>("new_boundary") = {_pinned_node};
      params.set<std::vector<unsigned int>>("nodes") = {
          getParam<unsigned int>("pressure_pinned_node")};
      _app.appendMeshGenerator("ExtraNodesetGenerator", _pinned_node, params);
    }
  }

  if (_current_task == "add_navier_stokes_variables")
  {
    _dim = _mesh->dimension();
    for (const auto & subdomain_name : _blocks)
    {
      SubdomainID id = _mesh->getSubdomainID(subdomain_name);
      _block_ids.insert(id);
      if (_problem->getCoordSystem(id) != Moose::COORD_XYZ)
        mooseError("RZ has not been added in action");
    }
    if (_blocks.size() == 0)
    {
      for (auto & id : _mesh->meshSubdomains())
        if (_problem->getCoordSystem(id) != Moose::COORD_XYZ)
          mooseError("RZ has not been added in action");
    }
    if (_velocity_function.size() != _velocity_boundary.size() * _dim)
      paramError("velocity_function",
                 "Size is not the same as the number of boundaries in 'velocity_boundary' times "
                 "the mesh dimension");

    // FIXME: need to check boundaries are non-overlapping and enclose the blocks

    auto var_type = AddVariableAction::variableType(_fe_type);
    auto base_params = _factory.getValidParams(var_type);
    if (_block_ids.size() != 0)
      for (const SubdomainID & id : _block_ids)
        base_params.set<std::vector<SubdomainName>>("block").push_back(Moose::stringify(id));
    base_params.set<MooseEnum>("family") = Moose::stringify(_fe_type.family);
    base_params.set<MooseEnum>("order") = _fe_type.order.get_order();

    // add primal variables
    InputParameters params(base_params);
    params.set<MooseEnum>("order") = _fe_type.order.get_order();

    if (_use_ad)
    {
      // AD is using vector variables
      if (_fe_type.family != LAGRANGE)
        mooseError("AD has to use LAGRANGE variable family");
      FEType fetype(_fe_type.order.get_order(), LAGRANGE_VEC);
      auto vec_var_type = AddVariableAction::variableType(fetype);
      auto adparams = _factory.getValidParams(vec_var_type);
      if (_block_ids.size() != 0)
        for (const SubdomainID & id : _block_ids)
          adparams.set<std::vector<SubdomainName>>("block").push_back(Moose::stringify(id));
      adparams.set<MooseEnum>("family") = Moose::stringify(fetype.family);
      adparams.set<MooseEnum>("order") = _fe_type.order.get_order();

      auto vscaling = getParam<RealVectorValue>("velocity_scaling");
      adparams.set<std::vector<Real>>("scaling").push_back(vscaling(0));
      _problem->addVariable(vec_var_type, NS::velocity, adparams);

      // add normal velocity aux variables
      if (getParam<bool>("add_standard_velocity_variables_for_ad"))
      {
        _problem->addAuxVariable(var_type, NS::velocity_x, base_params);
        if (_dim >= 2)
          _problem->addAuxVariable(var_type, NS::velocity_y, base_params);
        if (_dim >= 3)
          _problem->addAuxVariable(var_type, NS::velocity_z, base_params);
      }
    }
    else
    {
      auto vscaling = getParam<RealVectorValue>("velocity_scaling");
      params.set<std::vector<Real>>("scaling") = {vscaling(0)};
      _problem->addVariable(var_type, NS::velocity_x, params);
      if (_dim >= 2)
      {
        params.set<std::vector<Real>>("scaling") = {vscaling(1)};
        _problem->addVariable(var_type, NS::velocity_y, params);
      }
      if (_dim >= 3)
      {
        params.set<std::vector<Real>>("scaling") = {vscaling(2)};
        _problem->addVariable(var_type, NS::velocity_z, params);
      }
    }

    if (getParam<bool>("add_temperature_equation") &&
        !_problem
             ->getNonlinearSystemBase(_problem->nlSysNum(getParam<NonlinearSystemName>("nl_sys")))
             .hasVariable(_temperature_variable_name))
    {
      params.set<std::vector<Real>>("scaling") = {getParam<Real>("temperature_scaling")};
      _problem->addVariable(var_type, _temperature_variable_name, params);
    }

    // for non-stablized form, the FE order for pressure need to be at least one order lower
    int order = _fe_type.order.get_order();
    if (!getParam<bool>("pspg"))
      order -= 1;
    params.set<MooseEnum>("order") = order;
    params.set<std::vector<Real>>("scaling") = {getParam<Real>("pressure_scaling")};
    _problem->addVariable(var_type, _pressure_variable_name, params);
  }

  if (_current_task == "add_navier_stokes_ics")
  {
    auto vvalue = getParam<RealVectorValue>("initial_velocity");
    Real pvalue = getParam<Real>("initial_pressure");

    if (_use_ad)
    {
      if (vvalue.norm() != 0)
      {
        InputParameters params = _factory.getValidParams("VectorConstantIC");
        params.set<VariableName>("variable") = NS::velocity;
        params.set<Real>("x_value") = vvalue(0);
        if (_dim >= 2)
          params.set<Real>("y_value") = vvalue(1);
        if (_dim >= 3)
          params.set<Real>("z_value") = vvalue(2);
        _problem->addInitialCondition("VectorConstantIC", "velocity_ic", params);
      }
    }
    else
    {
      if (vvalue(0) != 0)
      {
        InputParameters params = _factory.getValidParams("ConstantIC");
        params.set<VariableName>("variable") = NS::velocity_x;
        params.set<Real>("value") = vvalue(0);
        _problem->addInitialCondition("ConstantIC", NS::velocity_x + "_ic", params);
      }
      if (vvalue(1) != 0 && _dim >= 2)
      {
        InputParameters params = _factory.getValidParams("ConstantIC");
        params.set<VariableName>("variable") = NS::velocity_y;
        params.set<Real>("value") = vvalue(1);
        _problem->addInitialCondition("ConstantIC", NS::velocity_y + "_ic", params);
      }
      if (vvalue(2) != 0 && _dim >= 3)
      {
        InputParameters params = _factory.getValidParams("ConstantIC");
        params.set<VariableName>("variable") = NS::velocity_z;
        params.set<Real>("value") = vvalue(2);
        _problem->addInitialCondition("ConstantIC", NS::velocity_z + "_ic", params);
      }
    }

    if (getParam<bool>("add_temperature_equation"))
    {
      Real tvalue = getParam<Real>("initial_temperature");
      InputParameters params = _factory.getValidParams("ConstantIC");
      params.set<VariableName>("variable") = _temperature_variable_name;
      params.set<Real>("value") = tvalue;
      _problem->addInitialCondition("ConstantIC", "temperature_ic", params);
    }

    if (pvalue != 0)
    {
      InputParameters params = _factory.getValidParams("ConstantIC");
      params.set<VariableName>("variable") = _pressure_variable_name;
      params.set<Real>("value") = pvalue;
      _problem->addInitialCondition("ConstantIC", "pressure_ic", params);
    }
  }

  if (_current_task == "add_navier_stokes_kernels")
  {
    if (_type == "transient")
      addINSTimeKernels();

    // Add all the inviscid flux Kernels.
    addINSMass();
    addINSMomentum();

    if (getParam<bool>("add_temperature_equation"))
      addINSTemperature();

    if (_use_ad && getParam<bool>("add_standard_velocity_variables_for_ad"))
      addINSVelocityAux();
  }

  if (_current_task == "add_navier_stokes_bcs")
  {
    if (_velocity_boundary.size() > 0)
      addINSVelocityBC();

    if (_has_pinned_node)
      addINSPinnedPressureBC();

    if (_no_bc_boundary.size() > 0)
      addINSNoBCBC();

    if (_pressure_boundary.size() > 0)
      addINSPressureBC();

    if (getParam<bool>("add_temperature_equation"))
    {
      if (_fixed_temperature_boundary.size() > 0)
        addINSTemperatureBC();
    }
  }

  if (_current_task == "add_material" && _use_ad)
  {
    auto set_common_parameters = [&](InputParameters & params)
    {
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      params.set<CoupledName>("velocity") = {NS::velocity};
      params.set<CoupledName>(NS::pressure) = {_pressure_variable_name};
      params.set<MaterialPropertyName>("mu_name") =
          getParam<MaterialPropertyName>("dynamic_viscosity_name");
      params.set<MaterialPropertyName>("rho_name") = getParam<MaterialPropertyName>("density_name");
    };

    auto set_common_3eqn_parameters = [&](InputParameters & params)
    {
      set_common_parameters(params);
      params.set<CoupledName>("temperature") = {_temperature_variable_name};
      params.set<MaterialPropertyName>("cp_name") =
          getParam<MaterialPropertyName>("specific_heat_name");
    };

    if (getParam<bool>("add_temperature_equation"))
    {
      if (getParam<bool>("supg") || getParam<bool>("pspg"))
      {
        InputParameters params = _factory.getValidParams("INSADStabilized3Eqn");
        set_common_3eqn_parameters(params);
        params.set<Real>("alpha") = getParam<Real>("alpha");
        params.set<MaterialPropertyName>("k_name") =
            getParam<MaterialPropertyName>("thermal_conductivity_name");
        _problem->addMaterial("INSADStabilized3Eqn", "ins_ad_material", params);
      }
      else
      {
        InputParameters params = _factory.getValidParams("INSAD3Eqn");
        set_common_3eqn_parameters(params);
        _problem->addMaterial("INSAD3Eqn", "ins_ad_material", params);
      }
    }
    else
    {
      if (getParam<bool>("supg") || getParam<bool>("pspg"))
      {
        InputParameters params = _factory.getValidParams("INSADTauMaterial");
        set_common_parameters(params);
        params.set<Real>("alpha") = getParam<Real>("alpha");
        _problem->addMaterial("INSADTauMaterial", "ins_ad_material", params);
      }
      else
      {
        InputParameters params = _factory.getValidParams("INSADMaterial");
        set_common_parameters(params);
        _problem->addMaterial("INSADMaterial", "ins_ad_material", params);
      }
    }
  }
}

void
INSAction::addINSTimeKernels()
{
  if (_use_ad)
  {
    const std::string kernel_type = "INSADMomentumTimeDerivative";
    InputParameters params = _factory.getValidParams(kernel_type);
    if (_blocks.size() > 0)
      params.set<std::vector<SubdomainName>>("block") = _blocks;
    params.set<NonlinearVariableName>("variable") = NS::velocity;
    _problem->addKernel(kernel_type, "ins_velocity_time_deriv", params);

    if (getParam<bool>("add_temperature_equation"))
    {
      const std::string kernel_type = "INSADHeatConductionTimeDerivative";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_variable_name;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      _problem->addKernel(kernel_type, "ins_temperature_time_deriv", params);
    }
  }
  else
  {
    const std::string kernel_type = "INSMomentumTimeDerivative";
    InputParameters params = _factory.getValidParams(kernel_type);
    if (_blocks.size() > 0)
      params.set<std::vector<SubdomainName>>("block") = _blocks;
    params.set<MaterialPropertyName>("rho_name") = getParam<MaterialPropertyName>("density_name");

    const static std::string momentums[3] = {NS::velocity_x, NS::velocity_y, NS::velocity_z};
    for (unsigned int component = 0; component < _dim; ++component)
    {
      params.set<NonlinearVariableName>("variable") = momentums[component];
      _problem->addKernel(kernel_type, momentums[component] + "_time_deriv", params);
    }

    if (getParam<bool>("add_temperature_equation"))
    {
      const std::string kernel_type = "INSTemperatureTimeDerivative";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_variable_name;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      params.set<MaterialPropertyName>("rho_name") = getParam<MaterialPropertyName>("density_name");
      params.set<MaterialPropertyName>("cp_name") =
          getParam<MaterialPropertyName>("specific_heat_name");
      _problem->addKernel(kernel_type, "ins_temperature_time_deriv", params);
    }
  }
}

void
INSAction::addINSMass()
{
  if (_use_ad)
  {
    {
      const std::string kernel_type = "INSADMass";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _pressure_variable_name;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      _problem->addKernel(kernel_type, "ins_mass", params);
    }

    if (getParam<bool>("pspg"))
    {
      const std::string kernel_type = "INSADMassPSPG";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _pressure_variable_name;
      params.set<MaterialPropertyName>("rho_name") = getParam<MaterialPropertyName>("density_name");
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      _problem->addKernel(kernel_type, "ins_mass_pspg", params);
    }
  }
  else
  {
    const std::string kernel_type = "INSMass";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _pressure_variable_name;
    setKernelCommonParams(params);
    params.set<bool>("pspg") = getParam<bool>("pspg");
    _problem->addKernel(kernel_type, "ins_mass", params);
  }
}

void
INSAction::addINSVelocityAux()
{
  const static std::string momentums[3] = {NS::velocity_x, NS::velocity_y, NS::velocity_z};
  const static std::string coord[3] = {"x", "y", "z"};
  InputParameters params = _factory.getValidParams("VectorVariableComponentAux");
  params.set<CoupledName>("vector_variable") = {NS::velocity};
  for (unsigned int component = 0; component < _dim; ++component)
  {
    params.set<AuxVariableName>("variable") = momentums[component];
    params.set<MooseEnum>("component") = coord[component];
    _problem->addAuxKernel("VectorVariableComponentAux", momentums[component] + "_aux", params);
  }
}

void
INSAction::addINSMomentum()
{
  if (_use_ad)
  {
    {
      const std::string kernel_type = "INSADMomentumAdvection";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::velocity;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      _problem->addKernel(kernel_type, "ins_momentum_convection", params);
    }

    {
      const std::string kernel_type = "INSADMomentumViscous";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::velocity;
      params.set<MooseEnum>("viscous_form") = (getParam<bool>("laplace") ? "laplace" : "traction");
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      _problem->addKernel(kernel_type, "ins_momentum_viscous", params);
    }

    {
      const std::string kernel_type = "INSADMomentumPressure";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::velocity;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      params.set<bool>("integrate_p_by_parts") = getParam<bool>("integrate_p_by_parts");
      params.set<CoupledName>(NS::pressure) = {_pressure_variable_name};
      _problem->addKernel(kernel_type, "ins_momentum_pressure", params);
    }

    auto gravity = getParam<RealVectorValue>("gravity");
    if (gravity.norm() != 0)
    {
      const std::string kernel_type = "INSADGravityForce";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::velocity;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      params.set<RealVectorValue>("gravity") = gravity;
      _problem->addKernel(kernel_type, "ins_momentum_gravity", params);
    }

    if (getParam<bool>("supg"))
    {
      const std::string kernel_type = "INSADMomentumSUPG";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::velocity;
      params.set<std::vector<VariableName>>("velocity") = {NS::velocity};
      params.set<MaterialPropertyName>("tau_name") = "tau";
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      _problem->addKernel(kernel_type, "ins_momentum_supg", params);
    }

    if (getParam<bool>("boussinesq_approximation"))
    {
      const std::string kernel_type = "INSADBoussinesqBodyForce";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::velocity;
      params.set<std::vector<VariableName>>("temperature") = {_temperature_variable_name};
      params.set<RealVectorValue>("gravity") = gravity;
      params.set<MaterialPropertyName>("alpha_name") =
          getParam<MaterialPropertyName>("thermal_expansion_name");
      params.set<MaterialPropertyName>("ref_temp") =
          getParam<MaterialPropertyName>("reference_temperature_name");
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      _problem->addKernel(kernel_type, "ins_momentum_boussinesq_force", params);
    }

    if (getParam<bool>("has_coupled_force"))
    {
      const std::string kernel_type = "INSADMomentumCoupledForce";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::velocity;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      if (isParamValid("coupled_force_var"))
        params.set<CoupledName>("coupled_vector_var") = getParam<CoupledName>("coupled_force_var");
      if (isParamValid("coupled_force_vector_function"))
        params.set<std::vector<FunctionName>>("vector_function") =
            getParam<std::vector<FunctionName>>("coupled_force_vector_function");

      _problem->addKernel(kernel_type, "ins_momentum_coupled_force", params);
    }
  }
  else
  {
    const static std::string momentums[3] = {NS::velocity_x, NS::velocity_y, NS::velocity_z};
    std::string kernel_type;
    if (getParam<bool>("laplace"))
      kernel_type = "INSMomentumLaplaceForm";
    else
      kernel_type = "INSMomentumTractionForm";

    InputParameters params = _factory.getValidParams(kernel_type);
    setKernelCommonParams(params);

    // Extra stuff needed by momentum Kernels
    params.set<bool>("integrate_p_by_parts") = getParam<bool>("integrate_p_by_parts");
    params.set<bool>("supg") = getParam<bool>("supg");

    for (unsigned int component = 0; component < _dim; ++component)
    {
      params.set<NonlinearVariableName>("variable") = momentums[component];
      params.set<unsigned int>("component") = component;
      _problem->addKernel(kernel_type, momentums[component] + std::string("_if"), params);
    }
  }
}

void
INSAction::addINSTemperature()
{
  if (_use_ad)
  {
    {
      const std::string kernel_type = "INSADEnergyAdvection";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_variable_name;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      _problem->addKernel(kernel_type, "ins_temperature_convection", params);
    }
    {
      const std::string kernel_type = "ADHeatConduction";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_variable_name;
      params.set<MaterialPropertyName>("thermal_conductivity") =
          getParam<MaterialPropertyName>("thermal_conductivity_name");
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      _problem->addKernel(kernel_type, "ins_temperature_conduction", params);
    }

    if (getParam<bool>("supg"))
    {
      const std::string kernel_type = "INSADEnergySUPG";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_variable_name;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      params.set<CoupledName>("velocity") = {NS::velocity};
      params.set<MaterialPropertyName>("tau_name") = "tau_energy";
      _problem->addKernel(kernel_type, "ins_temperature_supg", params);
    }

    if (getParam<bool>("has_ambient_convection"))
    {
      const std::string kernel_type = "INSADEnergyAmbientConvection";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_variable_name;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      params.set<Real>("alpha") = getParam<Real>("ambient_convection_alpha");
      params.set<Real>("T_ambient") = getParam<Real>("ambient_temperature");
      _problem->addKernel(kernel_type, "ins_temperature_ambient_convection", params);
    }

    if (getParam<bool>("has_heat_source"))
    {
      const std::string kernel_type = "INSADEnergySource";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_variable_name;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      if (isParamValid("heat_source_var"))
        params.set<CoupledName>("source_variable") = getParam<CoupledName>("heat_source_var");
      else if (isParamValid("heat_source_function"))
        params.set<FunctionName>("source_function") =
            getParam<FunctionName>("heat_source_function");
      else
        mooseError("Either the 'heat_source_var' or 'heat_source_function' param must be "
                   "set if adding the 'INSADEnergySource' through the incompressible Navier-Stokes "
                   "action.");
      _problem->addKernel(kernel_type, "ins_temperature_source", params);
    }
  }
  else
  {
    const std::string kernel_type = "INSTemperature";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_variable_name;
    params.set<CoupledName>("u") = {NS::velocity_x};
    if (_dim >= 2)
      params.set<CoupledName>("v") = {NS::velocity_y};
    if (_dim >= 3)
      params.set<CoupledName>("w") = {NS::velocity_z};
    params.set<MaterialPropertyName>("k_name") =
        getParam<MaterialPropertyName>("thermal_conductivity_name");
    params.set<MaterialPropertyName>("rho_name") = getParam<MaterialPropertyName>("density_name");
    params.set<MaterialPropertyName>("cp_name") =
        getParam<MaterialPropertyName>("specific_heat_name");
    if (_blocks.size() > 0)
      params.set<std::vector<SubdomainName>>("block") = _blocks;
    _problem->addKernel(kernel_type, "ins_temperature", params);
  }
}

void
INSAction::addINSVelocityBC()
{
  const static std::string momentums[3] = {NS::velocity_x, NS::velocity_y, NS::velocity_z};
  for (unsigned int i = 0; i < _velocity_boundary.size(); ++i)
  {
    if (_use_ad)
    {
      InputParameters params = _factory.getValidParams("ADVectorFunctionDirichletBC");

      {
        const FunctionName funcx = _velocity_function[i * _dim];
        if (funcx == "NA")
          params.set<bool>("set_x_comp") = false;
        else
        {
          std::stringstream ss(funcx);
          Real val;
          if ((ss >> val).fail() || !ss.eof())
          {
            if (!_problem->hasFunction(funcx))
            {
              InputParameters func_params = _factory.getValidParams("ConstantFunction");
              func_params.set<Real>("value") = val;
              _problem->addFunction("ConstantFunction", funcx, func_params);
            }
          }
          params.set<FunctionName>("function_x") = funcx;
        }
      }

      if (_dim >= 2)
      {
        const FunctionName funcy = _velocity_function[i * _dim + 1];
        if (funcy == "NA")
          params.set<bool>("set_y_comp") = false;
        else
        {
          std::stringstream ss(funcy);
          Real val;
          if ((ss >> val).fail() || !ss.eof())
          {
            if (!_problem->hasFunction(funcy))
            {
              InputParameters func_params = _factory.getValidParams("ConstantFunction");
              func_params.set<Real>("value") = val;
              _problem->addFunction("ConstantFunction", funcy, func_params);
            }
          }
          params.set<FunctionName>("function_y") = funcy;
        }
      }

      if (_dim >= 3)
      {
        const FunctionName funcz = _velocity_function[i * _dim + 1];
        if (funcz == "NA")
          params.set<bool>("set_z_comp") = false;
        else
        {
          std::stringstream ss(funcz);
          Real val;
          if ((ss >> val).fail() || !ss.eof())
          {
            if (!_problem->hasFunction(funcz))
            {
              InputParameters func_params = _factory.getValidParams("ConstantFunction");
              func_params.set<Real>("value") = val;
              _problem->addFunction("ConstantFunction", funcz, func_params);
            }
          }
          params.set<FunctionName>("function_z") = funcz;
        }
      }

      params.set<NonlinearVariableName>("variable") = NS::velocity;
      params.set<std::vector<BoundaryName>>("boundary") = {_velocity_boundary[i]};
      _problem->addBoundaryCondition(
          "ADVectorFunctionDirichletBC", "ins_velocity_bc_" + _velocity_boundary[i], params);
    }
    else
    {
      for (unsigned int component = 0; component < _dim; ++component)
      {
        const FunctionName func = _velocity_function[i * _dim + component];
        if (func == "NA")
          continue;

        std::stringstream ss(func);
        Real val;
        if ((ss >> val).fail() || !ss.eof())
        {
          InputParameters params = _factory.getValidParams("FunctionDirichletBC");
          params.set<FunctionName>("function") = func;
          params.set<NonlinearVariableName>("variable") = momentums[component];
          params.set<std::vector<BoundaryName>>("boundary") = {_velocity_boundary[i]};
          _problem->addBoundaryCondition(
              "FunctionDirichletBC", momentums[component] + "_" + _velocity_boundary[i], params);
        }
        else
        {
          InputParameters params = _factory.getValidParams("DirichletBC");
          params.set<Real>("value") = val;
          params.set<NonlinearVariableName>("variable") = momentums[component];
          params.set<std::vector<BoundaryName>>("boundary") = {_velocity_boundary[i]};
          _problem->addBoundaryCondition(
              "DirichletBC", momentums[component] + "_" + _velocity_boundary[i], params);
        }
      }
    }
  }
}

void
INSAction::addINSTemperatureBC()
{
  for (unsigned int i = 0; i < _fixed_temperature_boundary.size(); ++i)
  {
    const FunctionName func = _temperature_function[i];
    if (func == "NA")
      continue;

    std::stringstream ss(func);
    Real val;
    if ((ss >> val).fail() || !ss.eof())
    {
      InputParameters params = _factory.getValidParams("FunctionDirichletBC");
      params.set<FunctionName>("function") = func;
      params.set<NonlinearVariableName>("variable") = _temperature_variable_name;
      params.set<std::vector<BoundaryName>>("boundary") = {_fixed_temperature_boundary[i]};
      _problem->addBoundaryCondition("FunctionDirichletBC",
                                     _temperature_variable_name + "_" +
                                         _fixed_temperature_boundary[i],
                                     params);
    }
    else
    {
      InputParameters params = _factory.getValidParams("DirichletBC");
      params.set<Real>("value") = val;
      params.set<NonlinearVariableName>("variable") = _temperature_variable_name;
      params.set<std::vector<BoundaryName>>("boundary") = {_fixed_temperature_boundary[i]};
      _problem->addBoundaryCondition(
          "DirichletBC", _temperature_variable_name + "_" + _fixed_temperature_boundary[i], params);
    }
  }
}

void
INSAction::addINSPressureBC()
{
  for (unsigned int i = 0; i < _pressure_boundary.size(); ++i)
  {
    const FunctionName func = _pressure_function[i];
    std::stringstream ss(func);
    Real val;
    if ((ss >> val).fail() || !ss.eof())
    {
      InputParameters params = _factory.getValidParams("FunctionDirichletBC");
      params.set<FunctionName>("function") = func;
      params.set<NonlinearVariableName>("variable") = _pressure_variable_name;
      params.set<std::vector<BoundaryName>>("boundary") = {_pressure_boundary[i]};
      _problem->addBoundaryCondition(
          "FunctionDirichletBC", NS::pressure + _pressure_boundary[i], params);
    }
    else
    {
      InputParameters params = _factory.getValidParams("DirichletBC");
      params.set<Real>("value") = val;
      params.set<NonlinearVariableName>("variable") = _pressure_variable_name;
      params.set<std::vector<BoundaryName>>("boundary") = {_pressure_boundary[i]};
      _problem->addBoundaryCondition("DirichletBC", NS::pressure + _pressure_boundary[i], params);
    }
  }
}

void
INSAction::addINSPinnedPressureBC()
{
  InputParameters params = _factory.getValidParams("DirichletBC");
  params.set<Real>("value") = 0;
  params.set<NonlinearVariableName>("variable") = _pressure_variable_name;
  params.set<std::vector<BoundaryName>>("boundary") = {_pinned_node};
  _problem->addBoundaryCondition("DirichletBC", "pressure_pin", params);
}

void
INSAction::addINSNoBCBC()
{
  if (_use_ad)
  {
    const std::string kernel_type = "INSADMomentumNoBCBC";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = NS::velocity;
    if (_blocks.size() > 0)
      params.set<std::vector<SubdomainName>>("block") = _blocks;
    params.set<bool>("integrate_p_by_parts") = getParam<bool>("integrate_p_by_parts");
    params.set<CoupledName>(NS::pressure) = {_pressure_variable_name};
    params.set<MooseEnum>("viscous_form") = (getParam<bool>("laplace") ? "laplace" : "traction");
    _problem->addBoundaryCondition(kernel_type, "ins_momentum_nobc_bc", params);
  }
  else
  {
    const static std::string momentums[3] = {NS::velocity_x, NS::velocity_y, NS::velocity_z};
    std::string kernel_type;
    if (getParam<bool>("laplace"))
      kernel_type = "INSMomentumNoBCBCLaplaceForm";
    else
      kernel_type = "INSMomentumNoBCBCTractionForm";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<std::vector<BoundaryName>>("boundary") = _no_bc_boundary;
    setNoBCCommonParams(params);
    for (unsigned int component = 0; component < _dim; ++component)
    {
      params.set<NonlinearVariableName>("variable") = momentums[component];
      _problem->addBoundaryCondition(kernel_type, momentums[component] + "_nobc_bc", params);
    }
  }
}

void
INSAction::setKernelCommonParams(InputParameters & params)
{
  if (_blocks.size() > 0)
    params.set<std::vector<SubdomainName>>("block") = _blocks;

  // coupled variables
  params.set<CoupledName>("u") = {NS::velocity_x};
  if (_dim >= 2)
    params.set<CoupledName>("v") = {NS::velocity_y};
  if (_dim >= 3)
    params.set<CoupledName>("w") = {NS::velocity_z};
  params.set<CoupledName>(NS::pressure) = {_pressure_variable_name};
  params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  params.set<MaterialPropertyName>("mu_name") =
      getParam<MaterialPropertyName>("dynamic_viscosity_name");
  params.set<MaterialPropertyName>("rho_name") = getParam<MaterialPropertyName>("density_name");
  params.set<Real>("alpha") = getParam<Real>("alpha");
  params.set<bool>("laplace") = getParam<bool>("laplace");
  // this parameter only affecting Jacobian evaluation in non-AD
  params.set<bool>("convective_term") = getParam<bool>("convective_term");
  // FIXME: this parameter seems not changing solution much?
  params.set<bool>("transient_term") = (_type == "transient");
}

void
INSAction::setNoBCCommonParams(InputParameters & params)
{
  // coupled variables
  params.set<CoupledName>("u") = {NS::velocity_x};
  if (_dim >= 2)
    params.set<CoupledName>("v") = {NS::velocity_y};
  if (_dim >= 3)
    params.set<CoupledName>("w") = {NS::velocity_z};
  params.set<CoupledName>(NS::pressure) = {_pressure_variable_name};
  params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  params.set<MaterialPropertyName>("mu_name") =
      getParam<MaterialPropertyName>("dynamic_viscosity_name");
  params.set<MaterialPropertyName>("rho_name") = getParam<MaterialPropertyName>("density_name");
  params.set<bool>("integrate_p_by_parts") = getParam<bool>("integrate_p_by_parts");
}
