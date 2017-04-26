/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// MOOSE includes
#include "DomainIntegralAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "CrackFrontDefinition.h"
#include "InteractionIntegralAuxFields.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<DomainIntegralAction>()
{
  InputParameters params = validParams<Action>();
  addCrackFrontDefinitionParams(params);
  MultiMooseEnum integral_vec("JIntegral InteractionIntegralKI InteractionIntegralKII "
                              "InteractionIntegralKIII InteractionIntegralT");
  params.addRequiredParam<MultiMooseEnum>("integrals",
                                          integral_vec,
                                          "Domain integrals to calculate.  Choices are: " +
                                              integral_vec.getRawNames());
  params.addParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where this boundary condition applies");
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
  params.addParam<bool>(
      "convert_J_to_K", false, "Convert J-integral to stress intensity factor K.");
  params.addParam<Real>("poissons_ratio", "Poisson's ratio");
  params.addParam<Real>("youngs_modulus", "Young's modulus");
  params.addParam<std::vector<SubdomainName>>(
      "block", "The block ids where InteractionIntegralAuxFields is defined");
  params.addParam<VariableName>("disp_x", "", "The x displacement");
  params.addParam<VariableName>("disp_y", "", "The y displacement");
  params.addParam<VariableName>("disp_z", "", "The z displacement");
  params.addParam<VariableName>("temp", "", "The temperature");
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
  // params.addParam<std::string>("xfem_qrule", "volfrac", "XFEM quadrature rule to use");
  return params;
}

