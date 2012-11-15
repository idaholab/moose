#include "ThermalContactAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "AddSlaveFluxVectorAction.h"
#include "Conversion.h"
#include "MooseApp.h"

static unsigned int n = 0;                                  // numbering for gap heat transfer objects (we can have them on multiple interfaces)
static const std::string GAP_VALUE_VAR_NAME = "gap_value";
static const std::string PENETRATION_VAR_NAME = "penetration";

template<>
InputParameters validParams<ThermalContactAction>()
{
  MooseEnum orders("CONSTANT, FIRST, SECOND, THIRD, FOURTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::string>("type", "A string representing the Moose object that will be used for heat conduction over the gap");
  params.addParam<std::string>("gap_type", "GapValueAux", "A string representing the Moose object that will be used for computing the gap size");
  params.addRequiredParam<NonlinearVariableName>("variable", "The variable for thermal contact");
  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");
  params.addParam<NonlinearVariableName>("disp_x", "The x displacement");
  params.addParam<NonlinearVariableName>("disp_y", "The y displacement");
  params.addParam<NonlinearVariableName>("disp_z", "The z displacement");
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addParam<bool>("warnings", false, "Whether to output warning messages concerning nodes not being found");
  params.addParam<std::vector<std::string> >("save_in", "The Auxiliary Variable to (optionally) save the boundary flux in");
  
  params.addParam<bool>("quadrature", false, "Whether or not to use quadrature point based gap heat transfer");

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

  bool quadrature = getParam<bool>("quadrature");

  InputParameters action_params = ActionFactory::instance()->getValidParams("AddBCAction");
  action_params.set<ActionWarehouse *>("awh") = getParam<ActionWarehouse *>("awh");
  action_params.set<std::string>("type") = getParam<std::string>("type");
  Action *action = ActionFactory::instance()->create("AddBCAction", "BCs/gap_bc_" + Moose::stringify(n), action_params);
  MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
  mooseAssert (moose_object_action, "Dynamic Cast failed");

  InputParameters & params = moose_object_action->getObjectParams();

  // get valid params for the BC specified in 'type' field
  InputParameters bc_params = Factory::instance()->getValidParams(getParam<std::string>("type"));
  Moose::app->parser().extractParams(_name, bc_params);
  params += bc_params;

  if(isParamValid("save_in"))
  {
    params.set<std::vector<std::string> >("save_in") = getParam<std::vector<std::string> >("save_in");
  }

  params.set<NonlinearVariableName>("variable") = getParam<NonlinearVariableName>("variable");

  if(!quadrature)
  { 
    std::vector<AuxVariableName> vars(1);
    vars[0] = PENETRATION_VAR_NAME;
    params.set<std::vector<AuxVariableName> >("gap_distance") = vars;
    vars[0] = GAP_VALUE_VAR_NAME;
    params.set<std::vector<AuxVariableName> >("gap_temp") = vars;
  }
  else
  {
    params.set<bool>("quadrature") = true;
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");
    params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
    params.set<bool>("warnings") = getParam<bool>("warnings");
    params.set<bool>("use_displaced_mesh") = true;
  } 

  std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("slave"));
  params.set<std::vector<BoundaryName> >("boundary") = bnds;

  if (isParamValid("disp_x"))
  {
    params.addCoupledVar("disp_x", "The x displacement");
    std::vector<NonlinearVariableName> disp_x(1, getParam<NonlinearVariableName>("disp_x"));
    params.set< std::vector<NonlinearVariableName> >("disp_x") = disp_x;
  }
  if (isParamValid("disp_y"))
  {
    params.addCoupledVar("disp_y", "The y displacement");
    std::vector<NonlinearVariableName> disp_y(1, getParam<NonlinearVariableName>("disp_y"));
    params.set< std::vector<NonlinearVariableName> >("disp_y") = disp_y;
  }
  if (isParamValid("disp_z"))
  {
    params.addCoupledVar("disp_z", "The z displacement");
    std::vector<NonlinearVariableName> disp_z(1, getParam<NonlinearVariableName>("disp_z"));
    params.set< std::vector<NonlinearVariableName> >("disp_z") = disp_z;
  }

  // add it to the warehouse
  _awh.addActionBlock(action);

  // When we're doing quadrature based gap heat transfer we create two boundary conditions - one on each side
  // So here we're going to add the other one
  if(quadrature)
  {
    Action *second_action = ActionFactory::instance()->create("AddBCAction", "BCs/gap_bc_" + Moose::stringify(n+10000), action_params);
    MooseObjectAction *second_moose_object_action = dynamic_cast<MooseObjectAction *>(second_action);
    mooseAssert (second_moose_object_action, "Dynamic Cast failed");

    InputParameters & second_params = second_moose_object_action->getObjectParams();
    second_params = params; // Copy the params for the BC we just made    

    // Swap master and slave for this one
    std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("master"));
    second_params.set<std::vector<BoundaryName> >("boundary") = bnds;
    second_params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("slave");
    
    _awh.addActionBlock(second_action);
  }
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

  bool quadrature = getParam<bool>("quadrature");

  // We need to add the variables only once...
  if (n == 0)
  {
      InputParameters action_params = ActionFactory::instance()->getValidParams("AddVariableAction");

      action_params.set<ActionWarehouse *>("awh") = getParam<ActionWarehouse *>("awh");
      action_params.set<std::string>("action") = "add_aux_variable";
      action_params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
      
      if(quadrature)
      {
        action_params.set<MooseEnum>("order") = "CONSTANT";
        action_params.set<MooseEnum>("family") = "MONOMIAL";
      } 

      // gap_value
      Action *action = ActionFactory::instance()->create("AddVariableAction", "AuxVariables/" + GAP_VALUE_VAR_NAME, action_params);
      _awh.addActionBlock(action);
      // penetration
      action = ActionFactory::instance()->create("AddVariableAction", "AuxVariables/" + PENETRATION_VAR_NAME, action_params);
      _awh.addActionBlock(action);

      action_params = ActionFactory::instance()->getValidParams("CopyNodalVarsAction");
      action_params.set<ActionWarehouse *>("awh") = getParam<ActionWarehouse *>("awh");
      action_params.set<std::string>("action") = "copy_nodal_aux_vars";
      // gap_value
      action = ActionFactory::instance()->create("CopyNodalVarsAction", "AuxVariables/" + GAP_VALUE_VAR_NAME, action_params);
      _awh.addActionBlock(action);
      // penetration
      action = ActionFactory::instance()->create("CopyNodalVarsAction", "AuxVariables/" + PENETRATION_VAR_NAME, action_params);
      _awh.addActionBlock(action);
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
  action_params.set<ActionWarehouse *>("awh") = getParam<ActionWarehouse *>("awh");
  action_params.set<std::string>("action") = "add_aux_bc";

  {
    action_params.set<std::string>("type") = getParam<std::string>("gap_type");
    Action *action = ActionFactory::instance()->create("AddBCAction", "AuxBCs/gap_value_" + Moose::stringify(n), action_params);
    MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    mooseAssert (moose_object_action, "Dynamic Cast failed");

    InputParameters & params = moose_object_action->getObjectParams();
    params.set<AuxVariableName>("variable") = GAP_VALUE_VAR_NAME;
    std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("slave"));
    params.set<std::vector<BoundaryName> >("boundary") = bnds;
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");

    std::vector<VariableName> vars(1, getParam<NonlinearVariableName>("variable"));
    params.set<std::vector<VariableName> >("paired_variable") = vars;

    params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
    if (isParamValid("tangential_tolerance"))
    {
      params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");
    }
    params.set<bool>("warnings") = getParam<bool>("warnings");
    // add it to the warehouse
    _awh.addActionBlock(action);
  }

  std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("slave"));
  {
    action_params.set<std::string>("type") = "PenetrationAux";
    Action *action = ActionFactory::instance()->create("AddBCAction", "AuxBCs/penetration_" + Moose::stringify(n), action_params);
    MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    mooseAssert (moose_object_action, "Dynamic Cast failed");

    InputParameters & params = moose_object_action->getObjectParams();
    params.set<AuxVariableName>("variable") = PENETRATION_VAR_NAME;
    params.set<std::vector<BoundaryName> >("boundary") = bnds;
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");
    if (isParamValid("tangential_tolerance"))
    {
      params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");
    }
    // add it to the warehouse
    _awh.addActionBlock(action);
  }

}

