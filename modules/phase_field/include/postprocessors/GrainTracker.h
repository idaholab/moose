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
  Real getNodeValue(unsigned int node_id, unsigned int var_idx=0) const;

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
    UniqueGrain(unsigned int var_idx, const std::vector<BoundingBoxInfo *> & b_box_ptrs, const Point & p_centroid, const std::set<unsigned int> *nodes_pt);
    ~UniqueGrain();

    unsigned int variable_idx;
    Point centroid;
    std::vector<BoundingBoxInfo *> box_ptrs;
    STATUS status;
    /**
     * Pointer to the actual nodes ids.  Note: This pointer is not always valid.  It is invalid
     * after new sets are built before "trackGrains" has been re-run.  This is intentional and lets us
     * avoid making unnecessary copies of the set when we don't need it.
     */
    const std::set<unsigned int> *nodes_ptr;
  };


  void buildBoundingBoxes();
  void trackGrains();
  void remapGrains();
  void updateNodeInfo();
  Point calculateCentroid(const std::vector<BoundingBoxInfo *> & box_ptrs) const;

  const unsigned int _tracking_step;
  const Real _hull_buffer;
  std::vector<std::list<BoundingBoxInfo *> > _bounding_boxes;
  std::map<unsigned int, UniqueGrain *> _unique_grains;

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

  /// Inidicates whether remapping should be done or not
  const bool _remap;
};

#endif
