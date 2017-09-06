/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMAction.h"

// MOOSE includes
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "NonlinearSystem.h"
#include "Executioner.h"
#include "MooseEnum.h"
#include "Parser.h"
#include "Factory.h"

#include "GeometricCutUserObject.h"

#include "libmesh/transient_system.h"
#include "libmesh/string_to_enum.h"

// XFEM includes
#include "XFEM.h"
#include "XFEMElementPairLocator.h"

template <>
InputParameters
validParams<XFEMAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<std::vector<UserObjectName>>(
      "geometric_cut_userobjects",
      "List of names of GeometricCutUserObjects with cut info and methods");
  params.addParam<std::string>("qrule", "volfrac", "XFEM quadrature rule to use");
  params.addParam<bool>("output_cut_plane", false, "Output the XFEM cut plane and volume fraction");
  params.addParam<bool>("use_crack_growth_increment", false, "Use fixed crack growth increment");
  params.addParam<Real>("crack_growth_increment", 0.1, "Crack growth increment");
  return params;
}

XFEMAction::XFEMAction(InputParameters params)
  : Action(params),
    _geom_cut_userobjects(getParam<std::vector<UserObjectName>>("geometric_cut_userobjects")),
    _xfem_qrule(getParam<std::string>("qrule")),
    _xfem_cut_plane(false),
    _xfem_use_crack_growth_increment(getParam<bool>("use_crack_growth_increment")),
    _xfem_crack_growth_increment(getParam<Real>("crack_growth_increment"))
{
  _order = "CONSTANT";
  _family = "MONOMIAL";
  if (isParamValid("output_cut_plane"))
    _xfem_cut_plane = getParam<bool>("output_cut_plane");
}

