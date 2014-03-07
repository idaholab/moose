#include "ThermalContactMaterialsAction.h"
#include "ThermalContactAuxVarsAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "Conversion.h"

static unsigned int n = 0;

template<>
InputParameters validParams<ThermalContactMaterialsAction>()
{
  MooseEnum orders("CONSTANT, FIRST, SECOND, THIRD, FOURTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");
  params.addRequiredParam<std::string>("type", "A string representing the Moose object that will be used for heat conduction over the gap");
  params.addRequiredParam<NonlinearVariableName>("variable", "The variable for thermal contact");
  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");
  params.addParam<Real>("gap_conductivity", 1.0, "The thermal conductivity of the gap material");
  params.addParam<FunctionName>("gap_conductivity_function", "Thermal conductivity of the gap material as a function.  Multiplied by gap_conductivity.");
  params.addParam<VariableName>("gap_conductivity_function_variable", "Variable to be used in gap_conductivity_function in place of time");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addParam<bool>("warnings", false, "Whether to output warning messages concerning nodes not being found");
  params.addParam<bool>("quadrature", false, "Whether or not to use quadrature point based gap heat transfer");
  params.addParam<VariableName>("contact_pressure", "The contact pressure variable");
  return params;
}

ThermalContactMaterialsAction::ThermalContactMaterialsAction( const std::string & name, InputParameters params ) :
  Action(name, params)
{
}

void
ThermalContactMaterialsAction::act()
{
  bool quadrature = getParam<bool>("quadrature");

  std::string type("GapConductance");
  if (getParam<std::string>("type") == "GapHeatTransferLWR")
  {
    type += "LWR";
  }

  InputParameters params = _factory.getValidParams(type);
  // Extract global params
  _app.parser().extractParams(_name, params);

  params.set<std::vector<VariableName> >("variable") = std::vector<VariableName>(1, getParam<NonlinearVariableName>("variable"));

  if(!quadrature)
  {
    params.set<std::vector<VariableName> >("gap_temp") = std::vector<VariableName>(1, ThermalContactAuxVarsAction::getGapValueName(_pars));
    std::vector<VariableName> vars(1);
    vars[0] = "penetration";
    params.set<std::vector<VariableName> >("gap_distance") = vars;
  }
  else
  {
    params.set<bool>("quadrature") = true;

    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");

    params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
  }

  params.set<bool>("warnings") = getParam<bool>("warnings");

  params.set<Real>("gap_conductivity") = getParam<Real>("gap_conductivity");
  if (isParamValid("gap_conductivity_function"))
  {
    params.set<FunctionName>("gap_conductivity_function") = getParam<FunctionName>("gap_conductivity_function");
  }
  if (isParamValid("gap_conductivity_function_variable"))
  {
    params.set<std::vector<VariableName> >("gap_conductivity_function_variable") = std::vector<VariableName>(1, getParam<VariableName>("gap_conductivity_function_variable"));
  }

  std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("slave"));
  params.set<std::vector<BoundaryName> >("boundary") = bnds;

  params.set<std::string>("appended_property_name") = getParam<std::string>("appended_property_name");

  if (getParam<std::string>("type") == "GapHeatTransferLWR")
  {
    std::vector<VariableName> v(1, ThermalContactAuxVarsAction::getGapConductivityName(_pars));
    params.set<std::vector<VariableName> >("gap_k") = v;
    if (isParamValid("contact_pressure"))
    {
      v[0] = getParam<VariableName>("contact_pressure");
      params.set<std::vector<VariableName> >("contact_pressure") = v;
    }
  }

  _problem->addMaterial(type,
    "gap_value_" + Moose::stringify(n),
    params);

  if (quadrature)
  {
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("slave");

    std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("master"));
    params.set<std::vector<BoundaryName> >("boundary") = bnds;
    if (getParam<std::string>("type") == "GapHeatTransferLWR")
    {
      params.set<bool>("slave_side") = false;
    }
    _problem->addMaterial(type,
      "gap_value_master_" + Moose::stringify(n),
      params);
  }

  ++n;
}
