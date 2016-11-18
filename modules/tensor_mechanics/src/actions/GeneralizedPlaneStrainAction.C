/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GeneralizedPlaneStrainAction.h"

#include "FEProblem.h"
#include "Conversion.h"

template<>
InputParameters validParams<GeneralizedPlaneStrainAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Set up the GeneralizedPlaneStrain environment");
  params.addRequiredParam<std::vector<NonlinearVariableName> >("displacements", "The displacement variables");
  params.addRequiredParam<NonlinearVariableName>("scalar_strain_zz", "The scalar_strain_zz variable");
  params.addParam<NonlinearVariableName>("temperature", "The temperature variable");
  params.addParam<FunctionName>("traction_zz", "0", "Function used to prescribe traction in the out-of-plane Z direction");
  params.addParam<Real>("factor", 1.0, "Scale factor applied to prescribed traction");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<std::vector<SubdomainName> >("block", "The list of ids of the blocks (subdomain) that the GeneralizedPlaneStrain kernels will be applied to");

  return params;
}

GeneralizedPlaneStrainAction::GeneralizedPlaneStrainAction(const InputParameters & params) :
    Action(params),
    _displacements(getParam<std::vector<NonlinearVariableName> >("displacements")),
    _ndisp(_displacements.size()),
    _scalar_strain_zz(getParam<NonlinearVariableName>("scalar_strain_zz"))
{
  if (_ndisp != 2)
    mooseError("GeneralizedPlaneStrain only works for two dimensional case!");
}

void
GeneralizedPlaneStrainAction::act()
{
  if (_current_task == "add_kernel")
  {
    std::string k_type = "GeneralizedPlaneStrainOffDiag";
    InputParameters params = _factory.getValidParams(k_type);
    params.set<std::vector<NonlinearVariableName> >("displacements") = _displacements;
    params.set<std::vector<VariableName> >("scalar_strain_zz") = { _scalar_strain_zz };

    if (isParamValid("base_name"))
      params.set<std::string>("base_name") = getParam<std::string>("base_name");

    if (isParamValid("block"))
      params.set<std::vector<SubdomainName> >("block") = getParam<std::vector<SubdomainName> >("block");

    params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      std::string k_name = _name + "GeneralizedPlaneStrainOffDiag_disp" + Moose::stringify(i);
      params.set<NonlinearVariableName>("variable") = _displacements[i];

      _problem->addKernel(k_type, k_name, params);
    }

    if (isParamValid("temperature"))
    {
      params.set<NonlinearVariableName>("temperature") = getParam<NonlinearVariableName>("temperature");

      std::string k_name = _name + "_GeneralizedPlaneStrainOffDiag_temp";
      params.set<NonlinearVariableName>("variable") = getParam<NonlinearVariableName>("temperature");

      _problem->addKernel(k_type, k_name, params);
    }
  }
  else if (_current_task == "add_user_object")
  {
    std::string uo_type = "GeneralizedPlaneStrainUserObject";
    InputParameters params = _factory.getValidParams(uo_type);

    std::string uo_name = _name + "_GeneralizedPlaneStrainUserObject";

    if (isParamValid("base_name"))
      params.set<std::string>("base_name") = getParam<std::string>("base_name");

    params.set<FunctionName>("traction_zz") = getParam<FunctionName>("traction_zz");
    params.set<Real>("factor") = getParam<Real>("factor");

    if (isParamValid("block"))
      params.set<std::vector<SubdomainName> >("block") = getParam<std::vector<SubdomainName> >("block");

    params.set<MultiMooseEnum>("execute_on") = "linear";
    params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

    _problem->addUserObject(uo_type, uo_name, params);
  }
  else if (_current_task == "add_scalar_kernel")
  {
    std::string sk_type = "GeneralizedPlaneStrain";
    InputParameters params = _factory.getValidParams(sk_type);
    params.set<NonlinearVariableName>("variable") = _scalar_strain_zz;

    // set the UserObjectName from previously added UserObject
    params.set<UserObjectName>("generalized_plane_strain") = _name + "_GeneralizedPlaneStrainUserObject";

    std::string sk_name = _name + "_GeneralizedPlaneStrain";
    _problem->addScalarKernel(sk_type, sk_name, params);
  }
}
