#include "GrainTracker.h"
#include "MooseMesh.h"
#include "AddV.h"
#include "GeneratedMesh.h"
#include "EBSDReader.h"

// LibMesh includes
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/sphere.h"

#include <limits>
#include <algorithm>

template<> void dataStore(std::ostream & stream, GrainTracker::UniqueGrain * & unique_grain, void * context)
{
  mooseAssert(unique_grain, "Unique Grain Pointer is NULL");

  storeHelper(stream, unique_grain->variable_idx, context);
  storeHelper(stream, unique_grain->status, context);
  storeHelper(stream, unique_grain->sphere_ptrs, context);

  // We do not need to store the nodes_ptrs structure. This information is not necessary for restart.
}

template<> void dataLoad(std::istream & stream, GrainTracker::UniqueGrain * & unique_grain, void * context)
{
  unsigned int variable_idx;
  GrainTracker::STATUS status;

  loadHelper(stream, variable_idx, context);
  loadHelper(stream, status, context);

  // Load the Bounding Spheres
  std::vector<GrainTracker::BoundingSphereInfo *> spheres;
  loadHelper(stream, spheres, context);

  unique_grain = new GrainTracker::UniqueGrain(variable_idx, spheres, NULL, status);
}

template<> void dataStore(std::ostream & stream, GrainTracker::BoundingSphereInfo * & bound_sphere_info, void * context)
{
  mooseAssert(bound_sphere_info, "Sphere pointer is NULL");
  storeHelper(stream, bound_sphere_info->member_node_id, context);
  storeHelper(stream, bound_sphere_info->b_sphere.center(), context);
  storeHelper(stream, bound_sphere_info->b_sphere.radius(), context);
}

template<> void dataLoad(std::istream & stream, GrainTracker::BoundingSphereInfo * & bound_sphere_info, void * context)
{
  unsigned int member_node_id;
  Point center;
  Real radius;

  loadHelper(stream, member_node_id, context);
  loadHelper(stream, center, context);
  loadHelper(stream, radius, context);

  bound_sphere_info = new GrainTracker::BoundingSphereInfo(member_node_id, center, radius);
}

template<>
InputParameters validParams<GrainTracker>()
{
  InputParameters params = validParams<NodalFloodCount>();
  params.addRequiredParam<unsigned int>("op_num","number of grains");
  params.addRequiredParam<std::string>("var_name_base","base for variable names");
  params.addParam<int>("tracking_step", 1, "The timestep for when we should start tracking grains");
  params.addParam<Real>("convex_hull_buffer", 1.0, "The buffer around the convex hull used to determine"
                                                   "when features intersect");
  params.addParam<bool>("remap_grains", true, "Indicates whether remapping should be done or not (default: true)");
  params.addParam<bool>("compute_op_maps", false, "Indicates whether the data structures that"
                                                  "hold the active order parameter information"
                                                  "should be populated or not");
  params.addParam<UserObjectName>("ebsd_reader", "Optional: EBSD Reader for initial condition");

  // We are using "addV" to add the variable parameter on the fly
  params.suppressParameter<std::vector<VariableName> >("variable");

  return params;
}

GrainTracker::GrainTracker(const std::string & name, InputParameters parameters) :
    NodalFloodCount(name, AddV(parameters, "variable")),
    _tracking_step(getParam<int>("tracking_step")),
    _hull_buffer(getParam<Real>("convex_hull_buffer")),
    _remap(getParam<bool>("remap_grains")),
    _nl(static_cast<FEProblem &>(_subproblem).getNonlinearSystem()),
    _unique_grains(declareRestartableData<std::map<unsigned int, UniqueGrain *> >("unique_grains")),
    _ebsd_reader(parameters.isParamValid("ebsd_reader") ? &getUserObject<EBSDReader>("ebsd_reader") : NULL),
    _compute_op_maps(getParam<bool>("compute_op_maps"))
{
  // Size the data structures to hold the correct number of maps
  _bounding_spheres.resize(_maps_size);
}

