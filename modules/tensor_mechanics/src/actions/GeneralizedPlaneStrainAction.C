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

template<>
InputParameters validParams<GeneralizedPlaneStrainAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Set up the GeneralizedPlaneStrain environment");
  params.addRequiredParam<std::vector<NonlinearVariableName> >("displacements", "The displacement variables");
  params.addParam<NonlinearVariableName>("scalar_strain_yy", "Scalar variable scalar_strain_yy for 1D Axisymmetric problem");
  params.addParam<NonlinearVariableName>("scalar_strain_zz", "Scalar variable scalar_strain_zz for 2D GeneralizedPlaneStrain problem");
  params.addParam<NonlinearVariableName>("temperature", "The temperature variable");
  params.addParam<FunctionName>("traction", "0", "Function used to prescribe traction in the out-of-plane Z direction");
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
    _has_scalar_strain_yy(isParamValid("scalar_strain_yy")),
    _has_scalar_strain_zz(isParamValid("scalar_strain_zz"))
{
  if (_ndisp > 2)
    mooseError("GeneralizedPlaneStrain only works for 1D axisymmetric and 2D generalized plane strain case!");

  if (_has_scalar_strain_yy && _has_scalar_strain_zz)
    mooseError("Must specify only scalar_strain_yy or scalar_strain_zz");
}

void
GeneralizedPlaneStrainAction::act()
{
  // get a list of all subdomains first
  const auto & subdomain_set = _problem->mesh().meshSubdomains();
  std::vector<SubdomainID> subdomains(subdomain_set.begin(), subdomain_set.end());

  // make sure all subdomains are using the same coordinate system
  Moose::CoordinateSystemType coord_system = _problem->getCoordSystem(subdomains[0]);
  for (auto s : subdomains)
    if (_problem->getCoordSystem(s) != coord_system)
      mooseError("The GeneralizedPlaneStrain action requires all subdomains to have the same coordinate system");

  if (_has_scalar_strain_zz && coord_system != Moose::COORD_XYZ)
    mooseError("scalar_strain_zz is for 2D generalized plane strain problems in XYZ coordinate system");
  else if (_has_scalar_strain_yy && coord_system != Moose::COORD_RZ)
    mooseError("scalar_strain_yy is for axisymmetric 1D problems in RZ coordinate system");

  if (_current_task == "add_kernel")
  {
    std::string k_type = "GeneralizedPlaneStrainOffDiag";
    InputParameters params = _factory.getValidParams(k_type);
    params.set<std::vector<NonlinearVariableName> >("displacements") = _displacements;

    if (_has_scalar_strain_yy)
      params.set<std::vector<VariableName> >("scalar_strain_yy") = {getParam<NonlinearVariableName>("scalar_strain_yy")};
    else if (_has_scalar_strain_zz)
      params.set<std::vector<VariableName> >("scalar_strain_zz") = {getParam<NonlinearVariableName>("scalar_strain_zz")};
    else
      mooseError("Must specify only scalar_strain_yy (1D Axisymmetric) or scalar_strain_zz (2D GeneralizedPlaneStrain)");

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

    params.set<FunctionName>("traction") = getParam<FunctionName>("traction");
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

    if (_has_scalar_strain_yy)
      params.set<NonlinearVariableName>("variable") = getParam<NonlinearVariableName>("scalar_strain_yy");
    else if (_has_scalar_strain_zz)
      params.set<NonlinearVariableName>("variable") = getParam<NonlinearVariableName>("scalar_strain_zz");
    else
      mooseError("Must specify only scalar_strain_yy (1D Axisymmetric) or scalar_strain_zz (2D GeneralizedPlaneStrain)");

    // set the UserObjectName from previously added UserObject
    params.set<UserObjectName>("generalized_plane_strain") = _name + "_GeneralizedPlaneStrainUserObject";

    std::string sk_name = _name + "_GeneralizedPlaneStrain";
    _problem->addScalarKernel(sk_type, sk_name, params);
  }
}
