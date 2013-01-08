#include "ThermalContactAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "AddSlaveFluxVectorAction.h"
#include "Conversion.h"
#include "MooseApp.h"

static unsigned int n = 0;                                  // numbering for gap heat transfer objects (we can have them on multiple interfaces)
static const std::string GAP_VALUE_VAR_BASE_NAME = "gap_";
static const std::string GAP_K_VALUE_VAR_BASE_NAME = "gap_k_";

template<>
InputParameters validParams<ThermalContactAction>()
{
  MooseEnum orders("CONSTANT, FIRST, SECOND, THIRD, FOURTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");
  params.addRequiredParam<std::string>("type", "A string representing the Moose object that will be used for heat conduction over the gap");
  params.addParam<std::string>("gap_type", "GapValueAux", "A string representing the Moose object that will be used for computing the gap size");
  params.addRequiredParam<NonlinearVariableName>("variable", "The variable for thermal contact");
  params.addParam<Real>("gap_conductivity", 1.0, "The thermal conductivity of the gap material");
  params.addParam<std::string>("conductivity_name", "thermal_conductivity", "The name of the MaterialProperty associated with conductivity "
                               "(\"thermal_conductivity\" in the case of heat conduction)");
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
    Action(name, params),
    _penetration_var_name("penetration"),
    _qpoint_penetration_var_name("qpoint_penetration")
{
}

std::string
ThermalContactAction::getGapValueName() const
{
  return GAP_VALUE_VAR_BASE_NAME + getParam<NonlinearVariableName>("variable");
}

std::string
ThermalContactAction::getGapConductivityName() const
{
  return GAP_K_VALUE_VAR_BASE_NAME + getParam<NonlinearVariableName>("variable");
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
    vars[0] = _penetration_var_name;
    params.set<std::vector<AuxVariableName> >("gap_distance") = vars;
    vars[0] = getGapValueName();
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

  params.set<std::string>("appended_property_name") = getParam<std::string>("appended_property_name");

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

  // We need to add variables only once per variable name.  However, we don't know how many unique variable
  //   names we will have.  So, we'll always add them.
  {
    InputParameters action_params = ActionFactory::instance()->getValidParams("AddVariableAction");

    action_params.set<ActionWarehouse *>("awh") = getParam<ActionWarehouse *>("awh");
    action_params.set<std::string>("action") = "add_aux_variable";
    action_params.set<MooseEnum>("order") = getParam<MooseEnum>("order");

    std::string penetration_var_name(_penetration_var_name);

    if(quadrature)
    {
      action_params.set<MooseEnum>("order") = "CONSTANT";
      action_params.set<MooseEnum>("family") = "MONOMIAL";
      penetration_var_name = _qpoint_penetration_var_name;
    }

    if (getParam<std::string>("type") == "GapHeatTransferLWR")
    {
      // Need to add gap_k variable to hold the conductivity across the gap
      InputParameters actionParams(action_params);

      Action * action = ActionFactory::instance()->create("AddVariableAction", "AuxVariables/" + getGapConductivityName(), actionParams);
      _awh.addActionBlock(action);

      // Now add constant conductivity variable
      actionParams.set<MooseEnum>("order") = "CONSTANT";
      actionParams.set<MooseEnum>("family") = "MONOMIAL";

      action = ActionFactory::instance()->create("AddVariableAction", "AuxVariables/conductivity_"+getParam<NonlinearVariableName>("variable"), actionParams);
      _awh.addActionBlock(action);
    }

    // gap_value
    Action *action = ActionFactory::instance()->create("AddVariableAction", "AuxVariables/" + getGapValueName(), action_params);
    _awh.addActionBlock(action);
    // penetration
    action = ActionFactory::instance()->create("AddVariableAction", "AuxVariables/" + penetration_var_name, action_params);
    _awh.addActionBlock(action);

    action_params = ActionFactory::instance()->getValidParams("CopyNodalVarsAction");
    action_params.set<ActionWarehouse *>("awh") = getParam<ActionWarehouse *>("awh");
    action_params.set<std::string>("action") = "copy_nodal_aux_vars";
    // gap_value
    action = ActionFactory::instance()->create("CopyNodalVarsAction", "AuxVariables/" + getGapValueName(), action_params);
    _awh.addActionBlock(action);
    // penetration
    action = ActionFactory::instance()->create("CopyNodalVarsAction", "AuxVariables/" + penetration_var_name, action_params);
    _awh.addActionBlock(action);
    if (getParam<std::string>("type") == "GapHeatTransferLWR")
    {
      // Need to add gap_k variable
      action = ActionFactory::instance()->create("CopyNodalVarsAction", "AuxVariables/" + getGapConductivityName(), action_params);
      _awh.addActionBlock(action);

      action = ActionFactory::instance()->create("CopyNodalVarsAction", "AuxVariables/conductivity_"+getParam<NonlinearVariableName>("variable"), action_params);
      _awh.addActionBlock(action);
    }

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
    params.set<AuxVariableName>("variable") = getGapValueName();
    std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("slave"));
    params.set<std::vector<BoundaryName> >("boundary") = bnds;
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");

    params.set<VariableName>("paired_variable") = getParam<NonlinearVariableName>("variable");

    params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
    if (isParamValid("tangential_tolerance"))
    {
      params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");
    }
    params.set<bool>("warnings") = getParam<bool>("warnings");
    // add it to the warehouse
    _awh.addActionBlock(action);

    if (getParam<bool>("quadrature"))
    {
      action_params.set<std::string>("type") = getParam<std::string>("gap_type");
      Action *action = ActionFactory::instance()->create("AddBCAction", "AuxBCs/gap_value_" + Moose::stringify(n+10000), action_params);
      MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
      mooseAssert (moose_object_action, "Dynamic Cast failed");

      InputParameters & params = moose_object_action->getObjectParams();
      params.set<AuxVariableName>("variable") = getGapValueName();
      std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("master"));
      params.set<std::vector<BoundaryName> >("boundary") = bnds;
      params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("slave");

      params.set<VariableName>("paired_variable") = getParam<NonlinearVariableName>("variable");

      params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
      if (isParamValid("tangential_tolerance"))
      {
        params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");
      }
      params.set<bool>("warnings") = getParam<bool>("warnings");
      // add it to the warehouse
      _awh.addActionBlock(action);
    }
  }

  std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("slave"));
  {
    action_params.set<std::string>("type") = "PenetrationAux";
    Action *action = ActionFactory::instance()->create("AddBCAction", "AuxBCs/penetration_" + Moose::stringify(n), action_params);
    MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    mooseAssert (moose_object_action, "Dynamic Cast failed");

    InputParameters & params = moose_object_action->getObjectParams();

    std::string penetration_var_name(_penetration_var_name);
    if (getParam<bool>("quadrature"))
    {
      penetration_var_name = _qpoint_penetration_var_name;
    }
    params.set<AuxVariableName>("variable") = penetration_var_name;
    params.set<std::vector<BoundaryName> >("boundary") = bnds;
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");
    if (isParamValid("tangential_tolerance"))
    {
      params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");
    }
    // add it to the warehouse
    _awh.addActionBlock(action);
  }

  if (getParam<std::string>("type") == "GapHeatTransferLWR")
  {
    // Need to add gap_k variable
    action_params.set<std::string>("type") = getParam<std::string>("gap_type");
    Action *action = ActionFactory::instance()->create("AddBCAction", "AuxBCs/gap_k_" + Moose::stringify(n), action_params);
    MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    mooseAssert (moose_object_action, "Dynamic Cast failed");

    InputParameters & params = moose_object_action->getObjectParams();
    params.set<AuxVariableName>("variable") = getGapConductivityName();
    params.set<std::vector<BoundaryName> >("boundary") = bnds;
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");

    params.set<VariableName>("paired_variable") = "conductivity_"+getParam<NonlinearVariableName>("variable");

    params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
    if (isParamValid("tangential_tolerance"))
    {
      params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");
    }

    // For efficiency, run this at the beginning of each step...
    params.set<MooseEnum>("execute_on") = "timestep_begin";

    params.set<bool>("warnings") = getParam<bool>("warnings");
    // add it to the warehouse
    _awh.addActionBlock(action);


    // Now for the other direction
    action = ActionFactory::instance()->create("AddBCAction", "AuxBCs/gap_k_" + Moose::stringify(n+10000), action_params);
    moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    mooseAssert (moose_object_action, "Dynamic Cast failed");

    {
      InputParameters & params = moose_object_action->getObjectParams();
      params.set<AuxVariableName>("variable") = getGapConductivityName();
      std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("master"));
      params.set<std::vector<BoundaryName> >("boundary") = bnds;
      params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("slave");

      params.set<VariableName>("paired_variable") = "conductivity_"+getParam<NonlinearVariableName>("variable");

      params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
      if (isParamValid("tangential_tolerance"))
      {
        params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");
      }

      // For efficiency, run this at the beginning of each step...
      params.set<MooseEnum>("execute_on") = "timestep_begin";

      params.set<bool>("warnings") = getParam<bool>("warnings");
      // add it to the warehouse
      _awh.addActionBlock(action);
    }
  }
}

