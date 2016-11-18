/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ThermalContactAuxBCsAction.h"
#include "ThermalContactAuxVarsAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

static unsigned int n = 0;

template<>
InputParameters validParams<ThermalContactAuxBCsAction>()
{
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::string>("type", "A string representing the Moose object that will be used for heat conduction over the gap");
  params.addParam<std::string>("gap_aux_type", "GapValueAux", "A string representing the Moose object that will be used for computing the gap size");
  params.addRequiredParam<NonlinearVariableName>("variable", "The variable for thermal contact");
  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>("normal_smoothing_distance", "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method","Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addParam<bool>("warnings", false, "Whether to output warning messages concerning nodes not being found");
  params.addParam<bool>("quadrature", false, "Whether or not to use quadrature point based gap heat transfer");

  params.addParam<std::string>("conductivity_name", "thermal_conductivity", "The name of the MaterialProperty associated with conductivity "
                               "(\"thermal_conductivity\" in the case of heat conduction)");
  params.addParam<std::string>("conductivity_master_name", "thermal_conductivity", "The name of the MaterialProperty associated with conductivity "
                               "(\"thermal_conductivity\" in the case of heat conduction)");

  return params;
}

ThermalContactAuxBCsAction::ThermalContactAuxBCsAction( const InputParameters & params) :
  Action(params)
{
}

void
ThermalContactAuxBCsAction::act()
{
  bool quadrature = getParam<bool>("quadrature");

  InputParameters params = _factory.getValidParams(getParam<std::string>("gap_aux_type"));

  params.set<AuxVariableName>("variable") = ThermalContactAuxVarsAction::getGapValueName(_pars);

  std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("slave"));
  params.set<std::vector<BoundaryName> >("boundary") = bnds;
  params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");

  params.set<VariableName>("paired_variable") = getParam<NonlinearVariableName>("variable");

  params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
  if (isParamValid("tangential_tolerance"))
    params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");

  if (isParamValid("normal_smoothing_distance"))
    params.set<Real>("normal_smoothing_distance") = getParam<Real>("normal_smoothing_distance");

  if (isParamValid("normal_smoothing_method"))
    params.set<std::string>("normal_smoothing_method") = getParam<std::string>("normal_smoothing_method");

  params.set<bool>("warnings") = getParam<bool>("warnings");
  _problem->addAuxKernel(getParam<std::string>("gap_aux_type"), "gap_value_" + Moose::stringify(n), params);

  if (quadrature)
  {
    std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("master"));
    params.set<std::vector<BoundaryName> >("boundary") = bnds;
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("slave");
    _problem->addAuxKernel(getParam<std::string>("gap_aux_type"), "gap_value_master_" + Moose::stringify(n), params);

  }

  params = _factory.getValidParams("PenetrationAux");
  std::string penetration_var_name = "penetration";
  if (quadrature)
    penetration_var_name = "qpoint_penetration";

  params.set<AuxVariableName>("variable") = penetration_var_name;
  params.set<std::vector<BoundaryName> >("boundary") = bnds;
  params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");

  if (isParamValid("tangential_tolerance"))
    params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");

  if (isParamValid("normal_smoothing_distance"))
    params.set<Real>("normal_smoothing_distance") = getParam<Real>("normal_smoothing_distance");

  if (isParamValid("normal_smoothing_method"))
    params.set<std::string>("normal_smoothing_method") = getParam<std::string>("normal_smoothing_method");

  _problem->addAuxKernel("PenetrationAux", "penetration_" + Moose::stringify(n), params);

  ++n;
}
