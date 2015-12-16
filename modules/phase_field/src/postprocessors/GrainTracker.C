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

#include <limits>
#include <algorithm>

template<> void dataStore(std::ostream & stream, MooseSharedPointer<GrainTracker::UniqueGrain> & unique_grain, void * context)
{
//  mooseAssert(unique_grain, "Unique Grain Pointer is NULL");
//
//  storeHelper(stream, static_cast<FeatureFloodCount::FeatureData>(*this), void * context);
//  storeHelper(stream, unique_grain->status, context);

  // We do not need to store the entities_ptrs structure. This information is not necessary for restart.
}

template<> void dataLoad(std::istream & stream, MooseSharedPointer<GrainTracker::UniqueGrain> & unique_grain, void * context)
{
//  FeatureData feature;
//
//  loadHelper(stream, feature, context);
//
//
//  unsigned int variable_idx;
//  GrainTracker::STATUS status;
//
//  loadHelper(stream, variable_idx, context);
//  loadHelper(stream, status, context);
//
//  // Load the Bounding Spheres
//  std::vector<GrainTracker::BoundingSphereInfo *> spheres;
//  loadHelper(stream, spheres, context);
//
//  unique_grain = new GrainTracker::UniqueGrain(variable_idx, spheres, NULL, status);
}

//template<> void dataStore(std::ostream & stream, GrainTracker::BoundingSphereInfo * & bound_sphere_info, void * context)
//{
//  mooseAssert(bound_sphere_info, "Sphere pointer is NULL");
//  storeHelper(stream, bound_sphere_info->member_node_id, context);
//  storeHelper(stream, bound_sphere_info->b_sphere.center(), context);
//  storeHelper(stream, bound_sphere_info->b_sphere.radius(), context);
//}
//
//template<> void dataLoad(std::istream & stream, GrainTracker::BoundingSphereInfo * & bound_sphere_info, void * context)
//{
//  unsigned int member_node_id;
//  Point center;
//  Real radius;
//
//  loadHelper(stream, member_node_id, context);
//  loadHelper(stream, center, context);
//  loadHelper(stream, radius, context);
//
//  bound_sphere_info = new GrainTracker::BoundingSphereInfo(member_node_id, center, radius);
//}

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
}

