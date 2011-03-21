#include "NearestNodeValueAux.h"

#include "MooseMesh.h"

template<>
InputParameters validParams<NearestNodeValueAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<unsigned int>("paired_boundary", "The boundary to get the value from.");
  params.addRequiredCoupledVar("paired_variable", "The variable to get the value of.");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

NearestNodeValueAux::NearestNodeValueAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _nearest_node(getNearestNodeLocator(parameters.get<unsigned int>("paired_boundary"), getParam<std::vector<unsigned int> >("boundary")[0])),
    _paired_variable(coupled("paired_variable"))
{
  if(getParam<std::vector<unsigned int> >("boundary").size() > 1)
    mooseError("NearestNodeValueAux can only be used with one boundary at a time!");

  // FIXME: serialized solution
//  _moose_system.needSerializedSolution(true);
}

void NearestNodeValueAux::setup()
{
}

Real
NearestNodeValueAux::computeValue()
{
  // Assumes the variable you are coupling to is from the nonlinear system for now.
  Node * nearest = _nearest_node.nearestNode(_current_node->id());
  long int dof_number = nearest->dof_number(0, _paired_variable, 0);

  // FIXME: serialized solution
//  return _moose_system._serialized_solution(dof_number);
  return 0;
}
