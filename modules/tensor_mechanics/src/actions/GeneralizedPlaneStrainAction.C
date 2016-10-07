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
  params.addRequiredParam<std::vector<NonlinearVariableName> >("scalar_strain_zz", "The scalar_strain_zz variables");
  params.addParam<NonlinearVariableName>("temperature", "The temperature variable");
  params.addParam<FunctionName>("traction_zz", "0", "Function used to prescribe traction in the out-of-plane Z direction");
  params.addParam<Real>("factor", 1.0, "Scale factor applied to prescribed traction");
  params.addParam<bool>("use_displaced_mesh", true, "Whether to use displaced mesh");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<std::vector<SubdomainName> >("block", "The list of ids of the blocks (subdomain) that the GeneralizedPlaneStrain kernels will be applied to");

  return params;
}

GeneralizedPlaneStrainAction::GeneralizedPlaneStrainAction(const InputParameters & params) :
    Action(params),
    _displacements(getParam<std::vector<NonlinearVariableName> >("displacements")),
    _ndisp(_displacements.size()),
    _block(getParam<std::vector<SubdomainName> >("block")),
    _scalar_strain_zz(getParam<std::vector<NonlinearVariableName> >("scalar_strain_zz"))
{
  if (_ndisp != 2)
    mooseError("GeneralizedPlaneStrain only works for two dimensional case!");

  for (unsigned int i = 0; i < _scalar_strain_zz.size(); ++i)
    _scalar_strain_zz_var.push_back(_scalar_strain_zz[i]);

  if (isParamValid("block"))
    _nblock = _block.size();
  else
    _nblock = 1;

  if (_nblock != _scalar_strain_zz.size())
    mooseError("Number of scalar_strain_zz variable should equal to number of blocks which it will be applied to.");
}

void
GeneralizedPlaneStrainAction::act()
{
  for (unsigned int i = 0; i < _nblock; ++i)
  {
    if (_current_task == "add_kernel")
    {
      std::string k_type("GeneralizedPlaneStrainOffDiag");
      InputParameters params = _factory.getValidParams(k_type);

      params.set<std::vector<NonlinearVariableName> >("displacements") = _displacements;

      if (isParamValid("temperature"))
        params.set<NonlinearVariableName>("temperature") = getParam<NonlinearVariableName>("temperature");

      params.set<std::vector<VariableName> >("scalar_strain_zz") = {_scalar_strain_zz_var[i]};

      if (isParamValid("base_name"))
        params.set<std::string>("base_name") = getParam<std::string>("base_name");

      if (isParamValid("block"))
        params.set<std::vector<SubdomainName> >("block") = {_block[i]};

      params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

      for (unsigned int j = 0; j < _ndisp; ++j)
      {
        std::string k_name = "GeneralizedPlaneStrainOffDiag_" + Moose::stringify(i) + "_" + Moose::stringify(j);
        params.set<NonlinearVariableName>("variable") = _displacements[j];

        _problem->addKernel(k_type, k_name, params);
      }
    }
    else if (_current_task == "add_user_object")
    {
      std::string uo_type("GeneralizedPlaneStrainUserObject");
      InputParameters params = _factory.getValidParams(uo_type);

      std::string uo_name = "GeneralizedPlaneStrainUserObject_" + Moose::stringify(i);

      if (isParamValid("base_name"))
        params.set<std::string>("base_name") = getParam<std::string>("base_name");

      params.set<FunctionName>("traction_zz") = getParam<FunctionName>("traction_zz");
      params.set<Real>("factor") = getParam<Real>("factor");

      if (isParamValid("block"))
        params.set<std::vector<SubdomainName> >("block") = {_block[i]};

      params.set<MultiMooseEnum>("execute_on") = "linear";
      params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

      _problem->addUserObject(uo_type, uo_name, params);
    }
    else if (_current_task == "add_scalar_kernel")
    {
      std::string sk_type("GeneralizedPlaneStrain");
      InputParameters params = _factory.getValidParams(sk_type);

      std::string sk_name = "GeneralizedPlaneStrain_" + Moose::stringify(i);

      if (isParamValid("block"))
        params.set<std::vector<SubdomainName> >("block") = {_block[i]};

      params.set<NonlinearVariableName>("variable") = _scalar_strain_zz[i];

      // set the UserObjectName from previously added UserObject
      params.set<UserObjectName>("generalized_plane_strain") = "GeneralizedPlaneStrainUserObject_" + Moose::stringify(i);

      _problem->addScalarKernel(sk_type, sk_name, params);
    }
  }
}
