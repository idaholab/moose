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

  params.suppressParameter<std::vector<VariableName> >("variable");

  return params;
}

GrainTracker::GrainTracker(const std::string & name, InputParameters parameters) :
    NodalFloodCount(name, AddV(parameters, "variable")),
    _tracking_step(getParam<unsigned int>("tracking_step")),
    _nl(static_cast<FEProblem &>(_subproblem).getNonlinearSystem()),
    _gen_mesh(dynamic_cast<GeneratedMesh *>(&_mesh))
{
  _region_to_grain[0] = 0; // Zero indicates no grain - we need a place holder for this one postion
}

GrainTracker::~GrainTracker()
{
  for (std::map<unsigned int, UniqueGrain *>::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
    delete it->second;
}

Real
GrainTracker::getNodeValue(unsigned int node_id) const
{
  if (_t_step < _tracking_step)
    return 0;

  // When running threaded - we won't have the complete map
  // since we don't exchange areas marked with zero.  In this
  // case we can just return zero for nodes NOT in the map
  if (_bubble_maps[0].find(node_id) == _bubble_maps[0].end())   // Single mode ONLY
    return 0;

  unsigned int region_id = _bubble_maps[0].at(node_id);
  return _region_to_grain.at(region_id);
}

void
GrainTracker::initialize()
{
  NodalFloodCount::initialize();

  _gen_mesh->initPeriodicDistanceForVariable(_nl, _var_number);
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

  remapGrains();
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

      _bounding_boxes.push_back(new BoundingBoxInfo(some_node_id, translation_vector, min, max));
    }
  }
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
  std::vector<bool> used_idx(_bounding_boxes.size(), false);

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

      for (std::list<BoundingBoxInfo *>::iterator it2 = _bounding_boxes.begin(); it2 != _bounding_boxes.end(); /* No increment here! */)
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
          _bounding_boxes.erase(it2++);
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
        _unique_grains[counter] = new UniqueGrain(curr_var, box_ptrs, curr_centroid);
        _region_to_grain[counter] = counter;
      }
      else // See if we can match up new grains with the existing grains
      {
        std::map<unsigned int, UniqueGrain *>::iterator grain_it, closest_match;
        bool found_one = false;
        Real min_centroid_diff = std::numeric_limits<Real>::max();
        for (grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
        {
          // Skip over inactive grains
          if (grain_it->second->status == INACTIVE)
            continue;

          // Look for matching variable indexes first
          if (grain_it->second->variable_idx == curr_var)
          {
            // Now check to see how close this centroid is to the previous centroid
            Real curr_centroid_diff = _gen_mesh->minPeriodicDistance(grain_it->second->centroid, curr_centroid);

//          // DEBUG
//          std::cout << "Centroids: " << grain_it->second->centroid << curr_centroid << " Diff: " <<  curr_centroid_diff << "\n";
//          // DEBUG

            if (curr_centroid_diff <= min_centroid_diff)
            {
              found_one = true;
              closest_match = grain_it;
              min_centroid_diff = curr_centroid_diff;
              grain_it->second->status = MARKED;
              // TODO: Make sure we don't overwrite the same grain twice in this loop
            }
          }
        }

        if (!found_one)
          mooseError("Couldn't find a matching grain");

        // Now we want to update the grain information
        delete closest_match->second;
        // add the new
        closest_match->second = new UniqueGrain(curr_var, box_ptrs, curr_centroid);

        _region_to_grain[counter] = closest_match->first;
      }
      ++counter;
    }
  }

  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
    if (grain_it->second->status == NOT_MARKED)
      grain_it->second->status = INACTIVE;

  // Check to make sure that we consumed all of the bounding box datastructures
  if (!_bounding_boxes.empty())
    mooseError("BoundingBoxes where not completely used by the GrainTracker");

  //DEBUG
   std::cout << "TimeStep: " << _t_step << "\n";
   std::cout << "Unique Grain Size: " << _unique_grains.size() << "\n";
   for (std::map<unsigned int, UniqueGrain *>::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
   {
     std::cout << "Unique Number: " << it->first
               << "\nVariable idx: " << it->second->variable_idx
               << "\nBounding Boxes:\n";
       for (unsigned int i=0; i<it->second->box_ptrs.size(); ++i)
         std::cout << it->second->box_ptrs[i]->b_box->min() << it->second->box_ptrs[i]->b_box->max() << "\n";
   }
  //DEBUG
}

