#ifndef GRAINTRACKER_H
#define GRAINTRACKER_H

#include "NodalFloodCount.h"

// libMesh includes
#include "mesh_tools.h"
#include "auto_ptr.h"

class GrainTracker;
class GeneratedMesh;

template<>
InputParameters validParams<GrainTracker>();

class GrainTracker : public NodalFloodCount
{
public:
  GrainTracker(const std::string & name, InputParameters parameters);
  virtual ~GrainTracker();

  virtual void initialize();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();

  // Get the bubble map
  Real getNodeValue(unsigned int node_id) const;

protected:
  enum STATUS
  {
    NOT_MARKED,
    MARKED,
    INACTIVE
  };

  /**
   * This class holds the nodesets and bounding boxes for each
   * flooded region.
   */
  class BoundingBoxInfo
  {
  public:
    BoundingBoxInfo(unsigned int node_id, const RealVectorValue & trans_vector, const Point & min, const Point & max);

    unsigned int member_node_id;
    MeshTools::BoundingBox *b_box;
    RealVectorValue translation_vector;
  };

  /**
   * This class holds all the information we need to identify
   * a unique grain
   */
  class UniqueGrain
  {
  public:
    UniqueGrain(unsigned int var_idx, const std::vector<BoundingBoxInfo *> & b_box_ptrs, const Point & p_centroid);
    ~UniqueGrain();

    unsigned int variable_idx;
    Point centroid;
    std::vector<BoundingBoxInfo *> box_ptrs;
    STATUS status;
  };


  void buildBoundingBoxes();
  void trackGrains();
  Point calculateCentroid(const std::vector<BoundingBoxInfo *> & box_ptrs) const;

  unsigned int _tracking_step;
  std::list<BoundingBoxInfo *> _bounding_boxes;
  std::map<unsigned int, UniqueGrain *> _unique_grains;
  std::map<unsigned int, unsigned int> _region_to_grain;

  /**
   * Since PBCs always map both directions we will have to pick one and ignore the other
   * for the purpose of calculating a centroid.  We'll stick the first one we encounter
   * into this map so we know which direction we prefer.
   */
  std::map<unsigned int, unsigned int> _prefered_pb_pair;
  std::map<unsigned int, unsigned int> _nonprefered_pb_pair;

  /// A reference to the nonlinear system
  NonlinearSystem & _nl;

  GeneratedMesh *_gen_mesh;
};

#endif
