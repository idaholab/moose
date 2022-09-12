//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMAction.h"

// MOOSE includes
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "Executioner.h"
#include "MooseEnum.h"
#include "Parser.h"
#include "Factory.h"
#include "AddVariableAction.h"
#include "MooseApp.h"
#include "InputParameterWarehouse.h"

#include "GeometricCutUserObject.h"
#include "CrackFrontDefinition.h"

#include "libmesh/transient_system.h"
#include "libmesh/string_to_enum.h"

// XFEM includes
#include "XFEM.h"

registerMooseAction("XFEMApp", XFEMAction, "setup_xfem");

registerMooseAction("XFEMApp", XFEMAction, "add_aux_variable");

registerMooseAction("XFEMApp", XFEMAction, "add_aux_kernel");

registerMooseAction("XFEMApp", XFEMAction, "add_variable");

registerMooseAction("XFEMApp", XFEMAction, "add_kernel");

registerMooseAction("XFEMApp", XFEMAction, "add_bc");

InputParameters
XFEMAction::validParams()
{
  InputParameters params = Action::validParams();

  params.addParam<std::vector<UserObjectName>>(
      "geometric_cut_userobjects",
      "List of names of GeometricCutUserObjects with cut info and methods");
  params.addParam<std::string>("qrule", "volfrac", "XFEM quadrature rule to use");
  params.addRangeCheckedParam<unsigned int>(
      "debug_output_level",
      1,
      "debug_output_level <= 3",
      "Controls the amount of debug output from XFEM.  0: None, 1: Summary, 2: Details on "
      "modifications to mesh, 3: Full dump of element fragment algorithm mesh");
  params.addParam<bool>("output_cut_plane", false, "Output the XFEM cut plane and volume fraction");
  params.addParam<bool>("use_crack_growth_increment", false, "Use fixed crack growth increment");
  params.addParam<Real>("crack_growth_increment", 0.1, "Crack growth increment");
  params.addParam<bool>("use_crack_tip_enrichment", false, "Use crack tip enrichment functions");
  params.addParam<UserObjectName>("crack_front_definition",
                                  "The CrackFrontDefinition user object name (only "
                                  "needed if 'use_crack_tip_enrichment=true')");
  params.addParam<std::vector<VariableName>>("displacements",
                                             "Names of displacement variables (only "
                                             "needed if 'use_crack_tip_enrichment=true')");
  params.addParam<std::vector<VariableName>>("enrichment_displacements",
                                             "Names of enrichment displacement variables (only "
                                             "needed if 'use_crack_tip_enrichment=true')");
  params.addParam<std::vector<BoundaryName>>("cut_off_boundary",
                                             "Boundary that contains all nodes for which "
                                             "enrichment DOFs should be fixed away from crack tip "
                                             "(only needed if 'use_crack_tip_enrichment=true')");
  params.addParam<Real>("cut_off_radius",
                        "The cut off radius of crack tip enrichment functions (only needed if "
                        "'use_crack_tip_enrichment=true')");
  params.addClassDescription("Action to input general parameters and simulation options for use "
                             "in XFEM.");
  return params;
}

XFEMAction::XFEMAction(const InputParameters & params)
  : Action(params),
    _geom_cut_userobjects(getParam<std::vector<UserObjectName>>("geometric_cut_userobjects")),
    _xfem_qrule(getParam<std::string>("qrule")),
    _xfem_cut_plane(false),
    _xfem_use_crack_growth_increment(getParam<bool>("use_crack_growth_increment")),
    _xfem_crack_growth_increment(getParam<Real>("crack_growth_increment")),
    _use_crack_tip_enrichment(getParam<bool>("use_crack_tip_enrichment"))
{
  _order = "CONSTANT";
  _family = "MONOMIAL";
  if (isParamValid("output_cut_plane"))
    _xfem_cut_plane = getParam<bool>("output_cut_plane");

  if (_use_crack_tip_enrichment)
  {
    if (isParamValid("crack_front_definition"))
      _crack_front_definition = getParam<UserObjectName>("crack_front_definition");
    else
      mooseError("To add crack tip enrichment, crack_front_definition must be provided.");

    if (isParamValid("displacements"))
      _displacements = getParam<std::vector<VariableName>>("displacements");
    else
      mooseError("To add crack tip enrichment, displacements must be provided.");

    if (isParamValid("enrichment_displacements"))
    {
      _enrich_displacements = getParam<std::vector<VariableName>>("enrichment_displacements");
      if (_enrich_displacements.size() != 8 && _displacements.size() == 2)
        mooseError("The number of enrichment displacements should be total 8 for 2D.");
      else if (_enrich_displacements.size() != 12 && _displacements.size() == 3)
        mooseError("The number of enrichment displacements should be total 12 for 3D.");
    }
    else
      mooseError("To add crack tip enrichment, enrichment_displacements must be provided.");

    if (isParamValid("cut_off_boundary"))
      _cut_off_bc = getParam<std::vector<BoundaryName>>("cut_off_boundary");
    else
      mooseError("To add crack tip enrichment, cut_off_boundary must be provided.");

    if (isParamValid("cut_off_radius"))
      _cut_off_radius = getParam<Real>("cut_off_radius");
    else
      mooseError("To add crack tip enrichment, cut_off_radius must be provided.");
  }
}

