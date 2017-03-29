/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GeneralizedPlaneStrainAction.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<GeneralizedPlaneStrainAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Set up the GeneralizedPlaneStrain environment");
  params.addRequiredParam<std::vector<NonlinearVariableName>>("displacements",
                                                              "The displacement variables");
  params.addRequiredParam<NonlinearVariableName>("scalar_out_of_plane_strain",
                                                 "Scalar variable for the out-of-plane strain (in "
                                                 "y direction for 1D Axisymmetric or in z "
                                                 "direction for 2D Cartesian problems)");
  params.addParam<NonlinearVariableName>("temperature", "The temperature variable");
  params.addParam<FunctionName>("out_of_plane_pressure",
                                "0",
                                "Function used to prescribe pressure "
                                "in the out-of-plane direction (y "
                                "for 1D Axisymmetric or z for 2D "
                                "Cartesian problems)");
  params.addParam<Real>("factor", 1.0, "Scale factor applied to prescribed pressure");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<std::vector<SubdomainName>>("block",
                                              "The list of ids of the blocks (subdomain) "
                                              "that the GeneralizedPlaneStrain kernels "
                                              "will be applied to");

  return params;
}

GeneralizedPlaneStrainAction::GeneralizedPlaneStrainAction(const InputParameters & params)
  : Action(params),
    _displacements(getParam<std::vector<NonlinearVariableName>>("displacements")),
    _ndisp(_displacements.size())
{
  if (_ndisp > 2)
    mooseError("GeneralizedPlaneStrain only works for 1D axisymmetric or 2D generalized plane "
               "strain cases!");
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
        getParam<NonlinearVariableName>("scalar_out_of_plane_strain")};

    // add off-diagonal jacobian kernels for the displacements
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      std::string k_name = _name + "GeneralizedPlaneStrainOffDiag_disp" + Moose::stringify(i);
      params.set<NonlinearVariableName>("variable") = _displacements[i];

      _problem->addKernel(k_type, k_name, params);
    }

    // add temperature kernel only if temperature is a nonlinear variable (and not an auxvariable)
    if (isParamValid("temperature") &&
        _problem->getNonlinearSystemBase().hasVariable("temperature"))
    {
      params.set<NonlinearVariableName>("temperature") =
          getParam<NonlinearVariableName>("temperature");

      std::string k_name = _name + "_GeneralizedPlaneStrainOffDiag_temp";
      params.set<NonlinearVariableName>("variable") =
          getParam<NonlinearVariableName>("temperature");

      _problem->addKernel(k_type, k_name, params);
    }
  }

  //
  // Add user object
  //
  else if (_current_task == "add_user_object")
  {
    std::string uo_type = "GeneralizedPlaneStrainUserObject";
    InputParameters params = _factory.getValidParams(uo_type);

    params.applyParameters(parameters());
    params.set<MultiMooseEnum>("execute_on") = "linear";

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
        getParam<NonlinearVariableName>("scalar_out_of_plane_strain");

    // set the UserObjectName from previously added UserObject
    params.set<UserObjectName>("generalized_plane_strain") = uo_name;

    _problem->addScalarKernel(sk_type, _name + "_GeneralizedPlaneStrain", params);
  }
}
