//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppConservativeTransfer.h"
#include "KDTree.h"
#include "PointIndexedMap.h"

#include "libmesh/generic_projector.h"
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"
#include "libmesh/mesh_function.h"
#include "libmesh/parallel_algebra.h" // for communicator send and receive stuff

class Positions;
class MeshDivision;

/**
 * It is a general field transfer. It will do the following things
 * 1) From part of source domain to part of domain. Support subdomains/boundaries to
 *    subdomains/boundaries, mixing as appropriate
 * 2) interpolation and extrapolation, as appropriate
 * 3) Support higher order FEM
 * 4) Support mixed orders between source and target variables
 * 5) Support both distributed and replicated meshes
 * 6) Support both origin and target displaced meshes
 * 7) Support siblings transfers
 * 8) Support multiple child apps in both the transfer source and target
 */
class MultiAppGeneralFieldTransfer : public MultiAppConservativeTransfer
{
public:
  static InputParameters validParams();

  MultiAppGeneralFieldTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void getAppInfo() override;
  virtual void execute() override;
  virtual void postExecute() override;

  /// Get the source variable name, with the suffix for array/vector variables
  VariableName getFromVarName(unsigned int var_index);

  /// Get the target variable name, with the suffix for array/vector variables
  VariableName getToVarName(unsigned int var_index);

protected:
  /// Siblings transfers fully supported
  virtual void checkSiblingsTransferSupported() const override {}

  /*
   * Prepare evaluation of interpolation values
   * @param var_index index of the variable & component to prepare for. This routine is called once
   *        for each variable and component to transfer
   */
  virtual void prepareEvaluationOfInterpValues(const unsigned int var_index) = 0;

  /*
   * Evaluate interpolation values for incoming points
   * @param incoming_points vector of point requests with an additional integer to add constraints
   *                  on the source regions
   * @param outgoing_vals vector of (evaluated value, distance to value location) for each of the
   *                incoming point requests
   */
  virtual void
  evaluateInterpValues(const std::vector<std::pair<Point, unsigned int>> & incoming_points,
                       std::vector<std::pair<Real, Real>> & outgoing_vals) = 0;

  /*
   * Local from bounding boxes for current processor
   * @param local_bboxes vector of the local (to this process) bounding boxes
   *        for the source applications
   */
  void extractLocalFromBoundingBoxes(std::vector<BoundingBox> & local_bboxes);

  /*
   * Whether all source mesh checks pass on the given points:
   * - within the source mesh bounding box
   * - inside source block restriction
   * - inside source boundary restriction / in an element near the origin boundary restriction
   * - inside source mesh division and at the same index as in the target mesh division
   * - inside app mesh (if not already known to be inside a block or near a boundary)
   * @param i_from the index of the source problem/mesh
   * @param local_bboxes the bounding boxes for the local applications
   * @param pt the point to consider (in reference frame, not source or target frame)
   * @param mesh_div index of the point to consider in the target mesh division.
   *        If mesh divisions are used to match regions between two meshes, then the index
   *        must match in the target and source mesh divisions
   * @param distance distance from the point to the target mesh division. This may be used to favor
   *        a point over another, for example in a nearest-positions algorithm where we want
   *        to report the distance of the point pt to the closest position of the 1-NN partition
   */
  bool acceptPointInOriginMesh(unsigned int i_from,
                               const std::vector<BoundingBox> & local_bboxes,
                               const Point & pt,
                               const unsigned int mesh_div,
                               Real & distance) const;

  /*
   * Whether or not a given point is within the mesh of an origin (from) app
   * @param pl locator for the mesh of the source app
   * @param pt point in the local coordinates of the source app we're considering
   */
  bool inMesh(const libMesh::PointLocatorBase * const pl, const Point & pt) const;

  /*
   * Whether or not a given element is part of the given blocks
   * Passing the mesh is useful to override the definition of the block restriction
   */
  bool inBlocks(const std::set<SubdomainID> & blocks, const Elem * elem) const;
  virtual bool
  inBlocks(const std::set<SubdomainID> & blocks, const MooseMesh & mesh, const Elem * elem) const;

