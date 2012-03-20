#include "ThermalContactAction.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "AddSlaveFluxVectorAction.h"
#include "Conversion.h"

static unsigned int n = 0;                                  // numbering for gap heat transfer objects (we can have them on multiple interfaces)
static const std::string GAP_VALUE_VAR_NAME = "gap_value";
static const std::string PENETRATION_VAR_NAME = "penetration";

template<>
InputParameters validParams<ThermalContactAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::string>("type", "A string representing the Moose object that will be used for heat conduction over the gap");
  params.addParam<std::string>("gap_type", "GapValueAux", "A string representing the Moose object that will be used for computing the gap size");
  params.addRequiredParam<std::string>("variable", "The variable for thermal contact");
  params.addRequiredParam<unsigned int>("master", "The master surface");
  params.addRequiredParam<unsigned int>("slave", "The slave surface");
  params.addParam<std::string>("disp_x", "The x displacement");
  params.addParam<std::string>("disp_y", "The y displacement");
  params.addParam<std::string>("disp_z", "The z displacement");
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<std::string>("order", "FIRST", "The finite element order");

  return params;
}

ThermalContactAction::ThermalContactAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
ThermalContactAction::addBcs()
{
  /*
   * [./gap2]
   *   type = GapHeatTransfer
   *   variable = temp
   *   boundary = 10
   *   gap_distance = penetration
   *   gap_temp = gap_value
   *   disp_x = disp_x
   *   disp_y = disp_y
   *   disp_z = disp_z
   * [../]
   */

  InputParameters action_params = ActionFactory::instance()->getValidParams("AddBCAction");
  action_params.set<Parser *>("parser_handle") = getParam<Parser *>("parser_handle");
  action_params.set<std::string>("type") = getParam<std::string>("type");
  action_params.set<std::string>("name") = "BCs/gap_bc_" + Moose::stringify(n);
  Action *action = ActionFactory::instance()->create("AddBCAction", action_params);
  MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
  mooseAssert (moose_object_action, "Dynamic Cast failed");

  InputParameters & params = moose_object_action->getObjectParams();

  // get valid params for the BC specified in 'type' field
  InputParameters bc_params = Factory::instance()->getValidParams(getParam<std::string>("type"));
  _parser_handle.extractParams(_name, bc_params);
  params += bc_params;

  params.set<std::string>("variable") = getParam<std::string>("variable");
  std::vector<std::string> vars(1);
  vars[0] = PENETRATION_VAR_NAME;
  params.set<std::vector<std::string> >("gap_distance") = vars;
  vars[0] = GAP_VALUE_VAR_NAME;
  params.set<std::vector<std::string> >("gap_temp") = vars;
  std::vector<unsigned int> bnds(1, getParam<unsigned int>("slave"));
  params.set<std::vector<unsigned int> >("boundary") = bnds;

  if (isParamValid("disp_x"))
  {
    params.addCoupledVar("disp_x", "The x displacement");
    std::vector<std::string> disp_x(1, getParam<std::string>("disp_x"));
    params.set< std::vector<std::string> >("disp_x") = disp_x;
  }
  if (isParamValid("disp_y"))
  {
    params.addCoupledVar("disp_y", "The y displacement");
    std::vector<std::string> disp_y(1, getParam<std::string>("disp_y"));
    params.set< std::vector<std::string> >("disp_y") = disp_y;
  }
  if (isParamValid("disp_z"))
  {
    params.addCoupledVar("disp_z", "The z displacement");
    std::vector<std::string> disp_z(1, getParam<std::string>("disp_z"));
    params.set< std::vector<std::string> >("disp_z") = disp_z;
  }

  // add it to the warehouse
  Moose::action_warehouse.addActionBlock(action);
}

void
ThermalContactAction::addAuxVariables()
{
  /*
  [./gap_value]
    order = FIRST
    family = LAGRANGE
  [../]
  [./penetration]
    order = FIRST
    family = LAGRANGE
  [../]
  */

  // We need to add the variables only once
  if (n == 0)
  {
      InputParameters action_params = ActionFactory::instance()->getValidParams("AddVariableAction");
//    for (unsigned int i=0; i<action_params.size(); ++i)
//    {
      action_params.set<Parser *>("parser_handle") = getParam<Parser *>("parser_handle");
      action_params.set<std::string>("action") = "add_aux_variable";
      action_params.set<std::string>("name") = "AuxVariables/" + GAP_VALUE_VAR_NAME;
      action_params.set<std::string>("order") = getParam<std::string>("order");
      // gap_value
      Action *action = ActionFactory::instance()->create("AddVariableAction", action_params);
      Moose::action_warehouse.addActionBlock(action);
      // penetration
      action_params.set<std::string>("name") = "AuxVariables/" + PENETRATION_VAR_NAME;
      action = ActionFactory::instance()->create("AddVariableAction", action_params);
      Moose::action_warehouse.addActionBlock(action);
//    }

      action_params = ActionFactory::instance()->getValidParams("CopyNodalVarsAction");
      action_params.set<Parser *>("parser_handle") = getParam<Parser *>("parser_handle");
      action_params.set<std::string>("action") = "copy_nodal_aux_vars";
      action_params.set<std::string>("name") = "AuxVariables/" + GAP_VALUE_VAR_NAME;
      // gap_value
      action = ActionFactory::instance()->create("CopyNodalVarsAction", action_params);
      Moose::action_warehouse.addActionBlock(action);
      // penetration
      action_params.set<std::string>("name") = "AuxVariables/" + PENETRATION_VAR_NAME;
      action = ActionFactory::instance()->create("CopyNodalVarsAction", action_params);
      Moose::action_warehouse.addActionBlock(action);
  }
}