void
XFEMAction::act()
{

  MooseSharedPointer<XFEMInterface> xfem_interface = _problem->getXFEM();
  if (xfem_interface == NULL)
  {
    _pars.set<FEProblemBase *>("_fe_problem_base") = &*_problem;
    MooseSharedPointer<XFEM> new_xfem(new XFEM(_pars));
    _problem->initXFEM(new_xfem);
    xfem_interface = _problem->getXFEM();
  }

  MooseSharedPointer<XFEM> xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(xfem_interface);
  if (xfem == NULL)
    mooseError("dynamic cast of xfem object failed");

  if (_current_task == "setup_xfem")
  {
    xfem->setXFEMQRule(_xfem_qrule);

    xfem->setCrackGrowthMethod(_xfem_use_crack_growth_increment, _xfem_crack_growth_increment);

    MooseSharedPointer<XFEMElementPairLocator> new_xfem_epl(new XFEMElementPairLocator(xfem, 0));
    _problem->geomSearchData().addElementPairLocator(0, new_xfem_epl);

    if (_problem->getDisplacedProblem() != NULL)
    {
      MooseSharedPointer<XFEMElementPairLocator> new_xfem_epl2(
          new XFEMElementPairLocator(xfem, 0, true));
      _problem->getDisplacedProblem()->geomSearchData().addElementPairLocator(0, new_xfem_epl2);
    }

    // Pull in geometric cut user objects by name (getUserObjectByName)
    // Send to XFEM and store in vector of GeometricCutUserObjects (addGeometricCut)
    for (unsigned int i = 0; i < _geom_cut_userobjects.size(); ++i)
    {
      const UserObject * uo = &(_problem->getUserObjectBase(_geom_cut_userobjects[i]));
      xfem->addGeometricCut(dynamic_cast<const GeometricCutUserObject *>(uo));
    }
  }
  else if (_current_task == "add_aux_variable" && _xfem_cut_plane)
  {
    _problem->addAuxVariable(
        "xfem_cut_origin_x",
        FEType(Utility::string_to_enum<Order>(_order), Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable(
        "xfem_cut_origin_y",
        FEType(Utility::string_to_enum<Order>(_order), Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable(
        "xfem_cut_origin_z",
        FEType(Utility::string_to_enum<Order>(_order), Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable(
        "xfem_cut_normal_x",
        FEType(Utility::string_to_enum<Order>(_order), Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable(
        "xfem_cut_normal_y",
        FEType(Utility::string_to_enum<Order>(_order), Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable(
        "xfem_cut_normal_z",
        FEType(Utility::string_to_enum<Order>(_order), Utility::string_to_enum<FEFamily>(_family)));

    _problem->addAuxVariable(
        "xfem_cut2_origin_x",
        FEType(Utility::string_to_enum<Order>(_order), Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable(
        "xfem_cut2_origin_y",
        FEType(Utility::string_to_enum<Order>(_order), Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable(
        "xfem_cut2_origin_z",
        FEType(Utility::string_to_enum<Order>(_order), Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable(
        "xfem_cut2_normal_x",
        FEType(Utility::string_to_enum<Order>(_order), Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable(
        "xfem_cut2_normal_y",
        FEType(Utility::string_to_enum<Order>(_order), Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable(
        "xfem_cut2_normal_z",
        FEType(Utility::string_to_enum<Order>(_order), Utility::string_to_enum<FEFamily>(_family)));

    _problem->addAuxVariable(
        "xfem_volfrac",
        FEType(Utility::string_to_enum<Order>(_order), Utility::string_to_enum<FEFamily>(_family)));
  }
  else if (_current_task == "add_aux_kernel" && _xfem_cut_plane)
  {
    InputParameters params = _factory.getValidParams("XFEMVolFracAux");
    params.set<MultiMooseEnum>("execute_on") = "timestep_begin";
    params.set<AuxVariableName>("variable") = "xfem_volfrac";
    _problem->addAuxKernel("XFEMVolFracAux", "xfem_volfrac", params);

    params = _factory.getValidParams("XFEMCutPlaneAux");
    params.set<MultiMooseEnum>("execute_on") = "timestep_end";

    // first cut plane
    params.set<unsigned int>("plane_id") = 0;

    params.set<AuxVariableName>("variable") = "xfem_cut_origin_x";
    params.set<MooseEnum>("quantity") = "origin_x";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut_origin_x", params);

    params.set<AuxVariableName>("variable") = "xfem_cut_origin_y";
    params.set<MooseEnum>("quantity") = "origin_y";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut_origin_y", params);

    params.set<AuxVariableName>("variable") = "xfem_cut_origin_z";
    params.set<MooseEnum>("quantity") = "origin_z";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut_origin_z", params);

    params.set<AuxVariableName>("variable") = "xfem_cut_normal_x";
    params.set<MooseEnum>("quantity") = "normal_x";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut_normal_x", params);

    params.set<AuxVariableName>("variable") = "xfem_cut_normal_y";
    params.set<MooseEnum>("quantity") = "normal_y";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut_normal_y", params);

    params.set<AuxVariableName>("variable") = "xfem_cut_normal_z";
    params.set<MooseEnum>("quantity") = "normal_z";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut_normal_z", params);

    // second cut plane
    params.set<unsigned int>("plane_id") = 1;

    params.set<AuxVariableName>("variable") = "xfem_cut2_origin_x";
    params.set<MooseEnum>("quantity") = "origin_x";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut2_origin_x", params);

    params.set<AuxVariableName>("variable") = "xfem_cut2_origin_y";
    params.set<MooseEnum>("quantity") = "origin_y";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut2_origin_y", params);

    params.set<AuxVariableName>("variable") = "xfem_cut2_origin_z";
    params.set<MooseEnum>("quantity") = "origin_z";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut2_origin_z", params);

    params.set<AuxVariableName>("variable") = "xfem_cut2_normal_x";
    params.set<MooseEnum>("quantity") = "normal_x";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut2_normal_x", params);

    params.set<AuxVariableName>("variable") = "xfem_cut2_normal_y";
    params.set<MooseEnum>("quantity") = "normal_y";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut2_normal_y", params);

    params.set<AuxVariableName>("variable") = "xfem_cut2_normal_z";
    params.set<MooseEnum>("quantity") = "normal_z";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut2_normal_z", params);
  }
}
