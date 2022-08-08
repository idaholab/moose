//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
#include "NonlinearSystemBase.h"

registerMooseAction("TensorMechanicsApp", GeneralizedPlaneStrainAction, "add_scalar_kernel");

registerMooseAction("TensorMechanicsApp", GeneralizedPlaneStrainAction, "add_kernel");

registerMooseAction("TensorMechanicsApp", GeneralizedPlaneStrainAction, "add_user_object");

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

  return params;
}

GeneralizedPlaneStrainAction::GeneralizedPlaneStrainAction(const InputParameters & params)
  : Action(params),
    _displacements(getParam<std::vector<VariableName>>("displacements")),
    _ndisp(_displacements.size()),
    _out_of_plane_direction(getParam<MooseEnum>("out_of_plane_direction"))
{
}

void
GeneralizedPlaneStrainAction::act()
{
  // user object name
  const std::string uo_name = _name + "_GeneralizedPlaneStrainUserObject";

  //
  // Add off diagonal Jacobian kernels
  //
  if (_current_task == "add_kernel")
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
      if (_problem->getNonlinearSystemBase().hasVariable(temp[0]))
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

    _problem->addScalarKernel(sk_type, _name + "_GeneralizedPlaneStrain", params);
  }
}
