//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "DomainIntegralAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "CrackFrontDefinition.h"
#include "MooseMesh.h"
#include "Conversion.h"

#include "libmesh/string_to_enum.h"

registerMooseAction("TensorMechanicsApp", DomainIntegralAction, "add_user_object");

registerMooseAction("TensorMechanicsApp", DomainIntegralAction, "add_aux_variable");

registerMooseAction("TensorMechanicsApp", DomainIntegralAction, "add_aux_kernel");

registerMooseAction("TensorMechanicsApp", DomainIntegralAction, "add_postprocessor");

registerMooseAction("TensorMechanicsApp", DomainIntegralAction, "add_material");

InputParameters
DomainIntegralAction::validParams()
{
  InputParameters params = Action::validParams();
  addCrackFrontDefinitionParams(params);
  MultiMooseEnum integral_vec(
      "JIntegral CIntegral KFromJIntegral InteractionIntegralKI InteractionIntegralKII "
      "InteractionIntegralKIII InteractionIntegralT");
  params.addClassDescription(
      "Creates the MOOSE objects needed to compute fraction domain integrals");
  params.addRequiredParam<MultiMooseEnum>("integrals",
                                          integral_vec,
                                          "Domain integrals to calculate.  Choices are: " +
                                              integral_vec.getRawNames());
  params.addParam<std::vector<BoundaryName>>("boundary",
                                             "Boundary containing the crack front points");
  params.addParam<std::vector<Point>>("crack_front_points", "Set of points to define crack front");
  params.addParam<std::string>(
      "order", "FIRST", "Specifies the order of the FE shape function to use for q AuxVariables");
  params.addParam<std::string>(
      "family", "LAGRANGE", "Specifies the family of FE shape functions to use for q AuxVariables");
  params.addParam<std::vector<Real>>("radius_inner", "Inner radius for volume integral domain");
  params.addParam<std::vector<Real>>("radius_outer", "Outer radius for volume integral domain");
  params.addParam<unsigned int>("ring_first",
                                "The first ring of elements for volume integral domain");
  params.addParam<unsigned int>("ring_last",
                                "The last ring of elements for volume integral domain");
  params.addParam<std::vector<VariableName>>(
      "output_variable", "Variable values to be reported along the crack front");
  params.addParam<Real>("poissons_ratio", "Poisson's ratio");
  params.addParam<Real>("youngs_modulus", "Young's modulus");
  params.addParam<std::vector<SubdomainName>>("block", "The block ids where integrals are defined");

  params.addParam<std::vector<VariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<VariableName>("temperature", "", "The temperature");
  params.addParam<MaterialPropertyName>(
      "functionally_graded_youngs_modulus_crack_dir_gradient",
      "Gradient of the spatially varying Young's modulus provided in "
      "'functionally_graded_youngs_modulus' in the direction of crack extension.");
  params.addParam<MaterialPropertyName>(
      "functionally_graded_youngs_modulus",
      "Spatially varying elasticity modulus variable. This input is required when "
      "using the functionally graded material capability.");
  MooseEnum position_type("Angle Distance", "Distance");
  params.addParam<MooseEnum>(
      "position_type",
      position_type,
      "The method used to calculate position along crack front.  Options are: " +
          position_type.getRawNames());
  MooseEnum q_function_type("Geometry Topology", "Geometry");
  params.addParam<MooseEnum>("q_function_type",
                             q_function_type,
                             "The method used to define the integration domain. Options are: " +
                                 q_function_type.getRawNames());
  params.addParam<bool>(
      "equivalent_k",
      false,
      "Calculate an equivalent K from KI, KII and KIII, assuming self-similar crack growth.");
  params.addParam<bool>("output_q", true, "Output q");
  params.addRequiredParam<bool>(
      "incremental", "Flag to indicate whether an incremental or total model is being used.");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", "List of eigenstrains applied in the strain calculation");
  params.addDeprecatedParam<bool>("convert_J_to_K",
                                  false,
                                  "Convert J-integral to stress intensity factor K.",
                                  "This input parameter is deprecated and will be removed soon. "
                                  "Use 'integrals = KFromJIntegral' to request output of the "
                                  "conversion from the J-integral to stress intensity factors");
  params.addParam<std::vector<MaterialName>>(
      "inelastic_models",
      "The material objects to use to calculate the strain energy rate density.");
  params.addParam<MaterialPropertyName>("eigenstrain_gradient",
                                        "Material defining gradient of eigenstrain tensor");
  params.addParam<MaterialPropertyName>("body_force", "Material defining body force");
  params.addParam<bool>("use_automatic_differentiation",
                        false,
                        "Flag to use automatic differentiation (AD) objects when possible");
  params.addParam<bool>(
      "used_by_xfem_to_grow_crack",
      false,
      "Flag to trigger domainIntregal vector postprocessors to be executed on nonlinear.  This "
      "updates the values in the vector postprocessor which will allow the crack to grow in XFEM "
      "cutter objects that use the domainIntegral vector postprocssor values as a growth "
      "criterion.");
  return params;
}

