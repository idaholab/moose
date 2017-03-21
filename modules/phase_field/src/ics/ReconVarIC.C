/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ReconVarIC.h"
#include "EBSDReader.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<ReconVarIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<UserObjectName>("ebsd_reader", "The EBSDReader GeneralUserObject");
  params.addParam<unsigned int>("phase", "EBSD phase number to be assigned to this grain");
  params.addRequiredParam<unsigned int>("op_num",
                                        "Specifies the number of order parameters to create");
  params.addRequiredParam<unsigned int>("op_index", "The index for the current order parameter");
  params.addParam<bool>("all_op_elemental", false, "");
  params.addParam<bool>("advanced_op_assignment",
                        false,
                        "Enable advanced grain to op assignment (avoid invalid graph coloring)");
  return params;
}

ReconVarIC::ReconVarIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _mesh(_fe_problem.mesh()),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _consider_phase(isParamValid("phase")),
    _phase(_consider_phase ? getParam<unsigned int>("phase") : 0),
    _op_num(getParam<unsigned int>("op_num")),
    _op_index(getParam<unsigned int>("op_index")),
    _all_op_elemental(getParam<bool>("all_op_elemental")),
    _advanced_op_assignment(getParam<bool>("advanced_op_assignment")),
    _node_to_grain_weight_map(_ebsd_reader.getNodeToGrainWeightMap())
{
}

void
ReconVarIC::initialSetup()
{
  // number of grains this ICs deals with
  _grain_num = _consider_phase ? _ebsd_reader.getGrainNum(_phase) : _ebsd_reader.getGrainNum();

  // We do not want to have more order parameters than grains. That would leave some unused.
  if (_op_num > _grain_num)
    mooseError("ERROR in PolycrystalReducedIC: Number of order parameters (op_num) can't be larger "
               "than the number of grains (grain_num)");

  if (!_advanced_op_assignment)
  {
    // fetch all center points
    _centerpoints.resize(_grain_num);
    for (unsigned int index = 0; index < _grain_num; ++index)
      _centerpoints[index] = getCenterPoint(index);

    // Assign grains to each order parameter in a way that maximizes distance
    _assigned_op = PolycrystalICTools::assignPointsToVariables(_centerpoints, _op_num, _mesh, _var);
  }
  else
  {
    std::map<dof_id_type, unsigned int> entity_to_grain;

    const auto end = _mesh.getMesh().active_elements_end();
    for (auto el = _mesh.getMesh().active_elements_begin(); el != end; ++el)
    {
      Point centroid = (*el)->centroid();
      const EBSDAccessFunctors::EBSDPointData & d = _ebsd_reader.getData(centroid);
      const auto global_id = _ebsd_reader.getGlobalID(d._feature_id);
      const auto local_id = _ebsd_reader.getAvgData(global_id)._local_id;
      const auto index = _consider_phase ? local_id : global_id;

      entity_to_grain.insert(std::pair<dof_id_type, unsigned int>((*el)->id(), index));
    }

    auto grain_neighbor_graph =
        PolycrystalICTools::buildGrainAdjacencyGraph(entity_to_grain, _mesh, _grain_num, true);

    _assigned_op = PolycrystalICTools::assignOpsToGrains(grain_neighbor_graph, _grain_num, _op_num);
  }
}

Real
ReconVarIC::value(const Point & /*p*/)
{
  if (_all_op_elemental)
  {
    mooseAssert(_current_elem, "Current element is NULL");
    Point centroid = _current_elem->centroid();
    const EBSDAccessFunctors::EBSDPointData & d = _ebsd_reader.getData(centroid);
    const auto phase = d._phase;
    if (!_consider_phase || phase == _phase)
    {
      const unsigned int global_id = _ebsd_reader.getGlobalID(d._feature_id);
      const unsigned int local_id = _ebsd_reader.getAvgData(global_id)._local_id;
      auto index = _consider_phase ? local_id : global_id;

      mooseAssert(index < _assigned_op.size(), "EBSD Global ID is out of bounds");
      return _assigned_op[index];
    }
    return -1.0;
  }

  // Return error if current node is NULL
  if (_current_node == NULL)
    mooseError("The following node id is reporting a NULL condition: ", _current_node->id());

  // Make sure the _current_node is in the node_to_grain_weight_map (return error if not in map)
  std::map<dof_id_type, std::vector<Real>>::const_iterator it =
      _node_to_grain_weight_map.find(_current_node->id());

  if (it == _node_to_grain_weight_map.end())
    mooseError("The following node id is not in the node map: ", _current_node->id());

  // Increment through all grains at node_index (these are global IDs if consider_phase is false and
  // local IDs otherwise)
  for (unsigned int index = 0; index < _grain_num; ++index)
  {
    // If the current order parameter index (_op_index) is equal to the assigned index
    // (_assigned_op),
    // set the value from node_to_grain_weight_map
    Real value = (it->second)[_consider_phase ? _ebsd_reader.getGlobalID(_phase, index) : index];
    if (_assigned_op[index] == _op_index && value > 0.0)
      return value;
  }

  return 0.0;
}

Point
ReconVarIC::getCenterPoint(unsigned int index)
{
  if (_consider_phase)
    return _ebsd_reader.getAvgData(_phase, index)._p;
  else
    return _ebsd_reader.getAvgData(index)._p;
}