  /*
   * Whether or not a given node is part of an element in the given blocks
   * @param blocks list of blocks to examine.
   * @param mesh containing the element and the node
   * @param node node to consider
   * A node is in a block if an element it is part of is in that block
   */
  bool
  inBlocks(const std::set<SubdomainID> & blocks, const MooseMesh & mesh, const Node * node) const;

  /*
   * Whether or not a given point is part of an element in the given blocks
   * @param blocks list of blocks to examine
   * @param point locator to find the point in the blocks
   * @param pt point to examine, in the local coordinates (same as the point locator)
   */
  bool inBlocks(const std::set<SubdomainID> & blocks,
                const libMesh::PointLocatorBase * const pl,
                const Point & pt) const;

  /*
   * Whether or not a given node is part of the given boundaries
   */
  bool onBoundaries(const std::set<BoundaryID> & boundaries,
                    const MooseMesh & mesh,
                    const Node * node) const;

  /*
   * Whether or not a given element is near the specified boundaries
   * Depending on the '_elemental_boundary_restriction_on_sides' this can mean it shares a side or
   * a node with the boundary
   */
  bool onBoundaries(const std::set<BoundaryID> & boundaries,
                    const MooseMesh & mesh,
                    const Elem * elem) const;

  /*
   * Whether or not a point is inside an element that is near the specified boundaries
   * See onBoundaries(bd, mesh, elem) for definition of near
   * @param boundaries boundaries of interest for whether the point is near one
   * @param block_restriction limits the size of the mesh to search for the point.
   *        Note: an empty set means ALL blocks should be considered
   * @param mesh the mesh to look for the boundaries in
   * @param pl the point locator that searches the mesh
   * @param pt the point we want to know whether it is close to a boundary (local coordinates)
   */
  bool onBoundaries(const std::set<BoundaryID> & boundaries,
                    const std::set<SubdomainID> & block_restriction,
                    const MooseMesh & mesh,
                    const libMesh::PointLocatorBase * const pl,
                    const Point & pt) const;

  /**
   * Whether a point lies inside the mesh division delineated by the MeshDivision object
   * @param pt point to examine, in the local coordinates (source frame for from_direction=true)
   * @param i_local the index of the problem to consider, holding the mesh division to examine
   * @param only_from_this_mesh_div a mesh division index that must be matched when the
   * to/from_mesh_division_behavior for the direction examined is MATCH_DIVISION/SUBAPP_INDEX
   * It is ignored otherwise
   */
  bool acceptPointMeshDivision(const Point & pt,
                               const unsigned int i_local,
                               const unsigned int only_from_this_mesh_div) const;

  /**
   * Whether a point is closest to a position at the index specified than any other position
   * @param pos_index the index of the position to consider in the positions vector
   * @param pt the point
   * @return whether the point is closest to this position than any other in the positions vector
   */
  bool closestToPosition(unsigned int pos_index, const Point & pt) const;

  /// Origin array/vector variable components
  const std::vector<unsigned int> _from_var_components;

  /// Target array/vector variable components
  const std::vector<unsigned int> _to_var_components;

  /// Whether to use bounding boxes to determine the applications that may receive point requests
  /// then send value data, and at other various checks
  const bool _use_bounding_boxes;

  /// Whether to keep track of the distance from the requested point to the app position
  const bool _use_nearest_app;
  // NOTE: Keeping track of that distance is not optimal efficiency-wise, because we could find the
  //       closest app and only query the point from there. However, if the value from the closest
  //       app is invalid and the second closest is valid, then the results can vary in parallel
  //       If both apps are the same rank, the closest app is used, the point has an invalid value
  //       If each app are on a different rank, the second closest return a valid value, it gets
  //       used

  // Positions object to use to match target points and origin points as closest to the same
  // Position
  const Positions * _nearest_positions_obj;

  /// Whether the source app mesh must actually contain the points for them to be considered or whether
  /// the bounding box is enough. If false, we can interpolate between apps
  bool _source_app_must_contain_point;

  /// Origin block(s) restriction
  std::set<SubdomainID> _from_blocks;

  /// Target block(s) restriction
  std::set<SubdomainID> _to_blocks;

