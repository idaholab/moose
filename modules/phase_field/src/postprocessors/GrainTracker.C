#include "GrainTracker.h"
#include "MooseMesh.h"
#include "AddV.h"
#include "GeneratedMesh.h"

// LibMesh includes
#include "periodic_boundary_base.h"

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
  params.addParam<UserObjectName>("grain_remapper", "The GrainTracker UserObject to couple to for remap information (optional).");

  params.suppressParameter<std::vector<VariableName> >("variable");

  return params;
}

GrainTracker::GrainTracker(const std::string & name, InputParameters parameters) :
    NodalFloodCount(name, AddV(parameters, "variable")),
    _tracking_step(getParam<int>("tracking_step")),
    _hull_buffer(getParam<Real>("convex_hull_buffer")),
    _nl(static_cast<FEProblem &>(_subproblem).getNonlinearSystem()),
    _remap(getParam<bool>("remap_grains")),
    _grain_remapper(parameters.isParamValid("grain_remapper") ? &getUserObject<GrainTracker>("grain_remapper") : NULL)
{
  // Size the data structures to hold the correct number of maps
  _bounding_boxes.resize(_maps_size);
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
  
  for (std::map<unsigned int, UniqueGrain *>::const_iterator grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
  {
    if (grain_it->second->status == INACTIVE)
      continue;
    if (curr_elem->contains_point(grain_it->second->centroid, 1e-7))
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

  Moose::perf_log.push("buildboxs()","GrainTracker");
  buildBoundingBoxes();                      // Build bounding box information
  Moose::perf_log.pop("buildboxs()","GrainTracker");
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

  updateNodeInfo();
  Moose::perf_log.pop("finalize()","GrainTracker");
}

void
GrainTracker::buildBoundingBoxes()
{
  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  std::map<BoundaryID, std::set<unsigned int> > pb_nodes;
  // Build a list of periodic nodes
  _mesh.buildPeriodicNodeSets(pb_nodes, _var_number, _pbs);

//  // DEBUG
//  std::cout << "DEBUG!\n";
//  for (std::map<BoundaryID, std::set<unsigned int> >::iterator nodeset_it = pb_nodes.begin(); nodeset_it != pb_nodes.end(); ++nodeset_it)
//  {
//    std::cout << "Set: " << nodeset_it->first << "\n";
//    for (std::set<unsigned int>::iterator foo = nodeset_it->second.begin(); foo != nodeset_it->second.end(); ++foo)
//      std::cout << *foo << " ";
//    std::cout << "\n";
//  }
//  // DEBUG

  MeshBase & mesh = _mesh._mesh;
  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
  {
    for (std::list<BubbleData>::const_iterator it1 = _bubble_sets[map_num].begin(); it1 != _bubble_sets[map_num].end(); ++it1)
    {
      Point min( 1.e30,  1.e30,  1.e30);
      Point max(-1.e30, -1.e30, -1.e30);
      unsigned int some_node_id = *(it1->_nodes.begin());

      // Find the min/max of our bounding box
      for (std::set<unsigned int>::iterator it2 = it1->_nodes.begin(); it2 != it1->_nodes.end(); ++it2)
        for (unsigned int i=0; i<mesh.spatial_dimension(); ++i)
        {
          min(i) = std::min(min(i), mesh.point(*it2)(i));
          max(i) = std::max(max(i), mesh.point(*it2)(i));
        }

//    // DEBUG
//    std::cout << "Working on set: " << counter++ << "\n";
//    for (std::set<unsigned int>::iterator it2 = it1->_nodes.begin(); it2 != it1->_nodes.end(); ++it2)
//      std::cout << *it2 << " ";
//    std::cout << "\n";
//    // DEBUG

      RealVectorValue translation_vector;
      if (_pbs)
      {
        for (std::map<BoundaryID, std::set<unsigned int> >::iterator nodeset_it = pb_nodes.begin(); nodeset_it != pb_nodes.end(); ++nodeset_it)
        {
          std::set<unsigned int> intersection;
          std::set_intersection(it1->_nodes.begin(), it1->_nodes.end(), nodeset_it->second.begin(), nodeset_it->second.end(),
                                std::inserter(intersection, intersection.end()));

          if (!intersection.empty())
          {
//          // DEBUG
//          std::cout << "Intersection: ";
//          for (std::set<unsigned int>::iterator bar = intersection.begin(); bar != intersection.end(); ++bar)
//            std::cout << *bar << " ";
//          std::cout << "\n\n";
//          // DEBUG

            PeriodicBoundaryBase *pb = _pbs->boundary(nodeset_it->first);
            mooseAssert(pb != NULL, "Error Periodic Boundary is NULL in GrainTracker");

            bool prefered_direction = false;
            if (_prefered_pb_pair.find(pb->myboundary) != _prefered_pb_pair.end())
              prefered_direction = true;

            // See if we need to add this after making sure that it's not in the "non-prefered" orientation
            if (_nonprefered_pb_pair.find(pb->myboundary) == _nonprefered_pb_pair.end())
            {
              _prefered_pb_pair[pb->myboundary] = pb->pairedboundary;
              _nonprefered_pb_pair[pb->pairedboundary] = pb->myboundary;
              prefered_direction = true;
            }

            if (prefered_direction)
            {
              /**
               * Assumptions: This call will only work with serial mesh since we may not have this particular point
               *              on this particular processor in parallel.
               *              Also, we can only use a single translation vector with constant translation PBCs.
               */

              /**
               * TODO: This is a workaround for not having access to the translation vector.  We can take an arbitrary point,
               * get the corresponding point and subtract the two to recover the translation vector.  We need an accessor in
               * libMesh.
               */

//            // DEBUG
//            std::cout << "Primary: " << pb->myboundary << "\n";
//            std::cout << "Secondary: " << pb->pairedboundary << "\n";
//            // DEBUG

              Point boundary_point = mesh.point(*nodeset_it->second.begin());
              Point corresponding_point = pb->get_corresponding_pos(boundary_point);

              translation_vector += boundary_point - corresponding_point;
            }
          }
        }
      }

      _bounding_boxes[map_num].push_back(new BoundingBoxInfo(some_node_id, translation_vector, min, max));
    }
  }

//  // DEBUG
//  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
//  {
//    std::cout << "Bounding Boxes for variable index: " << map_num << "\n";
//    for (std::list<BoundingBoxInfo *>::iterator box_it = _bounding_boxes[map_num].begin(); box_it != _bounding_boxes[map_num].end(); ++box_it)
//      std::cout << (*box_it)->b_box->min() << (*box_it)->b_box->max() << "\n";
//  }
//  // DEBUG
}

void
GrainTracker::trackGrains()
{
  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  unsigned int counter=1;

  // TODO: Used to keep track of which bounding box indexes have been used by which unique_grains
  // std::vector<bool> used_idx(_bounding_boxes.size(), false);

  // Reset Status on active unique grains
  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
  {
    if (grain_it->second->status != INACTIVE)
      grain_it->second->status = NOT_MARKED;
  }

  // Get the remapped grains from the coupled object if it exists
  const std::set<std::pair<unsigned int, unsigned int> > *remapped_info = NULL;
  if (_grain_remapper)
    remapped_info = _grain_remapper->getRemappedGrains();

  // Loop over all the current regions and match them up to our grain list
  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
  {
    for (std::list<BubbleData>::const_iterator it1 = _bubble_sets[map_num].begin(); it1 != _bubble_sets[map_num].end(); ++it1)
    {
      std::vector<BoundingBoxInfo *> box_ptrs;
      unsigned int curr_var = it1->_var_idx;

      for (std::list<BoundingBoxInfo *>::iterator it2 = _bounding_boxes[map_num].begin(); it2 != _bounding_boxes[map_num].end(); /* No increment here! */)
      {
        /**
         * See which of the bounding boxes belong to the current region (bubble set) by looking at a
         * member node id.  A single region may have multiple bounding boxes as members if it spans
         * periodic boundaries
         */
        if (it1->_nodes.find((*it2)->member_node_id) != it1->_nodes.end())
        {
          // Transfer ownership of the bounding box info to "box_ptrs" which will be stored in the unique grain
          box_ptrs.push_back(*it2);
          // Now delete the current BoundingBox structure so that it won't be inspected or reused
          _bounding_boxes[map_num].erase(it2++);
        }
        else
          ++it2;
      }

      // Calculate the centroid from the list of bounding boxes taking into account periodic boundary conditions
      Point curr_centroid = calculateCentroid(box_ptrs);

      /**
       * If it's the first time through this routine for the simulation, we will generate the unique grain
       * numbers using a simple counter.  These will be the unique grain numbers that we must track for
       * the remainder of the simulation.
       */
      if (_t_step == _tracking_step) // Start tracking when the time_step == the tracking_step
        _unique_grains[counter] = new UniqueGrain(curr_var, box_ptrs, curr_centroid, &it1->_nodes);
      else // See if we can match up new grains with the existing grains
      {
        std::map<unsigned int, UniqueGrain *>::iterator grain_it, closest_match;
        bool found_one = false;
        Real min_centroid_diff = std::numeric_limits<Real>::max();

        for (grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
        { 
          if (grain_it->second->status == NOT_MARKED &&                   // Only consider grains that are unmarked AND
              (grain_it->second->variable_idx == curr_var ||              // have matching variable indicies OR
               (remapped_info &&                                          // have corresponding indicies that were part of a remap
                remapped_info->find(std::make_pair(grain_it->second->variable_idx, curr_var)) != remapped_info->end())))
          {            
            Real curr_centroid_diff = _mesh.minPeriodicDistance(grain_it->second->centroid, curr_centroid);
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
                    << "\nCentroid: " << curr_centroid << " (num boxes: " << box_ptrs.size() << ")"
                    << "\nCreating new unique grain: " << _unique_grains.size() + 1 << "\n";

          _unique_grains[_unique_grains.size() + 1] = new UniqueGrain(curr_var, box_ptrs, curr_centroid, &it1->_nodes);
        }
        else
        {
          if (closest_match->second->variable_idx != curr_var)
            std::cout << "Matching grain has new variable index, old: " << closest_match->second->variable_idx << " new: " << curr_var << std::endl;
          if (min_centroid_diff > 10*_hull_buffer)
            std::cout << "Warning: Centroid for grain: " << closest_match->first << " has moved by " << min_centroid_diff << " units.\n";
              
          // Now we want to update the grain information
          delete closest_match->second;
          // add the new grain (Note: The status of a new grain is MARKED)
          closest_match->second = new UniqueGrain(curr_var, box_ptrs, curr_centroid, &it1->_nodes);
        }
      }
      ++counter;
    }
  }

  // Any grain that we didn't match will now be inactive
  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
    if (grain_it->second->status == NOT_MARKED)
    {
      std::cout << "Marking Grain " << grain_it->first << " as INACTIVE (varible index: " << grain_it->second->variable_idx
                << ")\nCentroid: " << grain_it->second->centroid << "\n";
                
      grain_it->second->status = INACTIVE;
    }

  // Check to make sure that we consumed all of the bounding box datastructures
  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
    if (!_bounding_boxes[map_num].empty())
      mooseError("BoundingBoxes where not completely used by the GrainTracker");

  //DEBUG
   std::cout << "TimeStep: " << _t_step << "\n";
   std::cout << "Unique Grain Size: " << _unique_grains.size() << "\n";
   for (std::map<unsigned int, UniqueGrain *>::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
   {
     std::cout << "Unique Number: " << it->first
               << "\nStatus: " << it->second->status
               << "\nVariable idx: " << it->second->variable_idx
               << "\nCentroid: " << it->second->centroid
               << "\nBounding Boxes:\n";
       for (unsigned int i=0; i<it->second->box_ptrs.size(); ++i)
         std::cout << it->second->box_ptrs[i]->b_box->min() << it->second->box_ptrs[i]->b_box->max() << "\n";
       std::cout << "Nodes Ptr: " << it->second->nodes_ptr << std::endl;
   }
  //DEBUG
}

void
GrainTracker::remapGrains()
{
  // Clear the remapped grain structures
  _remapped_grains.clear();

  /**
   * Loop over each grain and see if the bounding boxes of the current grain intersect with the boxes of any other grains
   * represented by the same variable.
   */
  unsigned times_through_loop = 0;
  bool variables_remapped;
  do
  {
    std::cout << "Remap Loop: " << ++times_through_loop << std::endl;

    variables_remapped = false;
    for (std::map<unsigned int, UniqueGrain *>::iterator grain_it1 = _unique_grains.begin(); grain_it1 != _unique_grains.end(); ++grain_it1)
    {
      if (grain_it1->second->status == INACTIVE)
        continue;

      for (std::map<unsigned int, UniqueGrain *>::iterator grain_it2 = _unique_grains.begin(); grain_it2 != _unique_grains.end(); ++grain_it2)
      {
        // Don't compare a grain with itself and don't try to remap inactive grains
        if (grain_it1 == grain_it2 || grain_it2->second->status == INACTIVE)
          continue;

        if (grain_it1->second->variable_idx == grain_it2->second->variable_idx &&                  // Are the grains represented by the same variable?
            boundingRegionDistance(grain_it1->second->box_ptrs, grain_it2->second->box_ptrs) < 0)  // If so, do their boxes intersect?
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
GrainTracker::swapSolutionValues(std::map<unsigned int, UniqueGrain *>::iterator & grain_it1, std::map<unsigned int, UniqueGrain *>::iterator & grain_it2)
{
  NumericVector<Real> & solution         =  _nl.solution();
  NumericVector<Real> & solution_old     =  _nl.solutionOld();
  NumericVector<Real> & solution_older   =  _nl.solutionOlder();

  unsigned int curr_var_idx = grain_it1->second->variable_idx;
  /**
   * We have two grains that are getting close represented by the same order parameter.
   * We need to map to the variable whose closest grain to this one is furthest away (by centroids).
   */
//  Old Bounding Box Version
//  std::vector<Real> min_distances(_vars.size(), std::numeric_limits<Real>::max());
//  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it3 = _unique_grains.begin(); grain_it3 != _unique_grains.end(); ++grain_it3)
//  {
//    unsigned int potential_var_idx = grain_it3->second->variable_idx;
//
//    Real curr_centroid_diff = _mesh.minPeriodicDistance(grain_it1->second->centroid, grain_it3->second->centroid);
//    if (curr_centroid_diff < min_distances[potential_var_idx])
//      min_distances[potential_var_idx] = curr_centroid_diff;
//  }

  std::vector<Real> min_distances(_vars.size(), std::numeric_limits<Real>::max());
  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it3 = _unique_grains.begin(); grain_it3 != _unique_grains.end(); ++grain_it3)
  {
    unsigned int potential_var_idx = grain_it3->second->variable_idx;

    Real curr_bounding_box_diff = boundingRegionDistance(grain_it1->second->box_ptrs, grain_it3->second->box_ptrs);
    if (curr_bounding_box_diff < min_distances[potential_var_idx])
      min_distances[potential_var_idx] = curr_bounding_box_diff;
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

  // Update the map of remapped grains (source variable to destination variable index)
  _remapped_grains.insert(std::make_pair(grain_it1->second->variable_idx, new_variable_idx));

  // Update the variable index in the unique grain datastructure
  grain_it1->second->variable_idx = new_variable_idx;

  // Close all of the solution vectors
  solution.close();
  solution_old.close();
  solution_older.close();

  _fe_problem.getNonlinearSystem().sys().update();
}

void
GrainTracker::updateNodeInfo()
{
  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
    _bubble_maps[map_num].clear();

  std::map<unsigned int, Real> tmp_map;

  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
  {
    // DEBUG
//    std::cout << "Unique Number: " << grain_it->first
//              << "\nVariable idx: " << grain_it->second->variable_idx
//              << "\nBounding Boxes:\n";
//    for (unsigned int i=0; i<grain_it->second->box_ptrs.size(); ++i)
//      std::cout << grain_it->second->box_ptrs[i]->b_box->min() << grain_it->second->box_ptrs[i]->b_box->max() << "\n";
//    std::cout << "Nodes Ptr: " << grain_it->second->nodes_ptr << std::endl;
    //DEBUG

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
GrainTracker::boundingRegionDistance(std::vector<BoundingBoxInfo *> & boxes1, std::vector<BoundingBoxInfo *> & boxes2) const
{
  /* Original Box Routine
  RealVectorValue buffer;
  switch (_mesh.dimension())
  {
  case 1:
    mooseError("1D is not supported");
  case 2:
    buffer.assign(Point(_hull_buffer, _hull_buffer, 0));
  case 3:
    buffer.assign(Point(_hull_buffer, _hull_buffer, _hull_buffer));
  }

  // Check the individual bounding boxes
  for (std::vector<BoundingBoxInfo *>::iterator box_it1 = boxes1.begin(); box_it1 != boxes1.end(); ++box_it1)
  {
    MeshTools::BoundingBox box1 = *(*box_it1)->b_box;
    box1.min() -= buffer;
    box1.max() += buffer;

    for (std::vector<BoundingBoxInfo *>::iterator box_it2 = boxes2.begin(); box_it2 != boxes2.end(); ++box_it2)
    {
      MeshTools::BoundingBox box2 = *(*box_it2)->b_box;
      box2.min() -= buffer;
      box2.max() += buffer;

      if (box1.intersect(box2))  // Do the boxes intersect with the buffer zone added in
        return true;
    }
  }
  */
  Real min_distance = std::numeric_limits<Real>::max();

  for (std::vector<BoundingBoxInfo *>::iterator box_it1 = boxes1.begin(); box_it1 != boxes1.end(); ++box_it1)
  {
    Point centroid1(((*box_it1)->b_box->max() + (*box_it1)->b_box->min()) / 2.0);
    Real radius1 = ((*box_it1)->b_box->max() - centroid1).size() + _hull_buffer;

    for (std::vector<BoundingBoxInfo *>::iterator box_it2 = boxes2.begin(); box_it2 != boxes2.end(); ++box_it2)
    {
      Point centroid2(((*box_it2)->b_box->max() + (*box_it2)->b_box->min()) / 2.0);
      Real radius2 = ((*box_it2)->b_box->max() - centroid2).size() + _hull_buffer;

      // We need to see if these two grains are close to each other.  To do that, we need to look
      // at the minimum periodic distance between the two centroids, as well as the distance between
      // the outer edge of each grain's bounding sphere.
      Real curr_distance = _mesh.minPeriodicDistance(centroid1, centroid2)  // distance between the centroids
        - (radius1 + radius2);                                              // minus the sum of the two radii

      if (curr_distance < min_distance)
        min_distance = curr_distance;

//      if (s1.distance(s2) < 0)
//      {
//        std::cout << "Centroid1: " << centroid1
//                  << "\nRadius1: " << ((*box_it1)->b_box->max() - centroid1).size()
//                  << "\nAdj Radius1: " << radius1 << "\n";
//
//        std::cout << "Centroid2: " << centroid2
//                  << "\nRadius2: " << ((*box_it2)->b_box->max() - centroid2).size()
//                  << "\nAdj Radius2: " << radius2 << "\n";
//        return true;
//      }
    }
  }

  return min_distance;
}

Point
GrainTracker::calculateCentroid(const std::vector<BoundingBoxInfo *> & box_ptrs) const
{
  Point centroid;
  for (std::vector<BoundingBoxInfo *>::const_iterator it = box_ptrs.begin(); it != box_ptrs.end(); ++it)
  {
//    // DEBUG
//    std::cout << "Bounding Box: (" << (*it)->b_box->min() << ", " << (*it)->b_box->max() << ")\n";
//    // DEBUG
    Point curr_adj_centroid = (((*it)->b_box->max() + (*it)->b_box->min()) / 2.0) - (*it)->translation_vector;

//    // DEBUG
//    std::cout << "Translation Vector: " << (*it)->translation_vector << "\n";
//    std::cout << "Centroid: " << curr_adj_centroid << "\n";
//    // DEBUG

    centroid += curr_adj_centroid;
  }
  centroid /= box_ptrs.size();

  // Make sure that the final centroid is in the domain
  for (unsigned int i=0; i<LIBMESH_DIM; ++i)
  {
    if (centroid(i) < _mesh.getMinInDimension(i))
      centroid(i) += _mesh.dimensionWidth(i);
    else if (centroid(i) > _mesh.getMaxInDimension(i))
      centroid(i) -= _mesh.dimensionWidth(i);
  }

//  // DEBUG
//  std::cout << "Combined Centroid: " << centroid << "\n\n";
//  // DEBUG

  return centroid;
}

// BoundingBoxInfo
GrainTracker::BoundingBoxInfo::BoundingBoxInfo(unsigned int node_id, const RealVectorValue & trans_vector, const Point & min, const Point & max) :
    member_node_id(node_id),
    b_box(new MeshTools::BoundingBox(min, max)),
    translation_vector(trans_vector)
{}

// Unique Grain
GrainTracker::UniqueGrain::UniqueGrain(unsigned int var_idx, const std::vector<BoundingBoxInfo *> & b_box_ptrs, const Point & p_centroid, const std::set<unsigned int> *nodes_pt) :
    variable_idx(var_idx),
    centroid(p_centroid),
    box_ptrs(b_box_ptrs),
    status(MARKED),
    nodes_ptr(nodes_pt)
{}

GrainTracker::UniqueGrain::~UniqueGrain()
{
  for (unsigned int i=0; i<box_ptrs.size(); ++i)
  {
    delete box_ptrs[i]->b_box;
    delete box_ptrs[i];
  }
}
