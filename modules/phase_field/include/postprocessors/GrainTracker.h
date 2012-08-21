#ifndef GRAINTRACKER_H
#define GRAINTRACKER_H

#include "NodalFloodCount.h"

// libMesh includes
#include "mesh_tools.h"

class GrainTracker;

template<>
InputParameters validParams<GrainTracker>();

class GrainTracker : public NodalFloodCount
{
public:
  GrainTracker(const std::string & name, InputParameters parameters);
  virtual ~GrainTracker();

  virtual void finalize();

  // Get the bubble map
  Real getNodeValue(unsigned int node_id) const;

protected:
  /**
   * This class holds all the information we need to identify
   * a unique grain
   */
  class UniqueGrain
  {
  public:
    UniqueGrain(const Point & min, const Point & max, unsigned int idx);

    unsigned int variable_idx;
    Point centroid;
    MeshTools::BoundingBox box;
  };

  void buildBoundingBoxes();

  std::map<unsigned int, UniqueGrain *> _unique_grains;
  std::map<unsigned int, unsigned int> _region_to_grain;
};

#endif