DomainIntegralAction::DomainIntegralAction(const InputParameters & params)
  : Action(params),
    _boundary_names(getParam<std::vector<BoundaryName>>("boundary")),
    _closed_loop(getParam<bool>("closed_loop")),
    _use_crack_front_points_provider(false),
    _order(getParam<std::string>("order")),
    _family(getParam<std::string>("family")),
    _direction_method_moose_enum(getParam<MooseEnum>("crack_direction_method")),
    _end_direction_method_moose_enum(getParam<MooseEnum>("crack_end_direction_method")),
    _have_crack_direction_vector(isParamValid("crack_direction_vector")),
    _crack_direction_vector(
        _have_crack_direction_vector ? getParam<RealVectorValue>("crack_direction_vector") : 0.0),
    _have_crack_direction_vector_end_1(isParamValid("crack_direction_vector_end_1")),
    _crack_direction_vector_end_1(_have_crack_direction_vector_end_1
                                      ? getParam<RealVectorValue>("crack_direction_vector_end_1")
                                      : 0.0),
    _have_crack_direction_vector_end_2(isParamValid("crack_direction_vector_end_2")),
    _crack_direction_vector_end_2(_have_crack_direction_vector_end_2
                                      ? getParam<RealVectorValue>("crack_direction_vector_end_2")
                                      : 0.0),
    _treat_as_2d(getParam<bool>("2d")),
    _axis_2d(getParam<unsigned int>("axis_2d")),
    _has_symmetry_plane(isParamValid("symmetry_plane")),
    _symmetry_plane(_has_symmetry_plane ? getParam<unsigned int>("symmetry_plane")
                                        : std::numeric_limits<unsigned int>::max()),
    _position_type(getParam<MooseEnum>("position_type")),
    _q_function_type(getParam<MooseEnum>("q_function_type")),
    _get_equivalent_k(getParam<bool>("equivalent_k")),
    _use_displaced_mesh(false),
    _output_q(getParam<bool>("output_q")),
    _incremental(getParam<bool>("incremental")),
    _convert_J_to_K(isParamValid("convert_J_to_K") ? getParam<bool>("convert_J_to_K") : false),
    _fgm_crack(false),
    _use_ad(getParam<bool>("use_automatic_differentiation")),
    _used_by_xfem_to_grow_crack(getParam<bool>("used_by_xfem_to_grow_crack"))
{

  if (isParamValid("functionally_graded_youngs_modulus_crack_dir_gradient") !=
      isParamValid("functionally_graded_youngs_modulus"))
    paramError("functionally_graded_youngs_modulus_crack_dir_gradient",
               "You have selected to compute the interaction integral for a crack in FGM. That "
               "selection requires the user to provide a spatially varying elasticity modulus that "
               "defines the transition of material properties (i.e. "
               "'functionally_graded_youngs_modulus') and its "
               "spatial derivative in the crack direction (i.e. "
               "'functionally_graded_youngs_modulus_crack_dir_gradient').");

  if (isParamValid("functionally_graded_youngs_modulus_crack_dir_gradient") &&
      isParamValid("functionally_graded_youngs_modulus"))
  {
    _fgm_crack = true;
    _functionally_graded_youngs_modulus_crack_dir_gradient =
        getParam<MaterialPropertyName>("functionally_graded_youngs_modulus_crack_dir_gradient");
    _functionally_graded_youngs_modulus =
        getParam<MaterialPropertyName>("functionally_graded_youngs_modulus");
  }

  if (_q_function_type == GEOMETRY)
  {
    if (isParamValid("radius_inner") && isParamValid("radius_outer"))
    {
      _radius_inner = getParam<std::vector<Real>>("radius_inner");
      _radius_outer = getParam<std::vector<Real>>("radius_outer");
    }
    else
      mooseError("DomainIntegral error: must set radius_inner and radius_outer.");
    for (unsigned int i = 0; i < _radius_inner.size(); ++i)
      _ring_vec.push_back(i + 1);
  }
  else if (_q_function_type == TOPOLOGY)
  {
    if (isParamValid("ring_first") && isParamValid("ring_last"))
    {
      _ring_first = getParam<unsigned int>("ring_first");
      _ring_last = getParam<unsigned int>("ring_last");
    }
    else
      mooseError(
          "DomainIntegral error: must set ring_first and ring_last if q_function_type = Topology.");
    for (unsigned int i = _ring_first; i <= _ring_last; ++i)
      _ring_vec.push_back(i);
  }
  else
    paramError("q_function_type", "DomainIntegral error: invalid q_function_type.");

  if (isParamValid("crack_front_points"))
    _crack_front_points = getParam<std::vector<Point>>("crack_front_points");

  if (isParamValid("crack_front_points_provider"))
  {
    if (!isParamValid("number_points_from_provider"))
      paramError("number_points_from_provider",
                 "DomainIntegral error: when crack_front_points_provider is used, "
                 "number_points_from_provider must be provided.");
    _use_crack_front_points_provider = true;
    _crack_front_points_provider = getParam<UserObjectName>("crack_front_points_provider");
  }
  else if (isParamValid("number_points_from_provider"))
    paramError("crack_front_points_provider",
               "DomainIntegral error: number_points_from_provider is provided but "
               "crack_front_points_provider cannot be found.");
  if (isParamValid("crack_mouth_boundary"))
    _crack_mouth_boundary_names = getParam<std::vector<BoundaryName>>("crack_mouth_boundary");
  if (isParamValid("intersecting_boundary"))
    _intersecting_boundary_names = getParam<std::vector<BoundaryName>>("intersecting_boundary");
  if (_radius_inner.size() != _radius_outer.size())
    mooseError("Number of entries in 'radius_inner' and 'radius_outer' must match.");

  bool youngs_modulus_set(false);
  bool poissons_ratio_set(false);
  MultiMooseEnum integral_moose_enums = getParam<MultiMooseEnum>("integrals");
  for (unsigned int i = 0; i < integral_moose_enums.size(); ++i)
  {
    _displacements = getParam<std::vector<VariableName>>("displacements");

    if (_displacements.size() < 2)
      paramError(
          "displacements",
          "DomainIntegral error: The size of the displacements vector should at least be 2.");

    if (integral_moose_enums[i] != "JIntegral" && integral_moose_enums[i] != "CIntegral" &&
        integral_moose_enums[i] != "KFromJIntegral")
    {
      // Check that parameters required for interaction integrals are defined
      if (!(isParamValid("poissons_ratio")) || !(isParamValid("youngs_modulus")))
        mooseError(
            "DomainIntegral error: must set Poisson's ratio and Young's modulus for integral: ",
            integral_moose_enums[i]);

      if (!(isParamValid("block")))
        paramError("block",
                   "DomainIntegral error: must set block ID or name for integral: ",
                   integral_moose_enums[i]);

      _poissons_ratio = getParam<Real>("poissons_ratio");
      poissons_ratio_set = true;
      _youngs_modulus = getParam<Real>("youngs_modulus");
      youngs_modulus_set = true;
      _blocks = getParam<std::vector<SubdomainName>>("block");
    }

    _integrals.insert(INTEGRAL(int(integral_moose_enums.get(i))));
  }

  if ((_integrals.count(J_INTEGRAL) != 0 && _integrals.count(C_INTEGRAL) != 0) ||
      (_integrals.count(J_INTEGRAL) != 0 && _integrals.count(K_FROM_J_INTEGRAL) != 0) ||
      (_integrals.count(C_INTEGRAL) != 0 && _integrals.count(K_FROM_J_INTEGRAL) != 0))
    paramError("integrals",
               "JIntegral, CIntegral, and KFromJIntegral options are mutually exclusive");

  // Acommodate deprecated parameter convert_J_to_K
  if (_convert_J_to_K && _integrals.count(K_FROM_J_INTEGRAL) != 0)
  {
    _integrals.insert(K_FROM_J_INTEGRAL);
    _integrals.erase(J_INTEGRAL);
  }

  if (isParamValid("temperature"))
    _temp = getParam<VariableName>("temperature");

  if (_temp != "" && !isParamValid("eigenstrain_names"))
    paramError(
        "eigenstrain_names",
        "DomainIntegral error: must provide `eigenstrain_names` when temperature is coupled.");

  if (_get_equivalent_k && (_integrals.count(INTERACTION_INTEGRAL_KI) == 0 ||
                            _integrals.count(INTERACTION_INTEGRAL_KII) == 0 ||
                            _integrals.count(INTERACTION_INTEGRAL_KIII) == 0))
    paramError("integrals",
               "DomainIntegral error: must calculate KI, KII and KIII to get equivalent K.");

  if (isParamValid("output_variable"))
  {
    _output_variables = getParam<std::vector<VariableName>>("output_variable");
    if (_crack_front_points.size() > 0)
      paramError("output_variables",
                 "'output_variables' not yet supported with 'crack_front_points'");
  }

  if (_integrals.count(K_FROM_J_INTEGRAL) != 0)
  {
    if (!isParamValid("youngs_modulus") || !isParamValid("poissons_ratio"))
      mooseError("DomainIntegral error: must set Young's modulus and Poisson's ratio "
                 "if K_FROM_J_INTEGRAL is selected.");
    if (!youngs_modulus_set)
      _youngs_modulus = getParam<Real>("youngs_modulus");
    if (!poissons_ratio_set)
      _poissons_ratio = getParam<Real>("poissons_ratio");
  }

  if (_integrals.count(J_INTEGRAL) != 0 || _integrals.count(C_INTEGRAL) != 0 ||
      _integrals.count(K_FROM_J_INTEGRAL) != 0)
  {
    if (isParamValid("eigenstrain_gradient"))
      paramError("eigenstrain_gradient",
                 "'eigenstrain_gradient' cannot be specified when the computed integrals include "
                 "JIntegral, CIntegral, or KFromJIntegral");
    if (isParamValid("body_force"))
      paramError("body_force",
                 "'body_force' cannot be specified when the computed integrals include JIntegral, "
                 "CIntegral, or KFromJIntegral");
  }
  if (isParamValid("eigenstrain_gradient") && (_temp != "" || isParamValid("eigenstrain_names")))
    paramError("eigenstrain_gradient",
               "'eigenstrain_gradient' cannot be specified together with 'temperature' or "
               "'eigenstrain_names'. These are for separate, mutually exclusive systems for "
               "including the effect of eigenstrains");
}