void
ThermalContactAction::addAuxBcs()
{
  /*
  [./gapvalue]
    type = GapValueAux
    variable = gap_value
    boundary = 10
    paired_boundary = 5
    paired_variable = temp
  [../]
  [./penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 10
    paired_boundary = 5
  [../]
  */

  InputParameters action_params = ActionFactory::instance()->getValidParams("AddBCAction");
  action_params.set<Parser *>("parser_handle") = getParam<Parser *>("parser_handle");
  action_params.set<std::string>("action") = "add_aux_bc";

  {
    action_params.set<std::string>("type") = getParam<std::string>("gap_type");
    action_params.set<std::string>("name") = "AuxBCs/gap_value_" + Moose::stringify(n);
    Action *action = ActionFactory::instance()->create("AddBCAction", action_params);
    MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    mooseAssert (moose_object_action, "Dynamic Cast failed");

    InputParameters & params = moose_object_action->getObjectParams();
    params.set<std::string>("variable") = GAP_VALUE_VAR_NAME;
    std::vector<unsigned int> bnds(1, getParam<unsigned int>("slave"));
    params.set<std::vector<unsigned int> >("boundary") = bnds;
    params.set<unsigned int>("paired_boundary") = getParam<unsigned int>("master");
    std::vector<std::string> vars(1, getParam<std::string>("variable"));
    params.set<std::vector<std::string> >("paired_variable") = vars;
    params.set<std::string>("order") = getParam<std::string>("order");
    if (isParamValid("tangential_tolerance"))
    {
      params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");
    }
    // add it to the warehouse
    Moose::action_warehouse.addActionBlock(action);
  }

  {
    action_params.set<std::string>("type") = "PenetrationAux";
    action_params.set<std::string>("name") = "AuxBCs/penetration_" + Moose::stringify(n);
    Action *action = ActionFactory::instance()->create("AddBCAction", action_params);
    MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    mooseAssert (moose_object_action, "Dynamic Cast failed");

    InputParameters & params = moose_object_action->getObjectParams();
    params.set<std::string>("variable") = PENETRATION_VAR_NAME;
    std::vector<unsigned int> bnds(1, getParam<unsigned int>("slave"));
    params.set<std::vector<unsigned int> >("boundary") = bnds;
    params.set<unsigned int>("paired_boundary") = getParam<unsigned int>("master");
    if (isParamValid("tangential_tolerance"))
    {
      params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");
    }
    // add it to the warehouse
    Moose::action_warehouse.addActionBlock(action);
  }
}

void
ThermalContactAction::addDiracKernels()
{
  /*
  [./thermal_master]
    type = GapHeatPointSourceMaster
    variable = temp
    boundary = 5
    slave = 10
  [../]
  */

  InputParameters action_params = ActionFactory::instance()->getValidParams("AddDiracKernelAction");
  action_params.set<Parser *>("parser_handle") = getParam<Parser *>("parser_handle");
  action_params.set<std::string>("type") = "GapHeatPointSourceMaster";
  action_params.set<std::string>("name") = "DiracKernels/thermal_master_" + Moose::stringify(n);
  Action *action = ActionFactory::instance()->create("AddDiracKernelAction", action_params);
  MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
  mooseAssert (moose_object_action, "Dynamic Cast failed");

  InputParameters & params = moose_object_action->getObjectParams();
  params.set<std::string>("variable") = getParam<std::string>("variable");
  params.set<unsigned int>("boundary") = getParam<unsigned int>("master");
  params.set<unsigned int>("slave") = getParam<unsigned int>("slave");
  if (isParamValid("tangential_tolerance"))
  {
    params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");
  }


  // add it to the warehouse
  Moose::action_warehouse.addActionBlock(action);
}

void
ThermalContactAction::addVectors()
{
  // We need only one instance of flux vector
  if (n == 0)
  {
    InputParameters action_params = validParams<AddSlaveFluxVectorAction>();
    action_params.set<Parser *>("parser_handle") = getParam<Parser *>("parser_handle");
    action_params.set<std::string>("name") = "add_slave_flux_vector";
    Action *action = ActionFactory::instance()->create("AddSlaveFluxVectorAction", action_params);
    // add it to the warehouse
    Moose::action_warehouse.addActionBlock(action);
  }
}

void
ThermalContactAction::act()
{
  addBcs();
  addAuxVariables();
  addAuxBcs();
  // if using NearestNodeValueAux, we do not need the dirac kernel
  if (getParam<std::string>("gap_type") != "NearestNodeValueAux")
    addDiracKernels();
  addVectors();
  n++;
}
