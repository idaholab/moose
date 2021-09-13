//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FauxGrainTracker.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "Assembly.h"

registerMooseObject("PhaseFieldApp", FauxGrainTracker);

InputParameters
FauxGrainTracker::validParams()
{
  InputParameters params = GrainTrackerInterface::validParams();
  params.addClassDescription("Fake grain tracker object for cases where the number of grains is "
                             "equal to the number of order parameters.");

  return params;
}

FauxGrainTracker::FauxGrainTracker(const InputParameters & parameters)
  : FeatureFloodCount(parameters),
    GrainTrackerInterface(),
    _grain_count(0),
    _n_vars(_vars.size()),
    _tracking_step(getParam<int>("tracking_step"))
{
  // initialize faux data with identity map
  _op_to_grains.resize(_n_vars);
  for (MooseIndex(_op_to_grains) i = 0; i < _op_to_grains.size(); ++i)
    _op_to_grains[i] = i;

  _empty_var_to_features.resize(_n_vars, FeatureFloodCount::invalid_id);
}

FauxGrainTracker::~FauxGrainTracker() {}

Real
FauxGrainTracker::getEntityValue(dof_id_type entity_id,
                                 FeatureFloodCount::FieldType field_type,
                                 std::size_t var_idx) const
{
  if (var_idx == FeatureFloodCount::invalid_size_t)
    var_idx = 0;

  mooseAssert(var_idx < _n_vars, "Index out of range");

  switch (field_type)
  {
    case FieldType::UNIQUE_REGION:
    case FieldType::VARIABLE_COLORING:
    {
      auto entity_it = _entity_id_to_var_num.find(entity_id);

      if (entity_it != _entity_id_to_var_num.end())
        return entity_it->second;
      else
        return -1;
      break;
    }

    case FieldType::CENTROID:
    {
      if (_periodic_node_map.size())
        mooseDoOnce(mooseWarning(
            "Centroids are not correct when using periodic boundaries, contact the MOOSE team"));

      // If this element contains the centroid of one of features, return it's index
      const auto * elem_ptr = _mesh.elemPtr(entity_id);
      for (MooseIndex(_vars) var_num = 0; var_num < _n_vars; ++var_num)
      {
        const auto centroid = _centroid.find(var_num);
        if (centroid != _centroid.end())
          if (elem_ptr->contains_point(centroid->second))
            return 1;
      }

      return 0;
    }

    // We don't want to error here because this should be a drop in replacement for the real grain
    // tracker.
    // Instead we'll just return zero and continue
    default:
      return 0;
  }

  return 0;
}

const std::vector<unsigned int> &
FauxGrainTracker::getVarToFeatureVector(dof_id_type elem_id) const
{
  const auto pos = _entity_var_to_features.find(elem_id);
  if (pos != _entity_var_to_features.end())
  {
    mooseAssert(pos->second.size() == _n_vars, "Variable to feature vector not sized properly");
    return pos->second;
  }
  else
    return _empty_var_to_features;
}

unsigned int
FauxGrainTracker::getFeatureVar(unsigned int feature_id) const
{
  return feature_id;
}

std::size_t
FauxGrainTracker::getNumberActiveGrains() const
{
  return _variables_used.size();
}

std::size_t
FauxGrainTracker::getTotalFeatureCount() const
{
  return _grain_count;
}

Point
FauxGrainTracker::getGrainCentroid(unsigned int grain_index) const
{
  const auto grain_center = _centroid.find(grain_index);
  mooseAssert(grain_center != _centroid.end(),
              "Grain " << grain_index << " does not exist in data structure");

  return grain_center->second;
}

void
FauxGrainTracker::initialize()
{
  _entity_id_to_var_num.clear();
  _entity_var_to_features.clear();
  _variables_used.clear();
  if (_is_elemental)
  {
    _volume.clear();
    _vol_count.clear();
    _centroid.clear();
  }
}

