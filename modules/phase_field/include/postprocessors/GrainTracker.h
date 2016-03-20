/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GRAINTRACKER_H
#define GRAINTRACKER_H

#include "FeatureFloodCount.h"
#include "GrainTrackerInterface.h"

// libMesh includes
#include "libmesh/mesh_tools.h"
#include "libmesh/auto_ptr.h"
#include "libmesh/sphere.h"

class GrainTracker;
class EBSDReader;

template<>
InputParameters validParams<GrainTracker>();

class GrainTracker : public FeatureFloodCount, public GrainTrackerInterface
{
public:
  GrainTracker(const InputParameters & parameters);
  virtual ~GrainTracker();

  virtual void initialize();

  virtual void finalize();

  /**
   * Accessor for retrieving nodal field information (unique grains or variable indicies)
   * @param node_id the node identifier for which to retrieve field data
   * @param var_idx when using multi-map mode, the map number from which to retrieve data.
   * @param show_var_coloring pass true to view variable index for a region, false for unique grain information
   * @return the nodal value
   */
  virtual Real getEntityValue(dof_id_type node_id, FIELD_TYPE field_type, unsigned int var_idx=0) const;

  /**
   * Accessor for retrieving elemental field data (grain centroids).
   * @param element_id the element identifier for which to retrieve field data
   * @return the elemental value
   */
  virtual Real getElementalValue(dof_id_type element_id) const;

  /**
   * Returns a list of active unique grains for a particular elem based on the node numbering.  The outer vector
   * holds the ith node with the inner vector holds the list of active unique grains.
   * (unique_grain_id, variable_idx)
   */
  virtual const std::vector<std::pair<unsigned int, unsigned int> > & getElementalValues(dof_id_type elem_id) const;

  // Debugging routine used for printing grain data structure information
  void print();

  /// This struct holds the nodesets and bounding spheres for each flooded region.
  struct BoundingSphereInfo;

  /// This struct hold the information necessary to identify and track a unique grain;
  struct UniqueGrain;

protected:
  /// This routine is called at the of finalize to update the field data
  virtual void updateFieldInfo();

  /**
   * This method uses the bubble sets to build bounding spheres around each cluster of nodes.  In this class it will be called before periodic
   * information has been added so that each bubble piece will have a unique sphere.  It populates the _bounding_spheres vector.
   */
  void buildBoundingSpheres();

  /**
   * This method first finds all of the bounding spheres from the _bounding_spheres vector that belong to the same bubble by using the
   * overlapping periodic information (if periodic boundary conditions are active). If this is the first step that we beginning to track
   * grains, each of these sets of spheres and the centroid are used to designate a unique grain which is stored in the _unique_grains
   * datastructure.
   */
  void trackGrains();

  /**
   * This method is called after trackGrains to remap grains that are too close to each other.
   */
  void remapGrains();

  /**
   * This method swaps the values at all the nodes in grain_it1, with the values in grain_it2.
   */
  void swapSolutionValues(std::map<unsigned int, MooseSharedPointer<FeatureData> >::iterator & grain_it1,
                          std::map<unsigned int, MooseSharedPointer<FeatureData> >::iterator & grain_it2, unsigned int attempt_number);

  void swapSolutionValuesHelper(Node * curr_node, unsigned int curr_var_idx, unsigned int new_var_idx, NumericVector<Real> & solution, NumericVector<Real> & solution_old, NumericVector<Real> & solution_older);

  /**
   * This method returns the periodic distance between two bounding boxes.  If use_centroids_only is true, then the distance will be between the two
   * bounding box centers.  If ignore_radii is false, then the distance will be -1 or 1 depending on whether the intersect or not (respectively).
   */
  Real boundingRegionDistance(std::vector<MeshTools::BoundingBox> & bboxes1, std::vector<MeshTools::BoundingBox> bboxes2, bool use_centroids_only) const;

  Point centerOfMass(UniqueGrain & grain) const;

  void expandHalos();

  /**
   * Calculate the volume of each grain maintaining proper order and dumping results to CSV
   */
  virtual void calculateBubbleVolumes();

  /*************************************************
   *************** Data Structures *****************
   ************************************************/

  /// The timestep to begin tracking grains
  const int _tracking_step;

  /// The value added to each bounding sphere radius to detect earlier intersection
  const Real _hull_buffer;

  /// The thickness of the halo surrounding each grain
  const unsigned int _halo_level;

  /// Inidicates whether remapping should be done or not (remapping is independent of tracking)
  const bool _remap;

  /// A reference to the nonlinear system (used for retrieving solution vectors)
  NonlinearSystem & _nl;

  /// This data structure holds the raw lists of bounding spheres for each variable index.  It is used during the tracking routine
  std::vector<std::list<BoundingSphereInfo *> > _bounding_spheres;

  /// This data structure holds the map of unique grains.  The information is updated each timestep to track grains over time.
  std::map<unsigned int, MooseSharedPointer<FeatureData> > & _unique_grains;

  /**
   * This data structure holds unique grain to EBSD data map information. It's possible when using 2D scans of 3D microstructures
   * to end up with disjoint grains with the same orientation in a single slice. To properly handle this in the grain tracker
   * we need yet another map that takes a unique_grain number and retrieves the proper EBSD numbering (non-unique)
   */
  std::map<unsigned int, unsigned int> _unique_grain_to_ebsd_num;

  /// Optional ESBD Reader
  const EBSDReader * _ebsd_reader;

public:
  /// This struct holds the nodesets and bounding spheres for each flooded region.
  struct BoundingSphereInfo
  {
    BoundingSphereInfo(unsigned int node_id, const Point & center, Real radius);

    unsigned int member_node_id;
    libMesh::Sphere b_sphere;
  };

  /// This struct hold the information necessary to identify and track a unique grain;
//  struct UniqueGrain : public FeatureData
//  {
//    UniqueGrain(unsigned int var_idx, const std::vector<BoundingSphereInfo *> & b_sphere_ptrs, const std::set<dof_id_type> *nodes_pt, STATUS status);
//    ~UniqueGrain();
//
//    unsigned int variable_idx;
//    std::vector<BoundingSphereInfo *> sphere_ptrs;
//    STATUS status;
//    /**
//     * Pointer to the actual nodes ids.  Note: This pointer is not always valid.  It is invalid
//     * after new sets are built before "trackGrains" has been re-run.  This is intentional and lets us
//     * avoid making unnecessary copies of the set when we don't need it.
//     */
//    const std::set<dof_id_type> *entities_ptr;
//  };

  bool _compute_op_maps;

  bool _center_mass_tracking;

  /**
   * Data structure for active order parameter information on elements:
   * elem_id -> a vector of pairs each containing the grain number and the variable index representing that grain
   */
  std::map<dof_id_type, std::vector<std::pair<unsigned int, unsigned int> > > _elemental_data;

  std::ofstream _outfile;
};


template<> void dataStore(std::ostream & stream, GrainTracker::UniqueGrain * & unique_grain, void * context);
template<> void dataLoad(std::istream & stream, GrainTracker::UniqueGrain * & unique_grain, void * context);

template<> void dataStore(std::ostream & stream, GrainTracker::BoundingSphereInfo * & bound_sphere_info, void * context);
template<> void dataLoad(std::istream & stream, GrainTracker::BoundingSphereInfo * & bound_sphere_info, void * context);


#endif
