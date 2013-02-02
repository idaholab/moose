#include "GrainTracker.h"
#include "MooseMesh.h"
#include "AddV.h"
#include "GeneratedMesh.h"

// LibMesh includes
#include "periodic_boundary_base.h"
#include "sphere.h"

#include <limits>

template<>
InputParameters validParams<GrainTracker>()
{
  InputParameters params = validParams<NodalFloodCount>();
  params.addRequiredParam<int>("crys_num","number of grains");
  params.addRequiredParam<std::string>("var_name_base","base for variable names");
  params.addParam<int>("tracking_step", 1, "The timestep for when we should start tracking grains");
  params.addParam<Real>("convex_hull_buffer", 1.0, "The buffer around the convex hull used to determine when features intersect");
  params.addParam<bool>("remap_grains", true, "Indicates whether remapping should be done or not (default: true)");

  // We are using "addV" to add the variable parameter on the fly
  params.suppressParameter<std::vector<VariableName> >("variable");

  return params;
}

GrainTracker::GrainTracker(const std::string & name, InputParameters parameters) :
    NodalFloodCount(name, AddV(parameters, "variable")),
    _tracking_step(getParam<int>("tracking_step")),
    _hull_buffer(getParam<Real>("convex_hull_buffer")),
    _nl(static_cast<FEProblem &>(_subproblem).getNonlinearSystem()),
    _remap(getParam<bool>("remap_grains"))
{
  // Size the data structures to hold the correct number of maps
  _bounding_spheres.resize(_maps_size);
}

GrainTracker::~GrainTracker()
{
  for (std::map<unsigned int, UniqueGrain *>::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
    delete it->second;
}

Real
GrainTracker::getNodalValue(unsigned int node_id, unsigned int var_idx, bool show_var_coloring) const
{
  if (_t_step < _tracking_step)
    return 0;

  return NodalFloodCount::getNodalValue(node_id, var_idx, show_var_coloring);
}

Real
GrainTracker::getElementalValue(unsigned int element_id) const
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
      if (curr_elem->contains_point((*it)->b_sphere->center()))
        return grain_it->first;
  }
  
  return 0;
}

void
GrainTracker::initialize()
{
  NodalFloodCount::initialize();

  _mesh.initPeriodicDistanceForVariable(_nl, _var_number);
}

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
}


void
GrainTracker::finalize()
{
 // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;
  Moose::perf_log.push("finalize()","GrainTracker");

  // Exchange data in parallel
  pack(_packed_data, false);                 // Make sure we delay packing of periodic neighbor information
  Parallel::allgather(_packed_data, false);
  unpack(_packed_data);
  mergeSets();

  _packed_data.clear();

  Moose::perf_log.push("buildspheres()","GrainTracker");
  buildBoundingSpheres();                    // Build bounding sphere information
  Moose::perf_log.pop("buildspheres()","GrainTracker");
  pack(_packed_data, true);                  // Pack the data again but this time add periodic neighbor information
  Parallel::allgather(_packed_data, false);
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
}

void
GrainTracker::buildBoundingSpheres()
{
  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  std::map<BoundaryID, std::set<unsigned int> > pb_nodes;
  // Build a list of periodic nodes
  _mesh.buildPeriodicNodeSets(pb_nodes, _var_number, _pbs);
  MeshBase & mesh = _mesh._mesh;
  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
  {
    for (std::list<BubbleData>::const_iterator it1 = _bubble_sets[map_num].begin();
         it1 != _bubble_sets[map_num].end(); ++it1)
    {
      Point min( 1.e30,  1.e30,  1.e30);
      Point max(-1.e30, -1.e30, -1.e30);
      unsigned int some_node_id = *(it1->_nodes.begin());

      // Find the min/max of our bounding box to calculate our bounding sphere
      for (std::set<unsigned int>::iterator it2 = it1->_nodes.begin(); it2 != it1->_nodes.end(); ++it2)
        for (unsigned int i=0; i<mesh.spatial_dimension(); ++i)
        {
          min(i) = std::min(min(i), mesh.point(*it2)(i));
          max(i) = std::max(max(i), mesh.point(*it2)(i));
        }

      // Calulate our bounding sphere
      Point center(min + ((max - min) / 2.0));
      Real radius = (max - center).size();

      _bounding_spheres[map_num].push_back(new BoundingSphereInfo(some_node_id, center, radius));
    }
  }
}