  /// Target boundary(ies) restriction
  std::set<BoundaryID> _to_boundaries;

  /// Origin boundary(ies) restriction
  std::set<BoundaryID> _from_boundaries;

  /// Division of the origin mesh
  std::vector<const MeshDivision *> _from_mesh_divisions;

  /// Division of the target mesh
  std::vector<const MeshDivision *> _to_mesh_divisions;

  /// How to use the origin mesh divisions to restrict the transfer
  const MooseEnum & _from_mesh_division_behavior;

  /// How to use the target mesh divisions to restrict the transfer
  const MooseEnum & _to_mesh_division_behavior;

  /// Matching enum for the mesh division behaviors.
  enum MeshDivisionTransferUse
  {
    RESTRICTION,
    MATCH_DIVISION_INDEX,
    MATCH_SUBAPP_INDEX
  };

  /// Whether elemental variable boundary restriction is considered by element side or element nodes
  const bool _elemental_boundary_restriction_on_sides;

  /// Point locators, useful to examine point location with regards to domain restriction
  std::vector<std::unique_ptr<libMesh::PointLocatorBase>> _from_point_locators;

  /// First app each processor owns, indexed by processor
  /// If no app on the processor, will have a -1 for the app start instead
  std::vector<unsigned int> _global_app_start_per_proc;

  /// Whether or not a greedy strategy will be used
  /// If true, all the partitions will be checked for a given
  /// outgoing point
  bool _greedy_search;

  /// Whether to look for conflicts between origin points, multiple valid values for a target point
  bool _search_value_conflicts;

  /// Whether we already output the search value conflicts
  bool _already_output_search_value_conflicts;

  /// How many conflicts are output to console
  const unsigned int _search_value_conflicts_max_log;

  /**
   * @brief Detects whether two source values are valid and equidistant for a desired target
   * location
   * @param value_1 value from the first value source / subapp
   * @param value_2 value from the second value source / subapp
   * @param distance_1 distance from the first source
   * @param distance_2 distance from the second source
   * @return true if the values are different and distances from the source points/apps are the same
   */
  bool detectConflict(Real value_1, Real value_2, Real distance_1, Real distance_2) const;

  /**
   * Register a potential value conflict, e.g. two or more equidistant source points for a single
   * target point, with different values possible
   * @param problem problem ID for the point of interest.
   *        For local conflicts, use origin problem id, for received conflicts, use target id
   * @param dof_id id id of the DoF is transferring a DoF. If not, use -1
   * @param p point where the conflict happens
   * @param dist distance between the origin and the target
   * @param local if true, local conflict found when gathering data to send, if false,
   *        received value conflict found when receiving data from multiple source problems
   */
  void registerConflict(unsigned int problem, dof_id_type dof_id, Point p, Real dist, bool local);

private:
  /// The target variables
  std::vector<MooseVariableFieldBase *> _to_variables;

  /// A map from pid to a set of points
  typedef std::unordered_map<processor_id_type, std::vector<std::pair<Point, unsigned int>>>
      ProcessorToPointVec;

  /// Point information
  struct PointInfo
  {
    unsigned int problem_id;   // problem id
    dof_id_type dof_object_id; // node or elem id
  };

  /// InterpInfo
  struct InterpInfo
  {
    processor_id_type pid; // Processor id type
    Real interp;           // Interpolation
    Real distance;         // distance from target to source
  };

  /// A map from pid to a set of point info
  typedef std::unordered_map<processor_id_type, std::vector<PointInfo>> ProcessorToPointInfoVec;

  /// A vector, indexed by to-problem id, of maps from dof object to interpolation values
  typedef std::vector<std::unordered_map<dof_id_type, InterpInfo>> DofobjectToInterpValVec;

  /// A map from Point to interpolation values
  /// NOTE: this is not an asynchronous cache. It is built to completion during the transfer
  ///       and used as a whole to reconstruct the target variable
  typedef PointIndexedMap InterpCache;

  /// A vector of such caches, indexed by to_problem
  typedef std::vector<InterpCache> InterpCaches;

  /// The number of variables to transfer
  unsigned int _var_size;

