//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedPlaneStrainAction.h"

#include "Conversion.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "NonlinearSystemBase.h"

#include <set>

registerMooseAction("SolidMechanicsApp", GeneralizedPlaneStrainAction, "add_scalar_kernel");

registerMooseAction("SolidMechanicsApp", GeneralizedPlaneStrainAction, "add_kernel");

registerMooseAction("SolidMechanicsApp", GeneralizedPlaneStrainAction, "add_user_object");

registerMooseAction("SolidMechanicsApp", GeneralizedPlaneStrainAction, "add_variables_physics");

InputParameters
GeneralizedPlaneStrainAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Set up the GeneralizedPlaneStrain environment");
  params.addRequiredParam<std::vector<VariableName>>("displacements", "The displacement variables");
  params.addRequiredParam<VariableName>("scalar_out_of_plane_strain",
                                        "Scalar variable for the out-of-plane strain (in "
                                        "y direction for 1D Axisymmetric or in z "
                                        "direction for 2D Cartesian problems)");
  params.addParam<std::vector<VariableName>>("temperature", "The temperature variable");
  MooseEnum outOfPlaneDirection("x y z", "z");
  params.addParam<MooseEnum>(
      "out_of_plane_direction", outOfPlaneDirection, "The direction of the out-of-plane strain.");
  params.addParam<FunctionName>(
      "out_of_plane_pressure_function",
      "Function used to prescribe pressure (applied toward the body) in the out-of-plane direction "
      "(y for 1D Axisymmetric or z for 2D Cartesian problems)");
  params.addDeprecatedParam<FunctionName>(
      "out_of_plane_pressure",
      "Function used to prescribe pressure (applied toward the body) in the out-of-plane direction "
      "(y for 1D Axisymmetric or z for 2D Cartesian problems)",
      "This has been replaced by 'out_of_plane_pressure_function'");
  params.addParam<MaterialPropertyName>("out_of_plane_pressure_material",
                                        "0",
                                        "Material used to prescribe pressure (applied toward the "
                                        "body) in the out-of-plane direction");
  params.addDeprecatedParam<Real>(
      "factor",
      "Scale factor applied to prescribed out-of-plane pressure (both material and function)",
      "This has been replaced by 'pressure_factor'");
  params.addParam<Real>(
      "pressure_factor",
      "Scale factor applied to prescribed out-of-plane pressure (both material and function)");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<std::vector<SubdomainName>>("block",
                                              "The list of ids of the blocks (subdomain) "
                                              "that the GeneralizedPlaneStrain kernels "
                                              "will be applied to");
  params.addParam<std::vector<TagName>>(
      "extra_vector_tags",
      "The tag names for extra vectors that residual data should be saved into");
  params.addParam<std::vector<TagName>>("absolute_value_vector_tags",
                                        "The tag names for extra vectors that the absolute value "
                                        "of the residual should be accumulated into");
  params.addParam<bool>("use_automatic_differentiation",
                        false,
                        "Use automatic differentiation to assemble the generalized plane strain "
                        "equation and its coupling terms");

  return params;
}

GeneralizedPlaneStrainAction::GeneralizedPlaneStrainAction(const InputParameters & params)
  : Action(params),
    _displacements(getParam<std::vector<VariableName>>("displacements")),
    _ndisp(_displacements.size()),
    _out_of_plane_direction(getParam<MooseEnum>("out_of_plane_direction")),
    _use_ad(getParam<bool>("use_automatic_differentiation"))
{
}

unsigned int
GeneralizedPlaneStrainAction::inPlaneDisplacementIndex() const
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    if (i != _out_of_plane_direction)
      return i;

  paramError("displacements", "No in-plane displacement is available to anchor the action");
}

