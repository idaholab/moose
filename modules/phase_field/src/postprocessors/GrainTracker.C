/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// MOOSE includes
#include "GrainTracker.h"
#include "MooseMesh.h"
#include "GeneratedMesh.h"
#include "EBSDReader.h"
#include "NonlinearSystem.h"

// LibMesh includes
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/sphere.h"

#include <queue>
#include <limits>
#include <algorithm>

/**
 * GrainDistance sort functor
 */
class GrainDistanceSorter
{
public:
  // Max of the mins
  bool operator()(const std::list<GrainTracker::GrainDistance> & lhs, const std::list<GrainTracker::GrainDistance> & rhs) const
    {
      if (lhs.empty())
      {
        if (rhs.empty())
          return true;
        else
          return false;
      }
      else if (rhs.empty())
        return true;
      else
        return lhs.begin()->_distance > rhs.begin()->_distance;
    }
};

template<>
InputParameters validParams<GrainTracker>()
{
  InputParameters params = validParams<FeatureFloodCount>();
  params += validParams<GrainTrackerInterface>();
  params.addClassDescription("Grain Tracker object for running reduced order parameter simulations without grain coalescence.");

  return params;
}


GrainTracker::GrainTracker(const InputParameters & parameters) :
    FeatureFloodCount(parameters),
    GrainTrackerInterface(),
    _tracking_step(getParam<int>("tracking_step")),
    _hull_buffer(getParam<Real>("convex_hull_buffer")),
    _halo_level(getParam<unsigned int>("halo_level")),
    _remap(getParam<bool>("remap_grains")),
    _nl(static_cast<FEProblem &>(_subproblem).getNonlinearSystem()),
    _unique_grains(declareRestartableData<std::map<unsigned int, MooseSharedPointer<FeatureData> > >("unique_grains")),
    _ebsd_reader(parameters.isParamValid("ebsd_reader") ? &getUserObject<EBSDReader>("ebsd_reader") : NULL),
    _compute_op_maps(getParam<bool>("compute_op_maps")),
    _center_mass_tracking(getParam<bool>("center_of_mass_tracking"))
{
  // Size the data structures to hold the correct number of maps
  _bounding_spheres.resize(_maps_size);

  if (!_is_elemental && _compute_op_maps)
    mooseError("\"compute_op_maps\" is only supported with \"flood_entity_type = ELEMENTAL\"");

  _outfile.open("bboxes.txt");
}

GrainTracker::~GrainTracker()
{
  _outfile.close();
//  for (std::map<unsigned int, UniqueGrain *>::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
//    delete it->second;
}

Real
GrainTracker::getEntityValue(dof_id_type node_id, FIELD_TYPE field_type, unsigned int var_idx) const
{
  if (_t_step < _tracking_step)
    return 0;

  return FeatureFloodCount::getEntityValue(node_id, field_type, var_idx);
}

Real
GrainTracker::getElementalValue(dof_id_type element_id) const
{
//  // If this element contains the centroid of on of the grains, return the unique index
//  const Elem * curr_elem = _mesh.elem(element_id);
//
//  for (std::map<unsigned int, UniqueGrain *>::const_iterator grain_it = _unique_grains.begin();
//       grain_it != _unique_grains.end(); ++grain_it)
//  {
//    if (grain_it->second->status == INACTIVE)
//      continue;
//
//    for (std::vector<BoundingSphereInfo *>::const_iterator it = grain_it->second->sphere_ptrs.begin();
//         it != grain_it->second->sphere_ptrs.end(); ++it)
//      if (curr_elem->contains_point((*it)->b_sphere.center()))
//      {
//        if (!_ebsd_reader)
//          return grain_it->first;
//
//        std::map<unsigned int, unsigned int>::const_iterator ebsd_it = _unique_grain_to_ebsd_num.find(grain_it->first);
//
//        mooseAssert(ebsd_it != _unique_grain_to_ebsd_num.end(), "Bad mapping in unique_grain_to_ebsd_num");
//        return ebsd_it->second;
//      }
//  }

  return 0;
}

void
GrainTracker::initialize()
{
  FeatureFloodCount::initialize();

  _elemental_data.clear();
}