  /// Error out when some points can not be located
  bool _error_on_miss;

  /// Value to use when no received data is valid for a target location
  const Real _default_extrapolation_value;

  /// How much we should relax bounding boxes
  Real _bbox_factor;

  /// Set the bounding box sizes manually
  std::vector<Real> _fixed_bbox_size;

  /// Number of source/from applications per processor. This vector is indexed by processor id
  std::vector<unsigned int> _froms_per_proc;

  /// Bounding boxes for all source applications. The indexing of this vector is similar to
  ///'processor_id * n_local_subapps + i_local_subapp', except the number of local subapps can
  /// be different on each processor. Use the _from_per_proc vector to find the start/end index for a given
  /// processor. See MultiAppGeneralFieldTransfer::locatePointReceivers() for an example
  std::vector<BoundingBox> _from_bboxes;

  /// A map from processor to pointInfo vector
  ProcessorToPointInfoVec _processor_to_pointInfoVec;

  /// Keeps track of all local equidistant points to requested points, creating an indetermination
  /// in which values should be sent for that request
  /// We keep the origin problem ID, the dof ID, the point, and the distance origin-target
  /// If using nearest-positions the origin problem ID is not set
  std::vector<std::tuple<unsigned int, dof_id_type, Point, Real>> _local_conflicts;

  /// Keeps track of all received conflicts. Multiple problems (different subapps for example)
  /// are sending values for a target point that do not match and are equally valid/distant
  /// We keep the target problem ID, the point/dof ID, the point, and the origin-target distance.
  /// The distance indicates whether a potential conflict ended up materializing
  std::vector<std::tuple<unsigned int, dof_id_type, Point, Real>> _received_conflicts;

  /**
   * Initialize supporting attributes like bounding boxes, processor app indexes etc
   */
  void prepareToTransfer();

  /**
   * Performs the transfer for the variable of index i
   */
  void transferVariable(unsigned int i);

  /*
   * Extract to-points for which we need to compute interpolation values on the source domains
   */
  void extractOutgoingPoints(const unsigned int var_index, ProcessorToPointVec & outgoing_points);

  /*
   * Which processors include this point
   */
  void locatePointReceivers(const Point point, std::set<processor_id_type> & processors);

  /*
   * cache incoming values
   */
  void cacheIncomingInterpVals(
      processor_id_type pid,
      const unsigned int var_index,
      std::vector<PointInfo> & pointInfoVec,
      const std::vector<std::pair<Point, unsigned int>> & point_requests,
      const std::vector<std::pair<Real, Real>> & incoming_vals,
      DofobjectToInterpValVec & dofobject_to_valsvec, // for nodal + constant monomial
      InterpCaches & interp_caches,                   // for higher order elemental values
      InterpCaches & distance_caches);                // same but helps make origin point decisions

  /**
   * Remove potential value conflicts that did not materialize because another source was closer
   * Several equidistant valid values were received, but they were not closest
   * @param var_index the index of the variable of interest
   * @param dofobject_to_valsvec a data structure mapping dofobjects to received values and
   * distances (used for nodal-value-dof-only variables and constant monomials)
   * @param distance_caches a cache holding the distances received (used for higher order elemental
   * variables)
   */
  void examineReceivedValueConflicts(const unsigned int var_index,
                                     const DofobjectToInterpValVec & dofobject_to_valsvec,
                                     const InterpCaches & distance_caches);

  /**
   * Remove potential value conflicts that did not materialize because another source was closer
   * Several equidistant valid values were found when computing values to send, but they were not
   * closest, another value got selected
   * @param var_index the index of the variable of interest
   * @param dofobject_to_valsvec a data structure mapping dofobjects to received values and
   * distances (used for nodal-value-dof-only variables and constant monomials)
   * @param distance_caches a cache holding the distances received (used for higher order elemental
   * variables)
   */
  void examineLocalValueConflicts(const unsigned int var_index,
                                  const DofobjectToInterpValVec & dofobject_to_valsvec,
                                  const InterpCaches & distance_caches);