void
GrainTracker::remapGrains()
{
  /**
   * Loop over each grain and see if the bounding boxes of the current grain intersect with the boxes of any other grains
   * represented by the same variable.
   */
  Point p;
  switch (_gen_mesh->dimension())
  {
  case 1:
    mooseError("1D is not supported");
  case 2:
    p.assign(Point(15, 15, 0));
  case 3:
    p.assign(Point(15, 15, 15));
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
        box1.min() -= p;  
        box1.max() += p;
        
        for (std::vector<BoundingBoxInfo *>::iterator box_it2 = grain_box2.begin(); box_it2 != grain_box2.end(); ++box_it2)
        {
          MeshTools::BoundingBox box2 = *(*box_it2)->b_box;
          box2.min() -= p;  
          box2.max() += p;
          
          if (box1.intersect(box2) &&                                               // Do the boxes intersect
              grain_it1->second->variable_idx == grain_it2->second->variable_idx)   // and are the represented by the same variable?
          {
            /**
             * If we get into this case, that means that we have two grains that are getting close represented by the same order parameter.
             * We need to map to the variable furthest away from this one (by centroids).
             */
            Real max_distance = 0;
            Real variable_idx = std::numeric_limits<unsigned int>::max();
            for (std::map<unsigned int, UniqueGrain *>::iterator grain_it3 = _unique_grains.begin(); grain_it3 != _unique_grains.end(); ++grain_it3)
            {
              // Skip grains that are represented by this variable, you can't remap to yourself!
              if (grain_it1->second->variable_idx == grain_it3->second->variable_idx)
                continue;

              Real curr_centroid_diff = _gen_mesh->minPeriodicDistance(grain_it1->second->centroid, grain_it3->second->centroid);
              if (max_distance < curr_centroid_diff)
              {
                max_distance = curr_centroid_diff;
                variable_idx = grain_it1->second->variable_idx;
              }
            }

            std::cout << "Grain #: " << grain_it1->first << " intersects Grain #: " << grain_it2->first << "\n";
            std::cout << "Remapping to: " << variable_idx << " at a distance of " << max_distance << "\n";
            /**
             * At this point we've found a grain that needs to be remapped.  We need to flood all of it's values again with a lower tolerance
             * than we may have used previously.  There really isn't any good way around this.  We'd have to maintain multiple bubble maps
             * for multiple order parameters if we wanted to flood them all completely and then we'd have overlapping regions in series of maps.
             */

//            // Now we need to reflood this particular grain so that we can remap all of its values to another variable
//            std::map<unsigned int, int> local_grain_map;
//            for (std::vector<BoundingBoxInfo *>::iterator box_it3 = grain_box1.begin(); box_it1 != grain_box1.end(); ++box_it1)
//            {
//              // retrieve a Node * from the mesh based for each bounding box
//              Node &node = _mesh.node((*box_it3)->member_node_id);
// 
//              // TODO: Figure out how to remark all nodes in this grain in parallel
//              //       The member_node_id is the first id in the _bubble_sets data structure.
//              //       We should be able to loop over the _bubble_sets looking the the matching set
//              //       of nodes.
//              //
//              //       In parallel we can use this information to fully seed the information we need
//              //       to reflood (actually we probably can't - DAMNIT)
//            }
// 
//            //DEBUG
//            std::cout << "The Following Nodes will be remapped:\n";
//            for (std::map<unsigned int, int>::iterator node_it = local_grain_map.begin(); node_it != local_grain_map.end(); ++node_it)
//            {
//              if (node_it->second == 1)
//                std::cout << node_it->first << " ";
//            }
//            std::cout << "\n";
//            //DEBUG

            // TODO : Make sure we aren't intersecting still
          }
        }
      }
    }
  }
}

/*
void
GrainTracker::reflood(const Node *node, std::map<unsigned int, int> &bubble_map, int current_idx)
{
  if (node == NULL)
    return;
  
  unsigned int node_id = node->id();

  // Has this node already been marked? - if so move along
  if (bubble_map.find(node_id) != bubble_map.end())
    return;

  
  //  This node hasn't been marked - check to see if it in a bubble.
  //  If current_idx is set (>= zero) then we are in the process of marking a bubble.
  if (_vars[current_idx]->getNodalValue(*node) < 1e-5)
  {
    // No - mark and return
    _bubble_map[node_id] = 0;
    return;
  }

  std::vector< const Node * > neighbors;
  MeshTools::find_nodal_neighbors(_mesh._mesh, *node, _nodes_to_elem_map, neighbors);
  // Important!  If this node doesn't have any neighbors (i.e. floating node in the center
  // of an element) we need to just unmark it for now since it can't be easiliy flooded
  if (neighbors.size() == 0)
  {
    _bubble_map[node_id] = 0;
    return;
  }

  // Yay! A bubble -> Mark it! (If region is zero, that signifies that this is a new bubble)
  bubble_map[node_id] = 1;

  // Flood neighboring nodes that are also above this threshold with recursion
  for (unsigned int i=0; i<neighbors.size(); ++i)
    // Only recurse on nodes this processor owns
    if (isNodeValueValid(neighbors[i]->id()))
      reflood(neighbors[i], bubble_map, current_idx);
}
*/

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
    if (centroid(i) < _gen_mesh->getMinInDimension(i))
      centroid(i) += _gen_mesh->dimensionWidth(i);
    else if (centroid(i) > _gen_mesh->getMaxInDimension(i))
      centroid(i) -= _gen_mesh->dimensionWidth(i);
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
GrainTracker::UniqueGrain::UniqueGrain(unsigned int var_idx, const std::vector<BoundingBoxInfo *> & b_box_ptrs, const Point & p_centroid) :
    variable_idx(var_idx),
    centroid(p_centroid),
    box_ptrs(b_box_ptrs),
    status(MARKED)
{}

GrainTracker::UniqueGrain::~UniqueGrain()
{
  for (unsigned int i=0; i<box_ptrs.size(); ++i)
  {
    delete box_ptrs[i]->b_box;
    delete box_ptrs[i];
  }
}