void
GrainTracker::trackGrains()
{
  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  unsigned int counter=1;
  
  // Reset Status on active unique grains
  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it = _unique_grains.begin();
       grain_it != _unique_grains.end(); ++grain_it)
  {
    if (grain_it->second->status != INACTIVE)
      grain_it->second->status = NOT_MARKED;
  }
  
  // Loop over all the current regions and match them up to our grain list
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
      
      /**
       * If it's the first time through this routine for the simulation, we will generate the unique grain
       * numbers using a simple counter.  These will be the unique grain numbers that we must track for
       * the remainder of the simulation.
       */
      if (_t_step == _tracking_step) // Start tracking when the time_step == the tracking_step
        _unique_grains[counter] = new UniqueGrain(curr_var, sphere_ptrs, &it1->_nodes);
      else // See if we can match up new grains with the existing grains
      {
        std::map<unsigned int, UniqueGrain *>::iterator grain_it, closest_match;
        bool found_one = false;
        Real min_centroid_diff = std::numeric_limits<Real>::max();

        for (grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
        { 
          if (grain_it->second->status == NOT_MARKED &&                   // Only consider grains that are unmarked AND
              grain_it->second->variable_idx == curr_var)                 // have matching variable indicies 
          {
            Real curr_centroid_diff = minCentroidDiff(sphere_ptrs, grain_it->second->sphere_ptrs);
            if (curr_centroid_diff <= min_centroid_diff)
            {
              found_one = true;
              closest_match = grain_it;
              min_centroid_diff = curr_centroid_diff;
            }
          }
        }

        if (!found_one)
        {
          std::cout << "Couldn't find a matching grain while working on variable index: " << curr_var
                    << " (num spheres: " << sphere_ptrs.size() << ")"
                    << "\nCreating new unique grain: " << _unique_grains.size() + 1 << "\n";

          _unique_grains[_unique_grains.size() + 1] = new UniqueGrain(curr_var, sphere_ptrs, &it1->_nodes);
        }
        else
        {
          if (closest_match->second->variable_idx != curr_var)
            std::cout << "Matching grain has new variable index, old: " << closest_match->second->variable_idx
                      << " new: " << curr_var << std::endl;
          if (min_centroid_diff > 10*_hull_buffer)
            std::cout << "Warning: Centroid for grain: " << closest_match->first << " has moved by "
                      << min_centroid_diff << " units.\n";
              
          // Now we want to update the grain information
          delete closest_match->second;
          // add the new grain (Note: The status of a new grain is MARKED)
          closest_match->second = new UniqueGrain(curr_var, sphere_ptrs, &it1->_nodes);
        }
      }
      ++counter;
    }
  }

  // Any grain that we didn't match will now be inactive
  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it = _unique_grains.begin();
       grain_it != _unique_grains.end(); ++grain_it)
    if (grain_it->second->status == NOT_MARKED)
    {
      std::cout << "Marking Grain " << grain_it->first << " as INACTIVE (varible index: "
                << grain_it->second->variable_idx << ")\n";
      grain_it->second->status = INACTIVE;
    }

  // Check to make sure that we consumed all of the bounding sphere datastructures
  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
    if (!_bounding_spheres[map_num].empty())
      mooseError("BoundingSpheres where not completely used by the GrainTracker");
}

