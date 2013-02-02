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

  virtual void initialize();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();

  // Retrieve field information
  virtual Real getNodalValue(unsigned int node_id, unsigned int var_idx=0, bool show_var_coloring=false) const;
  virtual Real getElementalValue(unsigned int element_id) const;

protected:
  enum STATUS
  {
    NOT_MARKED,
    MARKED,
    INACTIVE
  };

  /**
   * This class holds the nodesets and bounding spheres for each
   * flooded region.
   */
  class BoundingSphereInfo
  {
  public:
    BoundingSphereInfo(unsigned int node_id, const RealVectorValue & trans_vector, const Point & center, Real radius);

    unsigned int member_node_id;
    libMesh::Sphere *b_sphere;
//    RealVectorValue translation_vector;
  };

  /**
   * This class holds all the information we need to identify
   * a unique grain
   */
  class UniqueGrain
  {
  public:
    UniqueGrain(unsigned int var_idx, const std::vector<BoundingSphereInfo *> & b_sphere_ptrs, /*const Point & p_centroid,*/ const std::set<unsigned int> *nodes_pt);
    ~UniqueGrain();

    unsigned int variable_idx;
//    Point centroid;
    std::vector<BoundingSphereInfo *> sphere_ptrs;
    STATUS status;
    /**
     * Pointer to the actual nodes ids.  Note: This pointer is not always valid.  It is invalid
     * after new sets are built before "trackGrains" has been re-run.  This is intentional and lets us
     * avoid making unnecessary copies of the set when we don't need it.
     */
    const std::set<unsigned int> *nodes_ptr;
  };

  /**
   * This routine uses the bubble sets to build bounding spheres around each cluster of nodes.  In this class it will be called before periodic
   * information has been added so that each bubble piece will have a unique sphere.  It populates the _bounding_spheres vector.
   */
  void buildBoundingSpheres();

  /**
   * This rountine first finds all of the bounding spheres from the _bounding_spheres vector that belong to the same bubble by using the overlapping
   * periodic information (if periodic boundary conditions are active).  These spheres are used to caluclate a centroid for the current grain.
   *
   * If this is the first step that we beginning to track grains, each of these sets of spheres and the centroid are used to designate a unique grain
   * which is stored in the _unique_grains datastructure.
   */
  void trackGrains();
  void remapGrains();
  void swapSolutionValues(std::map<unsigned int, UniqueGrain *>::iterator & grain_it1, std::map<unsigned int, UniqueGrain *>::iterator & grain_it2);
  void updateNodeInfo();

  Real boundingRegionDistance(std::vector<BoundingSphereInfo *> & spheres1, std::vector<BoundingSphereInfo *> & spheres2) const;
//  Point calculateCentroid(const std::vector<BoundingSphereInfo *> & sphere_ptrs) const;
  Real minCentroidDiff(const std::vector<BoundingSphereInfo *> & sphere_ptrs1, const std::vector<BoundingSphereInfo *> & sphere_ptrs2) const;
  
  const int _tracking_step;
  const Real _hull_buffer;
  std::vector<std::list<BoundingSphereInfo *> > _bounding_spheres;
  std::map<unsigned int, UniqueGrain *> _unique_grains;

  std::set<std::pair<unsigned int, unsigned int> > _remapped_grains;

  /**
   * Since PBCs always map both directions we will have to pick one and ignore the other
   * for the purpose of calculating a centroid.  We'll stick the first one we encounter
   * into this map so we know which direction we prefer.
   */
  std::map<unsigned int, unsigned int> _prefered_pb_pair;
  std::map<unsigned int, unsigned int> _nonprefered_pb_pair;

  /// A reference to the nonlinear system
  NonlinearSystem & _nl;

  /// Inidicates whether remapping should be done or not
  const bool _remap;

  const GrainTracker * _grain_remapper;

public:
  const std::set<std::pair<unsigned int, unsigned int> > * getRemappedGrains() const
  {
    return &_remapped_grains;
  }
};

#endif