void
ThermalContactAction::addAuxKernels()
{
  /*
  [./conductivity]
    type = MaterialRealAux
    variable = conductivity
    property = thermal_conductivity
  [../]
  */

  if (getParam<std::string>("type") == "GapHeatTransferLWR")
  {
    InputParameters action_params = ActionFactory::instance()->getValidParams("AddKernelAction");
    action_params.set<ActionWarehouse *>("awh") = getParam<ActionWarehouse *>("awh");
    action_params.set<std::string>("action") = "add_aux_kernel";

    action_params.set<std::string>("type") = "MaterialRealAux";
    Action * action = ActionFactory::instance()->create("AddKernelAction", "AuxKernels/conductivity", action_params);
    MooseObjectAction * moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    mooseAssert (moose_object_action, "Dynamic Cast failed");

    InputParameters & params = moose_object_action->getObjectParams();
    params.set<AuxVariableName>("variable") = "conductivity_"+getParam<NonlinearVariableName>("variable");
    params.set<std::string>("property") = getParam<std::string>("conductivity_name");
    // For efficiency, run this at the end of each step...
    params.set<MooseEnum>("execute_on") = "timestep";
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
      params.set<std::vector<AuxVariableName> >("gap_temp") = std::vector<AuxVariableName>(1, getGapValueName());
      std::vector<AuxVariableName> vars(1);
      vars[0] = _penetration_var_name;
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

    params.set<Real>("gap_conductivity") = getParam<Real>("gap_conductivity");

    std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("slave"));
    params.set<std::vector<BoundaryName> >("boundary") = bnds;

    params.set<std::string>("appended_property_name") = getParam<std::string>("appended_property_name");

    if (getParam<std::string>("type") == "GapHeatTransferLWR")
    {
      std::vector<VariableName> v(1, getGapConductivityName());
      params.set<std::vector<VariableName> >("gap_k") = v;
    }

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

      second_params.set<Real>("gap_conductivity") = getParam<Real>("gap_conductivity");

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
  addAuxKernels();
  addMaterials();
  if (getParam<std::string>("gap_type") != "NearestNodeValueAux")
  {
    addDiracKernels();
  }
  addVectors();
  n++;
}