GrainTracker::~GrainTracker()
{
  for (std::map<unsigned int, UniqueGrain *>::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
    delete it->second;
}

void
GrainTracker::initialize()
{
  NodalFloodCount::initialize();

  _nodal_data.clear();
}

Real
GrainTracker::getNodalValue(dof_id_type node_id, unsigned int var_idx, bool show_var_coloring) const
{
  if (_t_step < _tracking_step)
    return 0;

  return NodalFloodCount::getNodalValue(node_id, var_idx, show_var_coloring);
}

Real
GrainTracker::getElementalValue(dof_id_type element_id) const
{
  // If this element contains the centroid of on of the grains, return the unique index
  const Elem * curr_elem = _mesh.elem(element_id);

  for (std::map<unsigned int, UniqueGrain *>::const_iterator grain_it = _unique_grains.begin();
       grain_it != _unique_grains.end(); ++grain_it)
  {
    if (grain_it->second->status == INACTIVE)
      continue;

    for (std::vector<BoundingSphereInfo *>::const_iterator it = grain_it->second->sphere_ptrs.begin();
         it != grain_it->second->sphere_ptrs.end(); ++it)
      if (curr_elem->contains_point((*it)->b_sphere.center()))
        return grain_it->first;
  }

  return 0;
}

/*
void
GrainTracker::threadJoin(const UserObject & y)
{
  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  const GrainTracker & pps = dynamic_cast<const GrainTracker &>(y);

  pack(_packed_data, false);

  std::vector<unsigned int> pps_packed_data;
  pps.pack(pps_packed_data, false);

  // Append the packed data structures together
  std::copy(pps_packed_data.begin(), pps_packed_data.end(), std::back_inserter(_packed_data));

  // Calculate thread Memory Usage
  if (_track_memory)
    _bytes_used += pps.calculateUsage();
}
*/


void
GrainTracker::finalize()
{
 // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;
  Moose::perf_log.push("finalize()","GrainTracker");

  // Exchange data in parallel
  pack(_packed_data, false);                 // Make sure we delay packing of periodic neighbor information
  _communicator.allgather(_packed_data, false);
  unpack(_packed_data);
  mergeSets();

  Moose::perf_log.push("buildspheres()","GrainTracker");
  buildBoundingSpheres();                    // Build bounding sphere information
  Moose::perf_log.pop("buildspheres()","GrainTracker");

//  NodalFloodCount::updateFieldInfo();
  _packed_data.clear();
  pack(_packed_data, true);                  // Pack the data again but this time add periodic neighbor information
  _communicator.allgather(_packed_data, false);
  unpack(_packed_data);
  mergeSets();

  Moose::perf_log.push("trackGrains()","GrainTracker");
  trackGrains();
  Moose::perf_log.pop("trackGrains()","GrainTracker");

  Moose::perf_log.push("remapGrains()","GrainTracker");
  if (_remap)
    remapGrains();
  Moose::perf_log.pop("remapGrains()","GrainTracker");

  updateFieldInfo();
  Moose::perf_log.pop("finalize()","GrainTracker");

  // Calculate and out output bubble volume data
  if (_pars.isParamValid("bubble_volume_file"))
  {
    calculateBubbleVolumes();
    std::vector<Real> data; data.reserve(_all_bubble_volumes.size() + 2);
    data.push_back(_fe_problem.timeStep());
    data.push_back(_fe_problem.time());
    data.insert(data.end(), _all_bubble_volumes.begin(), _all_bubble_volumes.end());
    writeCSVFile(getParam<FileName>("bubble_volume_file"), data);
  }

  if (_compute_op_maps)
  {
    for (std::map<unsigned int, UniqueGrain *>::const_iterator grain_it = _unique_grains.begin();
         grain_it != _unique_grains.end(); ++grain_it)
    {
      if (grain_it->second->status != INACTIVE)
      {
        std::set<dof_id_type>::const_iterator node_it_end = grain_it->second->nodes_ptr->end();
        for (std::set<dof_id_type>::const_iterator node_it = grain_it->second->nodes_ptr->begin(); node_it != node_it_end; ++node_it)
          _nodal_data[*node_it].push_back(std::make_pair(grain_it->first, grain_it->second->variable_idx));
      }
    }
  }

  // Calculate memory usage
  if (_track_memory)
  {
    _bytes_used += calculateUsage();
    _communicator.sum(_bytes_used);
    formatBytesUsed();
  }
}


const std::vector<std::pair<unsigned int, unsigned int> > &
GrainTracker::getNodalValues(dof_id_type node_id) const
{
  const std::map<unsigned int, std::vector<std::pair<unsigned int, unsigned int> > >::const_iterator pos = _nodal_data.find(node_id);

  if (pos != _nodal_data.end())
    return pos->second;
  else
  {
#if DEBUG
    mooseDoOnce(Moose::out << "Nodal values not in structure for node: " << node_id << " this may be normal.");
#endif
    return _empty;
  }
}

std::vector<std::vector<std::pair<unsigned int, unsigned int> > >
GrainTracker::getElementalValues(dof_id_type elem_id) const
{
  std::vector<std::vector<std::pair<unsigned int, unsigned int> > > elem_info;

  const Elem * curr_elem = _mesh.elem(elem_id);
  elem_info.resize(curr_elem->n_nodes());

  for (unsigned int i=0; i<elem_info.size(); ++i)
    // Note: This map only works with Linear Lagrange on First Order Elements
    elem_info[_qp_to_node[i]] = getNodalValues(curr_elem->node(i));

  return elem_info;
}


void
GrainTracker::buildBoundingSpheres()
{
  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  std::map<BoundaryID, std::set<dof_id_type> > pb_nodes;
  // Build a list of periodic nodes
  _mesh.buildPeriodicNodeSets(pb_nodes, _var_number, _pbs);
  MeshBase & mesh = _mesh.getMesh();

  unsigned long total_node_count = 0;
  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
  {
    /**
     * Create a pair of vectors of real values that is 3 (for the x,y,z components) times
     * the length of the current _bubble_sets length. Each processor will update the
     * vector for the nodes that it owns.  Then a parallel exchange will all for the
     * global min/maxs of each bubble.
     */
    std::vector<Real> min_points(_bubble_sets[map_num].size()*3,  1e30);
    std::vector<Real> max_points(_bubble_sets[map_num].size()*3, -1e30);

    unsigned int set_counter=0;
    for (std::list<BubbleData>::const_iterator it1 = _bubble_sets[map_num].begin();
         it1 != _bubble_sets[map_num].end(); ++it1)
    {
      total_node_count += it1->_nodes.size();

      // Find the min/max of our bounding box to calculate our bounding sphere
      for (std::set<dof_id_type>::iterator it2 = it1->_nodes.begin(); it2 != it1->_nodes.end(); ++it2)
      {
        Node *node = mesh.query_node_ptr(*it2);
        if (node)
          for (unsigned int i=0; i<mesh.spatial_dimension(); ++i)
          {
            min_points[set_counter*3+i] = std::min(min_points[set_counter*3+i], (*node)(i));
            max_points[set_counter*3+i] = std::max(max_points[set_counter*3+i], (*node)(i));
          }
      }

      ++set_counter;
    }

    _communicator.min(min_points);
    _communicator.max(max_points);

    set_counter = 0;
    for (std::list<BubbleData>::const_iterator it1 = _bubble_sets[map_num].begin();
         it1 != _bubble_sets[map_num].end(); ++it1)
    {
      Point min(min_points[set_counter*3], min_points[set_counter*3+1], min_points[set_counter*3+2]);
      Point max(max_points[set_counter*3], max_points[set_counter*3+1], max_points[set_counter*3+2]);

      // Calulate our bounding sphere
      Point center(min + ((max - min) / 2.0));

      // The radius is the different between the outer edge of the "bounding box"
      // and the center plus the "hull buffer" value
      Real radius = (max - center).size() + _hull_buffer;

      unsigned int some_node_id = *(it1->_nodes.begin());
      _bounding_spheres[map_num].push_back(new BoundingSphereInfo(some_node_id, center, radius));

      ++set_counter;
    }
  }

  Moose::out << "\nTotal Node Count: " << total_node_count << "\n";
}

void
GrainTracker::trackGrains()
{
  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  // Reset Status on active unique grains
  std::vector<unsigned int> map_sizes(_maps_size);
  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it = _unique_grains.begin();
       grain_it != _unique_grains.end(); ++grain_it)
  {
    if (grain_it->second->status != INACTIVE)
    {
      grain_it->second->status = NOT_MARKED;
      map_sizes[grain_it->second->variable_idx]++;
    }
  }

  // Print out info on the number of unique grains per variable vs the incoming bubble set sizes
  if (_t_step > _tracking_step)
  {
    for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
    {
      Moose::out << "\nGrains active index " << map_num << ": " << map_sizes[map_num] << " -> " << _bubble_sets[map_num].size();
      if (map_sizes[map_num] != _bubble_sets[map_num].size())
        Moose::out << "**";
    }
    Moose::out << std::endl;
  }

  std::vector<UniqueGrain *> new_grains; new_grains.reserve(_unique_grains.size());

  // Loop over all the current regions and build our unique grain structures
  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
  {
    for (std::list<BubbleData>::const_iterator it1 = _bubble_sets[map_num].begin();
         it1 != _bubble_sets[map_num].end(); ++it1)
    {
      std::vector<BoundingSphereInfo *> sphere_ptrs;
      unsigned int curr_var = it1->_var_idx;

      for (std::list<BoundingSphereInfo *>::iterator it2 = _bounding_spheres[map_num].begin();
           it2 != _bounding_spheres[map_num].end(); /* No increment here! */)
      {
        /**
         * See which of the bounding spheres belong to the current region (bubble set) by looking at a
         * member node id.  A single region may have multiple bounding spheres as members if it spans
         * periodic boundaries
         */
        if (it1->_nodes.find((*it2)->member_node_id) != it1->_nodes.end())
        {
          // Transfer ownership of the bounding sphere info to "sphere_ptrs" which will be stored in the unique grain
          sphere_ptrs.push_back(*it2);
          // Now delete the current BoundingSpherestructure so that it won't be inspected or reused
          _bounding_spheres[map_num].erase(it2++);
        }
        else
          ++it2;
      }

      // Create our new grains from this timestep that we will use to match up against the existing grains
      new_grains.push_back(new UniqueGrain(curr_var, sphere_ptrs, &it1->_nodes, NOT_MARKED));
    }
  }

  /**
   * If it's the first time through this routine for the simulation, we will generate the unique grain
   * numbers using a simple counter.  These will be the unique grain numbers that we must track for
   * the remainder of the simulation.
   */
  if (_t_step == _tracking_step)   // Start tracking when the time_step == the tracking_step
  {
    if (_ebsd_reader)
    {
      Real grain_num = _ebsd_reader->getGrainNum();

      std::vector<Point> center_points(grain_num);

      for (unsigned int gr=0; gr < grain_num; ++gr)
      {
        const EBSDReader::EBSDAvgData & d = _ebsd_reader->getAvgData(gr);
        center_points[gr] = d.p;
      }

      // To find the minimum distance we will use the boundingRegionDistance routine.
      // To do that, we need to build BoundingSphereObjects with a few dummy values, radius and node_id will be ignored
      BoundingSphereInfo ebsd_sphere(0, Point(0, 0, 0), 1);
      std::vector<BoundingSphereInfo *> ebsd_vector(1);
      ebsd_vector[0] = &ebsd_sphere;
      std::set<unsigned int> used_indices;

      for (unsigned int i=0; i < new_grains.size(); ++i)
      {
        Real min_centroid_diff = std::numeric_limits<Real>::max();
        unsigned int closest_match_idx = 0;

        for (unsigned int j=0; j<center_points.size(); ++j)
        {
          // Update the ebsd sphere to be used in the boundingRegionDistance calculation
          ebsd_sphere.b_sphere.center() = center_points[j];

          Real curr_centroid_diff = boundingRegionDistance(ebsd_vector, new_grains[i]->sphere_ptrs, true);
          if (curr_centroid_diff <= min_centroid_diff)
          {
            closest_match_idx = j;
            min_centroid_diff = curr_centroid_diff;
          }
        }

        if (used_indices.find(closest_match_idx) != used_indices.end())
          mooseError("Error finding unique closest match in ESBD initial condition");
        used_indices.insert(closest_match_idx);

        // Finally assign the grain index
        /**
         * TODO: Verify this mapping, the zeroth index is reserved for places where there are no grains
         */
        _unique_grains[closest_match_idx+1] = new_grains[closest_match_idx];
      }
    }
    else
    {
      for (unsigned int i=1; i <= new_grains.size(); ++i)
      {
        new_grains[i-1]->status = MARKED;
        _unique_grains[i] = new_grains[i-1];                   // Transfer ownership of the memory
      }
    }
    return;  // Return early - no matching or tracking to do
  }

  /**
   * To track grains across timesteps, we will loop over our unique grains and link each one up with one of our new
   * unique grains.  The criteria for doing this will be to find the unique grain in the new list with a matching variable
   * index whose centroid is closest to this unique grain.
   */
  std::map<unsigned int, std::vector<unsigned int> > new_grain_idx_to_existing_grain_idx;

  for (std::map<unsigned int, UniqueGrain *>::iterator curr_it = _unique_grains.begin(); curr_it != _unique_grains.end(); ++curr_it)
  {
    if (curr_it->second->status == INACTIVE)                         // Don't try to find matches for inactive grains
      continue;

    unsigned int closest_match_idx;
    // bool found_one = false;
    Real min_centroid_diff = std::numeric_limits<Real>::max();

    for (unsigned int new_grain_idx=0; new_grain_idx<new_grains.size(); ++new_grain_idx)
    {
      if (curr_it->second->variable_idx == new_grains[new_grain_idx]->variable_idx)  // Do the variables indicies match?
      {
        Real curr_centroid_diff = boundingRegionDistance(curr_it->second->sphere_ptrs, new_grains[new_grain_idx]->sphere_ptrs, true);
        if (curr_centroid_diff <= min_centroid_diff)
        {
          // found_one = true;
          closest_match_idx = new_grain_idx;
          min_centroid_diff = curr_centroid_diff;
        }
      }
    }

    // Keep track of which new grains the existing ones want to map to
    new_grain_idx_to_existing_grain_idx[closest_match_idx].push_back(curr_it->first);
  }

  /**
   * It's possible that multiple existing grains will map to a single new grain.  This will happen any time a grain disappears during this timestep.
   * We need to figure out the rightful owner in this case and inactivate the old grain.
   */
  for (std::map<unsigned int, std::vector<unsigned int> >::iterator it = new_grain_idx_to_existing_grain_idx.begin();
       it != new_grain_idx_to_existing_grain_idx.end(); ++it)
  {
    // If there is only a single mapping - we've found the correct grain
    if (it->second.size() == 1)
    {
      unsigned int curr_idx = (it->second)[0];
      delete _unique_grains[curr_idx];                      // clean-up old grain
      new_grains[it->first]->status = MARKED;               // Mark it
      _unique_grains[curr_idx] = new_grains[it->first];     // transfer ownership of new grain
    }

    // More than one existing grain is mapping to a new one
    else
    {
      Real min_centroid_diff = std::numeric_limits<Real>::max();
      unsigned int min_idx;
      for (unsigned int i=0; i < it->second.size(); ++i)
      {
        Real curr_centroid_diff = boundingRegionDistance(new_grains[it->first]->sphere_ptrs, _unique_grains[(it->second)[i]]->sphere_ptrs, true);
        if (curr_centroid_diff <= min_centroid_diff)
        {
          min_idx = i;
          min_centroid_diff = curr_centroid_diff;
        }
      }

      // One more time over the competing indicies.  We will mark the non-winners as inactive and transfer ownership to the winner (the closest centroid).
      for (unsigned int i=0; i < it->second.size(); ++i)
      {
        unsigned int curr_idx = (it->second)[i];
        if (i == min_idx)
        {
          delete _unique_grains[curr_idx];                      // clean-up old grain
          new_grains[it->first]->status = MARKED;               // Mark it
          _unique_grains[curr_idx] = new_grains[it->first];     // transfer ownership of new grain
        }
        else
        {
          Moose::out << "Marking Grain " << curr_idx << " as INACTIVE (varible index: "
                    << _unique_grains[curr_idx]->variable_idx <<  ")\n";
          _unique_grains[curr_idx]->status = INACTIVE;
        }
      }
    }
  }


  /**
   * Next we need to look at our new list and see which grains weren't matched up.  These are new grains.
   */
  for (unsigned int i=0; i<new_grains.size(); ++i)
    if (new_grains[i]->status == NOT_MARKED)
    {
      Moose::out << COLOR_YELLOW
                 << "*****************************************************************************\n"
                 << "Couldn't find a matching grain while working on variable index: " << new_grains[i]->variable_idx
                 << "\nCreating new unique grain: " << _unique_grains.size() + 1
                 << "\n*****************************************************************************\n" << COLOR_DEFAULT;
      new_grains[i]->status = MARKED;
      _unique_grains[_unique_grains.size() + 1] = new_grains[i];   // transfer ownership
    }


  /**
   * Finally we need to mark any grains in the unique list that aren't marked as inactive.  These are the variables that
   * unique grains that didn't match up to any bounding sphere.  Should only happen if it's the last active grain for
   * this particular variable.
   */
  for (std::map<unsigned int, UniqueGrain *>::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
    if (it->second->status == NOT_MARKED)
    {
      Moose::out << "Marking Grain " << it->first << " as INACTIVE (varible index: "
                    << it->second->variable_idx <<  ")\n";
      it->second->status = INACTIVE;
    }


  // Sanity check to make sure that we consumed all of the bounding sphere datastructures
  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
    if (!_bounding_spheres[map_num].empty())
      mooseError("BoundingSpheres where not completely used by the GrainTracker");
}

void
GrainTracker::remapGrains()
{
  // Don't remap grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  /**
   * Loop over each grain and see if the bounding spheres of the current grain intersect with the spheres of any other grains
   * represented by the same variable.
   */
  unsigned times_through_loop = 0;
  bool variables_remapped;
  do
  {
    Moose::out << "Remap Loop: " << ++times_through_loop << std::endl;

    variables_remapped = false;
    for (std::map<unsigned int, UniqueGrain *>::iterator grain_it1 = _unique_grains.begin();
         grain_it1 != _unique_grains.end(); ++grain_it1)
    {
      if (grain_it1->second->status == INACTIVE)
        continue;

      for (std::map<unsigned int, UniqueGrain *>::iterator grain_it2 = _unique_grains.begin();
           grain_it2 != _unique_grains.end(); ++grain_it2)
      {
        // Don't compare a grain with itself and don't try to remap inactive grains
        if (grain_it1 == grain_it2 || grain_it2->second->status == INACTIVE)
          continue;

        if (grain_it1->second->variable_idx == grain_it2->second->variable_idx &&                  // Are the grains represented by the same variable?
            boundingRegionDistance(grain_it1->second->sphere_ptrs, grain_it2->second->sphere_ptrs, false) < 0)  // If so, do their spheres intersect?
        {
          // If so, remap one of them
          swapSolutionValues(grain_it1, grain_it2, times_through_loop);

          // Since something was remapped, we need to inspect all the grains again to make sure that previously ok grains
          // aren't in some new nearly intersecting state.  Setting this Boolean to true will trigger the loop again
          variables_remapped = true;

          // Since the current grain has just been remapped we don't want to loop over any more potential grains (the inner for loop)
          break;
        }
      }
    }

    if (times_through_loop >= 5)
      mooseError(COLOR_RED << "Five passes through the remapping loop and grains are still being remapped, perhaps you need more op variables?" << COLOR_DEFAULT);

  } while (variables_remapped);
  Moose::out << "Done Remapping" << std::endl;
}

void
GrainTracker::swapSolutionValues(std::map<unsigned int, UniqueGrain *>::iterator & grain_it1,
                                 std::map<unsigned int, UniqueGrain *>::iterator & grain_it2,
                                 unsigned int attempt_number)
{
  NumericVector<Real> & solution         =  _nl.solution();
  NumericVector<Real> & solution_old     =  _nl.solutionOld();
  NumericVector<Real> & solution_older   =  _nl.solutionOlder();

  unsigned int curr_var_idx = grain_it1->second->variable_idx;
  /**
   * We have two grains that are getting close represented by the same order parameter.
   * We need to map to the variable whose closest grain to this one is furthest away by sphere to sphere distance.
   */
  std::vector<Real> min_distances(_vars.size(), std::numeric_limits<Real>::max());

  // Make sure that we don't attempt to remap to the same variable
  min_distances[curr_var_idx] = -std::numeric_limits<Real>::max();

  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it3 = _unique_grains.begin();
       grain_it3 != _unique_grains.end(); ++grain_it3)
  {
    if (grain_it3->second->status == INACTIVE || grain_it3->second->variable_idx == curr_var_idx)
      continue;

    unsigned int potential_var_idx = grain_it3->second->variable_idx;

    Real curr_bounding_sphere_diff = boundingRegionDistance(grain_it1->second->sphere_ptrs, grain_it3->second->sphere_ptrs, false);
    if (curr_bounding_sphere_diff < min_distances[potential_var_idx])
      min_distances[potential_var_idx] = curr_bounding_sphere_diff;
  }

  /**
   * We have a vector of the distances to the closest grains represented by each of our variables.  We just need to pick
   * a suitable grain to replace with.  We will start with the maximum of this this list: (max of the mins), but will settle
   * for next to largest and so forth as we make more attempts at remapping grains.  This is a graph coloring problem so
   * more work will be required to optimize this process.
   * Note: We don't have an explicit check here to avoid remapping a  variable to itself.  This is unecessary since the
   * min_distance of a variable is explicitly set up above.
   */
  unsigned int nth_largest_idx = min_distances.size() - attempt_number - 1;

  // nth element destroys the original array so we need to copy it first
  std::vector<Real> min_distances_copy(min_distances);
  std::nth_element(min_distances_copy.begin(), min_distances_copy.end()+nth_largest_idx, min_distances_copy.end());

  // Now find the location of the nth element in the original vector
  unsigned int new_variable_idx = std::distance(min_distances.begin(),
                                                std::find(min_distances.begin(),
                                                          min_distances.end(),
                                                          min_distances_copy[nth_largest_idx]));

  Moose::out
    << COLOR_YELLOW
    << "Grain #: " << grain_it1->first << " intersects Grain #: " << grain_it2->first
    << " (variable index: " << grain_it1->second->variable_idx << ")\n"
    << COLOR_DEFAULT;

  if (min_distances[new_variable_idx] < 0)
  {
    Moose::out
      << COLOR_YELLOW
      << "*****************************************************************************************************\n"
      << "Warning: No suitable variable found for remapping. Will attempt to remap in next loop if necessary...\n"
      << "*****************************************************************************************************\n"
      << COLOR_DEFAULT;
    return;
  }

  Moose::out
    << COLOR_GREEN
    << "Remapping to: " << new_variable_idx << " whose closest grain is at a distance of " << min_distances[new_variable_idx] << "\n"
    << COLOR_DEFAULT;

  MeshBase & mesh = _mesh.getMesh();
  // Remap the grain
  for (std::set<dof_id_type>::const_iterator node_it = grain_it1->second->nodes_ptr->begin();
       node_it != grain_it1->second->nodes_ptr->end(); ++node_it)
  {
    Node *curr_node = mesh.query_node_ptr(*node_it);

    if (curr_node && curr_node->processor_id() == processor_id())
    {
      _subproblem.reinitNode(curr_node, 0);

      // Swap the values from one variable to the other
      {
        VariableValue & value = _vars[curr_var_idx]->nodalSln();
        VariableValue & value_old = _vars[curr_var_idx]->nodalSlnOld();
        VariableValue & value_older = _vars[curr_var_idx]->nodalSlnOlder();

        // Copy Value from intersecting variable to new variable
        dof_id_type & dof_index = _vars[new_variable_idx]->nodalDofIndex();

        // Set the only DOF for this variable on this node
        solution.set(dof_index, value[0]);
        solution_old.set(dof_index, value_old[0]);
        solution_older.set(dof_index, value_older[0]);
      }
      {
        VariableValue & value = _vars[new_variable_idx]->nodalSln();
        VariableValue & value_old = _vars[new_variable_idx]->nodalSlnOld();
        VariableValue & value_older = _vars[new_variable_idx]->nodalSlnOlder();

        // Copy Value from variable to the intersecting variable
        dof_id_type & dof_index = _vars[curr_var_idx]->nodalDofIndex();

        // Set the only DOF for this variable on this node
        solution.set(dof_index, value[0]);
        solution_old.set(dof_index, value_old[0]);
        solution_older.set(dof_index, value_older[0]);
      }
    }
  }

  // Update the variable index in the unique grain datastructure
  grain_it1->second->variable_idx = new_variable_idx;

  // Close all of the solution vectors
  solution.close();
  solution_old.close();
  solution_older.close();

  _fe_problem.getNonlinearSystem().sys().update();
}

