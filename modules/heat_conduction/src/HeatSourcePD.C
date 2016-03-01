/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HeatSourcePD.h"

#include "FEProblem.h"
#include "MooseMesh.h"
#include "Material.h"
#include "Function.h"
#include "Assembly.h"
using namespace std;

template<>
InputParameters validParams<HeatSourcePD>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");
  params.addParam<Real>("power_density", 1.0, "power_density");
  params.addParam<FunctionName>("power_density_function", "", "Function describing the volumetric heat source");
  params.addCoupledVar("total_bonds","Variable to contain total number of bonds connected to node");
  return params;
}

HeatSourcePD::HeatSourcePD(const InputParameters & parameters)
  :Kernel(parameters),
   _total_bonds_var(getVar("total_bonds",0)),
   _node_volume(getMaterialProperty<Real>("node_volume" + getParam<std::string>("appended_property_name"))),
   _power_density(isParamValid("power_density") ? getParam<Real>("power_density") : 0),
   _power_density_function(getParam<FunctionName>("power_density_function") != "" ? &getFunction("power_density_function") : NULL)
{
}

HeatSourcePD::~HeatSourcePD()
{
}

void
HeatSourcePD::computeResidual()
{
//get the total_bonds for each node
  AuxiliarySystem & aux = _fe_problem.getAuxiliarySystem();
  const NumericVector<Number> & sln = *aux.currentSolution();
  const Node* const node0 = _current_elem->get_node(0);
  const Node* const node1 = _current_elem->get_node(1);
  long int tb_dof0 = node0->dof_number(aux.number(), _total_bonds_var->number(), 0);
  long int tb_dof1 = node1->dof_number(aux.number(), _total_bonds_var->number(), 0);

  if(_power_density_function)
  {
    Point p;
    _power_density = _power_density_function->value(_t, p);
  }

  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  mooseAssert(re.size() == 2, "Truss elements must have two nodes");
  _local_re.resize(re.size());
  _local_re.zero();

  _local_re(0) = -_power_density * _node_volume[0] / sln(tb_dof0);
  _local_re(1) = -_power_density * _node_volume[0] / sln(tb_dof1);

  re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}