void
XFEMAction::act()
{

  std::shared_ptr<XFEMInterface> xfem_interface = _problem->getXFEM();
  if (xfem_interface == NULL)
  {
    const auto & params = _app.getInputParameterWarehouse().getInputParameters();
    InputParameters & pars(*(params.find(uniqueActionName())->second.get()));
    pars.set<FEProblemBase *>("_fe_problem_base") = &*_problem;
    std::shared_ptr<XFEM> new_xfem(new XFEM(_pars));
    _problem->initXFEM(new_xfem);
    xfem_interface = _problem->getXFEM();
  }

  std::shared_ptr<XFEM> xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(xfem_interface);
  if (xfem == NULL)
    mooseError("dynamic cast of xfem object failed");

  if (_current_task == "setup_xfem")
  {
    xfem->setXFEMQRule(_xfem_qrule);

    xfem->setCrackGrowthMethod(_xfem_use_crack_growth_increment, _xfem_crack_growth_increment);
    xfem->setDebugOutputLevel(getParam<unsigned int>("debug_output_level"));
  }
  else if (_current_task == "add_variable" && _use_crack_tip_enrichment)
  {
    auto var_params = _factory.getValidParams("MooseVariable");
    var_params.set<MooseEnum>("family") = "LAGRANGE";
    var_params.set<MooseEnum>("order") = "FIRST";

    for (const auto & enrich_disp : _enrich_displacements)
      _problem->addVariable("MooseVariable", enrich_disp, var_params);
  }
  else if (_current_task == "add_kernel" && _use_crack_tip_enrichment)
  {
    for (unsigned int i = 0; i < _enrich_displacements.size(); ++i)
    {
      InputParameters params = _factory.getValidParams("CrackTipEnrichmentStressDivergenceTensors");
      params.set<NonlinearVariableName>("variable") = _enrich_displacements[i];
      params.set<unsigned int>("component") = i / 4;
      params.set<unsigned int>("enrichment_component") = i % 4;
      params.set<UserObjectName>("crack_front_definition") = _crack_front_definition;
      params.set<std::vector<VariableName>>("enrichment_displacements") = _enrich_displacements;
      params.set<std::vector<VariableName>>("displacements") = _displacements;
      _problem->addKernel(
          "CrackTipEnrichmentStressDivergenceTensors", _enrich_displacements[i], params);
    }
  }
  else if (_current_task == "add_bc" && _use_crack_tip_enrichment)
  {
    for (unsigned int i = 0; i < _enrich_displacements.size(); ++i)
    {
      InputParameters params = _factory.getValidParams("CrackTipEnrichmentCutOffBC");
      params.set<NonlinearVariableName>("variable") = _enrich_displacements[i];
      params.set<Real>("value") = 0;
      params.set<std::vector<BoundaryName>>("boundary") = _cut_off_bc;
      params.set<Real>("cut_off_radius") = _cut_off_radius;
      params.set<UserObjectName>("crack_front_definition") = _crack_front_definition;
      _problem->addBoundaryCondition(
          "CrackTipEnrichmentCutOffBC", _enrich_displacements[i], params);
    }
  }
  else if (_current_task == "add_aux_variable" && _xfem_cut_plane)
  {
    auto var_params = _factory.getValidParams("MooseVariableConstMonomial");

    _problem->addAuxVariable("MooseVariableConstMonomial", "xfem_cut_origin_x", var_params);
    _problem->addAuxVariable("MooseVariableConstMonomial", "xfem_cut_origin_y", var_params);
    _problem->addAuxVariable("MooseVariableConstMonomial", "xfem_cut_origin_z", var_params);
    _problem->addAuxVariable("MooseVariableConstMonomial", "xfem_cut_normal_x", var_params);
    _problem->addAuxVariable("MooseVariableConstMonomial", "xfem_cut_normal_y", var_params);
    _problem->addAuxVariable("MooseVariableConstMonomial", "xfem_cut_normal_z", var_params);
    _problem->addAuxVariable("MooseVariableConstMonomial", "xfem_cut2_origin_x", var_params);
    _problem->addAuxVariable("MooseVariableConstMonomial", "xfem_cut2_origin_y", var_params);
    _problem->addAuxVariable("MooseVariableConstMonomial", "xfem_cut2_origin_z", var_params);
    _problem->addAuxVariable("MooseVariableConstMonomial", "xfem_cut2_normal_x", var_params);
    _problem->addAuxVariable("MooseVariableConstMonomial", "xfem_cut2_normal_y", var_params);
    _problem->addAuxVariable("MooseVariableConstMonomial", "xfem_cut2_normal_z", var_params);
    _problem->addAuxVariable("MooseVariableConstMonomial", "xfem_volfrac", var_params);
  }
  else if (_current_task == "add_aux_kernel" && _xfem_cut_plane)
  {
    InputParameters params = _factory.getValidParams("XFEMVolFracAux");
    params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;
    params.set<AuxVariableName>("variable") = "xfem_volfrac";
    _problem->addAuxKernel("XFEMVolFracAux", "xfem_volfrac", params);

    params = _factory.getValidParams("XFEMCutPlaneAux");
    params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

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
