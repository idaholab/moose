/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "FauxGrainTracker.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<FauxGrainTracker>()
{
  InputParameters params = validParams<GrainTrackerInterface>();
  params.addClassDescription("Fake grain tracker object for cases where the number of grains is equal to the number of order parameters.");

  return params;
}


FauxGrainTracker::FauxGrainTracker(const InputParameters & parameters) :
    FeatureFloodCount(parameters),
    GrainTrackerInterface(),
    _tracking_step(getParam<int>("tracking_step"))
{
  // initialize faux data with identity map
  _faux_data.resize(_vars.size());
  for (unsigned int var_num = 0; var_num < _faux_data.size(); ++var_num)
    _faux_data[var_num] = std::make_pair(var_num, var_num);
}

FauxGrainTracker::~FauxGrainTracker()
{
}

Real
FauxGrainTracker::getEntityValue(dof_id_type entity_id, FeatureFloodCount::FieldType field_type, unsigned int var_idx) const
{
  auto use_default = false;
  if (var_idx == std::numeric_limits<unsigned int>::max())
  {
    use_default = true;
    var_idx = 0;
  }

  mooseAssert(var_idx < _vars.size(), "Index out of range");

  switch (field_type)
  {
    case FieldType::UNIQUE_REGION:
    case FieldType::VARIABLE_COLORING:
    {
      auto entity_it = _entity_id_to_var_num.find(entity_id);

      if (entity_it != _entity_id_to_var_num.end())
        return entity_it->second;
      else
        return 0;
      break;
    }

    // We don't want to error here because this should be a drop in replacement for the real grain tracker.
    // Instead we'll just return zero and continue
    default:
      return 0;
  }

  return 0;
}

const std::vector<std::pair<unsigned int, unsigned int> > &
FauxGrainTracker::getElementalValues(dof_id_type /*elem_id*/) const
{
  return _faux_data;
}

void
FauxGrainTracker::initialize()
{
  _entity_id_to_var_num.clear();
  _variables_used.clear();
}

void
FauxGrainTracker::execute()
{
  Moose::perf_log.push("execute()", "FauxGrainTracker");

  const MeshBase::element_iterator end = _mesh.getMesh().active_local_elements_end();
  for (MeshBase::element_iterator el = _mesh.getMesh().active_local_elements_begin(); el != end; ++el)
  {
    const Elem * current_elem = *el;

    // Loop over elements or nodes and populate the data structure with the first variable with a value above a threshold
    if (_is_elemental)
    {
      std::vector<Point> centroid(1, current_elem->centroid());
      _fe_problem.reinitElemPhys(current_elem, centroid, 0);

      for (unsigned int var_num = 0; var_num < _vars.size(); ++var_num)
      {
        Number entity_value = _vars[var_num]->sln()[0];

        if ((_use_less_than_threshold_comparison && (entity_value >= _threshold))
            || (!_use_less_than_threshold_comparison && (entity_value <= _threshold)))
        {
          _entity_id_to_var_num[current_elem->id()] = var_num;
          _variables_used.insert(var_num);
          break;
        }
      }
    }
    else
    {
      unsigned int n_nodes = current_elem->n_vertices();
      for (unsigned int i = 0; i < n_nodes; ++i)
      {
        const Node * current_node = current_elem->get_node(i);

        for (unsigned int var_num = 0; var_num < _vars.size(); ++var_num)
        {
          Number entity_value = _vars[var_num]->getNodalValue(*current_node);
          if ((_use_less_than_threshold_comparison && (entity_value >= _threshold))
              || (!_use_less_than_threshold_comparison && (entity_value <= _threshold)))
          {
            _entity_id_to_var_num[current_node->id()] = var_num;
            _variables_used.insert(var_num);
            break;
          }
        }
      }
    }
  }

  Moose::perf_log.pop("execute()", "FauxGrainTracker");
}

void
FauxGrainTracker::finalize()
{
  Moose::perf_log.push("finalize()", "FauxGrainTracker");

  _communicator.set_union(_variables_used, 0);
  _communicator.set_union(_entity_id_to_var_num, 0);

  Moose::perf_log.pop("finalize()", "FauxGrainTracker");
}

Real
FauxGrainTracker::getValue()
{
  return _variables_used.size();
}
