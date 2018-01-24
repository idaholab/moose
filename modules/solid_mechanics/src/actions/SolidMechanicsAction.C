/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "SolidMechanicsAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<SolidMechanicsAction>()
{
  InputParameters params = validParams<Action>();
  MooseEnum elemType("truss undefined", "undefined");
  params.addParam<MooseEnum>("type", elemType, "The element type: " + elemType.getRawNames());
  params.addParam<NonlinearVariableName>("disp_x", "", "The x displacement");
  params.addParam<NonlinearVariableName>("disp_y", "", "The y displacement");
  params.addParam<NonlinearVariableName>("disp_z", "", "The z displacement");
  params.addParam<NonlinearVariableName>("disp_r", "", "The r displacement");
  params.addParam<NonlinearVariableName>("temp", "", "The temperature");
  params.addParam<Real>("zeta", 0.0, "Stiffness dependent damping parameter for Rayleigh damping");
  params.addParam<Real>("alpha", 0.0, "alpha parameter for HHT time integration");
  params.addParam<std::string>(
      "appended_property_name", "", "Name appended to material properties to make them unique");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<std::vector<SubdomainName>>(
      "block", "The list of ids of the blocks (subdomain) that these kernels will be applied to");
  params.addParam<bool>("volumetric_locking_correction",
                        true,
                        "Set to false to turn off volumetric locking correction");

  params.addParam<std::vector<AuxVariableName>>(
      "save_in_disp_x", "Auxiliary variables to save the x displacement residuals.");
  params.addParam<std::vector<AuxVariableName>>(
      "save_in_disp_y", "Auxiliary variables to save the y displacement residuals.");
  params.addParam<std::vector<AuxVariableName>>(
      "save_in_disp_z", "Auxiliary variables to save the z displacement residuals.");
  params.addParam<std::vector<AuxVariableName>>(
      "save_in_disp_r", "Auxiliary variables to save the r displacement residuals.");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in_disp_x",
      "Auxiliary variables to save the x displacement diagonal preconditioner terms.");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in_disp_y",
      "Auxiliary variables to save the y displacement diagonal preconditioner terms.");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in_disp_z",
      "Auxiliary variables to save the z displacement diagonal preconditioner terms.");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in_disp_r",
      "Auxiliary variables to save the r displacement diagonal preconditioner terms.");
  return params;
}

SolidMechanicsAction::SolidMechanicsAction(const InputParameters & params)
  : Action(params),
    _disp_x(getParam<NonlinearVariableName>("disp_x")),
    _disp_y(getParam<NonlinearVariableName>("disp_y")),
    _disp_z(getParam<NonlinearVariableName>("disp_z")),
    _disp_r(getParam<NonlinearVariableName>("disp_r")),
    _temp(getParam<NonlinearVariableName>("temp")),
    _zeta(getParam<Real>("zeta")),
    _alpha(getParam<Real>("alpha"))
{
}