void
GrainTracker::remapGrains()
{
  /**
   * Loop over each grain and see if the bounding spheres of the current grain intersect with the spheres of any other grains
   * represented by the same variable.
   */
  unsigned times_through_loop = 0;
  bool variables_remapped;
  do
  {
    std::cout << "Remap Loop: " << ++times_through_loop << std::endl;

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
            boundingRegionDistance(grain_it1->second->sphere_ptrs, grain_it2->second->sphere_ptrs) < 0)  // If so, do their spheres intersect?
        {
          // If so, remap one of them
          swapSolutionValues(grain_it1, grain_it2);

          // Since something was remapped, we need to inspect all the grains again to make sure that previously ok grains
          // aren't in some new nearly intersecting state.  Setting this Boolean to true will trigger the loop again
          variables_remapped = true;

          // Since the current grain has just been remapped we don't want to loop over any more potential grains (the inner for loop)
          break;
        }
      }
    }

    if (times_through_loop >= 5)
      mooseError("Five passes through the remapping loop and grains are still being remapped, perhaps you need more op variables?");

  } while (variables_remapped);
  std::cout << "Done Remapping\n";
}

void
GrainTracker::swapSolutionValues(std::map<unsigned int, UniqueGrain *>::iterator & grain_it1,
                                 std::map<unsigned int, UniqueGrain *>::iterator & grain_it2)
{
  NumericVector<Real> & solution         =  _nl.solution();
  NumericVector<Real> & solution_old     =  _nl.solutionOld();
  NumericVector<Real> & solution_older   =  _nl.solutionOlder();

  unsigned int curr_var_idx = grain_it1->second->variable_idx;
  /**
   * We have two grains that are getting close represented by the same order parameter.
   * We need to map to the variable whose closest grain to this one is furthest away (by centroids).
   */
  std::vector<Real> min_distances(_vars.size(), std::numeric_limits<Real>::max());
  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it3 = _unique_grains.begin();
       grain_it3 != _unique_grains.end(); ++grain_it3)
  {
    unsigned int potential_var_idx = grain_it3->second->variable_idx;

    Real curr_bounding_sphere_diff = boundingRegionDistance(grain_it1->second->sphere_ptrs,
                                                            grain_it3->second->sphere_ptrs);
    if (curr_bounding_sphere_diff < min_distances[potential_var_idx])
      min_distances[potential_var_idx] = curr_bounding_sphere_diff;
  }


  /**
   * We have a vector of the distances to the closest grains represented by each of our variables.  We just need to pick
   * the maximum of this this list: (max of the mins).  Note: We don't have an explicit check here to avoid remapping a
   * variable to itself.  This is unecessary since the min_distance of a variable to itself will always be zero thus making
   * it's selection impossible when looking for a max distance.
   */
  unsigned int new_variable_idx = std::distance(min_distances.begin(), std::max_element(min_distances.begin(), min_distances.end()));

  std::cout << "Grain #: " << grain_it1->first << " intersects Grain #: " << grain_it2->first << std::endl;
  std::cout << "Remapping to: " << new_variable_idx << " whose closest grain is at a distance of " << min_distances[new_variable_idx] << std::endl;

  // Remap the grain
  for (std::set<unsigned int>::const_iterator node_it = grain_it1->second->nodes_ptr->begin();
       node_it != grain_it1->second->nodes_ptr->end(); ++node_it)
  {
    Node * curr_node = _mesh.node_ptr(*node_it);

    if (curr_node->processor_id() == libMesh::processor_id())
    {
      _subproblem.reinitNode(curr_node, 0);

      // Swap the values from one variable to the other
      {
        VariableValue & value = _vars[curr_var_idx]->nodalSln();
        VariableValue & value_old = _vars[curr_var_idx]->nodalSlnOld();
        VariableValue & value_older = _vars[curr_var_idx]->nodalSlnOlder();

        // Copy Value from intersecting variable to new variable
        unsigned int & dof_index = _vars[new_variable_idx]->nodalDofIndex();

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
        unsigned int & dof_index = _vars[curr_var_idx]->nodalDofIndex();

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

  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
  {
    unsigned int curr_var = grain_it->second->variable_idx;
    unsigned int map_idx = (_single_map_mode || _condense_map_info) ? 0 : curr_var;
    
    if (grain_it->second->status != INACTIVE)
      for (std::set<unsigned int>::const_iterator node_it = grain_it->second->nodes_ptr->begin();
           node_it != grain_it->second->nodes_ptr->end(); ++node_it)
      {
        const Node & curr_node = _mesh.node(*node_it);

        if (_vars[grain_it->second->variable_idx]->getNodalValue(curr_node) > tmp_map[curr_node.id()])
        {
          _bubble_maps[map_idx][curr_node.id()] = grain_it->first;
          if (_var_index_mode)
            _var_index_maps[map_idx][curr_node.id()] = grain_it->second->variable_idx;
        }
      }
  }
}

Real
GrainTracker::boundingRegionDistance(std::vector<BoundingSphereInfo *> & spheres1, std::vector<BoundingSphereInfo *> & spheres2) const
{
  Real min_distance = std::numeric_limits<Real>::max();

  for (std::vector<BoundingSphereInfo *>::iterator sphere_it1 = spheres1.begin(); sphere_it1 != spheres1.end(); ++sphere_it1)
  {
    libMesh::Sphere *sphere1 = (*sphere_it1)->b_sphere;
    
    for (std::vector<BoundingSphereInfo *>::iterator sphere_it2 = spheres2.begin(); sphere_it2 != spheres2.end(); ++sphere_it2)
    {
      libMesh::Sphere *sphere2 = (*sphere_it2)->b_sphere;
      
      // We need to see if these two grains are close to each other.  To do that, we need to look
      // at the minimum periodic distance between the two centroids, as well as the distance between
      // the outer edge of each grain's bounding sphere.
      Real curr_distance = _mesh.minPeriodicDistance(sphere1->center(), sphere2->center())  // distance between the centroids
        - (sphere1->radius() + sphere2->radius() + 2*_hull_buffer);                         // minus the sum of the two radii and buffer zones

      if (curr_distance < min_distance)
        min_distance = curr_distance;
    }
  }

  return min_distance;
}

Real
GrainTracker::minCentroidDiff(const std::vector<BoundingSphereInfo *> & sphere_ptrs1, const std::vector<BoundingSphereInfo *> & sphere_ptrs2) const
{
  Real diff = std::numeric_limits<Real>::max();
  for (std::vector<BoundingSphereInfo *>::const_iterator it1 = sphere_ptrs1.begin(); it1 != sphere_ptrs1.end(); ++it1)
    for (std::vector<BoundingSphereInfo *>::const_iterator it2 = sphere_ptrs2.begin(); it2 != sphere_ptrs2.end(); ++it2)
    {
      Real curr_diff = ((*it1)->b_sphere->center() - (*it2)->b_sphere->center()).size();
      if (curr_diff < diff)
        diff = curr_diff;
    }

  return diff;
}

// BoundingSphereInfo
GrainTracker::BoundingSphereInfo::BoundingSphereInfo(unsigned int node_id, const Point & center, Real radius) :
    member_node_id(node_id),
    b_sphere(new libMesh::Sphere(center, radius))
{}

// Unique Grain
GrainTracker::UniqueGrain::UniqueGrain(unsigned int var_idx, const std::vector<BoundingSphereInfo *> & b_sphere_ptrs,
                                       const std::set<unsigned int> *nodes_pt) :
    variable_idx(var_idx),
    sphere_ptrs(b_sphere_ptrs),
    status(MARKED),
    nodes_ptr(nodes_pt)
{}

GrainTracker::UniqueGrain::~UniqueGrain()
{
  for (unsigned int i=0; i<sphere_ptrs.size(); ++i)
  {
    delete sphere_ptrs[i]->b_sphere;
    delete sphere_ptrs[i];
  }
}