  /// Report on conflicts between overlapping child apps, equidistant origin points etc
  void outputValueConflicts(const unsigned int var_index,
                            const DofobjectToInterpValVec & dofobject_to_valsvec,
                            const InterpCaches & distance_caches);

  /*
   * Set values to solution
   * @param var the variable to set
   * @param dofobject_to_valsvec a vector of maps from DoF to values, for each to_problem
   *                             Used for nodal + constant monomial variables
   * @param interp_caches a vector of maps from point to value, for each to_problem
   *                      Used for higher order elemental variables
   */
  void setSolutionVectorValues(const unsigned int var_index,
                               const DofobjectToInterpValVec & dofobject_to_valsvec,
                               const InterpCaches & interp_caches);

  /*
   * Cache pointInfo
   */
  void cacheOutgoingPointInfo(const Point point,
                              const dof_id_type dof_object_id,
                              const unsigned int problem_id,
                              ProcessorToPointVec & outgoing_points);

  /**
   * Compute minimum distance
   * @param p the point of interest
   * @param bbox the bounding box to find the minimum distance from
   */
  Real bboxMinDistance(const Point & p, const BoundingBox & bbox) const;

  /**
   * Compute max distance
   * @param p the point of interest
   * @param bbox the bounding box to find the maximum distance from
   */
  Real bboxMaxDistance(const Point & p, const BoundingBox & bbox) const;

  /**
   * @brief Obtains the max dimensions to scale all points in the mesh
   * @return the maximum dimension in each coordinate axis of all target problems
   */
  Point getMaxToProblemsBBoxDimensions() const;

  /**
   * Get from bounding boxes for given domains and boundaries
   */
  std::vector<BoundingBox> getRestrictedFromBoundingBoxes() const;

  /**
   * Get global index for the first app each processes owns
   * Requires a global communication, must be called on every domain simultaneously
   */
  std::vector<unsigned int> getGlobalStartAppPerProc() const;
};

// Anonymous namespace for data, functors to use with GenericProjector.
namespace GeneralFieldTransfer
{
// Transfer::OutOfMeshValue is an actual number.  Why?  Why!
static_assert(std::numeric_limits<Real>::has_infinity,
              "What are you trying to use for Real?  It lacks infinity!");
extern Number BetterOutOfMeshValue;

inline bool
isBetterOutOfMeshValue(Number val)
{
  // Might need to be changed for e.g. NaN
  return val == GeneralFieldTransfer::BetterOutOfMeshValue;
}

// We need two functors that record point (value and gradient,
// respectively) requests, so we know what queries we need to make
// to other processors

/**
 * Value request recording base class
 */
template <typename Output>
class RecordRequests
{
protected:
  typedef typename libMesh::TensorTools::MakeBaseNumber<Output>::type DofValueType;

public:
  typedef typename libMesh::TensorTools::MakeReal<Output>::type RealType;
  typedef DofValueType ValuePushType;
  typedef Output FunctorValue;

  RecordRequests() {}

  RecordRequests(RecordRequests & primary) : _primary(&primary) {}

  ~RecordRequests()
  {
    if (_primary)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      _primary->_points_requested.insert(
          _primary->_points_requested.end(), _points_requested.begin(), _points_requested.end());
    }
  }

  void init_context(libMesh::FEMContext &) {}

  Output eval_at_node(const libMesh::FEMContext &,
                      unsigned int /*variable_index*/,
                      unsigned int /*elem_dim*/,
                      const Node & n,
                      bool /*extra_hanging_dofs*/,
                      const Real /*time*/)
  {
    _points_requested.push_back(n);
    return 0;
  }

  Output eval_at_point(const libMesh::FEMContext &,
                       unsigned int /*variable_index*/,
                       const Point & n,
                       const Real /*time*/,
                       bool /*skip_context_check*/)
  {
    _points_requested.push_back(n);
    return 0;
  }

  bool is_grid_projection() { return false; }

  void eval_mixed_derivatives(const libMesh::FEMContext & /*c*/,
                              unsigned int /*i*/,
                              unsigned int /*dim*/,
                              const Node & /*n*/,
                              std::vector<Output> & /*derivs*/)
  {
    mooseError("Not implemented");
  } // this is only for grid projections