DomainIntegralAction::~DomainIntegralAction() {}

void
DomainIntegralAction::act()
{
  const std::string uo_name("crackFrontDefinition");
  const std::string ak_base_name("q");
  const std::string av_base_name("q");
  const unsigned int num_crack_front_points = calcNumCrackFrontPoints();
  const std::string aux_stress_base_name("aux_stress");
  const std::string aux_grad_disp_base_name("aux_grad_disp");

  std::string ad_prepend = "";
  if (_use_ad)
    ad_prepend = "AD";

  if (_current_task == "add_user_object")
  {
    const std::string uo_type_name("CrackFrontDefinition");

    InputParameters params = _factory.getValidParams(uo_type_name);
    if (_use_crack_front_points_provider && _used_by_xfem_to_grow_crack)
      params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END, EXEC_NONLINEAR};
    else
      params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};

    params.set<MooseEnum>("crack_direction_method") = _direction_method_moose_enum;
    params.set<MooseEnum>("crack_end_direction_method") = _end_direction_method_moose_enum;
    if (_have_crack_direction_vector)
      params.set<RealVectorValue>("crack_direction_vector") = _crack_direction_vector;
    if (_have_crack_direction_vector_end_1)
      params.set<RealVectorValue>("crack_direction_vector_end_1") = _crack_direction_vector_end_1;
    if (_have_crack_direction_vector_end_2)
      params.set<RealVectorValue>("crack_direction_vector_end_2") = _crack_direction_vector_end_2;
    if (_crack_mouth_boundary_names.size() != 0)
      params.set<std::vector<BoundaryName>>("crack_mouth_boundary") = _crack_mouth_boundary_names;
    if (_intersecting_boundary_names.size() != 0)
      params.set<std::vector<BoundaryName>>("intersecting_boundary") = _intersecting_boundary_names;
    params.set<bool>("2d") = _treat_as_2d;
    params.set<unsigned int>("axis_2d") = _axis_2d;
    if (_has_symmetry_plane)
      params.set<unsigned int>("symmetry_plane") = _symmetry_plane;
    if (_boundary_names.size() != 0)
      params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    if (_crack_front_points.size() != 0)
      params.set<std::vector<Point>>("crack_front_points") = _crack_front_points;
    if (_use_crack_front_points_provider)
      params.applyParameters(parameters(),
                             {"crack_front_points_provider, number_points_from_provider"});
    if (_closed_loop)
      params.set<bool>("closed_loop") = _closed_loop;
    params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;
    if (_integrals.count(INTERACTION_INTEGRAL_T) != 0)
    {
      params.set<VariableName>("disp_x") = _displacements[0];
      params.set<VariableName>("disp_y") = _displacements[1];
      if (_displacements.size() == 3)
        params.set<VariableName>("disp_z") = _displacements[2];
      params.set<bool>("t_stress") = true;
    }

    unsigned int nrings = 0;
    if (_q_function_type == TOPOLOGY)
    {
      params.set<bool>("q_function_rings") = true;
      params.set<unsigned int>("last_ring") = _ring_last;
      params.set<unsigned int>("first_ring") = _ring_first;
      nrings = _ring_last - _ring_first + 1;
    }
    else if (_q_function_type == GEOMETRY)
    {
      params.set<std::vector<Real>>("j_integral_radius_inner") = _radius_inner;
      params.set<std::vector<Real>>("j_integral_radius_outer") = _radius_outer;
      nrings = _ring_vec.size();
    }

    params.set<unsigned int>("nrings") = nrings;
    params.set<MooseEnum>("q_function_type") = _q_function_type;

    _problem->addUserObject(uo_type_name, uo_name, params);
  }
  else if (_current_task == "add_aux_variable" && _output_q)
  {
    for (unsigned int ring_index = 0; ring_index < _ring_vec.size(); ++ring_index)
    {
      std::string aux_var_type;
      if (_family == "LAGRANGE")
        aux_var_type = "MooseVariable";
      else if (_family == "MONOMIAL")
        aux_var_type = "MooseVariableConstMonomial";
      else if (_family == "SCALAR")
        aux_var_type = "MooseVariableScalar";
      else
        mooseError("Unsupported finite element family in, " + name() +
                   ".  Please use LAGRANGE, MONOMIAL, or SCALAR");

      auto params = _factory.getValidParams(aux_var_type);
      params.set<MooseEnum>("order") = _order;
      params.set<MooseEnum>("family") = _family;

      if (_treat_as_2d && _use_crack_front_points_provider == false)
      {
        std::ostringstream av_name_stream;
        av_name_stream << av_base_name << "_" << _ring_vec[ring_index];
        _problem->addAuxVariable(aux_var_type, av_name_stream.str(), params);
      }
      else
      {
        for (unsigned int cfp_index = 0; cfp_index < num_crack_front_points; ++cfp_index)
        {
          std::ostringstream av_name_stream;
          av_name_stream << av_base_name << "_" << cfp_index + 1 << "_" << _ring_vec[ring_index];
          _problem->addAuxVariable(aux_var_type, av_name_stream.str(), params);
        }
      }
    }
  }

  else if (_current_task == "add_aux_kernel" && _output_q)
  {
    std::string ak_type_name;
    unsigned int nrings = 0;
    if (_q_function_type == GEOMETRY)
    {
      ak_type_name = "DomainIntegralQFunction";
      nrings = _ring_vec.size();
    }
    else if (_q_function_type == TOPOLOGY)
    {
      ak_type_name = "DomainIntegralTopologicalQFunction";
      nrings = _ring_last - _ring_first + 1;
    }

    InputParameters params = _factory.getValidParams(ak_type_name);
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    params.set<UserObjectName>("crack_front_definition") = uo_name;
    params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

    for (unsigned int ring_index = 0; ring_index < nrings; ++ring_index)
    {
      if (_q_function_type == GEOMETRY)
      {
        params.set<Real>("j_integral_radius_inner") = _radius_inner[ring_index];
        params.set<Real>("j_integral_radius_outer") = _radius_outer[ring_index];
      }
      else if (_q_function_type == TOPOLOGY)
      {
        params.set<unsigned int>("ring_index") = _ring_first + ring_index;
      }

      if (_treat_as_2d && _use_crack_front_points_provider == false)
      {
        std::ostringstream ak_name_stream;
        ak_name_stream << ak_base_name << "_" << _ring_vec[ring_index];
        std::ostringstream av_name_stream;
        av_name_stream << av_base_name << "_" << _ring_vec[ring_index];
        params.set<AuxVariableName>("variable") = av_name_stream.str();
        _problem->addAuxKernel(ak_type_name, ak_name_stream.str(), params);
      }
      else
      {
        for (unsigned int cfp_index = 0; cfp_index < num_crack_front_points; ++cfp_index)
        {
          std::ostringstream ak_name_stream;
          ak_name_stream << ak_base_name << "_" << cfp_index + 1 << "_" << _ring_vec[ring_index];
          std::ostringstream av_name_stream;
          av_name_stream << av_base_name << "_" << cfp_index + 1 << "_" << _ring_vec[ring_index];
          params.set<AuxVariableName>("variable") = av_name_stream.str();
          params.set<unsigned int>("crack_front_point_index") = cfp_index;
          _problem->addAuxKernel(ak_type_name, ak_name_stream.str(), params);
        }
      }
    }
  }

  else if (_current_task == "add_postprocessor")
  {
    for (std::set<INTEGRAL>::iterator sit = _integrals.begin(); sit != _integrals.end(); ++sit)
    {
      std::string pp_base_name;
      switch (*sit)
      {
        case J_INTEGRAL:
          pp_base_name = "J";
          break;

        case C_INTEGRAL:
          pp_base_name = "C";
          break;

        case K_FROM_J_INTEGRAL:
          pp_base_name = "K";
          break;

        case INTERACTION_INTEGRAL_KI:
          pp_base_name = "II_KI";
          break;

        case INTERACTION_INTEGRAL_KII:
          pp_base_name = "II_KII";
          break;

        case INTERACTION_INTEGRAL_KIII:
          pp_base_name = "II_KIII";
          break;

        case INTERACTION_INTEGRAL_T:
          pp_base_name = "II_T";
          break;
      }
      const std::string pp_type_name("VectorPostprocessorComponent");
      InputParameters params = _factory.getValidParams(pp_type_name);
      for (unsigned int ring_index = 0; ring_index < _ring_vec.size(); ++ring_index)
      {
        if (_treat_as_2d && _use_crack_front_points_provider == false)
        {
          params.set<VectorPostprocessorName>("vectorpostprocessor") =
              pp_base_name + "_2DVPP_" + Moose::stringify(_ring_vec[ring_index]);
          std::string pp_name = pp_base_name + +"_" + Moose::stringify(_ring_vec[ring_index]);
          params.set<unsigned int>("index") = 0;
          params.set<std::string>("vector_name") =
              pp_base_name + "_" + Moose::stringify(_ring_vec[ring_index]);
          _problem->addPostprocessor(pp_type_name, pp_name, params);
        }
        else
        {
          for (unsigned int cfp_index = 0; cfp_index < num_crack_front_points; ++cfp_index)
          {
            params.set<VectorPostprocessorName>("vectorpostprocessor") =
                pp_base_name + "_" + Moose::stringify(_ring_vec[ring_index]);
            std::string pp_name = pp_base_name + "_" + Moose::stringify(cfp_index + 1) + "_" +
                                  Moose::stringify(_ring_vec[ring_index]);
            params.set<unsigned int>("index") = cfp_index;
            params.set<std::string>("vector_name") =
                pp_base_name + "_" + Moose::stringify(_ring_vec[ring_index]);
            _problem->addPostprocessor(pp_type_name, pp_name, params);
          }
        }
      }
    }

    if (_get_equivalent_k)
    {
      std::string pp_base_name("Keq");
      const std::string pp_type_name("VectorPostprocessorComponent");
      InputParameters params = _factory.getValidParams(pp_type_name);
      for (unsigned int ring_index = 0; ring_index < _ring_vec.size(); ++ring_index)
      {
        if (_treat_as_2d && _use_crack_front_points_provider == false)
        {
          params.set<VectorPostprocessorName>("vectorpostprocessor") =
              pp_base_name + "_2DVPP_" + Moose::stringify(_ring_vec[ring_index]);
          std::string pp_name = pp_base_name + +"_" + Moose::stringify(_ring_vec[ring_index]);
          params.set<unsigned int>("index") = 0;
          params.set<std::string>("vector_name") =
              pp_base_name + "_" + Moose::stringify(_ring_vec[ring_index]);
          _problem->addPostprocessor(pp_type_name, pp_name, params);
        }
        else
        {
          for (unsigned int cfp_index = 0; cfp_index < num_crack_front_points; ++cfp_index)
          {
            params.set<VectorPostprocessorName>("vectorpostprocessor") =
                pp_base_name + "_" + Moose::stringify(_ring_vec[ring_index]);
            std::string pp_name = pp_base_name + "_" + Moose::stringify(cfp_index + 1) + "_" +
                                  Moose::stringify(_ring_vec[ring_index]);
            params.set<unsigned int>("index") = cfp_index;
            params.set<std::string>("vector_name") =
                pp_base_name + "_" + Moose::stringify(_ring_vec[ring_index]);
            _problem->addPostprocessor(pp_type_name, pp_name, params);
          }
        }
      }
    }

    for (unsigned int i = 0; i < _output_variables.size(); ++i)
    {
      const std::string ov_base_name(_output_variables[i]);
      const std::string pp_type_name("CrackFrontData");
      InputParameters params = _factory.getValidParams(pp_type_name);
      params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
      params.set<UserObjectName>("crack_front_definition") = uo_name;
      if (_treat_as_2d && _use_crack_front_points_provider == false)
      {
        std::ostringstream pp_name_stream;
        pp_name_stream << ov_base_name << "_crack";
        params.set<VariableName>("variable") = _output_variables[i];
        _problem->addPostprocessor(pp_type_name, pp_name_stream.str(), params);
      }
      else
      {
        for (unsigned int cfp_index = 0; cfp_index < num_crack_front_points; ++cfp_index)
        {
          std::ostringstream pp_name_stream;
          pp_name_stream << ov_base_name << "_crack_" << cfp_index + 1;
          params.set<VariableName>("variable") = _output_variables[i];
          params.set<unsigned int>("crack_front_point_index") = cfp_index;
          _problem->addPostprocessor(pp_type_name, pp_name_stream.str(), params);
        }
      }
    }
  }

  else if (_current_task == "add_vector_postprocessor")
  {
    if (_integrals.count(J_INTEGRAL) != 0 || _integrals.count(C_INTEGRAL) != 0 ||
        _integrals.count(K_FROM_J_INTEGRAL) != 0)
    {
      std::string vpp_base_name;
      std::string jintegral_selection = "JIntegral";

      if (_integrals.count(J_INTEGRAL) != 0)
      {
        vpp_base_name = "J";
        jintegral_selection = "JIntegral";
      }
      else if (_integrals.count(K_FROM_J_INTEGRAL) != 0)
      {
        vpp_base_name = "K";
        jintegral_selection = "KFromJIntegral";
      }
      else if (_integrals.count(C_INTEGRAL) != 0)
      {
        vpp_base_name = "C";
        jintegral_selection = "CIntegral";
      }

      if (_treat_as_2d && _use_crack_front_points_provider == false)
        vpp_base_name += "_2DVPP";

      const std::string vpp_type_name("JIntegral");
      InputParameters params = _factory.getValidParams(vpp_type_name);
      params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
      params.set<UserObjectName>("crack_front_definition") = uo_name;
      params.set<std::vector<SubdomainName>>("block") = {_blocks};
      params.set<MooseEnum>("position_type") = _position_type;

      if (_integrals.count(K_FROM_J_INTEGRAL) != 0)
      {
        params.set<Real>("youngs_modulus") = _youngs_modulus;
        params.set<Real>("poissons_ratio") = _poissons_ratio;
      }

      if (_has_symmetry_plane)
        params.set<unsigned int>("symmetry_plane") = _symmetry_plane;

      // Select the integral type to be computed in JIntegral vector postprocessor
      params.set<MooseEnum>("integral") = jintegral_selection;

      params.set<std::vector<VariableName>>("displacements") = _displacements;
      params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;
      for (unsigned int ring_index = 0; ring_index < _ring_vec.size(); ++ring_index)
      {
        params.set<unsigned int>("ring_index") = _ring_vec[ring_index];
        params.set<MooseEnum>("q_function_type") = _q_function_type;

        std::string vpp_name = vpp_base_name + "_" + Moose::stringify(_ring_vec[ring_index]);
        _problem->addVectorPostprocessor(vpp_type_name, vpp_name, params);
      }
    }

    if (_integrals.count(INTERACTION_INTEGRAL_KI) != 0 ||
        _integrals.count(INTERACTION_INTEGRAL_KII) != 0 ||
        _integrals.count(INTERACTION_INTEGRAL_KIII) != 0 ||
        _integrals.count(INTERACTION_INTEGRAL_T) != 0)
    {
      if (_has_symmetry_plane && (_integrals.count(INTERACTION_INTEGRAL_KII) != 0 ||
                                  _integrals.count(INTERACTION_INTEGRAL_KIII) != 0))
        paramError("symmetry_plane",
                   "In DomainIntegral, symmetry_plane option cannot be used with mode-II or "
                   "mode-III interaction integral");

      std::string vpp_base_name;
      std::string vpp_type_name(ad_prepend + "InteractionIntegral");

      InputParameters params = _factory.getValidParams(vpp_type_name);
      if (_use_crack_front_points_provider && _used_by_xfem_to_grow_crack)
        params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END, EXEC_NONLINEAR};
      else
        params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END};

      params.set<UserObjectName>("crack_front_definition") = uo_name;
      params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;
      params.set<std::vector<SubdomainName>>("block") = {_blocks};

      if (_has_symmetry_plane)
        params.set<unsigned int>("symmetry_plane") = _symmetry_plane;

      if (_fgm_crack)
      {
        params.set<MaterialPropertyName>(
            "functionally_graded_youngs_modulus_crack_dir_gradient") = {
            _functionally_graded_youngs_modulus_crack_dir_gradient};
        params.set<MaterialPropertyName>("functionally_graded_youngs_modulus") = {
            _functionally_graded_youngs_modulus};
      }

      params.set<Real>("poissons_ratio") = _poissons_ratio;
      params.set<Real>("youngs_modulus") = _youngs_modulus;
      params.set<std::vector<VariableName>>("displacements") = _displacements;
      if (_temp != "")
        params.set<std::vector<VariableName>>("temperature") = {_temp};

      if (parameters().isParamValid("eigenstrain_gradient"))
        params.set<MaterialPropertyName>("eigenstrain_gradient") =
            parameters().get<MaterialPropertyName>("eigenstrain_gradient");
      if (parameters().isParamValid("body_force"))
        params.set<MaterialPropertyName>("body_force") =
            parameters().get<MaterialPropertyName>("body_force");

      for (std::set<INTEGRAL>::iterator sit = _integrals.begin(); sit != _integrals.end(); ++sit)
      {
        switch (*sit)
        {
          case J_INTEGRAL:
            continue;

          case C_INTEGRAL:
            continue;

          case K_FROM_J_INTEGRAL:
            continue;

          case INTERACTION_INTEGRAL_KI:
            vpp_base_name = "II_KI";
            params.set<Real>("K_factor") =
                0.5 * _youngs_modulus / (1.0 - std::pow(_poissons_ratio, 2.0));
            params.set<MooseEnum>("sif_mode") = "KI";
            break;

          case INTERACTION_INTEGRAL_KII:
            vpp_base_name = "II_KII";
            params.set<Real>("K_factor") =
                0.5 * _youngs_modulus / (1.0 - std::pow(_poissons_ratio, 2.0));
            params.set<MooseEnum>("sif_mode") = "KII";
            break;

          case INTERACTION_INTEGRAL_KIII:
            vpp_base_name = "II_KIII";
            params.set<Real>("K_factor") = 0.5 * _youngs_modulus / (1.0 + _poissons_ratio);
            params.set<MooseEnum>("sif_mode") = "KIII";
            break;

          case INTERACTION_INTEGRAL_T:
            vpp_base_name = "II_T";
            params.set<Real>("K_factor") = _youngs_modulus / (1 - std::pow(_poissons_ratio, 2));
            params.set<MooseEnum>("sif_mode") = "T";
            break;
        }
        if (_treat_as_2d && _use_crack_front_points_provider == false)
          vpp_base_name += "_2DVPP";
        for (unsigned int ring_index = 0; ring_index < _ring_vec.size(); ++ring_index)
        {
          params.set<unsigned int>("ring_index") = _ring_vec[ring_index];
          params.set<MooseEnum>("q_function_type") = _q_function_type;

          std::string vpp_name = vpp_base_name + "_" + Moose::stringify(_ring_vec[ring_index]);
          _problem->addVectorPostprocessor(vpp_type_name, vpp_name, params);
        }
      }
    }

    if (_get_equivalent_k)
    {
      std::string vpp_base_name("Keq");
      if (_treat_as_2d && _use_crack_front_points_provider == false)
        vpp_base_name += "_2DVPP";
      const std::string vpp_type_name("MixedModeEquivalentK");
      InputParameters params = _factory.getValidParams(vpp_type_name);
      params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
      params.set<Real>("poissons_ratio") = _poissons_ratio;

      for (unsigned int ring_index = 0; ring_index < _ring_vec.size(); ++ring_index)
      {
        std::string ki_name = "II_KI_";
        std::string kii_name = "II_KII_";
        std::string kiii_name = "II_KIII_";
        params.set<unsigned int>("ring_index") = _ring_vec[ring_index];
        if (_treat_as_2d && _use_crack_front_points_provider == false)
        {
          params.set<VectorPostprocessorName>("KI_vectorpostprocessor") =
              ki_name + "2DVPP_" + Moose::stringify(_ring_vec[ring_index]);
          params.set<VectorPostprocessorName>("KII_vectorpostprocessor") =
              kii_name + "2DVPP_" + Moose::stringify(_ring_vec[ring_index]);
          params.set<VectorPostprocessorName>("KIII_vectorpostprocessor") =
              kiii_name + "2DVPP_" + Moose::stringify(_ring_vec[ring_index]);
        }
        else
        {
          params.set<VectorPostprocessorName>("KI_vectorpostprocessor") =
              ki_name + Moose::stringify(_ring_vec[ring_index]);
          params.set<VectorPostprocessorName>("KII_vectorpostprocessor") =
              kii_name + Moose::stringify(_ring_vec[ring_index]);
          params.set<VectorPostprocessorName>("KIII_vectorpostprocessor") =
              kiii_name + Moose::stringify(_ring_vec[ring_index]);
        }
        params.set<std::string>("KI_vector_name") =
            ki_name + Moose::stringify(_ring_vec[ring_index]);
        params.set<std::string>("KII_vector_name") =
            kii_name + Moose::stringify(_ring_vec[ring_index]);
        params.set<std::string>("KIII_vector_name") =
            kiii_name + Moose::stringify(_ring_vec[ring_index]);
        std::string vpp_name = vpp_base_name + "_" + Moose::stringify(_ring_vec[ring_index]);
        _problem->addVectorPostprocessor(vpp_type_name, vpp_name, params);
      }
    }

    if (!_treat_as_2d || _use_crack_front_points_provider == true)
    {
      for (unsigned int i = 0; i < _output_variables.size(); ++i)
      {
        const std::string vpp_type_name("VectorOfPostprocessors");
        InputParameters params = _factory.getValidParams(vpp_type_name);
        params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
        std::ostringstream vpp_name_stream;
        vpp_name_stream << _output_variables[i] << "_crack";
        std::vector<PostprocessorName> postprocessor_names;
        for (unsigned int cfp_index = 0; cfp_index < num_crack_front_points; ++cfp_index)
        {
          std::ostringstream pp_name_stream;
          pp_name_stream << vpp_name_stream.str() << "_" << cfp_index + 1;
          postprocessor_names.push_back(pp_name_stream.str());
        }
        params.set<std::vector<PostprocessorName>>("postprocessors") = postprocessor_names;
        _problem->addVectorPostprocessor(vpp_type_name, vpp_name_stream.str(), params);
      }
    }
  }

  else if (_current_task == "add_material")
  {
    if (_temp != "")
    {
      std::string mater_name;
      const std::string mater_type_name("ThermalFractureIntegral");
      mater_name = "ThermalFractureIntegral";

      InputParameters params = _factory.getValidParams(mater_type_name);
      params.set<std::vector<MaterialPropertyName>>("eigenstrain_names") =
          getParam<std::vector<MaterialPropertyName>>("eigenstrain_names");
      params.set<std::vector<VariableName>>("temperature") = {_temp};
      params.set<std::vector<SubdomainName>>("block") = {_blocks};
      _problem->addMaterial(mater_type_name, mater_name, params);
    }
    MultiMooseEnum integral_moose_enums = getParam<MultiMooseEnum>("integrals");
    bool have_j_integral = false;
    bool have_c_integral = false;

    for (auto ime : integral_moose_enums)
    {
      if (ime == "JIntegral" || ime == "CIntegral" || ime == "KFromJIntegral")
        have_j_integral = true;

      if (ime == "CIntegral")
        have_c_integral = true;
    }
    if (have_j_integral)
    {
      std::string mater_name;
      const std::string mater_type_name(ad_prepend + "StrainEnergyDensity");
      mater_name = ad_prepend + "StrainEnergyDensity";

      InputParameters params = _factory.getValidParams(mater_type_name);
      _incremental = getParam<bool>("incremental");
      params.set<bool>("incremental") = _incremental;
      params.set<std::vector<SubdomainName>>("block") = {_blocks};
      _problem->addMaterial(mater_type_name, mater_name, params);

      {
        std::string mater_name;
        const std::string mater_type_name(ad_prepend + "EshelbyTensor");
        mater_name = ad_prepend + "EshelbyTensor";

        InputParameters params = _factory.getValidParams(mater_type_name);
        _displacements = getParam<std::vector<VariableName>>("displacements");
        params.set<std::vector<VariableName>>("displacements") = _displacements;
        params.set<std::vector<SubdomainName>>("block") = {_blocks};

        if (have_c_integral)
          params.set<bool>("compute_dissipation") = true;

        if (_temp != "")
          params.set<std::vector<VariableName>>("temperature") = {_temp};

        _problem->addMaterial(mater_type_name, mater_name, params);
      }
      // Strain energy rate density needed for C(t)/C* integral
      if (have_c_integral)
      {
        std::string mater_name;
        const std::string mater_type_name(ad_prepend + "StrainEnergyRateDensity");
        mater_name = ad_prepend + "StrainEnergyRateDensity";

        InputParameters params = _factory.getValidParams(mater_type_name);
        params.set<std::vector<SubdomainName>>("block") = {_blocks};
        params.set<std::vector<MaterialName>>("inelastic_models") =
            getParam<std::vector<MaterialName>>("inelastic_models");

        _problem->addMaterial(mater_type_name, mater_name, params);
      }
    }
  }
}

