//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalEBSD.h"
#include "EBSDReader.h"

registerMooseObject("PhaseFieldApp", PolycrystalEBSD);

InputParameters
PolycrystalEBSD::validParams()
{
  InputParameters params = PolycrystalUserObjectBase::validParams();
  params.addClassDescription("Object for setting up a polycrystal structure from an EBSD Datafile");
  params.addParam<unsigned int>("phase", "The phase to use for all queries.");
  params.addParam<UserObjectName>("ebsd_reader", "EBSD Reader for initial condition");
  return params;
}

PolycrystalEBSD::PolycrystalEBSD(const InputParameters & parameters)
  : PolycrystalUserObjectBase(parameters),
    _phase(isParamValid("phase") ? getParam<unsigned int>("phase") : libMesh::invalid_uint),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _node_to_grain_weight_map(_ebsd_reader.getNodeToGrainWeightMap())
{
}

void
PolycrystalEBSD::getGrainsBasedOnPoint(const Point & point,
                                       std::vector<unsigned int> & grains) const
{
  const EBSDAccessFunctors::EBSDPointData & d = _ebsd_reader.getData(point);

  // See if we are in a phase that we are actually tracking
  if (_phase != libMesh::invalid_uint && _phase != d._phase)
  {
    grains.resize(0);
    return;
  }

  // Get the ids from the EBSD reader
  const auto global_id = _ebsd_reader.getGlobalID(d._feature_id);
  const auto local_id = _ebsd_reader.getAvgData(global_id)._local_id;

  grains.resize(1);
  grains[0] = _phase != libMesh::invalid_uint ? local_id : global_id;
}

unsigned int
PolycrystalEBSD::getNumGrains() const
{
  if (_phase != libMesh::invalid_uint)
    return _ebsd_reader.getGrainNum(_phase);
  else
    return _ebsd_reader.getGrainNum();
}

Real
PolycrystalEBSD::getNodalVariableValue(unsigned int op_index, const Node & n) const
{
  // Make sure the _current_node is in the node_to_grain_weight_map (return error if not in map)
  const auto it = _node_to_grain_weight_map.find(n.id());

  if (it == _node_to_grain_weight_map.end())
    mooseError("The following node id is not in the node map: ", n.id());

  // Increment through all grains at node_index (these are global IDs if consider_phase is false and
  // local IDs otherwise)
  const auto num_grains = getNumGrains();
  for (MooseIndex(num_grains) index = 0; index < num_grains; ++index)
  {
    // If the current order parameter index (_op_index) is equal to the assigned index
    // (_assigned_op),
    // set the value from node_to_grain_weight_map
    auto grain_index =
        _phase != libMesh::invalid_uint ? _ebsd_reader.getGlobalID(_phase, index) : index;
    mooseAssert(grain_index < it->second.size(), "grain_index out of range");
    auto value = (it->second)[grain_index];
    if (_grain_to_op.at(index) == op_index && value > 0.0)
      return value;
  }

  return 0.0;
}

Real
PolycrystalEBSD::getVariableValue(unsigned int op_index, const Point & p) const
{
  std::vector<unsigned int> grain_ids;
  getGrainsBasedOnPoint(p, grain_ids);

  if (grain_ids.empty())
    return -1.0;

  mooseAssert(grain_ids.size() == 1, "Expected only one grain at point in EBSDReader");
  auto index = grain_ids[0];

  return _grain_to_op.at(index) == op_index ? 1.0 : 0.0;
}
