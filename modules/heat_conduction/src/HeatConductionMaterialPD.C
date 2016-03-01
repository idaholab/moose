/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "HeatConductionMaterialPD.h"
#include "Material.h"
#include "Function.h"
#include "NonlinearSystem.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<HeatConductionMaterialPD>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("appended_property_name","","Name appended to material properties to make them unique");
  params.addRequiredParam<NonlinearVariableName>("temp","Variable containing the temperature");
  params.addRequiredParam<int>("pddim", "Peridynamic dimension is required in heat conduction material block.");
  params.addParam<Real>("thermal_conductivity",0,"The thermal conductivity value");
  params.addParam<FunctionName>("thermal_conductivity_function","","Thermal conductivity as a function of temperature.");
  params.addParam<Real>("mesh_spacing",1,"Distance between to adjacent horizontal nodes.");
  params.addParam<Real>("domain_thickness",1,"The thickness of the domain for 2D problem.");

  return params;
}

HeatConductionMaterialPD::HeatConductionMaterialPD(const InputParameters & parameters) :
    Material(parameters),
    _my_thermal_conductivity(isParamValid("thermal_conductivity") ? getParam<Real>("thermal_conductivity") : 0),
    _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
    _thermal_conductivity_function(getParam<FunctionName>("thermal_conductivity_function") != "" ? &getFunction("thermal_conductivity_function") : NULL),
    
    _pddim(isParamValid("pddim") ? getParam<int>("pddim") : 3),
    _mesh_spacing(isParamValid("mesh_spacing") ? getParam<Real>("mesh_spacing") : 1.0),
    _bond_response(declareProperty<Real>("bond_response")),
    _bond_response_dif_temp(declareProperty<Real>("bond_response_dif_temp")),
    _node_volume(declareProperty<Real>("node_volume"))
{
  NonlinearVariableName temp = parameters.get<NonlinearVariableName>("temp");
  _temp_var = &_fe_problem.getVariable(_tid, temp);
}

HeatConductionMaterialPD::~HeatConductionMaterialPD()
{
}

void
HeatConductionMaterialPD::computeProperties()
{
//get the original length of each truss element
  Real origin_length = _current_elem->volume();

//obtain the temperature solution at the two nodes for each truss element
  const Node* const node0 = _current_elem->get_node(0);
  const Node* const node1 = _current_elem->get_node(1);
  NonlinearSystem & nonlinear_sys = _fe_problem.getNonlinearSystem();
  const NumericVector<Number>& ghosted_solution = *nonlinear_sys.currentSolution();
  unsigned int temp_dof0(node0->dof_number(nonlinear_sys.number(), _temp_var->number(), 0));
  unsigned int temp_dof1(node1->dof_number(nonlinear_sys.number(), _temp_var->number(), 0));
 
  Real temp_node0 = ghosted_solution(temp_dof0);
  Real temp_node1 = ghosted_solution(temp_dof1);

// the temperature of the connecting bond is calculated as the avarage of the temperature of two end nodes, this value will be used for temperature (and possibly spatial location) dependent thermal conductivity and specific heat calculation
  Real temp_avg = (temp_node0 + temp_node1) / 2.0;

  for(unsigned int _qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    if (_thermal_conductivity_function)
    {
      Point p;
      _thermal_conductivity[_qp] = _thermal_conductivity_function->value(temp_avg, p);
    }
    else
    {
      _thermal_conductivity[_qp] = _my_thermal_conductivity;
    }
 
    if (_pddim == 2)
    {
      _node_volume[_qp] = std::pow(_mesh_spacing, 2);
      _bond_response[_qp] = 6.0 * _thermal_conductivity[_qp] / (3.14159265358 * std::pow(3.0 * _mesh_spacing, 3)) * (temp_node1 - temp_node0) / origin_length * _node_volume[_qp] * _node_volume[_qp];
      _bond_response_dif_temp[_qp] = 6.0 * _thermal_conductivity[_qp] / (3.14159265358 * std::pow(3.0 * _mesh_spacing, 3)) / origin_length * _node_volume[_qp] * _node_volume[_qp];
    }
    else if (_pddim == 3)
    {
      _node_volume[_qp] = std::pow(_mesh_spacing, 3);
      _bond_response[_qp] = 6.0 * _thermal_conductivity[_qp] / (3.14159265358 * std::pow(3.0 * _mesh_spacing, 4)) * (temp_node1 - temp_node0) / origin_length * _node_volume[_qp] * _node_volume[_qp];
      _bond_response_dif_temp[_qp] = 6.0 * _thermal_conductivity[_qp] / (3.14159265358 * std::pow(3.0 * _mesh_spacing, 4)) / origin_length * _node_volume[_qp] * _node_volume[_qp];
    }
  }
}