unsigned int
DomainIntegralAction::calcNumCrackFrontPoints()
{
  unsigned int num_points = 0;
  if (_boundary_names.size() != 0)
  {
    std::vector<BoundaryID> bids = _mesh->getBoundaryIDs(_boundary_names, true);
    std::set<unsigned int> nodes;

    ConstBndNodeRange & bnd_nodes = *_mesh->getBoundaryNodeRange();
    for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin(); nd != bnd_nodes.end(); ++nd)
    {
      const BndNode * bnode = *nd;
      BoundaryID boundary_id = bnode->_bnd_id;

      for (unsigned int ibid = 0; ibid < bids.size(); ++ibid)
      {
        if (boundary_id == bids[ibid])
        {
          nodes.insert(bnode->_node->id());
          break;
        }
      }
    }
    num_points = nodes.size();
  }
  else if (_crack_front_points.size() != 0)
    num_points = _crack_front_points.size();
  else if (_use_crack_front_points_provider)
    num_points = getParam<unsigned int>("number_points_from_provider");
  else
    mooseError("Must define either 'boundary' or 'crack_front_points'");
  return num_points;
}

void
DomainIntegralAction::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)
{
  if (_integrals.count(INTERACTION_INTEGRAL_T) != 0)
  {
    InputParameters params = _factory.getValidParams("CrackFrontDefinition");
    addRelationshipManagers(input_rm_type, params);
  }
}