void
SolidMechanicsAction::act()
{
  // list of subdomains IDs per coordinate system
  std::map<Moose::CoordinateSystemType, std::vector<SubdomainName>> coord_map;
  std::set<SubdomainID> subdomains;

  if (isParamValid("block")) // Should it be restricted to certain blocks?
  {
    Moose::out << "Restricting to blocks!" << std::endl;
    std::vector<SubdomainName> block = getParam<std::vector<SubdomainName>>("block");
    for (unsigned int i = 0; i < block.size(); i++)
      subdomains.insert(_problem->mesh().getSubdomainID(block[i]));
  }
  else // Put it everywhere
    subdomains = _problem->mesh().meshSubdomains();

  for (std::set<SubdomainID>::const_iterator it = subdomains.begin(); it != subdomains.end(); ++it)
  {
    SubdomainID sid = *it;
    Moose::CoordinateSystemType coord_type = _problem->getCoordSystem(sid);

    // Convert the SubdomainID into SubdomainName since kernel params take SubdomainNames (this is
    // retarded...)
    std::stringstream ss;
    ss << sid;
    SubdomainName sname = ss.str();

    coord_map[coord_type].push_back(sname);
  }

  for (std::map<Moose::CoordinateSystemType, std::vector<SubdomainName>>::iterator it =
           coord_map.begin();
       it != coord_map.end();
       ++it)
  {
    Moose::CoordinateSystemType coord_type = (*it).first;
    std::vector<SubdomainName> & blocks = (*it).second;

    // Determine whether RZ or RSPHERICAL
    bool rz(false);
    bool rspherical(false);
    unsigned int dim(1);
    std::vector<std::string> keys;
    std::vector<VariableName> vars;
    std::vector<std::vector<AuxVariableName>> save_in;
    std::vector<std::vector<AuxVariableName>> diag_save_in;
    std::string type("StressDivergence");
    if (getParam<MooseEnum>("type") == 0) // truss
    {
      type = "StressDivergenceTruss";
    }
    else if (coord_type == Moose::COORD_RZ)
    {
      rz = true;
      type = "StressDivergenceRZ";
      dim = 2;
      keys.push_back("disp_r");
      keys.push_back("disp_z");
      vars.push_back(_disp_r);
      vars.push_back(_disp_z);
      save_in.resize(dim);
      if (isParamValid("save_in_disp_r"))
        save_in[0] = getParam<std::vector<AuxVariableName>>("save_in_disp_r");

      if (isParamValid("save_in_disp_z"))
        save_in[1] = getParam<std::vector<AuxVariableName>>("save_in_disp_z");

      diag_save_in.resize(dim);
      if (isParamValid("diag_save_in_disp_r"))
        diag_save_in[0] = getParam<std::vector<AuxVariableName>>("diag_save_in_disp_r");

      if (isParamValid("diag_save_in_disp_z"))
        diag_save_in[1] = getParam<std::vector<AuxVariableName>>("diag_save_in_disp_z");
    }
    else if (coord_type == Moose::COORD_RSPHERICAL)
    {
      rspherical = true;
      type = "StressDivergenceRSpherical";
      dim = 1;
      keys.push_back("disp_r");
      vars.push_back(_disp_r);
      save_in.resize(dim);
      if (isParamValid("save_in_disp_r"))
        save_in[0] = getParam<std::vector<AuxVariableName>>("save_in_disp_r");

      diag_save_in.resize(dim);
      if (isParamValid("diag_save_in_disp_r"))
        diag_save_in[0] = getParam<std::vector<AuxVariableName>>("diag_save_in_disp_r");
    }

    if (!rz && !rspherical && _disp_x == "")
    {
      mooseError("disp_x must be specified");
    }

    if (!rz && !rspherical)
    {
      keys.push_back("disp_x");
      vars.push_back(_disp_x);
      if (_disp_y != "")
      {
        ++dim;
        keys.push_back("disp_y");
        vars.push_back(_disp_y);
        if (_disp_z != "")
        {
          ++dim;
          keys.push_back("disp_z");
          vars.push_back(_disp_z);
        }
      }

      save_in.resize(dim);
      if (isParamValid("save_in_disp_x"))
        save_in[0] = getParam<std::vector<AuxVariableName>>("save_in_disp_x");

      if (isParamValid("save_in_disp_y"))
        save_in[1] = getParam<std::vector<AuxVariableName>>("save_in_disp_y");

      if (isParamValid("save_in_disp_z"))
        save_in[2] = getParam<std::vector<AuxVariableName>>("save_in_disp_z");

      diag_save_in.resize(dim);
      if (isParamValid("diag_save_in_disp_x"))
        diag_save_in[0] = getParam<std::vector<AuxVariableName>>("diag_save_in_disp_x");

      if (isParamValid("diag_save_in_disp_y"))
        diag_save_in[1] = getParam<std::vector<AuxVariableName>>("diag_save_in_disp_y");

      if (isParamValid("diag_save_in_disp_z"))
        diag_save_in[2] = getParam<std::vector<AuxVariableName>>("diag_save_in_disp_z");
    }

    unsigned int num_coupled(dim);
    if (_temp != "")
    {
      ++num_coupled;
      keys.push_back("temp");
      vars.push_back(_temp);
    }

    InputParameters params = _factory.getValidParams(type);
    for (unsigned j(0); j < num_coupled; ++j)
    {
      params.addCoupledVar(keys[j], "");
      params.set<std::vector<VariableName>>(keys[j]) = {vars[j]};
    }

    params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
    params.set<std::string>("appended_property_name") =
        getParam<std::string>("appended_property_name");

    for (unsigned int i(0); i < dim; ++i)
    {
      std::stringstream name;
      name << _name;
      name << i;

      params.set<unsigned int>("component") = i;

      params.set<NonlinearVariableName>("variable") = vars[i];
      params.set<std::vector<SubdomainName>>("block") = blocks;
      params.set<std::vector<AuxVariableName>>("save_in") = save_in[i];
      params.set<std::vector<AuxVariableName>>("diag_save_in") = diag_save_in[i];
      params.set<Real>("zeta") = _zeta;
      params.set<Real>("alpha") = _alpha;
      params.set<bool>("volumetric_locking_correction") =
          getParam<bool>("volumetric_locking_correction");
      _problem->addKernel(type, name.str(), params);
    }
  }
}