void
GeneralizedPlaneStrainAction::act()
{
  // user object name
  const std::string uo_name = _name + "_GeneralizedPlaneStrainUserObject";

  if (_current_task == "add_variables_physics")
  {
    if (_use_ad)
    {
      std::set<SubdomainID> block_ids;
      if (isParamValid("block"))
        for (const auto & block : getParam<std::vector<SubdomainName>>("block"))
        {
          const auto id = _mesh->getSubdomainID(block);
          if (id == Moose::INVALID_BLOCK_ID)
            paramError("block", "Subdomain '", block, "' was not found in the mesh");
          block_ids.insert(id);
        }

      const auto & subdomains = block_ids.empty() ? _problem->mesh().meshSubdomains() : block_ids;
      if (subdomains.empty())
        mooseError("No subdomains found for the generalized plane strain action");

      const auto coord_system = _problem->getCoordSystem(*subdomains.begin());
      for (const auto subdomain : subdomains)
        if (_problem->getCoordSystem(subdomain) != coord_system)
          paramError("block",
                     "Generalized plane strain requires all selected subdomains to use the same "
                     "coordinate system");

      if (coord_system == Moose::COORD_RZ)
      {
        if (_ndisp != 1)
          paramError("displacements",
                     "One radial displacement is required for 1D axisymmetric generalized plane "
                     "strain");
      }
      else if (coord_system == Moose::COORD_XYZ)
      {
        const unsigned int required_displacements = _out_of_plane_direction == 2 ? 2 : 3;
        if (_ndisp != required_displacements)
          paramError("displacements",
                     required_displacements,
                     " displacement variables are required when the out-of-plane direction is ",
                     getParam<MooseEnum>("out_of_plane_direction"));
      }
      else
        paramError("out_of_plane_direction",
                   "Generalized plane strain supports only Cartesian and axisymmetric coordinate "
                   "systems");
    }

    const auto anchor_displacement = inPlaneDisplacementIndex();
    const auto & anchor_variable = _problem->getVariable(
        0, _displacements[anchor_displacement], Moose::VarKindType::VAR_SOLVER);
    const auto solver_sys_num = anchor_variable.sys().number();
    if (!_problem->isSolverSystemNonlinear(solver_sys_num))
      paramError("displacements", "The in-plane displacements must be nonlinear variables");

    for (unsigned int i = 0; i < _ndisp; ++i)
      if (i != _out_of_plane_direction &&
          _problem->getVariable(0, _displacements[i], Moose::VarKindType::VAR_SOLVER)
                  .sys()
                  .number() != solver_sys_num)
        paramError("displacements",
                   "All in-plane displacements must belong to the same nonlinear system");

    auto & nonlinear_system = _problem->getNonlinearSystemBase(solver_sys_num);
    const auto & scalar_variable = getParam<VariableName>("scalar_out_of_plane_strain");
    if (nonlinear_system.hasScalarVariable(scalar_variable))
      return;
    if (_problem->hasScalarVariable(scalar_variable))
      paramError("scalar_out_of_plane_strain",
                 "Variable '",
                 scalar_variable,
                 "' already exists but is not a nonlinear scalar variable in system '",
                 nonlinear_system.name(),
                 "'");
    if (_problem->hasVariable(scalar_variable))
      paramError("scalar_out_of_plane_strain",
                 "Variable '",
                 scalar_variable,
                 "' already exists as a field variable; a scalar variable is required");

    InputParameters params = _factory.getValidParams("MooseVariableScalar");
    params.set<MooseEnum>("family") = "SCALAR";
    params.set<MooseEnum>("order") = "FIRST";
    params.set<SolverSystemName>("solver_sys") = nonlinear_system.name();
    _problem->addVariable("MooseVariableScalar", scalar_variable, params);
  }

  //
  // Add off diagonal Jacobian kernels
  //
  else if (_current_task == "add_kernel" && _use_ad)
  {
    const std::string k_type = "ADGeneralizedPlaneStrain";
    InputParameters params = _factory.getValidParams(k_type);

    params.applyParameters(parameters(),
                           {"scalar_out_of_plane_strain",
                            "out_of_plane_pressure",
                            "out_of_plane_pressure_function",
                            "factor",
                            "pressure_factor"});
    params.set<std::vector<VariableName>>("scalar_out_of_plane_strain") = {
        getParam<VariableName>("scalar_out_of_plane_strain")};

    if (parameters().isParamSetByUser("out_of_plane_pressure"))
      params.set<FunctionName>("out_of_plane_pressure") =
          getParam<FunctionName>("out_of_plane_pressure");
    if (parameters().isParamSetByUser("out_of_plane_pressure_function"))
      params.set<FunctionName>("out_of_plane_pressure_function") =
          getParam<FunctionName>("out_of_plane_pressure_function");
    if (parameters().isParamSetByUser("factor"))
      params.set<Real>("factor") = getParam<Real>("factor");
    if (parameters().isParamSetByUser("pressure_factor"))
      params.set<Real>("pressure_factor") = getParam<Real>("pressure_factor");

    const auto anchor_displacement = inPlaneDisplacementIndex();
    params.set<NonlinearVariableName>("variable") = _displacements[anchor_displacement];
    _problem->addKernel(k_type, _name + "_ADGeneralizedPlaneStrain", params);
  }
  else if (_use_ad && (_current_task == "add_user_object" || _current_task == "add_scalar_kernel"))
  {
    // ADKernelScalarBase assembles both the elemental resultant and the scalar equation, so the
    // legacy UserObject and ScalarKernel are intentionally unnecessary in AD mode.
  }
  else if (_current_task == "add_kernel")
  {
    std::string k_type = "GeneralizedPlaneStrainOffDiag";
    InputParameters params = _factory.getValidParams(k_type);

    params.applyParameters(parameters(), {"scalar_out_of_plane_strain"});
    params.set<std::vector<VariableName>>("scalar_out_of_plane_strain") = {
        getParam<VariableName>("scalar_out_of_plane_strain")};

    // add off-diagonal jacobian kernels for the displacements
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      if (_out_of_plane_direction == i)
        continue;

      std::string k_name = _name + "GeneralizedPlaneStrainOffDiag_disp" + Moose::stringify(i);
      params.set<NonlinearVariableName>("variable") = _displacements[i];

      _problem->addKernel(k_type, k_name, params);
    }

    // add temperature kernel only if temperature is a nonlinear variable (and not an auxvariable)
    if (isParamValid("temperature"))
    {
      auto temp = getParam<std::vector<VariableName>>("temperature");
      if (temp.size() > 1)
        mooseError("Only one variable may be specified in 'temperature'");
      if (_problem->getNonlinearSystemBase(/*nl_sys_num=*/0).hasVariable(temp[0]))
      {
        std::string k_name = _name + "_GeneralizedPlaneStrainOffDiag_temp";
        params.set<NonlinearVariableName>("variable") = temp[0];

        _problem->addKernel(k_type, k_name, params);
      }
    }
  }

  //
  // Add user object
  //
  else if (_current_task == "add_user_object")
  {
    std::string uo_type = "GeneralizedPlaneStrainUserObject";
    InputParameters params = _factory.getValidParams(uo_type);

    // Skipping selected parameters in applyParameters() and then manually setting them only if they
    // are set by the user is just to prevent both the current and deprecated variants of these
    // parameters from both getting passed to the UserObject. Once we get rid of the deprecated
    // versions, we can just set them all with applyParameters().
    params.applyParameters(
        parameters(),
        {"out_of_plane_pressure", "out_of_plane_pressure_function", "factor", "pressure_factor"});
    if (parameters().isParamSetByUser("out_of_plane_pressure"))
      params.set<FunctionName>("out_of_plane_pressure") =
          getParam<FunctionName>("out_of_plane_pressure");
    if (parameters().isParamSetByUser("out_of_plane_pressure_function"))
      params.set<FunctionName>("out_of_plane_pressure_function") =
          getParam<FunctionName>("out_of_plane_pressure_function");
    if (parameters().isParamSetByUser("factor"))
      params.set<Real>("factor") = getParam<Real>("factor");
    if (parameters().isParamSetByUser("pressure_factor"))
      params.set<Real>("pressure_factor") = getParam<Real>("pressure_factor");

    _problem->addUserObject(uo_type, uo_name, params);
  }

  //
  // Add scalar kernel
  //
  else if (_current_task == "add_scalar_kernel")
  {
    std::string sk_type = "GeneralizedPlaneStrain";
    InputParameters params = _factory.getValidParams(sk_type);

    params.set<NonlinearVariableName>("variable") =
        getParam<VariableName>("scalar_out_of_plane_strain");

    // set the UserObjectName from previously added UserObject
    params.set<UserObjectName>("generalized_plane_strain") = uo_name;

    if (isParamValid("extra_vector_tags"))
      params.set<std::vector<TagName>>("extra_vector_tags") =
          getParam<std::vector<TagName>>("extra_vector_tags");
    if (isParamValid("absolute_value_vector_tags"))
      params.set<std::vector<TagName>>("absolute_value_vector_tags") =
          getParam<std::vector<TagName>>("absolute_value_vector_tags");

    _problem->addScalarKernel(sk_type, _name + "_GeneralizedPlaneStrain", params);
  }
}
