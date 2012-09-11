#include "GrainTracker.h"
#include "MooseMesh.h"
#include "AddV.h"

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
    _tracking_step(getParam<unsigned int>("tracking_step"))
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

  unsigned int region_id = _bubble_map.at(node_id);
  return _region_to_grain.at(region_id);
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

  buildBoundingBoxes();                      // Build bounding box information
  pack(_packed_data, true);                  // Pack the data again but this time add periodic neighbor information
  Parallel::allgather(_packed_data, false);
  unpack(_packed_data);
  mergeSets();

  trackGrains();
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
  for (std::list<BubbleData>::const_iterator it1 = _bubble_sets.begin(); it1 != _bubble_sets.end(); ++it1)
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

          PeriodicBoundary *pb = _pbs->boundary(nodeset_it->first);
          mooseAssert(pb != NULL, "Error Periodic Boundary is NULL in GrainTracker");

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
          Point boundary_point = mesh.point(*nodeset_it->second.begin());
          Point corresponding_point = pb->get_corresponding_pos(boundary_point);

          translation_vector += boundary_point - corresponding_point;
        }
      }
    }

    // DEBUG
    std::cout << "Bounding Box: (" << min << ", " << max << ")\n";
    // DEBUG
    _bounding_boxes.push_back(new BoundingBoxInfo(some_node_id, translation_vector, min, max));
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
  for (std::list<BubbleData>::const_iterator it1 = _bubble_sets.begin(); it1 != _bubble_sets.end(); ++it1)
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
          Real curr_centroid_diff = (grain_it->second->centroid - curr_centroid).size();
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

  for (std::map<unsigned int, UniqueGrain *>::iterator grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
    if (grain_it->second->status == NOT_MARKED)
      grain_it->second->status = INACTIVE;

  // Check to make sure that we consumed all of the bounding box datastructures
  if (!_bounding_boxes.empty())
    mooseError("BoundingBoxes where not completely used by the GrainTracker");

  //DEBUG
//   std::cout << "TimeStep: " << _t_step << "\n";
//   std::cout << "Unique Grain Size: " << _unique_grains.size() << "\n";
//   for (std::map<unsigned int, UniqueGrain *>::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
//   {
//     std::cout << "Unique Number: " << it->first
//               << "\nVariable idx: " << it->second->variable_idx
//               << "\nBounding Boxes:\n";
//       for (unsigned int i=0; i<it->second->box_ptrs.size(); ++i)
//         std::cout << it->second->box_ptrs[i]->min() << it->second->box_ptrs[i]->max() << "\n";
//   }
  //DEBUG
}

Point
GrainTracker::calculateCentroid(const std::vector<BoundingBoxInfo *> & box_ptrs) const
{
  Point centroid;
  for (std::vector<BoundingBoxInfo *>::const_iterator it = box_ptrs.begin(); it != box_ptrs.end(); ++it)
  {
    Point curr_adj_centroid = (((*it)->b_box->max() + (*it)->b_box->min()) / 2.0) + (*it)->translation_vector;

    centroid += curr_adj_centroid;
  }
  centroid /= box_ptrs.size();

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