  void eval_old_dofs(
      const Elem &, unsigned int, unsigned int, std::vector<dof_id_type> &, std::vector<Output> &)
  {
    mooseError("Not implemented");
  }

  void eval_old_dofs(const Elem &,
                     const libMesh::FEType &,
                     unsigned int,
                     unsigned int,
                     std::vector<dof_id_type> &,
                     std::vector<Output> &)
  {
    mooseError("Not implemented");
  }

  std::vector<Point> & points_requested() { return _points_requested; }

private:
  /// Vector of points requested
  std::vector<Point> _points_requested;

  RecordRequests * _primary = nullptr;
};

// We need a null action functor to use
// with them (because we won't be ready to set any values at that point)
template <typename Val>
class NullAction
{
public:
  typedef Val InsertInput;

  NullAction() {}

  void insert(dof_id_type, Val) {}

  void insert(const std::vector<dof_id_type> &, const DenseVector<Val> &) {}
};

// We need two functors that respond to point (value and gradient,
// respectively) requests based on the cached values of queries answered by
// other processors.

/**
 * Value request response base class
 */
template <typename Output>
class CachedData
{
protected:
  typedef typename libMesh::TensorTools::MakeBaseNumber<Output>::type DofValueType;

public:
  typedef PointIndexedMap Cache;

  typedef typename libMesh::TensorTools::MakeReal<Output>::type RealType;
  typedef DofValueType ValuePushType;
  typedef Output FunctorValue;

  /**
   * Constructor
   * @param cache a map/cache to search for points in
   * @param backup a function that can be queried for a point value when the cache doesnt have it
   */
  CachedData(const Cache & cache, const libMesh::FunctionBase<Output> & backup, Real default_value)
    : _cache(cache), _backup(backup.clone()), _default_value(default_value)
  {
  }

  /// Copy constructor
  CachedData(const CachedData & primary)
    : _cache(primary._cache),
      _backup(primary._backup->clone()),
      _default_value(primary._default_value)
  {
  }

  void init_context(libMesh::FEMContext &) {}

  /// Gets a value at the node location
  Output eval_at_node(const libMesh::FEMContext &,
                      unsigned int /*i*/,
                      unsigned int /*elem_dim*/,
                      const Node & n,
                      bool /*extra_hanging_dofs*/,
                      const Real /*time*/)
  {
    auto it = _cache.find(n);
    if (it == _cache.end())
    {
      if (_default_value != GeneralFieldTransfer::BetterOutOfMeshValue)
        return _default_value;
      else
        return (*_backup)(n);
    }
    else
      return it->second;
  }

  /// Gets a value at a point
  Output eval_at_point(const libMesh::FEMContext &,
                       unsigned int /*i*/,
                       const Point & n,
                       const Real /*time*/,
                       bool /*skip_context_check*/)
  {
    auto it = _cache.find(n);
    if (it == _cache.end())
    {
      if (_default_value != GeneralFieldTransfer::BetterOutOfMeshValue)
        return _default_value;
      else
        return (*_backup)(n);
    }
    else
      return it->second;
  }

  bool is_grid_projection() { return false; }

  void eval_mixed_derivatives(const libMesh::FEMContext & /*c*/,
                              unsigned int /*i*/,
                              unsigned int /*dim*/,
                              const Node & /*n*/,
                              std::vector<Output> & /*derivs*/)
  {
    mooseError("Not implemented");
  } // this is only for grid projections

  void eval_old_dofs(
      const Elem &, unsigned int, unsigned int, std::vector<dof_id_type> &, std::vector<Output> &)
  {
    mooseError("Not implemented");
  }

  void eval_old_dofs(const Elem &,
                     const libMesh::FEType &,
                     unsigned int,
                     unsigned int,
                     std::vector<dof_id_type> &,
                     std::vector<Output> &)
  {
    mooseError("Not implemented");
  }

private:
  /// Data to return for cached points
  const Cache & _cache;

  /// Function to evaluate for uncached points
  std::unique_ptr<libMesh::FunctionBase<Output>> _backup;

  /// Default value when no point is found
  const Real _default_value;
};

}