DomainIntegralAction::DomainIntegralAction(const InputParameters & params)
  : Action(params),
    _boundary_names(getParam<std::vector<BoundaryName>>("boundary")),
    _order(getParam<std::string>("order")),
    _family(getParam<std::string>("family")),
    _direction_method_moose_enum(getParam<MooseEnum>("crack_direction_method")),
    _end_direction_method_moose_enum(getParam<MooseEnum>("crack_end_direction_method")),
    _have_crack_direction_vector(false),
    _have_crack_direction_vector_end_1(false),
    _have_crack_direction_vector_end_2(false),
    _treat_as_2d(getParam<bool>("2d")),
    _axis_2d(getParam<unsigned int>("axis_2d")),
    _convert_J_to_K(false),
    _has_symmetry_plane(isParamValid("symmetry_plane")),
    _symmetry_plane(_has_symmetry_plane ? getParam<unsigned int>("symmetry_plane")
                                        : std::numeric_limits<unsigned int>::max()),
    _position_type(getParam<MooseEnum>("position_type")),
    _q_function_type(getParam<MooseEnum>("q_function_type")),
    _get_equivalent_k(getParam<bool>("equivalent_k")),
    _use_displaced_mesh(false)
{
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
    mooseError("DomainIntegral error: invalid q_function_type.");

  if (isParamValid("crack_front_points"))
  {
    _crack_front_points = getParam<std::vector<Point>>("crack_front_points");
  }
  if (isParamValid("crack_direction_vector"))
  {
    _crack_direction_vector = getParam<RealVectorValue>("crack_direction_vector");
    _have_crack_direction_vector = true;
  }
  if (isParamValid("crack_direction_vector_end_1"))
  {
    _crack_direction_vector_end_1 = getParam<RealVectorValue>("crack_direction_vector_end_1");
    _have_crack_direction_vector_end_1 = true;
  }
  if (isParamValid("crack_direction_vector_end_2"))
  {
    _crack_direction_vector_end_2 = getParam<RealVectorValue>("crack_direction_vector_end_2");
    _have_crack_direction_vector_end_2 = true;
  }
  if (isParamValid("crack_mouth_boundary"))
    _crack_mouth_boundary_names = getParam<std::vector<BoundaryName>>("crack_mouth_boundary");
  if (isParamValid("intersecting_boundary"))
    _intersecting_boundary_names = getParam<std::vector<BoundaryName>>("intersecting_boundary");
  if (_radius_inner.size() != _radius_outer.size())
    mooseError("Number of entries in 'radius_inner' and 'radius_outer' must match.");

  bool youngs_modulus_set(false);
  bool poissons_ratio_set(false);
  MultiMooseEnum integral_moose_enums = getParam<MultiMooseEnum>("integrals");
  if (integral_moose_enums.size() == 0)
    mooseError("Must specify at least one domain integral to perform.");
  for (unsigned int i = 0; i < integral_moose_enums.size(); ++i)
  {
    if (integral_moose_enums[i] != "JIntegral")
    {
      // Check that parameters required for interaction integrals are defined
      if (!(isParamValid("disp_x")) || !(isParamValid("disp_y")))
        mooseError("DomainIntegral error: must set displacements for integral: ",
                   integral_moose_enums[i]);

      if (!(isParamValid("poissons_ratio")) || !(isParamValid("youngs_modulus")))
        mooseError(
            "DomainIntegral error: must set Poisson's ratio and Young's modulus for integral: ",
            integral_moose_enums[i]);

      if (!(isParamValid("block")))
        mooseError("DomainIntegral error: must set block ID or name for integral: ",
                   integral_moose_enums[i]);

      _poissons_ratio = getParam<Real>("poissons_ratio");
      poissons_ratio_set = true;
      _youngs_modulus = getParam<Real>("youngs_modulus");
      youngs_modulus_set = true;
      _blocks = getParam<std::vector<SubdomainName>>("block");
      _disp_x = getParam<VariableName>("disp_x");
      _disp_y = getParam<VariableName>("disp_y");
      _disp_z = getParam<VariableName>("disp_z");
      if (isParamValid("temp"))
        _temp = getParam<VariableName>("temp");
    }

    _integrals.insert(INTEGRAL(int(integral_moose_enums.get(i))));
  }

  if (_get_equivalent_k && (_integrals.count(INTERACTION_INTEGRAL_KI) == 0 ||
                            _integrals.count(INTERACTION_INTEGRAL_KII) == 0 ||
                            _integrals.count(INTERACTION_INTEGRAL_KIII) == 0))
    mooseError("DomainIntegral error: must calculate KI, KII and KIII to get equivalent K.");

  if (isParamValid("output_variable"))
  {
    _output_variables = getParam<std::vector<VariableName>>("output_variable");
    if (_crack_front_points.size() > 0)
      mooseError("'output_variables' not yet supported with 'crack_front_points'");
  }

  if (isParamValid("convert_J_to_K"))
    _convert_J_to_K = getParam<bool>("convert_J_to_K");
  if (_convert_J_to_K)
  {
    if (!isParamValid("youngs_modulus") || !isParamValid("poissons_ratio"))
      mooseError("DomainIntegral error: must set Young's modulus and Poisson's ratio for "
                 "J-integral if convert_J_to_K = true.");
    if (!youngs_modulus_set)
      _youngs_modulus = getParam<Real>("youngs_modulus");
    if (!poissons_ratio_set)
      _poissons_ratio = getParam<Real>("poissons_ratio");
  }
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

  if (_current_task == "add_user_object")
  {
    const std::string uo_type_name("CrackFrontDefinition");

    InputParameters params = _factory.getValidParams(uo_type_name);
    MooseUtils::setExecuteOnFlags(params, {EXEC_INITIAL, EXEC_NONLINEAR});
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
    params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;
    if (_integrals.count(INTERACTION_INTEGRAL_T) != 0)
    {
      params.set<VariableName>("disp_x") = _disp_x;
      params.set<VariableName>("disp_y") = _disp_y;
      if (_disp_z != "")
        params.set<VariableName>("disp_z") = _disp_z;
      params.set<bool>("t_stress") = true;
    }
    if (_q_function_type == TOPOLOGY)
    {
      params.set<bool>("q_function_rings") = true;
      params.set<unsigned int>("last_ring") = _ring_last;
    }

    _problem->addUserObject(uo_type_name, uo_name, params);
  }
  else if (_current_task == "add_aux_variable")
  {
    for (unsigned int ring_index = 0; ring_index < _ring_vec.size(); ++ring_index)
    {
      if (_treat_as_2d)
      {
        std::ostringstream av_name_stream;
        av_name_stream << av_base_name << "_" << _ring_vec[ring_index];
        _problem->addAuxVariable(av_name_stream.str(),
                                 FEType(Utility::string_to_enum<Order>(_order),
                                        Utility::string_to_enum<FEFamily>(_family)));
      }
      else
      {
        for (unsigned int cfp_index = 0; cfp_index < num_crack_front_points; ++cfp_index)
        {
          std::ostringstream av_name_stream;
          av_name_stream << av_base_name << "_" << cfp_index + 1 << "_" << _ring_vec[ring_index];
          _problem->addAuxVariable(av_name_stream.str(),
                                   FEType(Utility::string_to_enum<Order>(_order),
                                          Utility::string_to_enum<FEFamily>(_family)));
        }
      }
    }
  }
  else if (_current_task == "add_aux_kernel")
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
    MooseUtils::setExecuteOnFlags(params, {EXEC_INITIAL, EXEC_TIMESTEP_END});
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

      if (_treat_as_2d)
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
    if (_integrals.count(J_INTEGRAL) != 0)
    {
      std::string pp_base_name;
      if (_convert_J_to_K)
        pp_base_name = "K";
      else
        pp_base_name = "J";
      const std::string pp_type_name("JIntegral");
      InputParameters params = _factory.getValidParams(pp_type_name);
      MooseUtils::setExecuteOnFlags(params, {EXEC_TIMESTEP_END});
      params.set<UserObjectName>("crack_front_definition") = uo_name;
      params.set<bool>("convert_J_to_K") = _convert_J_to_K;
      if (_convert_J_to_K)
      {
        params.set<Real>("youngs_modulus") = _youngs_modulus;
        params.set<Real>("poissons_ratio") = _poissons_ratio;
      }
      if (_has_symmetry_plane)
        params.set<unsigned int>("symmetry_plane") = _symmetry_plane;
      params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;
      for (unsigned int ring_index = 0; ring_index < _ring_vec.size(); ++ring_index)
      {
        if (_treat_as_2d)
        {
          std::ostringstream av_name_stream;
          av_name_stream << av_base_name << "_" << _ring_vec[ring_index];
          std::ostringstream pp_name_stream;
          pp_name_stream << pp_base_name << "_" << _ring_vec[ring_index];
          std::vector<VariableName> qvars;
          qvars.push_back(av_name_stream.str());
          params.set<std::vector<VariableName>>("q") = qvars;
          _problem->addPostprocessor(pp_type_name, pp_name_stream.str(), params);
        }
        else
        {
          for (unsigned int cfp_index = 0; cfp_index < num_crack_front_points; ++cfp_index)
          {
            std::ostringstream av_name_stream;
            av_name_stream << av_base_name << "_" << cfp_index + 1 << "_" << _ring_vec[ring_index];
            std::ostringstream pp_name_stream;
            pp_name_stream << pp_base_name << "_" << cfp_index + 1 << "_" << _ring_vec[ring_index];
            std::vector<VariableName> qvars;
            qvars.push_back(av_name_stream.str());
            params.set<std::vector<VariableName>>("q") = qvars;
            params.set<unsigned int>("crack_front_point_index") = cfp_index;
            _problem->addPostprocessor(pp_type_name, pp_name_stream.str(), params);
          }
        }
      }
    }
    if (_integrals.count(INTERACTION_INTEGRAL_KI) != 0 ||
        _integrals.count(INTERACTION_INTEGRAL_KII) != 0 ||
        _integrals.count(INTERACTION_INTEGRAL_KIII) != 0 ||
        _integrals.count(INTERACTION_INTEGRAL_T) != 0)
    {

      if (_has_symmetry_plane && (_integrals.count(INTERACTION_INTEGRAL_KII) != 0 ||
                                  _integrals.count(INTERACTION_INTEGRAL_KIII) != 0))
        mooseError("In DomainIntegral, symmetry_plane option cannot be used with mode-II or "
                   "mode-III interaction integral");

      const std::string pp_base_name("II");
      const std::string pp_type_name("InteractionIntegral");
      InputParameters params = _factory.getValidParams(pp_type_name);
      MooseUtils::setExecuteOnFlags(params, {EXEC_TIMESTEP_END});
      params.set<UserObjectName>("crack_front_definition") = uo_name;
      params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;
      if (_has_symmetry_plane)
        params.set<unsigned int>("symmetry_plane") = _symmetry_plane;
      params.set<Real>("poissons_ratio") = _poissons_ratio;
      params.set<Real>("youngs_modulus") = _youngs_modulus;
      params.set<std::vector<VariableName>>("disp_x") = {_disp_x};
      params.set<std::vector<VariableName>>("disp_y") = {_disp_y};
      if (_disp_z != "")
        params.set<std::vector<VariableName>>("disp_z") = {_disp_z};
      if (_temp != "")
        params.set<std::vector<VariableName>>("temp") = {_temp};
      if (_has_symmetry_plane)
        params.set<unsigned int>("symmetry_plane") = _symmetry_plane;

      for (std::set<INTEGRAL>::iterator sit = _integrals.begin(); sit != _integrals.end(); ++sit)
      {
        std::string pp_base_name;
        std::string aux_mode_name;
        switch (*sit)
        {
          case J_INTEGRAL:
            continue;

          case INTERACTION_INTEGRAL_KI:
            pp_base_name = "II_KI";
            aux_mode_name = "_I_";
            params.set<Real>("K_factor") =
                0.5 * _youngs_modulus / (1.0 - std::pow(_poissons_ratio, 2.0));
            break;

          case INTERACTION_INTEGRAL_KII:
            pp_base_name = "II_KII";
            aux_mode_name = "_II_";
            params.set<Real>("K_factor") =
                0.5 * _youngs_modulus / (1.0 - std::pow(_poissons_ratio, 2.0));
            break;

          case INTERACTION_INTEGRAL_KIII:
            pp_base_name = "II_KIII";
            aux_mode_name = "_III_";
            params.set<Real>("K_factor") = 0.5 * _youngs_modulus / (1.0 + _poissons_ratio);
            break;

          case INTERACTION_INTEGRAL_T:
            pp_base_name = "II_T";
            aux_mode_name = "_T_";
            params.set<Real>("K_factor") = _youngs_modulus / (1 - std::pow(_poissons_ratio, 2));
            params.set<bool>("t_stress") = true;
            break;
        }

        for (unsigned int ring_index = 0; ring_index < _ring_vec.size(); ++ring_index)
        {
          if (_treat_as_2d)
          {
            std::ostringstream av_name_stream;
            av_name_stream << av_base_name << "_" << _ring_vec[ring_index];
            std::ostringstream pp_name_stream;
            pp_name_stream << pp_base_name << "_" << _ring_vec[ring_index];
            std::string aux_stress_name = aux_stress_base_name + aux_mode_name + "1";
            std::string aux_grad_disp_name = aux_grad_disp_base_name + aux_mode_name + "1";
            params.set<MaterialPropertyName>("aux_stress") = aux_stress_name;
            params.set<MaterialPropertyName>("aux_grad_disp") = aux_grad_disp_name;
            std::vector<VariableName> qvars;
            qvars.push_back(av_name_stream.str());
            params.set<std::vector<VariableName>>("q") = qvars;
            _problem->addPostprocessor(pp_type_name, pp_name_stream.str(), params);
          }
          else
          {
            for (unsigned int cfp_index = 0; cfp_index < num_crack_front_points; ++cfp_index)
            {
              std::ostringstream av_name_stream;
              av_name_stream << av_base_name << "_" << cfp_index + 1 << "_"
                             << _ring_vec[ring_index];
              std::ostringstream pp_name_stream;
              pp_name_stream << pp_base_name << "_" << cfp_index + 1 << "_"
                             << _ring_vec[ring_index];
              std::ostringstream cfn_index_stream;
              cfn_index_stream << cfp_index + 1;
              std::ostringstream aux_stress_name_stream;
              aux_stress_name_stream << aux_stress_base_name << aux_mode_name << cfp_index + 1;
              std::ostringstream aux_grad_disp_name_stream;
              aux_grad_disp_name_stream << aux_grad_disp_base_name << aux_mode_name
                                        << cfp_index + 1;
              params.set<MaterialPropertyName>("aux_stress") = aux_stress_name_stream.str();
              params.set<MaterialPropertyName>("aux_grad_disp") = aux_grad_disp_name_stream.str();
              std::vector<VariableName> qvars;
              qvars.push_back(av_name_stream.str());
              params.set<std::vector<VariableName>>("q") = qvars;
              params.set<unsigned int>("crack_front_point_index") = cfp_index;
              _problem->addPostprocessor(pp_type_name, pp_name_stream.str(), params);
            }
          }
        }
      }
    }
    for (unsigned int i = 0; i < _output_variables.size(); ++i)
    {
      const std::string ov_base_name(_output_variables[i]);
      const std::string pp_type_name("CrackFrontData");
      InputParameters params = _factory.getValidParams(pp_type_name);
      MooseUtils::setExecuteOnFlags(params, {EXEC_TIMESTEP_END});
      params.set<UserObjectName>("crack_front_definition") = uo_name;
      if (_treat_as_2d)
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
    if (_get_equivalent_k)
    {
      std::string pp_base_name("Keq");
      const std::string pp_type_name("MixedModeEquivalentK");
      InputParameters params = _factory.getValidParams(pp_type_name);
      MooseUtils::setExecuteOnFlags(params, {EXEC_TIMESTEP_END});
      params.set<Real>("poissons_ratio") = _poissons_ratio;
      for (unsigned int ring_index = 0; ring_index < _ring_vec.size(); ++ring_index)
      {
        if (_treat_as_2d)
        {
          std::ostringstream ki_name_stream;
          ki_name_stream << "II_KI_" << _ring_vec[ring_index];
          std::ostringstream kii_name_stream;
          kii_name_stream << "II_KII_" << _ring_vec[ring_index];
          std::ostringstream kiii_name_stream;
          kiii_name_stream << "II_KIII_" << _ring_vec[ring_index];
          params.set<PostprocessorName>("KI_name") = ki_name_stream.str();
          params.set<PostprocessorName>("KII_name") = kii_name_stream.str();
          params.set<PostprocessorName>("KIII_name") = kiii_name_stream.str();
          std::ostringstream pp_name_stream;
          pp_name_stream << pp_base_name << "_" << _ring_vec[ring_index];
          _problem->addPostprocessor(pp_type_name, pp_name_stream.str(), params);
        }
        else
        {
          for (unsigned int cfp_index = 0; cfp_index < num_crack_front_points; ++cfp_index)
          {
            std::ostringstream ki_name_stream;
            ki_name_stream << "II_KI_" << cfp_index + 1 << "_" << _ring_vec[ring_index];
            std::ostringstream kii_name_stream;
            kii_name_stream << "II_KII_" << cfp_index + 1 << "_" << _ring_vec[ring_index];
            std::ostringstream kiii_name_stream;
            kiii_name_stream << "II_KIII_" << cfp_index + 1 << "_" << _ring_vec[ring_index];
            params.set<PostprocessorName>("KI_name") = ki_name_stream.str();
            params.set<PostprocessorName>("KII_name") = kii_name_stream.str();
            params.set<PostprocessorName>("KIII_name") = kiii_name_stream.str();
            std::ostringstream pp_name_stream;
            pp_name_stream << pp_base_name << "_" << cfp_index + 1 << "_" << _ring_vec[ring_index];
            params.set<unsigned int>("crack_front_point_index") = cfp_index;
            _problem->addPostprocessor(pp_type_name, pp_name_stream.str(), params);
          }
        }
      }
    }
  }
  else if (_current_task == "add_vector_postprocessor")
  {
    if (!_treat_as_2d)
    {
      for (std::set<INTEGRAL>::iterator sit = _integrals.begin(); sit != _integrals.end(); ++sit)
      {
        std::string pp_base_name;
        switch (*sit)
        {
          case J_INTEGRAL:
            if (_convert_J_to_K)
              pp_base_name = "K";
            else
              pp_base_name = "J";
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
        const std::string vpp_type_name("CrackDataSampler");
        InputParameters params = _factory.getValidParams(vpp_type_name);
        MooseUtils::setExecuteOnFlags(params, {EXEC_TIMESTEP_END});
        params.set<UserObjectName>("crack_front_definition") = uo_name;
        params.set<MooseEnum>("sort_by") = "id";
        params.set<MooseEnum>("position_type") = _position_type;
        for (unsigned int ring_index = 0; ring_index < _ring_vec.size(); ++ring_index)
        {
          std::vector<PostprocessorName> postprocessor_names;
          std::ostringstream vpp_name_stream;
          vpp_name_stream << pp_base_name << "_" << _ring_vec[ring_index];
          for (unsigned int cfp_index = 0; cfp_index < num_crack_front_points; ++cfp_index)
          {
            std::ostringstream pp_name_stream;
            pp_name_stream << pp_base_name << "_" << cfp_index + 1 << "_" << _ring_vec[ring_index];
            postprocessor_names.push_back(pp_name_stream.str());
          }
          params.set<std::vector<PostprocessorName>>("postprocessors") = postprocessor_names;
          _problem->addVectorPostprocessor(vpp_type_name, vpp_name_stream.str(), params);
        }
      }

      for (unsigned int i = 0; i < _output_variables.size(); ++i)
      {
        const std::string vpp_type_name("VectorOfPostprocessors");
        InputParameters params = _factory.getValidParams(vpp_type_name);
        MooseUtils::setExecuteOnFlags(params, {EXEC_TIMESTEP_END});
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
    if (_get_equivalent_k && !_treat_as_2d)
    {
      std::string pp_base_name("Keq");
      const std::string vpp_type_name("CrackDataSampler");
      InputParameters params = _factory.getValidParams(vpp_type_name);
      MooseUtils::setExecuteOnFlags(params, {EXEC_TIMESTEP_END});
      params.set<UserObjectName>("crack_front_definition") = uo_name;
      params.set<MooseEnum>("sort_by") = "id";
      params.set<MooseEnum>("position_type") = _position_type;
      for (unsigned int ring_index = 0; ring_index < _ring_vec.size(); ++ring_index)
      {
        std::vector<PostprocessorName> postprocessor_names;
        std::ostringstream vpp_name_stream;
        vpp_name_stream << pp_base_name << "_" << _ring_vec[ring_index];
        for (unsigned int cfp_index = 0; cfp_index < num_crack_front_points; ++cfp_index)
        {
          std::ostringstream pp_name_stream;
          pp_name_stream << pp_base_name << "_" << cfp_index + 1 << "_" << _ring_vec[ring_index];
          postprocessor_names.push_back(pp_name_stream.str());
        }
        params.set<std::vector<PostprocessorName>>("postprocessors") = postprocessor_names;
        _problem->addVectorPostprocessor(vpp_type_name, vpp_name_stream.str(), params);
      }
    }
  }
  else if (_current_task == "add_material")
  {

    int n_int_integrals = 0;
    int i_ki = 0;
    int i_kii = 0;
    int i_kiii = 0;
    int i_t = 0;

    if (_integrals.count(INTERACTION_INTEGRAL_KI) != 0)
    {
      i_ki = n_int_integrals;
      n_int_integrals++;
    }
    if (_integrals.count(INTERACTION_INTEGRAL_KII) != 0)
    {
      i_kii = n_int_integrals;
      n_int_integrals++;
    }
    if (_integrals.count(INTERACTION_INTEGRAL_KIII) != 0)
    {
      i_kiii = n_int_integrals;
      n_int_integrals++;
    }
    if (_integrals.count(INTERACTION_INTEGRAL_T) != 0)
    {
      i_t = n_int_integrals;
      n_int_integrals++;
    }

    std::vector<MooseEnum> sif_mode_enum_vec(
        InteractionIntegralAuxFields::getSIFModesVec(n_int_integrals));

    if (_integrals.count(INTERACTION_INTEGRAL_KI) != 0)
      sif_mode_enum_vec[i_ki] = "KI";
    if (_integrals.count(INTERACTION_INTEGRAL_KII) != 0)
      sif_mode_enum_vec[i_kii] = "KII";
    if (_integrals.count(INTERACTION_INTEGRAL_KIII) != 0)
      sif_mode_enum_vec[i_kiii] = "KIII";
    if (_integrals.count(INTERACTION_INTEGRAL_T) != 0)
      sif_mode_enum_vec[i_t] = "T";

    if (sif_mode_enum_vec.size() > 0)
    {
      const std::string mater_base_name("auxFields");
      const std::string mater_type_name("InteractionIntegralAuxFields");
      InputParameters params = _factory.getValidParams(mater_type_name);
      params.set<UserObjectName>("crack_front_definition") = uo_name;
      params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;
      params.set<std::vector<MooseEnum>>("sif_modes") = sif_mode_enum_vec;
      params.set<Real>("poissons_ratio") = _poissons_ratio;
      params.set<Real>("youngs_modulus") = _youngs_modulus;
      params.set<std::vector<SubdomainName>>("block") = _blocks;

      if (_treat_as_2d)
      {
        std::ostringstream mater_name_stream;
        mater_name_stream << mater_base_name << "_1";
        params.set<unsigned int>("crack_front_point_index") = 0;
        params.set<std::string>("appended_index_name") = "1";
        _problem->addMaterial(mater_type_name, mater_name_stream.str(), params);
      }
      else
      {
        for (unsigned int cfp_index = 0; cfp_index < num_crack_front_points; ++cfp_index)
        {
          std::ostringstream mater_name_stream;
          mater_name_stream << mater_base_name << "_" << cfp_index + 1;
          params.set<unsigned int>("crack_front_point_index") = cfp_index;
          std::ostringstream cfn_index_stream;
          cfn_index_stream << cfp_index + 1;
          params.set<std::string>("appended_index_name") = cfn_index_stream.str();
          _problem->addMaterial(mater_type_name, mater_name_stream.str(), params);
        }
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
  else
    mooseError("Must define either 'boundary' or 'crack_front_points'");
  return num_points;
}
