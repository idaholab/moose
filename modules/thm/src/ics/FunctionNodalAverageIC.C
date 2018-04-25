#include "FunctionNodalAverageIC.h"

registerMooseObject("RELAP7App", FunctionNodalAverageIC);

template <>
InputParameters
validParams<FunctionNodalAverageIC>()
{
  InputParameters params = validParams<InitialCondition>();

  params.addClassDescription(
      "Initial conditions for an elemental variable from a function using nodal average.");

  params.addRequiredParam<FunctionName>("function", "The initial condition function.");

  return params;
}

FunctionNodalAverageIC::FunctionNodalAverageIC(const InputParameters & parameters)
  : InitialCondition(parameters), _func(getFunction("function"))
{
}

Real
FunctionNodalAverageIC::value(const Point & /*p*/)
{
  const unsigned int n_nodes = _current_elem->n_nodes();

  Real sum = 0.0;
  for (unsigned int i = 0; i < n_nodes; i++)
  {
    const Node & node = _current_elem->node_ref(i);
    sum += _func.value(_t, node);
  }

  return sum / n_nodes;
}