void
GrainTracker::updateFieldInfo()
{
  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
    _bubble_maps[map_num].clear();

  std::map<unsigned int, Real> tmp_map;
  MeshBase & mesh = _mesh.getMesh();

  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
  {
    unsigned int curr_var = grain_it->second->variable_idx;
    unsigned int map_idx = (_single_map_mode || _condense_map_info) ? 0 : curr_var;

    if (grain_it->second->status != INACTIVE)
      for (std::set<dof_id_type>::iterator node_it = grain_it->second->nodes_ptr->begin();
           node_it != grain_it->second->nodes_ptr->end(); ++node_it)
      {
        Node *curr_node = mesh.query_node_ptr(*node_it);

        if (curr_node &&
            _mesh.isSemiLocal(curr_node) &&
            _vars[grain_it->second->variable_idx]->getNodalValue(*curr_node) > tmp_map[curr_node->id()])
        {
          _bubble_maps[map_idx][curr_node->id()] = grain_it->first;
          if (_var_index_mode)
            _var_index_maps[map_idx][curr_node->id()] = grain_it->second->variable_idx;
        }
      }
  }
}



Real
GrainTracker::boundingRegionDistance(std::vector<BoundingSphereInfo *> & spheres1, std::vector<BoundingSphereInfo *> & spheres2, bool ignore_radii) const
{
  /**
   * The region that each grain covers is represented by a bounding sphere large enough to encompassing all the points
   * within that grain.  When using periodic boundaries, we may have several discrete "pieces" of a grain each represented
   * by a bounding sphere.  The distance between any two grains is defined as the minimum distance between any pair of spheres,
   * one selected from each grain.
   */
  Real min_distance = std::numeric_limits<Real>::max();
  for (std::vector<BoundingSphereInfo *>::iterator sphere_it1 = spheres1.begin(); sphere_it1 != spheres1.end(); ++sphere_it1)
  {
    libMesh::Sphere &sphere1 = (*sphere_it1)->b_sphere;

    for (std::vector<BoundingSphereInfo *>::iterator sphere_it2 = spheres2.begin(); sphere_it2 != spheres2.end(); ++sphere_it2)
    {
      libMesh::Sphere &sphere2 = (*sphere_it2)->b_sphere;

      // We need to see if these two spheres intersect on the domain.  To do that we need to account for periodicity of the mesh
      Real curr_distance = _mesh.minPeriodicDistance(_var_number, sphere1.center(), sphere2.center())    // distance between the centroids
        - (ignore_radii ? 0 : (sphere1.radius() + sphere2.radius()));                                    // minus the sum of the two radii

      if (curr_distance < min_distance)
        min_distance = curr_distance;
    }
  }

  return min_distance;
}



unsigned long
GrainTracker::calculateUsage() const
{
  unsigned long bytes = NodalFloodCount::calculateUsage();

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    bytes += bytesHelper(_bounding_spheres[map_num]);
  }

  // not counted: _nodal_data

  bytes += bytesHelper(_unique_grains);

  return bytes;
}


// BoundingSphereInfo
GrainTracker::BoundingSphereInfo::BoundingSphereInfo(unsigned int node_id, const Point & center, Real radius) :
    member_node_id(node_id),
    b_sphere(center, radius)
{}

// Unique Grain
GrainTracker::UniqueGrain::UniqueGrain(unsigned int var_idx,
                                       const std::vector<BoundingSphereInfo *> & b_sphere_ptrs,
                                       const std::set<dof_id_type> *nodes_pt,
                                       STATUS status) :
    variable_idx(var_idx),
    sphere_ptrs(b_sphere_ptrs),
    status(status),
    nodes_ptr(nodes_pt)
{}

GrainTracker::UniqueGrain::~UniqueGrain()
{
  for (unsigned int i=0; i<sphere_ptrs.size(); ++i)
    delete sphere_ptrs[i];
}

const unsigned int GrainTracker::_qp_to_node[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };
