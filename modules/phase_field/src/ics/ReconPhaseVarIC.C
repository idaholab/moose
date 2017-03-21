/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ReconPhaseVarIC.h"

template <>
InputParameters
validParams<ReconPhaseVarIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<UserObjectName>("ebsd_reader",
                                          "The EBSDReader object holding the EBSD data");
  params.addRequiredParam<unsigned int>("phase", "EBSD phase number this variable is to represent");
  return params;
}

ReconPhaseVarIC::ReconPhaseVarIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _mesh(_fe_problem.mesh()),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _phase(getParam<unsigned int>("phase")),
    _node_to_phase_weight_map(_ebsd_reader.getNodeToPhaseWeightMap())
{
}

Real
ReconPhaseVarIC::value(const Point & /*p*/)
{
  // Return error if current node is NULL
  if (_current_node == nullptr)
    mooseError("_current_node is reporting NULL");

  // Make sure the _current_node is in the _node_to_phase_weight_map (return error if not in map)
  std::map<dof_id_type, std::vector<Real>>::const_iterator it =
      _node_to_phase_weight_map.find(_current_node->id());
  if (it == _node_to_phase_weight_map.end())
    mooseError("The following node id is not in the node map: ", _current_node->id());

  // make sure we have enough ophase weights
  if (_phase >= it->second.size())
    mooseError("Requested an out-of-range phase number");

  return it->second[_phase];
}