void
FauxGrainTracker::execute()
{
  Moose::perf_log.push("execute()", "FauxGrainTracker");

  for (const auto & current_elem : _mesh.getMesh().active_local_element_ptr_range())
  {
    // Loop over elements or nodes and populate the data structure with the first variable with a
    // value above a threshold
    if (_is_elemental)
    {
      std::vector<Point> centroid(1, current_elem->vertex_average());
      _fe_problem.reinitElemPhys(current_elem, centroid, 0);

      auto entity = current_elem->id();
      auto insert_pair =
          moose_try_emplace(_entity_var_to_features,
                            entity,
                            std::vector<unsigned int>(_n_vars, FeatureFloodCount::invalid_id));
      auto & vec_ref = insert_pair.first->second;

      for (MooseIndex(_vars) var_num = 0; var_num < _n_vars; ++var_num)
      {
        auto entity_value = _vars[var_num]->sln()[0];

        if ((_use_less_than_threshold_comparison && (entity_value >= _threshold)) ||
            (!_use_less_than_threshold_comparison && (entity_value <= _threshold)))
        {
          _entity_id_to_var_num[current_elem->id()] = var_num;
          _variables_used.insert(var_num);
          _volume[var_num] += _assembly.elementVolume(current_elem);
          _vol_count[var_num]++;
          // Sum the centroid values for now, we'll average them later
          _centroid[var_num] += current_elem->vertex_average();
          vec_ref[var_num] = var_num;
          break;
        }
      }
    }
    else
    {
      unsigned int n_nodes = current_elem->n_vertices();
      for (unsigned int i = 0; i < n_nodes; ++i)
      {
        const Node * current_node = current_elem->node_ptr(i);

        for (MooseIndex(_vars) var_num = 0; var_num < _n_vars; ++var_num)
        {
          auto entity_value = _vars[var_num]->getNodalValue(*current_node);
          if ((_use_less_than_threshold_comparison && (entity_value >= _threshold)) ||
              (!_use_less_than_threshold_comparison && (entity_value <= _threshold)))
          {
            _entity_id_to_var_num[current_node->id()] = var_num;
            _variables_used.insert(var_num);
            break;
          }
        }
      }
    }
  }

  _grain_count = std::max(_grain_count, _variables_used.size());

  Moose::perf_log.pop("execute()", "FauxGrainTracker");
}

void
FauxGrainTracker::finalize()
{
  Moose::perf_log.push("finalize()", "FauxGrainTracker");

  _communicator.set_union(_variables_used);
  _communicator.set_union(_entity_id_to_var_num);

  if (_is_elemental)
    for (MooseIndex(_vars) var_num = 0; var_num < _n_vars; ++var_num)
    {
      /**
       * Convert elements of the maps into simple values or vector of Real.
       * libMesh's _communicator.sum() does not work on std::maps
       */
      unsigned int vol_count;
      std::vector<Real> grain_data(4);

      const auto count = _vol_count.find(var_num);
      if (count != _vol_count.end())
        vol_count = count->second;

      const auto vol = _volume.find(var_num);
      if (vol != _volume.end())
        grain_data[0] = vol->second;

      const auto centroid = _centroid.find(var_num);
      if (centroid != _centroid.end())
      {
        grain_data[1] = centroid->second(0);
        grain_data[2] = centroid->second(1);
        grain_data[3] = centroid->second(2);
      }
      // combine centers & volumes from all MPI ranks
      gatherSum(vol_count);
      gatherSum(grain_data);
      _volume[var_num] = grain_data[0];
      _centroid[var_num] = {grain_data[1], grain_data[2], grain_data[3]};
      _centroid[var_num] /= vol_count;
    }

  Moose::perf_log.pop("finalize()", "FauxGrainTracker");
}

Real
FauxGrainTracker::getValue()
{
  return static_cast<Real>(_variables_used.size());
}

bool
FauxGrainTracker::doesFeatureIntersectBoundary(unsigned int /*feature_id*/) const
{
  mooseDoOnce(mooseWarning("FauxGrainTracker::doesFeatureIntersectboundary() is unimplemented"));

  return false;
}
