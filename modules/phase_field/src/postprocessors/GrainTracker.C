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
  unpack(_packed_data);                      // No need to exchange data in parallel, it's already on each processor from the previous gather
  mergeSets();

  trackGrains();
}


void
GrainTracker::buildBoundingBoxes()
{
  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  MeshBase & mesh = _mesh._mesh;

  for (std::list<BubbleData>::const_iterator it1 = _bubble_sets.begin(); it1 != _bubble_sets.end(); ++it1)
  {
    Point min( 1.e30,  1.e30,  1.e30);
    Point max(-1.e30, -1.e30, -1.e30);
    unsigned int some_node_id = *(it1->_nodes.begin());

    for (std::set<unsigned int>::iterator it2 = it1->_nodes.begin(); it2 != it1->_nodes.end(); ++it2)
      for (unsigned int i=0; i<mesh.spatial_dimension(); ++i)
      {
        min(i) = std::min(min(i), mesh.point(*it2)(i));
        max(i) = std::max(max(i), mesh.point(*it2)(i));
      }

    _bounding_boxes.push_back(BoundingBoxInfo(some_node_id, min, max));
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

  // Used to keep track of which bounding box indexes have been used by which unique_grains
  std::vector<bool> used_idx(_bounding_boxes.size(), false);

  for (std::list<BubbleData>::const_iterator it1 = _bubble_sets.begin(); it1 != _bubble_sets.end(); ++it1)
  {
    std::vector<MeshTools::BoundingBox *> box_ptrs;
    unsigned int curr_var = it1->_var_idx;

    for (std::list<BoundingBoxInfo>::iterator it2 = _bounding_boxes.begin(); it2 != _bounding_boxes.end(); /* No increment here! */)
    {
      /**
       * See which of the bounding boxes belong to the current region (bubble set) by looking at a
       * member node id.  A single region may have multiple bounding boxes as members if it spans
       * periodic boundaries
       */
      if (it1->_nodes.find(it2->member_node_id) != it1->_nodes.end())
      {
        // Transfer ownership of the bounding box to "box_ptrs" which will be stored in the unique grain
        box_ptrs.push_back(it2->b_box);
        // Now delete the current BoundingBox structure so that it won't be inspected or reused
        _bounding_boxes.erase(it2++);
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
    {
      _unique_grains[counter] = new UniqueGrain(curr_var, box_ptrs);
      _region_to_grain[counter] = counter;
    }
    else // See if we can match up new grains with the existing grains
    {
      std::map<unsigned int, UniqueGrain *>::iterator grain_it;
      bool found_it = false;
      for (grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
      {
        // Look for matching variable indexes first
        // TODO: Look at centroids, bounding boxes, and other data
        // TODO: Make sure we don't overwrite the same grain twice in this loop
        if (grain_it->second->variable_idx == curr_var)
        {
          found_it = true;
          break;
        }
      }

      if (!found_it)
        mooseError("Couldn't find a matching grain");

      // Now we want to update the grain information
      delete grain_it->second;
      // add the new
      grain_it->second = new UniqueGrain(curr_var, box_ptrs);

      _region_to_grain[counter] = grain_it->first;
    }
    ++counter;
  }

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

// BoundingBoxInfo
GrainTracker::BoundingBoxInfo::BoundingBoxInfo(unsigned int node_id, const Point & min, const Point & max) :
    member_node_id(node_id),
    b_box(new MeshTools::BoundingBox(min, max))
{}

// Unique Grain
GrainTracker::UniqueGrain::UniqueGrain(unsigned int var_idx, const std::vector<MeshTools::BoundingBox *> & b_box_ptrs) :
    variable_idx(var_idx),
    box_ptrs(b_box_ptrs)
{}

GrainTracker::UniqueGrain::~UniqueGrain()
{
  for (unsigned int i=0; i<box_ptrs.size(); ++i)
    delete box_ptrs[i];
}