void
ThermalContactAction::addMaterials()
{
  bool quadrature = getParam<bool>("quadrature");

  InputParameters action_params = ActionFactory::instance()->getValidParams("AddMaterialAction");
  action_params.set<ActionWarehouse *>("awh") = getParam<ActionWarehouse *>("awh");
  action_params.set<std::string>("action") = "add_material";

  {
    std::string type("GapConductance");
    if (getParam<std::string>("type") == "GapHeatTransferLWR")
    {
      type += "LWR";
    }
    action_params.set<std::string>("type") = type;
    Action *action = ActionFactory::instance()->create("AddMaterialAction", "Materials/gap_value_" + Moose::stringify(n), action_params);
    MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    mooseAssert (moose_object_action, "Dynamic Cast failed");

    InputParameters & params = moose_object_action->getObjectParams();

    // get valid params for the Material
    InputParameters material_params = Factory::instance()->getValidParams(type);
    Moose::app->parser().extractParams(_name, material_params);
    params += material_params;

    params.set<std::vector<VariableName> >("variable") = std::vector<VariableName>(1, getParam<NonlinearVariableName>("variable"));

    if(!quadrature)
    {  
      params.set<std::vector<AuxVariableName> >("gap_temp") = std::vector<AuxVariableName>(1, GAP_VALUE_VAR_NAME);
      std::vector<AuxVariableName> vars(1);
      vars[0] = PENETRATION_VAR_NAME;
      params.set<std::vector<AuxVariableName> >("gap_distance") = vars;
    }
    else
    {
      std::vector<VariableName> vars(1);
      vars[0] = getParam<NonlinearVariableName>("variable");
      params.set<std::vector<VariableName> >("temp") = vars;
      params.set<bool>("quadrature") = true;
      
      params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");

      params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
      params.set<bool>("warnings") = getParam<bool>("warnings");
    }

    std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("slave"));
    params.set<std::vector<BoundaryName> >("boundary") = bnds;

    // add it to the warehouse
    _awh.addActionBlock(action);

    // Since we have BCs on both sides when we're doing quadrature based transfer we need to add a material to the other side
    if(quadrature)
    {
      Action * second_action = ActionFactory::instance()->create("AddMaterialAction", "Materials/gap_value_" + Moose::stringify(n+10000), action_params);
      MooseObjectAction * second_moose_object_action = dynamic_cast<MooseObjectAction *>(second_action);
      mooseAssert (second_moose_object_action, "Dynamic Cast failed");

      InputParameters & second_params = second_moose_object_action->getObjectParams();
      second_params = params;

      second_params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("slave");
      
      std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("master"));
      second_params.set<std::vector<BoundaryName> >("boundary") = bnds;

      // add it to the warehouse
      _awh.addActionBlock(second_action);
    }
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
  action_params.set<ActionWarehouse *>("awh") = getParam<ActionWarehouse *>("awh");
  action_params.set<std::string>("type") = "GapHeatPointSourceMaster";
  Action *action = ActionFactory::instance()->create("AddDiracKernelAction", "DiracKernels/thermal_master_" + Moose::stringify(n), action_params);
  MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
  mooseAssert (moose_object_action, "Dynamic Cast failed");

  InputParameters & params = moose_object_action->getObjectParams();
  params.set<NonlinearVariableName>("variable") = getParam<NonlinearVariableName>("variable");
  params.set<BoundaryName>("boundary") = getParam<BoundaryName>("master");
  params.set<BoundaryName>("slave") = getParam<BoundaryName>("slave");
  if (isParamValid("tangential_tolerance"))
  {
    params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");
  }


  // add it to the warehouse
  _awh.addActionBlock(action);
}

void
ThermalContactAction::addVectors()
{
  // We need only one instance of flux vector
  if (n == 0)
  {
    InputParameters action_params = validParams<AddSlaveFluxVectorAction>();
    action_params.set<ActionWarehouse *>("awh") = getParam<ActionWarehouse *>("awh");
    Action *action = ActionFactory::instance()->create("AddSlaveFluxVectorAction", "add_slave_flux_vector", action_params);
    // add it to the warehouse
    _awh.addActionBlock(action);
  }
}

void
ThermalContactAction::act()
{
  addBcs();
  addAuxVariables();
  addAuxBcs();
  addMaterials();
  if (getParam<std::string>("gap_type") != "NearestNodeValueAux")
  {
    addDiracKernels();
  }
  addVectors();
  n++;
}
