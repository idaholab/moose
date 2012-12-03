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
  params.addParam<unsigned int>("tracking_step", 1, "The timestep for when we should start tracking grains");
  params.addParam<Real>("convex_hull_buffer", 1.0, "The buffer around the convex hull used to determine when features intersect");
  params.addParam<bool>("remap_grains", true, "Indicates whether remapping should be done or not (default: true)");

  params.suppressParameter<std::vector<VariableName> >("variable");

  return params;
}

GrainTracker::GrainTracker(const std::string & name, InputParameters parameters) :
    NodalFloodCount(name, AddV(parameters, "variable")),
    _tracking_step(getParam<unsigned int>("tracking_step")),
    _hull_buffer(getParam<Real>("convex_hull_buffer")),
    _nl(static_cast<FEProblem &>(_subproblem).getNonlinearSystem()),
    _remap(getParam<bool>("remap_grains"))
{
  // Size the data structures to hold the correct number of maps
  _bounding_boxes.resize(_maps_size);
  
//  _region_to_grain[0] = 0; // Zero indicates no grain - we need a place holder for this one postion
}

GrainTracker::~GrainTracker()
{
  for (std::map<unsigned int, UniqueGrain *>::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
    delete it->second;
}

Real
GrainTracker::getNodeValue(unsigned int node_id, unsigned int var_idx) const
{
  mooseAssert(var_idx < _maps_size, "Index out of range");
  
  if (_t_step < _tracking_step)
    return 0;
  
  if (_bubble_maps[var_idx].find(node_id) == _bubble_maps[var_idx].end())
    return 0;
  else
    return _bubble_maps[var_idx].at(node_id);
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

  // Exchange data in parallel
  pack(_packed_data, false);                 // Make sure we delay packing of periodic neighbor information
  Parallel::allgather(_packed_data, false);
  unpack(_packed_data);
  mergeSets();

  _packed_data.clear();

  buildBoundingBoxes();                      // Build bounding box information
  pack(_packed_data, true);                  // Pack the data again but this time add periodic neighbor information
  Parallel::allgather(_packed_data, false);
  unpack(_packed_data);
  mergeSets();

  trackGrains();

  if (_remap)
    remapGrains();

  updateNodeInfo();
  
  // Update the region offsets so we can get unique bubble numbers in multimap mode
//  updateRegionOffsets();
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
  unsigned int counter = 0;
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

  MeshBase & mesh = _mesh._mesh;

  unsigned int counter=1;

  // TODO: Used to keep track of which bounding box indexes have been used by which unique_grains
  // std::vector<bool> used_idx(_bounding_boxes.size(), false);

  // Reset Status on active unique grains
  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
    if (grain_it->second->status != INACTIVE)
      grain_it->second->status = NOT_MARKED;

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
      {
        _unique_grains[counter] = new UniqueGrain(curr_var, box_ptrs, curr_centroid, &it1->_nodes);
//        _region_to_grain[counter] = counter;
      }
      else // See if we can match up new grains with the existing grains
      {
        std::map<unsigned int, UniqueGrain *>::iterator grain_it, closest_match;
        bool found_one = false;
        Real min_centroid_diff = std::numeric_limits<Real>::max();
        
        for (grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
        {
          // Only unmarked grains and with matching variable indicies will be considered
          if (grain_it->second->status == NOT_MARKED &&
              grain_it->second->variable_idx == curr_var)
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
          std::cerr << "Couldn't find a matching grain while working on variable index: " << map_num
                    << "\nCentroid: " << curr_centroid << " (num boxes: " << box_ptrs.size() << ")\n";
          mooseError("FAIL");
        }
        
          
        

        // Now we want to update the grain information
        delete closest_match->second;
        // add the new
        closest_match->second = new UniqueGrain(curr_var, box_ptrs, curr_centroid, &it1->_nodes);

//        _region_to_grain[counter] = closest_match->first;
      }
      ++counter;
    }
  }
  
  // Any grain that we didn't match will now be inactive
  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
    if (grain_it->second->status == NOT_MARKED)
    {
      std::cout << "Marking Grain: " << grain_it->first << " as INACTIVE\n";
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
  NumericVector<Real> & solution         =  _nl.solution();
  NumericVector<Real> & solution_old     =  _nl.solutionOld();
  NumericVector<Real> & solution_older   =  _nl.solutionOlder();
  
  /**
   * Loop over each grain and see if the bounding boxes of the current grain intersect with the boxes of any other grains
   * represented by the same variable.
   */
  Point buffer;
  switch (_mesh.dimension())
  {
  case 1:
    mooseError("1D is not supported");
  case 2:
    buffer.assign(Point(_hull_buffer, _hull_buffer, 0));
  case 3:
    buffer.assign(Point(_hull_buffer, _hull_buffer, _hull_buffer));
  }
  
  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it1 = _unique_grains.begin(); grain_it1 != _unique_grains.end(); ++grain_it1)
  {
    std::vector<BoundingBoxInfo *> & grain_box1 = grain_it1->second->box_ptrs;
    
    for (std::map<unsigned int, UniqueGrain *>::iterator grain_it2 = _unique_grains.begin(); grain_it2 != _unique_grains.end(); ++grain_it2)
    {
      // Don't compare a grain with itself
      if (grain_it1 == grain_it2)
        continue;

      std::vector<BoundingBoxInfo *> & grain_box2 = grain_it2->second->box_ptrs;

      // Check the individual bounding boxes
      for (std::vector<BoundingBoxInfo *>::iterator box_it1 = grain_box1.begin(); box_it1 != grain_box1.end(); ++box_it1)
      {
        MeshTools::BoundingBox box1 = *(*box_it1)->b_box;
        box1.min() -= buffer;  
        box1.max() += buffer;
        
        for (std::vector<BoundingBoxInfo *>::iterator box_it2 = grain_box2.begin(); box_it2 != grain_box2.end(); ++box_it2)
        {
          MeshTools::BoundingBox box2 = *(*box_it2)->b_box;
          box2.min() -= buffer;  
          box2.max() += buffer;
          
          if (box1.intersect(box2) &&                                               // Do the boxes intersect
              grain_it1->second->variable_idx == grain_it2->second->variable_idx)   // and are the represented by the same variable?
          {
            /**
             * If we get into this case, that means that we have two grains that are getting close represented by the same order parameter.
             * We need to map to the variable whose closest grain to this one is furthest away (by centroids).
             */
            std::vector<Real> min_distances(_vars.size(), std::numeric_limits<Real>::max());
            for (std::map<unsigned int, UniqueGrain *>::iterator grain_it3 = _unique_grains.begin(); grain_it3 != _unique_grains.end(); ++grain_it3)
            {
              unsigned int curr_var_idx = grain_it3->second->variable_idx;
              
              Real curr_centroid_diff = _mesh.minPeriodicDistance(grain_it1->second->centroid, grain_it3->second->centroid);
              if (curr_centroid_diff < min_distances[curr_var_idx])
                min_distances[curr_var_idx] = curr_centroid_diff;
            }

            /**
             * We have a vector of the distances to the closest grains represented by each of our variables.  We just need to pick
             * the maximum of this this list: (max of the mins).
             */
            unsigned int variable_idx = std::distance(min_distances.begin(), std::max_element(min_distances.begin(), min_distances.end()));

            std::cout << "Grain #: " << grain_it1->first << " intersects Grain #: " << grain_it2->first << "\n";
            std::cout << "Remapping to: " << variable_idx << " whose closest grain is at a distance of " << min_distances[variable_idx] << "\n";
            
            // Remap the grain
            {
              unsigned int curr_var_idx = grain_it1->second->variable_idx;
              for (std::set<unsigned int>::const_iterator node_it = grain_it1->second->nodes_ptr->begin();
                   node_it != grain_it1->second->nodes_ptr->end(); ++node_it)
              {
                Node * curr_node = _mesh.node_ptr(*node_it);

                if (curr_node->processor_id() == libMesh::processor_id())
                {
                  _subproblem.reinitNode(curr_node, 0);
                  {
                    VariableValue & value = _vars[curr_var_idx]->nodalSln();
                    VariableValue & value_old = _vars[curr_var_idx]->nodalSlnOld();
                    VariableValue & value_older = _vars[curr_var_idx]->nodalSlnOlder();
                  
                    // Copy Value from intersecting variable to new variable
                    unsigned int & dof_index = _vars[variable_idx]->nodalDofIndex();

                    // Set the only DOF for this variable on this node
                    solution.set(dof_index, value[0]);
                    solution_old.set(dof_index, value_old[0]);
                    solution_older.set(dof_index, value_older[0]);
                  }
                  {
                    VariableValue & value = _vars[variable_idx]->nodalSln();
                    VariableValue & value_old = _vars[variable_idx]->nodalSlnOld();
                    VariableValue & value_older = _vars[variable_idx]->nodalSlnOlder();
                  
                    // Copy Value from intersecting variable to new variable
                    unsigned int & dof_index = _vars[curr_var_idx]->nodalDofIndex();

                    // Set the only DOF for this variable on this node
                    solution.set(dof_index, value[0]);
                    solution_old.set(dof_index, value_old[0]);
                    solution_older.set(dof_index, value_older[0]);
                  }
                  
                  // Set the value in the intersecting variable to zero
//                  unsigned int & from_dof_index = _vars[curr_var_idx]->nodalDofIndex();
//                  solution.set(from_dof_index, 0.0);
//                  solution_old.set(from_dof_index, 0.0);
//                  solution_older.set(from_dof_index, 0.0);
                }
              }
              // Update the variable index in the unique grain datastructure
              grain_it1->second->variable_idx = variable_idx;
//              grain_it1->second->nodes_ptr = grain_it2->second->nodes_ptr;
//              // Update other data structures that will be effected by this remapping
//              _region_counts[variable_idx]++;
//              _region_counts[curr_var_idx]--;
//              updateRegionOffsets();

              solution.close();
              solution_old.close();
              solution_older.close();
              
              _fe_problem.getNonlinearSystem().sys().update();
            }
          }
        }
      }
    }
  }
}

void
GrainTracker::updateNodeInfo()
{
  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
    _bubble_maps[map_num].clear();

  // DEBUG
//  std::cout << "******************* UPDATE NODE INFO *********************************\n";
//  std::cout << "Unique Grain Size: " << _unique_grains.size() << "\n";
  // DEBUG
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
    if (grain_it->second->status != INACTIVE)
      for (std::set<unsigned int>::const_iterator node_it = grain_it->second->nodes_ptr->begin();
           node_it != grain_it->second->nodes_ptr->end(); ++node_it)
      {
        const Node & curr_node = _mesh.node(*node_it);
        _bubble_maps[_single_map_mode ? 0 : curr_var][curr_node.id()] = grain_it->first;
      }
  }
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