void
GrainTracker::finalize()
{
  Moose::perf_log.push("finalize()","GrainTracker");

  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  expandHalos();

  FeatureFloodCount::communicateAndMerge();

  std::cout << "Finished inside of FeatureFloodCount" << std::endl;

  Moose::perf_log.push("trackGrains()","GrainTracker");
  trackGrains();
  Moose::perf_log.pop("trackGrains()","GrainTracker");

  std::cout << "Finished inside of trackGrains" << std::endl;

  // DEBUGGING
  if (processor_id() == 0)
  {
    _outfile << "Time: " << _t << '\n';
    for (std::map<unsigned int, MooseSharedPointer<FeatureData> >::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
    {
      if (it->second->_status == MARKED)
      {
        MooseSharedPointer<FeatureData> feature = it->second;
        for (unsigned int i = 0; i < feature->_bboxes.size(); ++i)
          _outfile << it->first << ","
                   << feature->_bboxes[i].min()(0) << "," << feature->_bboxes[i].max()(0) << ","
                   << feature->_bboxes[i].min()(1) << "," << feature->_bboxes[i].max()(1) << ","
                   << feature->_bboxes[i].min()(2) << "," << feature->_bboxes[i].max()(2) <<'\n';
      }
    }
    _outfile << '\n';
    _outfile.flush();
  }
  // DEBUGGING



  Moose::perf_log.push("remapGrains()","GrainTracker");
  if (_remap)
    remapGrains();
  Moose::perf_log.pop("remapGrains()","GrainTracker");

  updateFieldInfo();

  std::cout << "Finished inside of updateFieldInfo" << std::endl;


  Moose::perf_log.pop("finalize()","GrainTracker");

  // Calculate and out output bubble volume data
  if (_pars.isParamValid("bubble_volume_file"))
  {
    calculateBubbleVolumes();
    std::vector<Real> data; data.reserve(_all_feature_volumes.size() + 2);
    data.push_back(_fe_problem.timeStep());
    data.push_back(_fe_problem.time());
    data.insert(data.end(), _all_feature_volumes.begin(), _all_feature_volumes.end());
    writeCSVFile(getParam<FileName>("bubble_volume_file"), data);
  }

  if (_compute_op_maps)
  {
    for (std::map<unsigned int, MooseSharedPointer<FeatureData> >::const_iterator grain_it = _unique_grains.begin();
         grain_it != _unique_grains.end(); ++grain_it)
    {
      if (grain_it->second->_status != INACTIVE)
      {
        std::set<dof_id_type>::const_iterator elem_it_end = grain_it->second->_local_ids.end();
        for (std::set<dof_id_type>::const_iterator elem_it = grain_it->second->_local_ids.begin(); elem_it != elem_it_end; ++elem_it)
        {
          mooseAssert(!_ebsd_reader || _unique_grain_to_ebsd_num.find(grain_it->first) != _unique_grain_to_ebsd_num.end(), "Bad mapping in unique_grain_to_ebsd_num");
          _elemental_data[*elem_it].push_back(std::make_pair(_ebsd_reader ? _unique_grain_to_ebsd_num[grain_it->first] : grain_it->first, grain_it->second->_var_idx));
        }
      }
    }
  }
}

const std::vector<std::pair<unsigned int, unsigned int> > &
GrainTracker::getElementalValues(dof_id_type elem_id) const
{
  const std::map<unsigned int, std::vector<std::pair<unsigned int, unsigned int> > >::const_iterator pos = _elemental_data.find(elem_id);

  if (pos != _elemental_data.end())
    return pos->second;
  else
  {
#if DEBUG
    mooseDoOnce(Moose::out << "Elemental values not in structure for elem: " << elem_id << " this may be normal.");
#endif
    return _empty;
  }
}

void
GrainTracker::print()
{
//  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it = _unique_grains.begin();
//       grain_it != _unique_grains.end(); ++grain_it)
//  {
//    Moose::out << "Grain " << grain_it->first << ":\n";
//
//    for (std::vector<BoundingSphereInfo *>::iterator it2 = grain_it->second->sphere_ptrs.begin(); it2 != grain_it->second->sphere_ptrs.end(); ++it2)
//      Moose::out << "Centroid: " << (*it2)->b_sphere.center() << " radius: " << (*it2)->b_sphere.radius() << '\n';
//  }
}


void
GrainTracker::buildBoundingSpheres()
{
//  // Don't track grains if the current simulation step is before the specified tracking step
//  if (_t_step < _tracking_step)
//    return;
//
//  MeshBase & mesh = _mesh.getMesh();
//
//  unsigned long total_node_count = 0;
//  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
//  {
//    /**
//     * Create a pair of vectors of real values that is 3 (for the x,y,z components) times
//     * the length of the current _feature_sets length. Each processor will update the
//     * vector for the nodes that it owns (parallel_mesh case).  Then a parallel exchange
//     * will all for the global min/maxs of each bubble.
//     */
//    std::vector<Real> min_points(_feature_sets[map_num].size()*3,  std::numeric_limits<Real>::max());
//    std::vector<Real> max_points(_feature_sets[map_num].size()*3, -std::numeric_limits<Real>::max());
//
//    unsigned int set_counter = 0;
//    for (std::list<FeatureData>::const_iterator it1 = _feature_sets[map_num].begin();
//         it1 != _feature_sets[map_num].end(); ++it1)
//    {
//      total_node_count += it1->_ghosted_ids.size();
//
//      // Find the min/max of our bounding box to calculate our bounding sphere
//      for (std::set<dof_id_type>::iterator it2 = it1->_ghosted_ids.begin(); it2 != it1->_ghosted_ids.end(); ++it2)
//      {
//        Point point;
//        Point * p_ptr = NULL;
//        if (_is_elemental)
//        {
//          Elem *elem = mesh.query_elem(*it2);
//          if (elem)
//          {
//            point = elem->centroid();
//            p_ptr = &point;
//          }
//        }
//        else
//          p_ptr = mesh.query_node_ptr(*it2);
//        if (p_ptr)
//          for (unsigned int i = 0; i < mesh.spatial_dimension(); ++i)
//          {
//            min_points[set_counter*3+i] = std::min(min_points[set_counter*3+i], (*p_ptr)(i));
//            max_points[set_counter*3+i] = std::max(max_points[set_counter*3+i], (*p_ptr)(i));
//          }
//      }
//
//      ++set_counter;
//    }
//
//    _communicator.min(min_points);
//    _communicator.max(max_points);
//
//    set_counter = 0;
//    for (std::list<FeatureData>::const_iterator it1 = _feature_sets[map_num].begin();
//         it1 != _feature_sets[map_num].end(); ++it1)
//    {
//      Point min(min_points[set_counter*3], min_points[set_counter*3+1], min_points[set_counter*3+2]);
//      Point max(max_points[set_counter*3], max_points[set_counter*3+1], max_points[set_counter*3+2]);
//
//      // Calulate our bounding sphere
//      Point center(min + ((max - min) / 2.0));
//
//      // The radius is the different between the outer edge of the "bounding box"
//      // and the center plus the "hull buffer" value
//      Real radius = (max - center).norm() + _hull_buffer;
//
//      unsigned int some_node_id = *(it1->_ghosted_ids.begin());
//      _bounding_spheres[map_num].push_back(new BoundingSphereInfo(some_node_id, center, radius));
//
//      ++set_counter;
//    }
//  }
}

Point
GrainTracker::centerOfMass(UniqueGrain & grain) const
{
//  // NOTE: Does not work with Parallel Mesh yet
//
//  if (!_is_elemental)
//    mooseError("Not intended to work with Nodal Floods");
//
  Point center_of_mass;
//  for (std::set<dof_id_type>::const_iterator entity_it = grain.entities_ptr->begin(); entity_it != grain.entities_ptr->end(); ++entity_it)
//  {
//    Elem *elem = _mesh.elem(*entity_it);
//    if (!elem)
//      mooseError("Couldn't find element " << *entity_it << " in centerOfMass calculation.");
//
//    center_of_mass += elem->centroid();
//  }
//  center_of_mass /= static_cast<Real>(grain.entities_ptr->size());
//
  return center_of_mass;
}

void
GrainTracker::expandHalos()
{
  processor_id_type n_procs = _app.n_processors();

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    for (processor_id_type rank = 0; rank < n_procs; ++rank)
      for (std::vector<FeatureData>::iterator it = _partial_feature_sets[rank][map_num].begin();
           it != _partial_feature_sets[rank][map_num].end(); ++it)
      {
        FeatureData & feature = *it;

        for (unsigned int halo_level = 1; halo_level < _halo_level; ++halo_level)
        {
          /**
           * Create a copy of the halo set so that as we insert new ids into the
           * set we don't continue to iterate on those new ids.
           */
          std::set<dof_id_type> orig_halo_ids(feature._halo_ids);

          for (std::set<dof_id_type>::iterator entity_it = orig_halo_ids.begin();
               entity_it != orig_halo_ids.end(); ++entity_it)
          {
            if (_is_elemental)
              visitElementalNeighbors(_mesh.elem(*entity_it), feature._var_idx, &feature, /*recurse =*/false);
            else
              visitNodalNeighbors(_mesh.nodePtr(*entity_it), feature._var_idx, &feature, /*recurse =*/false);
          }
        }
      }
}

void
GrainTracker::trackGrains()
{
  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  // Reset Status on active unique grains
  std::vector<unsigned int> map_sizes(_maps_size);
  for (std::map<unsigned int, MooseSharedPointer<FeatureData> >::iterator grain_it = _unique_grains.begin();
       grain_it != _unique_grains.end(); ++grain_it)
  {
    if (grain_it->second->_status != INACTIVE)
    {
      grain_it->second->_status = NOT_MARKED;
      map_sizes[grain_it->second->_var_idx]++;
    }
  }

  // Print out info on the number of unique grains per variable vs the incoming bubble set sizes
  if (_t_step > _tracking_step)
  {
    bool display_them = false;
    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    {
      _console << "\nGrains active index " << map_num << ": " << map_sizes[map_num] << " -> " << _feature_sets[map_num].size();
      if (map_sizes[map_num] > _feature_sets[map_num].size())
      {
        _console << "--";
        display_them = true;
      }
      else if (map_sizes[map_num] < _feature_sets[map_num].size())
      {
        _console << "++";
        display_them = true;
      }
    }
    _console << std::endl;

//    if (display_them)
//    {
//      for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
//        for (unsigned int i = 0; i < _feature_sets[map_num].size(); ++i)
//          std::cout << *_feature_sets[map_num][i] << '\n';
//    }
  }

//  std::vector<UniqueGrain *> new_grains; new_grains.reserve(_unique_grains.size());
//
//  // Loop over all the current regions and build our unique grain structures
//  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
//  {
//    for (std::list<FeatureData>::const_iterator it1 = _feature_sets[map_num].begin();
//         it1 != _feature_sets[map_num].end(); ++it1)
//    {
//      std::vector<BoundingSphereInfo *> sphere_ptrs;
//      unsigned int curr_var = it1->_var_idx;
//
//      for (std::list<BoundingSphereInfo *>::iterator it2 = _bounding_spheres[map_num].begin();
//           it2 != _bounding_spheres[map_num].end(); /* No increment here! */)
//      {
//        /**
//         * See which of the bounding spheres belong to the current region (bubble set) by looking at a
//         * member node id.  A single region may have multiple bounding spheres as members if it spans
//         * periodic boundaries
//         */
//        if (it1->_ghosted_ids.find((*it2)->member_node_id) != it1->_ghosted_ids.end())
//        {
//          // Transfer ownership of the bounding sphere info to "sphere_ptrs" which will be stored in the unique grain
//          sphere_ptrs.push_back(*it2);
//          // Now delete the current BoundingSpherestructure so that it won't be inspected or reused
//          _bounding_spheres[map_num].erase(it2++);
//        }
//        else
//          ++it2;
//      }
//
//      // Create our new grains from this timestep that we will use to match up against the existing grains
//      new_grains.push_back(new UniqueGrain(curr_var, sphere_ptrs, &it1->_ghosted_ids, NOT_MARKED));
//    }
//  }

  /**
   * If it's the first time through this routine for the simulation, we will generate the unique grain
   * numbers using a simple counter.  These will be the unique grain numbers that we must track for
   * the remainder of the simulation.
   */
  if (_t_step == _tracking_step)   // Start tracking when the time_step == the tracking_step
  {
    if (_ebsd_reader)
    {
//      Real grain_num = _ebsd_reader->getGrainNum();
//
//      std::vector<Point> center_points(grain_num);
//
//      for (unsigned int gr=0; gr < grain_num; ++gr)
//      {
//        const EBSDReader::EBSDAvgData & d = _ebsd_reader->getAvgData(gr);
//        center_points[gr] = d._p;
//
//        Moose::out << "EBSD Grain " << gr << " " << center_points[gr] << '\n';
//      }
//
//      // To find the minimum distance we will use the boundingRegionDistance routine.
//      // To do that, we need to build BoundingSphereObjects with a few dummy values, radius and node_id will be ignored
//      BoundingSphereInfo ebsd_sphere(0, Point(0, 0, 0), 1);
//      std::vector<BoundingSphereInfo *> ebsd_vector(1);
//      ebsd_vector[0] = &ebsd_sphere;
//
//      std::set<unsigned int> used_indices;
//      std::map<unsigned int, unsigned int> error_indices;
//
//      if (grain_num != new_grains.size() && processor_id() == 0)
//        mooseWarning("Mismatch:\nEBSD centers: " << grain_num << " Grain Tracker Centers: " << new_grains.size());
//
//      unsigned int next_index = grain_num;
//      for (unsigned int i = 0; i < new_grains.size(); ++i)
//      {
//        Real min_centroid_diff = std::numeric_limits<Real>::max();
//        unsigned int closest_match_idx = 0;
//
//        //Point center_of_mass = centerOfMass(*new_grains[i]);
//
//        for (unsigned int j = 0; j<center_points.size(); ++j)
//        {
//          // Update the ebsd sphere to be used in the boundingRegionDistance calculation
//          ebsd_sphere.b_sphere.center() = center_points[j];
//
//          Real curr_centroid_diff = boundingRegionDistance(ebsd_vector, new_grains[i]->sphere_ptrs, true);
//          //Real curr_centroid_diff = (center_points[j] - center_of_mass).size();
//          if (curr_centroid_diff <= min_centroid_diff)
//          {
//            closest_match_idx = j;
//            min_centroid_diff = curr_centroid_diff;
//          }
//        }
//
//        if (used_indices.find(closest_match_idx) != used_indices.end())
//        {
//          Moose::out << "Re-assigning center " << closest_match_idx << " -> " << next_index << " "
//                     << center_points[closest_match_idx] << " absolute distance: " << min_centroid_diff << '\n';
//          _unique_grains[next_index] = new_grains[i];
//
//          _unique_grain_to_ebsd_num[next_index] = closest_match_idx;
//
//          ++next_index;
//        }
//        else
//        {
//          Moose::out << "Assigning center " << closest_match_idx << " "
//                     << center_points[closest_match_idx] << " absolute distance: " << min_centroid_diff << '\n';
//          _unique_grains[closest_match_idx] = new_grains[i];
//
//          _unique_grain_to_ebsd_num[closest_match_idx] = closest_match_idx;
//
//          used_indices.insert(closest_match_idx);
//        }
//      }
//      if (!error_indices.empty())
//      {
//        for (std::map<unsigned int, UniqueGrain *>::const_iterator grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
//          Moose::out << "Grain " << grain_it->first << ": " << center_points[grain_it->first] << '\n';
//
//        Moose::out << "Error Indices:\n";
//        for (std::map<unsigned int, unsigned int>::const_iterator it = error_indices.begin(); it != error_indices.end(); ++it)
//          Moose::out << "Grain " << it->first << '(' << it->second << ')' << ": " << center_points[it->second] << '\n';
//
//        mooseError("Error with ESBD Mapping (see above unused indices)");
//      }
//
//      print();
    }
    else
    {
      unsigned int counter = 0;
      for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
        for (unsigned int feature_num = 0; feature_num < _feature_sets[map_num].size(); ++feature_num)
        {
          MooseSharedPointer<FeatureData> feature_ptr = _feature_sets[map_num][feature_num];

          feature_ptr->_status = MARKED;
          _unique_grains[counter++] = feature_ptr;
        }
    }
    return;  // Return early - no matching or tracking to do
  }

  /**
   * To track grains across time steps, we will loop over our unique grains and link each one up with one of our new
   * unique grains.  The criteria for doing this will be to find the unique grain in the new list with a matching variable
   * index whose centroid is closest to this unique grain.
   */
  std::map<std::pair<unsigned int, unsigned int>, std::vector<unsigned int> > new_grain_idx_to_existing_grain_idx;

  for (std::map<unsigned int, MooseSharedPointer<FeatureData> >::iterator curr_it = _unique_grains.begin(); curr_it != _unique_grains.end(); ++curr_it)
  {
    if (curr_it->second->_status == INACTIVE)                         // Don't try to find matches for inactive grains
      continue;

    unsigned int closest_match_idx;
    bool found_one = false;
    Real min_centroid_diff = std::numeric_limits<Real>::max();

    // We only need to examine grains that have matching variable indices
    unsigned int map_idx = _single_map_mode ? 0 : curr_it->second->_var_idx;
    for (unsigned int new_grain_idx = 0; new_grain_idx < _feature_sets[map_idx].size(); ++new_grain_idx)
    {
      if (curr_it->second->_var_idx == _feature_sets[map_idx][new_grain_idx]->_var_idx)  // Do the variables indicies match?
      {
        Real curr_centroid_diff = boundingRegionDistance(curr_it->second->_bboxes, _feature_sets[map_idx][new_grain_idx]->_bboxes, true);
        if (curr_centroid_diff <= min_centroid_diff)
        {
          found_one = true;
          closest_match_idx = new_grain_idx;
          min_centroid_diff = curr_centroid_diff;
        }
      }
    }

    if (found_one)
      // Keep track of which new grains the existing ones want to map to
      new_grain_idx_to_existing_grain_idx[std::make_pair(map_idx, closest_match_idx)].push_back(curr_it->first);
  }

  /**
   * It's possible that multiple existing grains will map to a single new grain (indicated by multiplicity in the
   * new_grain_idx_to_existing_grain_idx data structure).  This will happen any time a grain disappears during
   * this time step. We need to figure out the rightful owner in this case and inactivate the old grain.
   */
//  std::cout << "Mapping time:\n";
  for (std::map<std::pair<unsigned int, unsigned int>, std::vector<unsigned int> >::iterator it = new_grain_idx_to_existing_grain_idx.begin();
       it != new_grain_idx_to_existing_grain_idx.end(); ++it)
  {
//    std::cout << '(' << it->first.first << ',' << it->first.second << ") -> " << it->second.size() << '\n';


                                                                // map index     feature index
    MooseSharedPointer<FeatureData> feature_ptr = _feature_sets[it->first.first][it->first.second];

    // If there is only a single mapping - we've found the correct grain
    if (it->second.size() == 1)
    {
      unsigned int curr_idx = (it->second)[0];
      feature_ptr->_status = MARKED;                          // Mark it
      _unique_grains[curr_idx] = feature_ptr;                 // transfer ownership of new grain
    }

    // More than one existing grain is mapping to a new one (i.e. multiple values exist for a single key)
    else
    {
//      std::cout << "New Grain:\n" << *feature_ptr;

      Real min_centroid_diff = std::numeric_limits<Real>::max();
      unsigned int min_idx = 0;

//      std::cout << "Existing Competing Grains:\n";
      for (unsigned int i = 0; i < it->second.size(); ++i)
      {
        unsigned int curr_idx = (it->second)[i];

//        std::cout << *_unique_grains[curr_idx];

        Real curr_centroid_diff = boundingRegionDistance(feature_ptr->_bboxes, _unique_grains[curr_idx]->_bboxes, true);
        if (curr_centroid_diff <= min_centroid_diff)
        {
          min_idx = i;
          min_centroid_diff = curr_centroid_diff;
        }
      }

      // One more time over the competing indices.  We will mark the non-winners as inactive and transfer ownership to the winner (the closest centroid).
      for (unsigned int i = 0; i < it->second.size(); ++i)
      {
        unsigned int curr_idx = (it->second)[i];
        if (i == min_idx)
        {
          feature_ptr->_status = MARKED;                          // Mark it
          _unique_grains[curr_idx] = feature_ptr;                 // transfer ownership of new grain
        }
        else
        {
          _console << "Marking Grain " << curr_idx << " as INACTIVE (variable index: "
                   << _unique_grains[curr_idx]->_var_idx << ")\n"
                   << *_unique_grains[curr_idx];
          _unique_grains[curr_idx]->_status = INACTIVE;
        }
      }
    }
  }

  /**
   * Next we need to look at our new list and see which grains weren't matched up.  These are new grains.
   */
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    for (unsigned int i = 0; i < _feature_sets[map_num].size(); ++i)
      if (_feature_sets[map_num][i]->_status == NOT_MARKED)
      {
        _console << COLOR_YELLOW
                 << "*****************************************************************************\n"
                 << "Couldn't find a matching grain while working on variable index: " << _feature_sets[map_num][i]->_var_idx
                 << "\nCreating new unique grain: " << _unique_grains.size() << '\n' <<  *_feature_sets[map_num][i]
                 << "\n*****************************************************************************\n" << COLOR_DEFAULT;
        _feature_sets[map_num][i]->_status = MARKED;
        _unique_grains[_unique_grains.size()] = _feature_sets[map_num][i];   // transfer ownership
      }

  /**
   * Finally we need to mark any grains in the unique list that aren't marked as inactive.  These are the variables that
   * unique grains that didn't match up to any bounding sphere.  Should only happen if it's the last active grain for
   * this particular variable.
   */
  for (std::map<unsigned int, MooseSharedPointer<FeatureData> >::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
    if (it->second->_status == NOT_MARKED)
    {
      _console << "Marking Grain " << it->first << " as INACTIVE (variable index: "
               << it->second->_var_idx <<  ")\n"
               << *it->second;
      it->second->_status = INACTIVE;
    }

////  // Sanity check to make sure that we consumed all of the bounding sphere datastructures
////  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
////    if (!_bounding_spheres[map_num].empty())
////      mooseError("BoundingSpheres where not completely used by the GrainTracker");
}

void
GrainTracker::remapGrains()
{
  // Don't remap grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  std::cout << "Running remap Grains" << std::endl;

  /**
   * Loop over each grain and see if any grains represented by the same variable are "touching"
   */
  bool grains_remapped;
  do
  {
    grains_remapped = false;
    for (std::map<unsigned int, MooseSharedPointer<FeatureData> >::iterator grain_it1 = _unique_grains.begin();
         grain_it1 != _unique_grains.end(); ++grain_it1)
    {
      if (grain_it1->second->_status == INACTIVE)
        continue;

      for (std::map<unsigned int, MooseSharedPointer<FeatureData> >::iterator grain_it2 = _unique_grains.begin();
           grain_it2 != _unique_grains.end(); ++grain_it2)
      {
        // Don't compare a grain with itself and don't try to remap inactive grains
        if (grain_it1 == grain_it2 || grain_it2->second->_status == INACTIVE)
          continue;

        if (grain_it1->second->_var_idx == grain_it2->second->_var_idx &&   // Are the grains represented by the same variable?
            grain_it1->second->isStichable(*grain_it2->second) &&           // If so, do their bboxes intersect (coarse level check)?
            setsIntersect(grain_it1->second->_halo_ids.begin(),             // If so, do they actually overlap (tight "hull" check)?
                          grain_it1->second->_halo_ids.end(),
                          grain_it2->second->_halo_ids.begin(),
                          grain_it2->second->_halo_ids.end()))
        {
          Moose::out
            << COLOR_YELLOW
            << "Grain #" << grain_it1->first << " intersects Grain #" << grain_it2->first
            << " (variable index: " << grain_it1->second->_var_idx << ")\n"
            << COLOR_DEFAULT;

          for (unsigned int max = 0; max <= _max_renumbering_recursion; ++max)
            if (max < _max_renumbering_recursion)
            {
              if (attemptGrainRenumber(grain_it1->second, grain_it1->first, 0, max) || attemptGrainRenumber(grain_it2->second, grain_it2->first, 0, max))
                break;
            }
            else if (!attemptGrainRenumber(grain_it1->second, grain_it1->first, 0, max) && !attemptGrainRenumber(grain_it2->second, grain_it2->first, 0, max))
              mooseError(COLOR_RED << "Unable to find any suitable grains for remapping. Perhaps you need more op variables?\n\n" << COLOR_DEFAULT);

          grains_remapped = true;
        }
      }
    }
  }
  while (grains_remapped);
}

void
GrainTracker::computeMinDistancesFromGrain(MooseSharedPointer<FeatureData> grain,
                                           std::vector<std::list<GrainDistance> > & min_distances)
{
  /**
   * TODO: I should be able to sort these even better. Negative distances could represent the number
   * of grains overlapping the current grain with the same index.
   *
   *  In the diagram below assume we have 4 order parameters. The grain with the asterisk needs to be
   *  remapped. All order parameters are used in neighboring grains. For all "touching" grains, the value
   *  of the corresponding entry in min_distances will be a negative integer representing the number of
   *  immediate neighbors with that order parameter.
   *  Note: Only the first member of the pair (the distance) is shown in the array below.
   *
   *  e.g. [-2.0, -max, -1.0, -2.0]
   *
   *  After sorting, variable index 2 (value: -1.0) be at the end of the array and will be the first variable
   *  we attempt to renumber the current grain to.
   *
   *        __       ___
   *          \  0  /   \
   *        2  \___/  1  \___
   *           /   \     /   \
   *        __/  1  \___/  2  \
   *          \  *  /   \     /
   *        3  \___/  3  \___/
   *           /   \     /
   *        __/  0  \___/
   *
   */
  for (std::map<unsigned int, MooseSharedPointer<FeatureData> >::iterator grain_it = _unique_grains.begin();
       grain_it != _unique_grains.end(); ++grain_it)
  {
    if (grain_it->second->_status == INACTIVE || grain_it->second->_var_idx == grain->_var_idx)
      continue;

    unsigned int target_var_index = grain_it->second->_var_idx;
    unsigned int target_grain_id = grain_it->first;

    Real curr_sphere_diff = boundingRegionDistance(grain->_bboxes, grain_it->second->_bboxes, false);

    GrainDistance grain_distance_obj(curr_sphere_diff, target_grain_id, target_var_index);

    // To handle touching halos we penalize the top pick each time we see another
    if (curr_sphere_diff == -1.0 && !min_distances[target_var_index].empty())
    {
      Real last_distance = min_distances[target_var_index].begin()->_distance;
      if (last_distance < 0)
        grain_distance_obj._distance += last_distance;
    }

    // Insertion sort into a list
    std::list<GrainDistance>::iterator insert_it = min_distances[target_var_index].begin();
    while (insert_it != min_distances[target_var_index].end() && !(grain_distance_obj < *insert_it))
      ++insert_it;
    min_distances[target_var_index].insert(insert_it, grain_distance_obj);
  }
}

bool
GrainTracker::attemptGrainRenumber(MooseSharedPointer<FeatureData> grain, unsigned int grain_id, unsigned int depth, unsigned int max)
{
  // End the recursion of our breadth first search
  if (depth > max)
    return false;

  unsigned int curr_var_idx = grain->_var_idx;

  std::vector<std::list<GrainDistance> > min_distances(_vars.size());

  /**
   * We have two grains that are getting close represented by the same order parameter.
   * We need to map to the variable whose closest grain to this one is furthest away by sphere to sphere distance.
   */
//  std::vector<std::pair<Real, unsigned int> > min_distances(_vars.size(),
//                                                            std::pair<Real, unsigned int>(std::numeric_limits<Real>::max(),
//                                                                                          std::numeric_limits<unsigned int>::max()));
  computeMinDistancesFromGrain(grain, min_distances);

  /**
   * We have a vector of the distances to the closest grains represented by each of our variables.  We just need to pick
   * a suitable grain to replace with.  We will start with the maximum of this this list: (max of the mins), but will settle
   * for next to largest and so forth as we make more attempts at remapping grains.  This is a graph coloring problem so
   * more work will be required to optimize this process.
   *
   * Note: We don't have an explicit check here to avoid remapping a  variable to itself.  This is unnecessary since the
   * min_distance of a variable is explicitly set up above.
   */

  std::sort(min_distances.begin(), min_distances.end(), GrainDistanceSorter()/*, pair_sorter_first<Real, unsigned int>()*/);

  std::cout << "\n********************************************\nDistances list for grain " << grain_id << '\n';
  for (unsigned int i = 0; i < min_distances.size(); ++i)
  {
    for (std::list<GrainDistance>::iterator min_it = min_distances[i].begin(); min_it != min_distances[i].end(); ++min_it)
      std::cout << min_it->_distance << ": " << min_it->_grain_id << ": " <<  min_it->_var_index << '\n';
    std::cout << '\n';
  }

  for (unsigned int i = 0; i < min_distances.size(); ++i)
  {
    std::list<GrainDistance>::const_iterator target_it = min_distances[i].begin();
    if (target_it == min_distances[i].end())
      continue;

      //     unsigned int target_grain_idx = min_it->second;

//    if (target_grain_idx == std::numeric_limits<unsigned int>::max())
//      mooseError("Error1");
//    if (_unique_grains.find(target_grain_idx) == _unique_grains.end())
//      mooseError("Error2");

//    mooseAssert(target_grain_idx != std::numeric_limits<unsigned int>::max(), "Error in finding target grain index in attemptGrainRenumber");
//    unsigned int target_var_idx = target_grain->_var_idx;
//    mooseAssert(curr_var_idx != target_var_idx, "Error in variable index in attemptGrainRenumber");

//    if (curr_var_idx == target_var_idx)
//      mooseError("Error3");

//      std::cout << "Depth " << depth << " Var idx: " << target_var_idx << '\n';

//    Real distance = min_it->first;

    // If the distance is positive we can just remap and be done
    if (target_it->_distance > 0)
    {
      Moose::out
        << COLOR_GREEN
        << "- Depth " << depth << ": Remapping grain #" << grain_id << " from variable index " << curr_var_idx
        << " to " << target_it->_var_index << " whose closest grain (#" << target_it->_grain_id
        << ") is at a distance of " << target_it->_distance << "\n"
        << COLOR_DEFAULT;

      swapSolutionValues(grain, target_it->_var_index, depth);
      return true;
    }

    // If the distance isn't positive we just need to make sure that none of the grains represented by the
    // target variable index would intersect this one if we were to remap
    std::list<GrainDistance>::const_iterator next_target_it = target_it;
    bool intersection_hit = false;
    std::ostringstream oss;
    while (!intersection_hit && next_target_it != min_distances[i].end())
    {
      if (next_target_it->_distance > 0)
        break;

      mooseAssert(_unique_grains.find(next_target_it->_grain_id) != _unique_grains.end(), "Error in indexing target grain in attemptGrainRenumber");
      MooseSharedPointer<FeatureData> next_target_grain = _unique_grains[next_target_it->_grain_id];

      // If any grains touch we're done here
      if (setsIntersect(grain->_halo_ids.begin(), grain->_halo_ids.end(),
                        next_target_grain->_halo_ids.begin(), next_target_grain->_halo_ids.end()))
        intersection_hit = true;
      else
        oss << " #" << next_target_it->_grain_id;

      ++next_target_it;
    }

    if (!intersection_hit)
    {
      Moose::out
        << COLOR_GREEN
        << "- Depth " << depth << ": Remapping grain #" << grain_id << " from variable index " << curr_var_idx
        << " to " << target_it->_var_index << " whose closest grain(s):" << oss.str()
        << " are inside our bounding sphere but whose halo(s) are not touching.\n"
        << COLOR_DEFAULT;

      swapSolutionValues(grain, target_it->_var_index, depth);
      return true;
    }

    // If we reach this part of the loop, there is no simple renumbering that can be done.
    // Propose a new variable index for the current grain and recurse

    mooseAssert(_unique_grains.find(target_it->_grain_id) != _unique_grains.end(), "Error in indexing target grain in attemptGrainRenumber");

    grain->_var_idx = target_it->_var_index;
    if (attemptGrainRenumber(_unique_grains[target_it->_grain_id], target_it->_grain_id, depth+1, max))
    {
      // Recompute distances (we can destroy this vector now because we are returning)
      for (unsigned int j = 0; j < min_distances.size(); ++j)
        min_distances[j].clear();

      computeMinDistancesFromGrain(grain, min_distances);

      // DO NOT SORT

      // Update the distance after the remapping
      Real distance = min_distances[grain->_var_idx].empty() ? std::numeric_limits<Real>::max() : min_distances[grain->_var_idx].begin()->_distance;

//      std::cout << "Target Var: " << target_it->_var_index << " Other: " << curr_var_idx << '\n';

      // TODO: This is mostly duplicated code. I can be removed with a more clever version of this algorithm
      if (distance > 0)
      {
        Moose::out
          << COLOR_GREEN
          << "- Depth " << depth << ": Remapping grain #" << grain_id << " from variable index " << curr_var_idx
          << " to " << grain->_var_idx << " whose closest grain (#" << min_distances[grain->_var_idx].begin()->_grain_id
          << ") is at a distance of " << distance << "\n"
          << COLOR_DEFAULT;

        swapSolutionValues(grain, grain->_var_idx, depth);
        return true;
      }

      // If the distance isn't positive we just need to make sure that none of the grains represented by the
      // target variable index would intersect this one if we were to remap
      std::list<GrainDistance>::const_iterator next_target_it = min_distances[grain->_var_idx].begin();
      bool intersection_hit = false;
      std::ostringstream oss;
      while (!intersection_hit && next_target_it != min_distances[grain->_var_idx].end())
      {
        if (next_target_it->_distance > 0)
          break;

        mooseAssert(_unique_grains.find(next_target_it->_grain_id) != _unique_grains.end(), "Error in indexing target grain in attemptGrainRenumber");
        MooseSharedPointer<FeatureData> next_target_grain = _unique_grains[next_target_it->_grain_id];

        // If any grains touch we're done here
        if (setsIntersect(grain->_halo_ids.begin(), grain->_halo_ids.end(),
                          next_target_grain->_halo_ids.begin(), next_target_grain->_halo_ids.end()))
          intersection_hit = true;
        else
          oss << " #" << next_target_it->_grain_id;

        ++next_target_it;
      }

      if (!intersection_hit)
      {
        Moose::out
          << COLOR_GREEN
          << "- Depth " << depth << ": Remapping grain #" << grain_id << " from variable index " << curr_var_idx
          << " to " << grain->_var_idx << " whose closest grain(s):" << oss.str()
          << " are inside our bounding sphere but whose halo(s) are not touching.\n"
          << COLOR_DEFAULT;

        swapSolutionValues(grain, grain->_var_idx, depth);
        return true;
      }

      mooseError("Not finished implementing");
    }
    // Need to set our var index back after failed recursive step
    grain->_var_idx = curr_var_idx;
  }

  // No luck so far, if we at a depth greater than zero there's still hope.
  return false;
}

void
GrainTracker::swapSolutionValues(MooseSharedPointer<FeatureData> grain, unsigned int var_idx, unsigned int depth)
{
  MeshBase & mesh = _mesh.getMesh();

  // Remap the grain
  std::set<Node *> updated_nodes_tmp; // Used only in the elemental case
  for (std::set<dof_id_type>::const_iterator entity_it = grain->_local_ids.begin();
       entity_it != grain->_local_ids.end(); ++entity_it)
  {
    if (_is_elemental)
    {
      Elem * elem = mesh.query_elem(*entity_it);
      if (!elem)
        continue;

      for (unsigned int i = 0; i < elem->n_nodes(); ++i)
      {
        Node * curr_node = elem->get_node(i);
        if (updated_nodes_tmp.find(curr_node) == updated_nodes_tmp.end())
        {
          updated_nodes_tmp.insert(curr_node);         // cache this node so we don't attempt to remap it again within this loop
          swapSolutionValuesHelper(curr_node, grain->_var_idx, var_idx);
        }
      }
    }
    else
      swapSolutionValuesHelper(mesh.query_node_ptr(*entity_it), grain->_var_idx, var_idx);
  }

  // Update the variable index in the unique grain datastructure
  grain->_var_idx = var_idx;

  // Close all of the solution vectors (we only need to do this once after all swaps are complete)
  if (depth == 0)
  {
    _nl.solution().close();
    _nl.solutionOld().close();
    _nl.solutionOlder().close();

    _fe_problem.getNonlinearSystem().sys().update();
  }
}

void
GrainTracker::swapSolutionValuesHelper(Node * curr_node, unsigned int curr_var_idx, unsigned int new_var_idx)
{
  if (curr_node && curr_node->processor_id() == processor_id())
  {
    // Reinit the node so we can get and set values of the solution here
    _subproblem.reinitNode(curr_node, 0);

    // Swap the values from one variable to the other
    {
      const VariableValue & value = _vars[curr_var_idx]->nodalSln();
      const VariableValue & value_old = _vars[curr_var_idx]->nodalSlnOld();
      const VariableValue & value_older = _vars[curr_var_idx]->nodalSlnOlder();

      // Copy Value from intersecting variable to new variable
      dof_id_type & dof_index = _vars[new_var_idx]->nodalDofIndex();

      // Set the only DOF for this variable on this node
      _nl.solution().set(dof_index, value[0]);
      _nl.solutionOld().set(dof_index, value_old[0]);
      _nl.solutionOlder().set(dof_index, value_older[0]);
    }
    {
      const VariableValue & value = _vars[new_var_idx]->nodalSln();
      const VariableValue & value_old = _vars[new_var_idx]->nodalSlnOld();
      const VariableValue & value_older = _vars[new_var_idx]->nodalSlnOlder();

      // Copy Value from variable to the intersecting variable
      dof_id_type & dof_index = _vars[curr_var_idx]->nodalDofIndex();

      // Set the only DOF for this variable on this node
      _nl.solution().set(dof_index, value[0]);
      _nl.solutionOld().set(dof_index, value_old[0]);
      _nl.solutionOlder().set(dof_index, value_older[0]);
    }
  }
}

void
GrainTracker::updateFieldInfo()
{
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    _feature_maps[map_num].clear();

  _halo_ids.clear();

  std::map<unsigned int, Real> tmp_map;
  MeshBase & mesh = _mesh.getMesh();

  for (std::map<unsigned int, MooseSharedPointer<FeatureData> >::iterator grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
  {
//    std::cout << *grain_it->second;

    unsigned int curr_var = grain_it->second->_var_idx;
    unsigned int map_idx = (_single_map_mode || _condense_map_info) ? 0 : curr_var;

    if (grain_it->second->_status == INACTIVE)
      continue;

    for (std::set<dof_id_type>::iterator entity_it = grain_it->second->_local_ids.begin();
         entity_it != grain_it->second->_local_ids.end(); ++entity_it)
    {
      // Highest variable value at this entity wins
      Real entity_value = -std::numeric_limits<Real>::max();
      if (_is_elemental)
      {
        const Elem * elem = mesh.elem(*entity_it);
        std::vector<Point> centroid(1, elem->centroid());
        _fe_problem.reinitElemPhys(elem, centroid, 0);
        entity_value = _vars[curr_var]->sln()[0];
      }
      else
      {
        Node & node = mesh.node(*entity_it);
        entity_value = _vars[curr_var]->getNodalValue(node);
      }

      if (tmp_map.find(*entity_it) == tmp_map.end() || entity_value > tmp_map[*entity_it])
      {
        // TODO: Add an option for EBSD Reader
        _feature_maps[map_idx][*entity_it] = _ebsd_reader ? _unique_grain_to_ebsd_num[grain_it->first] : grain_it->first;
        if (_var_index_mode)
          _var_index_maps[map_idx][*entity_it] = grain_it->second->_var_idx;

        tmp_map[*entity_it] = entity_value;
      }
    }
    for (std::set<dof_id_type>::const_iterator entity_it = grain_it->second->_halo_ids.begin();
         entity_it != grain_it->second->_halo_ids.end(); ++entity_it)
      _halo_ids[*entity_it] = grain_it->second->_var_idx;

    for (std::set<dof_id_type>::const_iterator entity_it = grain_it->second->_ghosted_ids.begin();
         entity_it != grain_it->second->_ghosted_ids.end(); ++entity_it)
      _ghosted_entity_ids[*entity_it] = grain_it->first;
  }
}

Real
GrainTracker::boundingRegionDistance(std::vector<MeshTools::BoundingBox> & bboxes1, std::vector<MeshTools::BoundingBox> bboxes2, bool use_centroids_only) const
{
  /**
   * The region that each grain covers is represented by a bounding box large enough to encompassing all the points
   * within that grain.  When using periodic boundaries, we may have several discrete "pieces" of a grain each represented
   * by a bounding sphere.  The distance between any two grains is defined as the minimum distance between any pair of spheres,
   * one selected from each grain.
   */
  Real min_distance = std::numeric_limits<Real>::max();
  for (std::vector<MeshTools::BoundingBox>::const_iterator bbox_it1 = bboxes1.begin(); bbox_it1 != bboxes1.end(); ++bbox_it1)
  {
    const MeshTools::BoundingBox & bbox1 = *bbox_it1;
    const Point centroid1 = (bbox1.max() + bbox1.min()) / 2.0;
    Sphere sphere1(centroid1, (bbox1.max() - centroid1).norm());


    for (std::vector<MeshTools::BoundingBox>::const_iterator bbox_it2 = bboxes2.begin(); bbox_it2 != bboxes2.end(); ++bbox_it2)
    {
      const MeshTools::BoundingBox & bbox2 = *bbox_it2;
      const Point centroid2 = (bbox2.max() + bbox2.min()) / 2.0;
      Sphere sphere2(centroid2, (bbox2.max() - centroid2).norm());

      Real curr_distance = std::numeric_limits<Real>::max();

      if (use_centroids_only)
        // Here we'll calculate a distance between the centroids
        curr_distance = _mesh.minPeriodicDistance(_var_number, centroid1, centroid2);
      else
      {
        // We need to compute the distance between the boxes...
        // That's pretty hard so let's do it between spheres instead.
        curr_distance = sphere1.distance(sphere2);
        if (curr_distance < 0.0)
          curr_distance = -1.0; // All overlaps are treated the same
      }

      if (curr_distance < min_distance)
        min_distance = curr_distance;
    }
  }

  return min_distance;
}

void
GrainTracker::calculateBubbleVolumes()
{
  Moose::perf_log.push("calculateBubbleVolumes()", "GrainTracker");

  // The size of the bubble array will be sized to the max index of the unique grains map
  unsigned int max_id = _unique_grains.size() ? _unique_grains.rbegin()->first + 1: 0;
  _all_feature_volumes.resize(max_id, 0);

  const MeshBase::const_element_iterator el_end = _mesh.getMesh().active_local_elements_end();
  for (MeshBase::const_element_iterator el = _mesh.getMesh().active_local_elements_begin(); el != el_end; ++el)
  {
    Elem * elem = *el;
    unsigned int elem_n_nodes = elem->n_nodes();
    Real curr_volume = elem->volume();

    for (std::map<unsigned int, MooseSharedPointer<FeatureData> >::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
    {
      if (it->second->_status == INACTIVE)
        continue;

      if (_is_elemental)
      {
        dof_id_type elem_id = elem->id();
        if (it->second->_local_ids.find(elem_id) != it->second->_local_ids.end())
        {
          mooseAssert(it->first < _all_feature_volumes.size(), "_all_feature_volumes access out of bounds");
          _all_feature_volumes[it->first] += curr_volume;
          break;
        }
      }
      else
      {
        // Count the number of nodes on this element which are flooded.
        unsigned int flooded_nodes = 0;
        for (unsigned int node = 0; node < elem_n_nodes; ++node)
        {
          dof_id_type node_id = elem->node(node);
          if (it->second->_local_ids.find(node_id) != it->second->_local_ids.end())
            ++flooded_nodes;
        }

        // If a majority of the nodes for this element are flooded,
        // assign its volume to the current bubble_counter entry.
        if (flooded_nodes >= elem_n_nodes / 2)
          _all_feature_volumes[it->first] += curr_volume;
      }
    }
  }

  // do all the sums!
  _communicator.sum(_all_feature_volumes);

  Moose::perf_log.pop("calculateBubbleVolumes()", "GrainTracker");
}

// BoundingSphereInfo
//GrainTracker::BoundingSphereInfo::BoundingSphereInfo(unsigned int node_id, const Point & center, Real radius) :
//    member_node_id(node_id),
//    b_sphere(center, radius)
//{}

// Unique Grain
//GrainTracker::UniqueGrain::UniqueGrain(unsigned int var_idx,
//                                       const std::vector<BoundingSphereInfo *> & b_sphere_ptrs,
//                                       const std::set<dof_id_type> *entities_pt,
//                                       STATUS status) :
//    variable_idx(var_idx),
//    sphere_ptrs(b_sphere_ptrs),
//    status(status),
//    entities_ptr(entities_pt)
//{}

//GrainTracker::UniqueGrain::~UniqueGrain()
//{
//  for (unsigned int i=0; i<sphere_ptrs.size(); ++i)
//    delete sphere_ptrs[i];
//}
