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

  params.suppressParameter<std::vector<VariableName> >("variable");
  
  return params;
}

GrainTracker::GrainTracker(const std::string & name, InputParameters parameters) :
    NodalFloodCount(name, AddV(parameters, "variable"))
{
  AddV(parameters, "variable").print();
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
  NodalFloodCount::finalize();

  buildBoundingBoxes();

// DEBUG
//  for (std::map<unsigned int, unsigned int>::iterator it = _region_to_grain.begin(); it != _region_to_grain.end(); ++it)
//    std::cout << it->first << ": " << it->second << "\n";

//  for (std::map<unsigned int, UniqueGrain *>::iterator it = _unique_grains.begin(); it != _unique_grains.end(); ++it)
//    std::cout << it->first << ": " << it->second->variable_idx << "\t" << it->second->centroid << "\n";
// DEBUG
}

void
GrainTracker::buildBoundingBoxes()
{
  bool first_time = false;
  if (!_unique_grains.size())
    first_time = true;

  MeshBase & mesh = _mesh._mesh;

  unsigned int counter=1;
  for (std::list<BubbleData>::const_iterator it1 = _bubble_sets.begin(); it1 != _bubble_sets.end(); ++it1)
  {
    Point min( 1.e30,  1.e30,  1.e30);
    Point max(-1.e30, -1.e30, -1.e30);
    unsigned int curr_var = it1->_var_idx;

    for (std::set<unsigned int>::iterator it2 = it1->_nodes.begin(); it2 != it1->_nodes.end(); ++it2)
      for (unsigned int i=0; i<mesh.spatial_dimension(); ++i)
      {
        min(i) = std::min(min(i), mesh.point(*it2)(i));
        max(i) = std::max(max(i), mesh.point(*it2)(i));
      }

    // If it's the first time through, we will generate the unique grain numbers using counter
    if (first_time)
    {
      _unique_grains[counter] = new UniqueGrain(min, max, curr_var);
      _region_to_grain[counter] = counter;
    }
    else
    {
      // Now see if we can match up old grains with new grains with existing grains
      std::map<unsigned int, UniqueGrain *>::iterator grain_it;
      bool found_it = false;
      for (grain_it = _unique_grains.begin(); grain_it != _unique_grains.end(); ++grain_it)
      {
        // Look for matching variable indexes first
        // TODO: Look at centroids, bounding boxes, and other data
        // TODO: Make sure we don't overright the same grain twice in this loop
        if (grain_it->second->variable_idx == curr_var)
        {
          found_it = true;
          break;
        }
      }

      if (!found_it)
        mooseError("Couldn't find a matching grain");

      // delete the old
      delete grain_it->second;
      // add the new
      grain_it->second = new UniqueGrain(min, max, curr_var);

      _region_to_grain[counter] = grain_it->first;
    }
    ++counter;
  }
}

// Unique Grain
GrainTracker::UniqueGrain::UniqueGrain(const Point & min, const Point & max, unsigned int idx) :
    variable_idx(idx),
    centroid(max - min),
    box(min, max)
{}
