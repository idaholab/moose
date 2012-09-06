#ifndef GRAINTRACKER_H
#define GRAINTRACKER_H

#include "NodalFloodCount.h"

// libMesh includes
#include "mesh_tools.h"
#include "auto_ptr.h"

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
   * This class holds the nodesets and bounding boxes for each
   * flooded region.
   */
  class BoundingBoxInfo
  {
  public:
    BoundingBoxInfo(unsigned int node_id, const Point & min, const Point & max);

    unsigned int member_node_id;
    MeshTools::BoundingBox *b_box;
  };

  /**
   * This class holds all the information we need to identify
   * a unique grain
   */
  class UniqueGrain
  {
  public:
    UniqueGrain(unsigned int var_idx, const std::vector<MeshTools::BoundingBox *> & b_box_ptrs);
    ~UniqueGrain();

    unsigned int variable_idx;
    Point centroid;
    std::vector<MeshTools::BoundingBox *> box_ptrs;
  };


  void buildBoundingBoxes();
  void trackGrains();

  unsigned int _tracking_step;
  std::list<BoundingBoxInfo> _bounding_boxes;
  std::map<unsigned int, UniqueGrain *> _unique_grains;
  std::map<unsigned int, unsigned int> _region_to_grain;
};

#endif