GrainTracker::~GrainTracker()
{
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

  FeatureFloodCount::finalize();

  Moose::perf_log.push("trackGrains()","GrainTracker");
  trackGrains();
  Moose::perf_log.pop("trackGrains()","GrainTracker");
//
//  Moose::perf_log.push("remapGrains()","GrainTracker");
//  if (_remap)
//    remapGrains();
//  Moose::perf_log.pop("remapGrains()","GrainTracker");

  updateFieldInfo();
//  Moose::perf_log.pop("finalize()","GrainTracker");

//  // Calculate and out output bubble volume data
//  if (_pars.isParamValid("bubble_volume_file"))
//  {
//    calculateBubbleVolumes();
//    std::vector<Real> data; data.reserve(_all_feature_volumes.size() + 2);
//    data.push_back(_fe_problem.timeStep());
//    data.push_back(_fe_problem.time());
//    data.insert(data.end(), _all_feature_volumes.begin(), _all_feature_volumes.end());
//    writeCSVFile(getParam<FileName>("bubble_volume_file"), data);
//  }

//  if (_compute_op_maps)
//  {
//    for (std::map<unsigned int, UniqueGrain *>::const_iterator grain_it = _unique_grains.begin();
//         grain_it != _unique_grains.end(); ++grain_it)
//    {
//      if (grain_it->second->status != INACTIVE)
//      {
//        std::set<dof_id_type>::const_iterator elem_it_end = grain_it->second->entities_ptr->end();
//        for (std::set<dof_id_type>::const_iterator elem_it = grain_it->second->entities_ptr->begin(); elem_it != elem_it_end; ++elem_it)
//        {
//          mooseAssert(!_ebsd_reader || _unique_grain_to_ebsd_num.find(grain_it->first) != _unique_grain_to_ebsd_num.end(), "Bad mapping in unique_grain_to_ebsd_num");
//          _elemental_data[*elem_it].push_back(std::make_pair(_ebsd_reader ? _unique_grain_to_ebsd_num[grain_it->first] : grain_it->first, grain_it->second->variable_idx));
//        }
//      }
//    }
//  }
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
//      Real radius = (max - center).size() + _hull_buffer;
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

    if (display_them)
    {
      for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
        for (unsigned int i = 0; i < _feature_sets[map_num].size(); ++i)
          std::cout << *_feature_sets[map_num][i] << '\n';
    }
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
//        center_points[gr] = d.p;
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
  std::cout << "Mapping time:\n";
  for (std::map<std::pair<unsigned int, unsigned int>, std::vector<unsigned int> >::iterator it = new_grain_idx_to_existing_grain_idx.begin();
       it != new_grain_idx_to_existing_grain_idx.end(); ++it)
  {
    std::cout << '(' << it->first.first << ',' << it->first.second << ") -> " << it->second.size() << '\n';


                                                                // map index     feature index
    MooseSharedPointer<FeatureData> feature_ptr = _feature_sets[it->first.first][it->first.second];

    // If there is only a single mapping - we've found the correct grain
    if (it->second.size() == 1)
    {
      unsigned int curr_idx = (it->second)[0];
      feature_ptr->_status = MARKED;                          // Mark it
      _unique_grains[curr_idx] = feature_ptr;                 // transfer ownership of new grain
    }

    // More than one existing grain is mapping to a new one
    else
    {
      Real min_centroid_diff = std::numeric_limits<Real>::max();
      unsigned int min_idx = 0;
      for (unsigned int i = 0; i < it->second.size(); ++i)
      {
        unsigned int curr_idx = (it->second)[i];
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
                     << _unique_grains[curr_idx]->_var_idx <<  ")\n";
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
                 << "\nCreating new unique grain: " << _unique_grains.size() << *_feature_sets[map_num][i]
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
               << it->second->_var_idx <<  ")\n";
      it->second->_status = INACTIVE;
    }

  // DEBUGGING
  if (processor_id() == 0)
  {
    std::ofstream outfile("bboxes.txt");
    for (std::map<unsigned int, MooseSharedPointer<FeatureData> >::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
    {
      if (it->second->_status == MARKED)
      {
        MooseSharedPointer<FeatureData> feature = it->second;
        for (unsigned int i = 0; i < feature->_bboxes.size(); ++i)
          outfile << it->first << ","
                  << feature->_bboxes[i].max()(0) << "," << feature->_bboxes[i].max()(1) << "," << feature->_bboxes[i].max()(2) << ","
                  << feature->_bboxes[i].min()(0) << "," << feature->_bboxes[i].min()(1) << "," << feature->_bboxes[i].min()(2) <<'\n';
      }
    }
    outfile.close();
  }
  // DEBUGGING


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

//  /**
//   * Loop over each grain and see if the bounding spheres of the current grain intersect with the spheres of any other grains
//   * represented by the same variable.
//   */
//  unsigned times_through_loop = 0;
//  bool variables_remapped;
//  do
//  {
//    Moose::out << "Remap Loop: " << times_through_loop << std::endl;
//
//    variables_remapped = false;
//    for (std::map<unsigned int, UniqueGrain *>::iterator grain_it1 = _unique_grains.begin();
//         grain_it1 != _unique_grains.end(); ++grain_it1)
//    {
//      if (grain_it1->second->status == INACTIVE)
//        continue;
//
//      for (std::map<unsigned int, UniqueGrain *>::iterator grain_it2 = _unique_grains.begin();
//           grain_it2 != _unique_grains.end(); ++grain_it2)
//      {
//        // Don't compare a grain with itself and don't try to remap inactive grains
//        if (grain_it1 == grain_it2 || grain_it2->second->status == INACTIVE)
//          continue;
//
//        if (grain_it1->second->variable_idx == grain_it2->second->variable_idx &&                  // Are the grains represented by the same variable?
//            boundingRegionDistance(grain_it1->second->sphere_ptrs, grain_it2->second->sphere_ptrs, false) < 0)  // If so, do their spheres intersect?
//        {
//          // If so, remap one of them
//          swapSolutionValues(grain_it1, grain_it2, times_through_loop);
//
//          // Since something was remapped, we need to inspect all the grains again to make sure that previously ok grains
//          // aren't in some new nearly intersecting state.  Setting this Boolean to true will trigger the loop again
//          variables_remapped = true;
//
//          // Since the current grain has just been remapped we don't want to loop over any more potential grains (the inner for loop)
//          break;
//        }
//      }
//    }
//
//    if (++times_through_loop >= 5 && processor_id() == 0)
//      mooseError(COLOR_RED << "Five passes through the remapping loop and grains are still being remapped, perhaps you need more op variables?" << COLOR_DEFAULT);
//
//  } while (variables_remapped);
//  Moose::out << "Done Remapping" << std::endl;
}

void
GrainTracker::swapSolutionValues(std::map<unsigned int, UniqueGrain *>::iterator & grain_it1,
                                 std::map<unsigned int, UniqueGrain *>::iterator & grain_it2,
                                 unsigned int attempt_number)
{
  NumericVector<Real> & solution         =  _nl.solution();
  NumericVector<Real> & solution_old     =  _nl.solutionOld();
  NumericVector<Real> & solution_older   =  _nl.solutionOlder();

//  unsigned int curr_var_idx = grain_it1->second->_var_idx;
//  /**
//   * We have two grains that are getting close represented by the same order parameter.
//   * We need to map to the variable whose closest grain to this one is furthest away by sphere to sphere distance.
//   */
//  std::vector<Real> min_distances(_vars.size(), std::numeric_limits<Real>::max());
//
//  // Make sure that we don't attempt to remap to the same variable
//  min_distances[curr_var_idx] = -std::numeric_limits<Real>::max();
//
//  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it3 = _unique_grains.begin();
//       grain_it3 != _unique_grains.end(); ++grain_it3)
//  {
//    if (grain_it3->second->status == INACTIVE || grain_it3->second->_var_idx == curr_var_idx)
//      continue;
//
//    unsigned int potential_var_idx = grain_it3->second->_var_idx;
//
//    Real curr_bounding_sphere_diff = boundingRegionDistance(grain_it1->second->sphere_ptrs, grain_it3->second->sphere_ptrs, false);
//    if (curr_bounding_sphere_diff < min_distances[potential_var_idx])
//      min_distances[potential_var_idx] = curr_bounding_sphere_diff;
//  }
//
//  /**
//   * We have a vector of the distances to the closest grains represented by each of our variables.  We just need to pick
//   * a suitable grain to replace with.  We will start with the maximum of this this list: (max of the mins), but will settle
//   * for next to largest and so forth as we make more attempts at remapping grains.  This is a graph coloring problem so
//   * more work will be required to optimize this process.
//   * Note: We don't have an explicit check here to avoid remapping a  variable to itself.  This is unecessary since the
//   * min_distance of a variable is explicitly set up above.
//   */
//  unsigned int nth_largest_idx = min_distances.size() - attempt_number - 1;
//
//  // nth element destroys the original array so we need to copy it first
//  std::vector<Real> min_distances_copy(min_distances);
//  std::nth_element(min_distances_copy.begin(), min_distances_copy.end()+nth_largest_idx, min_distances_copy.end());
//
//  // Now find the location of the nth element in the original vector
//  unsigned int new_variable_idx = std::distance(min_distances.begin(),
//                                                std::find(min_distances.begin(),
//                                                          min_distances.end(),
//                                                          min_distances_copy[nth_largest_idx]));
//
//  Moose::out
//    << COLOR_YELLOW
//    << "Grain #: " << grain_it1->first << " intersects Grain #: " << grain_it2->first
//    << " (variable index: " << grain_it1->second->variable_idx << ")\n"
//    << COLOR_DEFAULT;
//
//  if (min_distances[new_variable_idx] < 0)
//  {
//    Moose::out
//      << COLOR_YELLOW
//      << "*****************************************************************************************************\n"
//      << "Warning: No suitable variable found for remapping. Will attempt to remap in next loop if necessary...\n"
//      << "*****************************************************************************************************\n"
//      << COLOR_DEFAULT;
//    return;
//  }
//
//  Moose::out
//    << COLOR_GREEN
//    << "Remapping to: " << new_variable_idx << " whose closest grain is at a distance of " << min_distances[new_variable_idx] << "\n"
//    << COLOR_DEFAULT;
//
//  MeshBase & mesh = _mesh.getMesh();
//
//  // Remap the grain
//  std::set<Node *> updated_nodes_tmp; // Used only in the elemental case
//  for (std::set<dof_id_type>::const_iterator entity_it = grain_it1->second->entities_ptr->begin();
//       entity_it != grain_it1->second->entities_ptr->end(); ++entity_it)
//  {
//    if (_is_elemental)
//    {
//      Elem *elem = mesh.query_elem(*entity_it);
//      if (!elem)
//        continue;
//
//      for (unsigned int i=0; i < elem->n_nodes(); ++i)
//      {
//        Node *curr_node = elem->get_node(i);
//        if (updated_nodes_tmp.find(curr_node) == updated_nodes_tmp.end())
//        {
//          updated_nodes_tmp.insert(curr_node);         // cache this node so we don't attempt to remap it again within this loop
//          swapSolutionValuesHelper(curr_node, curr_var_idx, new_variable_idx, solution, solution_old, solution_older);
//        }
//      }
//    }
//    else
//      swapSolutionValuesHelper(mesh.query_node_ptr(*entity_it), curr_var_idx, new_variable_idx, solution, solution_old, solution_older);
//  }
//
//  // Update the variable index in the unique grain datastructure
//  grain_it1->second->variable_idx = new_variable_idx;
//
//  // Close all of the solution vectors
//  solution.close();
//  solution_old.close();
//  solution_older.close();
//
//  _fe_problem.getNonlinearSystem().sys().update();

}

void
GrainTracker::swapSolutionValuesHelper(Node * curr_node, unsigned int curr_var_idx, unsigned int new_var_idx, NumericVector<Real> & solution, NumericVector<Real> & solution_old, NumericVector<Real> & solution_older)
{
//  if (curr_node && curr_node->processor_id() == processor_id())
//  {
//    // Reinit the node so we can get and set values of the solution here
//    _subproblem.reinitNode(curr_node, 0);
//
//    // Swap the values from one variable to the other
//    {
//      VariableValue & value = _vars[curr_var_idx]->nodalSln();
//      VariableValue & value_old = _vars[curr_var_idx]->nodalSlnOld();
//      VariableValue & value_older = _vars[curr_var_idx]->nodalSlnOlder();
//
//      // Copy Value from intersecting variable to new variable
//      dof_id_type & dof_index = _vars[new_var_idx]->nodalDofIndex();
//
//      // Set the only DOF for this variable on this node
//      solution.set(dof_index, value[0]);
//      solution_old.set(dof_index, value_old[0]);
//      solution_older.set(dof_index, value_older[0]);
//    }
//    {
//      VariableValue & value = _vars[new_var_idx]->nodalSln();
//      VariableValue & value_old = _vars[new_var_idx]->nodalSlnOld();
//      VariableValue & value_older = _vars[new_var_idx]->nodalSlnOlder();
//
//      // Copy Value from variable to the intersecting variable
//      dof_id_type & dof_index = _vars[curr_var_idx]->nodalDofIndex();
//
//      // Set the only DOF for this variable on this node
//      solution.set(dof_index, value[0]);
//      solution_old.set(dof_index, value_old[0]);
//      solution_older.set(dof_index, value_older[0]);
//    }
//  }
}

void
GrainTracker::updateFieldInfo()
{
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    _feature_maps[map_num].clear();

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

    for (std::vector<MeshTools::BoundingBox>::const_iterator bbox_it2 = bboxes2.begin(); bbox_it2 != bboxes2.end(); ++bbox_it2)
    {
      const MeshTools::BoundingBox & bbox2 = *bbox_it2;
      const Point centroid2 = (bbox2.max() + bbox2.min()) / 2.0;

      Real curr_distance = std::numeric_limits<Real>::max();

      if (use_centroids_only)
        // Here we'll calculate a distance between the centroids
        curr_distance = _mesh.minPeriodicDistance(_var_number, centroid1, centroid2);
      else
        curr_distance = bbox1.intersect(bbox2) ? -1.0 : 1.0;

      if (curr_distance < min_distance)
        min_distance = curr_distance;
    }
  }

  return min_distance;
}

void
GrainTracker::calculateBubbleVolumes()
{
//  Moose::perf_log.push("calculateBubbleVolumes()", "GrainTracker");
//
//  // The size of the bubble array will be sized to the max index of the unique grains map
//  unsigned int max_id = _unique_grains.size() ? _unique_grains.rbegin()->first + 1: 0;
//  _all_feature_volumes.resize(max_id, 0);
//
//  const MeshBase::const_element_iterator el_end = _mesh.getMesh().active_local_elements_end();
//  for (MeshBase::const_element_iterator el = _mesh.getMesh().active_local_elements_begin(); el != el_end; ++el)
//  {
//    Elem * elem = *el;
//    unsigned int elem_n_nodes = elem->n_nodes();
//    Real curr_volume = elem->volume();
//
//    for (std::map<unsigned int, UniqueGrain *>::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
//    {
//      if (it->second->status == INACTIVE)
//        continue;
//
//      if (_is_elemental)
//      {
//        dof_id_type elem_id = elem->id();
//        if (it->second->entities_ptr->find(elem_id) != it->second->entities_ptr->end())
//        {
//          mooseAssert(it->first < _all_feature_volumes.size(), "_all_feature_volumes access out of bounds");
//          _all_feature_volumes[it->first] += curr_volume;
//          break;
//        }
//      }
//      else
//      {
//        // Count the number of nodes on this element which are flooded.
//        unsigned int flooded_nodes = 0;
//        for (unsigned int node = 0; node < elem_n_nodes; ++node)
//        {
//          dof_id_type node_id = elem->node(node);
//          if (it->second->entities_ptr->find(node_id) != it->second->entities_ptr->end())
//            ++flooded_nodes;
//        }
//
//        // If a majority of the nodes for this element are flooded,
//        // assign its volume to the current bubble_counter entry.
//        if (flooded_nodes >= elem_n_nodes / 2)
//          _all_feature_volumes[it->first] += curr_volume;
//      }
//    }
//  }
//
//  // do all the sums!
//  _communicator.sum(_all_feature_volumes);
//
//  Moose::perf_log.pop("calculateBubbleVolumes()", "GrainTracker");
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
