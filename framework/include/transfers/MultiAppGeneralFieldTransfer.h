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
#include "libmesh/mesh_function.h"
#include "MooseHashing.h"
#include "KDTree.h"

#include "libmesh/generic_projector.h"
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel_algebra.h" // for communicator send and receive stuff

/**
 * It is a general field transfer. It will do the following things
 * 1) From part of source domain to part of domain. Support subdomains to
 *  subdomains
 * 2) Support vector vars and regular vars
 * 3) Support higher order FEM
 */
class MultiAppGeneralFieldTransfer : public MultiAppConservativeTransfer
{
public:
  static InputParameters validParams();

  MultiAppGeneralFieldTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void execute() override;

  // This needs to be moved into libMesh
  struct hash_point
  {
    std::size_t operator()(const Point & p) const
    {
      std::size_t seed = 0;
      Moose::hash_combine(seed, p(0), p(1), p(2));
      return seed;
    }
  };

protected:
  /*
   * Prepare evaluation of interpolation values
   */
  virtual void prepareEvaluationOfInterpValues(const VariableName & var_name) = 0;

  /*
   * Evaluate interpolation values for incoming points
   */
  virtual void evaluateInterpValues(const std::vector<Point> & incoming_points,
                                    std::vector<std::pair<Real, Real>> & outgoing_vals) = 0;

  /*
   * Local from bounding boxes for current processor
   */
  void extractLocalFromBoundingBoxes(std::vector<BoundingBox> & local_bboxes);

  /*
   * Whether or not a given element is part of the given blocks
   */
  bool hasBlocks(std::set<SubdomainID> & blocks, const Elem * elem) const;

  /*
   * Whether or not a given node is part of an element near the given blocks
   */
  bool hasBlocks(std::set<SubdomainID> & blocks, const MooseMesh & mesh, const Node * node) const;

  /*
   * Whether or not a given node is part of the given boundaries
   */
  bool
  hasBoundaries(std::set<BoundaryID> & boundaries, const MooseMesh & mesh, const Node * node) const;

  /*
   * Whether or not a given element has a side on the given boundaries
   */
  bool
  hasBoundaries(std::set<BoundaryID> & boundaries, const MooseMesh & mesh, const Elem * elem) const;

  /// Origin block(s) restriction
  std::set<SubdomainID> _from_blocks;

  /// Target block(s) restriction
  std::set<SubdomainID> _to_blocks;

  /// Target boundary(ies) restriction
  std::set<BoundaryID> _to_boundaries;

  /// Origin boundary(ies) restriction
  std::set<BoundaryID> _from_boundaries;

private:
  /// A map from pid to a set of points
  typedef std::unordered_map<processor_id_type, std::vector<Point>> ProcessorToPointVec;

  /// Point information
  struct PointInfor
  {
    unsigned int problem_id;   // problem id
    dof_id_type dof_object_id; // node or elem id
    dof_id_type offset;        // Useful when there are more than one point in a given dof object
  };

  /// InterpInfor
  struct InterpInfor
  {
    processor_id_type pid; // Processor id type
    Real interp;           // Interpolation
    Real distance;         // distance from target to source
  };

  /// A map from pid to a set of point info
  typedef std::unordered_map<processor_id_type, std::vector<PointInfor>> ProcessorToPointInforVec;

  /// A vector, indexed by to-problem id, of maps from dof object to interpolation values
  typedef std::vector<std::unordered_map<dof_id_type, InterpInfor>> DofobjectToInterpValVec;

  /// A map for caching a single variable's values
  typedef std::unordered_map<Point, Number, hash_point> InterpCache;

  /// A vector of such caches, indexed by to_problem
  typedef std::vector<InterpCache> InterpCaches;

  /// The number of variables to transfer
  unsigned int _var_size;

  /// Error out when some points can not be located
  bool _error_on_miss;

  /// How much we should relax bounding boxes
  Real _bbox_factor;

  /// Whether or not a greedy strategy will be used
  /// If true, all the partitions will be checked for a given
  /// outgoing point
  bool _greedy_search;

  /// Number of froms per processor
  std::vector<unsigned int> _froms_per_proc;

  /// Bounding boxes for all processors
  std::vector<BoundingBox> _bboxes;

  /// A map from processor to pointInfo vector
  ProcessorToPointInforVec _processor_to_pointInfoVec;

  /// Number of conflicts between points in the mesh
  unsigned int _num_overlaps;

  /**
   * Performs the transfer for the variable of index i
   */
  void transferVariable(unsigned int i);

  /*
   * Extract to-points for which we need to compute interpolation values on the source domains
   */
  void extractOutgoingPoints(const VariableName & var_name, ProcessorToPointVec & outgoing_points);

  /*
   * Which processors include this point
   */
  void locatePointReceivers(const Point point, std::set<processor_id_type> & processors);

  /*
   * cache incoming values
   */
  void cacheIncomingInterpVals(processor_id_type pid,
                               const VariableName & var_name,
                               std::vector<PointInfor> & pointInfoVec,
                               const std::vector<Point> & point_requests,
                               const std::vector<std::pair<Real, Real>> & incoming_vals,
                               DofobjectToInterpValVec & dofobject_to_valsvec,
                               InterpCaches & interp_caches);

  /*
   * Set values to solution
   */
  void setSolutionVectorValues(const VariableName & var,
                               const DofobjectToInterpValVec & dofobject_to_valsvec,
                               const InterpCaches & interp_caches);

  /*
   * Cache pointInfo
   */
  void cacheOutgoingPointInfor(const Point point,
                               const dof_id_type dof_object_id,
                               const unsigned int problem_id,
                               ProcessorToPointVec & outgoing_points);

  /*
   * Compute minimum distance
   */
  Real bboxMinDistance(const Point & p, const BoundingBox & bbox);

  /*
   * Compute max distance
   */
  Real bboxMaxDistance(const Point & p, const BoundingBox & bbox);

  /*
   * Get from bounding boxes for given domains and boundaries
   * */
  std::vector<BoundingBox> getRestrictedFromBoundingBoxes();
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
  typedef typename TensorTools::MakeBaseNumber<Output>::type DofValueType;

public:
  typedef typename TensorTools::MakeReal<Output>::type RealType;
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

  void init_context(FEMContext &) {}

  Output eval_at_node(const FEMContext &,
                      unsigned int libmesh_dbg_var(i),
                      unsigned int /*elem_dim*/,
                      const Node & n,
                      bool /*extra_hanging_dofs*/,
                      const Real /*time*/)
  {
    libmesh_assert_not_equal_to(i, 0);
    _points_requested.push_back(n);
    return 0;
  }

  Output eval_at_point(const FEMContext &,
                       unsigned int libmesh_dbg_var(i),
                       const Point & n,
                       const Real /*time*/,
                       bool /*skip_context_check*/)
  {
    libmesh_assert_not_equal_to(i, 0);
    _points_requested.push_back(n);
    return 0;
  }

  bool is_grid_projection() { return false; }

  void eval_mixed_derivatives(const FEMContext & /*c*/,
                              unsigned int /*i*/,
                              unsigned int /*dim*/,
                              const Node & /*n*/,
                              std::vector<Output> & /*derivs*/)
  {
    libmesh_error();
  } // this is only for grid projections

  void eval_old_dofs(
      const Elem &, unsigned int, unsigned int, std::vector<dof_id_type> &, std::vector<Output> &)
  {
    libmesh_error();
  }

  void eval_old_dofs(const Elem &,
                     const FEType &,
                     unsigned int,
                     unsigned int,
                     std::vector<dof_id_type> &,
                     std::vector<Output> &)
  {
    libmesh_error();
  }

  std::vector<Point> & points_requested() { return _points_requested; }

private:
  std::vector<Point> _points_requested;

  RecordRequests * _primary = nullptr;
};

// We need a null action functor to use
// with them (because we won't be ready to set any values at that
// point)
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
  typedef typename TensorTools::MakeBaseNumber<Output>::type DofValueType;

public:
  typedef std::unordered_map<Point, Output, MultiAppGeneralFieldTransfer::hash_point> Cache;

  typedef typename TensorTools::MakeReal<Output>::type RealType;
  typedef DofValueType ValuePushType;
  typedef Output FunctorValue;

  CachedData(const Cache & cache, const FunctionBase<Output> & backup)
    : _cache(cache), _backup(backup.clone())
  {
  }

  CachedData(const CachedData & primary) : _cache(primary._cache), _backup(primary._backup->clone())
  {
  }

  void init_context(FEMContext &) {}

  Output eval_at_node(const FEMContext &,
                      unsigned int /*i*/,
                      unsigned int /*elem_dim*/,
                      const Node & n,
                      bool /*extra_hanging_dofs*/,
                      const Real /*time*/)
  {
    auto it = _cache.find(n);
    if (it == _cache.end())
      return (*_backup)(n);
    else
      return it->second;
  }

  Output eval_at_point(const FEMContext &,
                       unsigned int /*i*/,
                       const Point & n,
                       const Real /*time*/,
                       bool /*skip_context_check*/)
  {
    auto it = _cache.find(n);
    if (it == _cache.end())
      return (*_backup)(n);
    else
      return it->second;
  }

  bool is_grid_projection() { return false; }

  void eval_mixed_derivatives(const FEMContext & /*c*/,
                              unsigned int /*i*/,
                              unsigned int /*dim*/,
                              const Node & /*n*/,
                              std::vector<Output> & /*derivs*/)
  {
    libmesh_error();
  } // this is only for grid projections

  void eval_old_dofs(
      const Elem &, unsigned int, unsigned int, std::vector<dof_id_type> &, std::vector<Output> &)
  {
    libmesh_error();
  }

  void eval_old_dofs(const Elem &,
                     const FEType &,
                     unsigned int,
                     unsigned int,
                     std::vector<dof_id_type> &,
                     std::vector<Output> &)
  {
    libmesh_error();
  }

private:
  // Data to return for cached points
  const Cache & _cache;

  // Function to evaluate for uncached points
  std::unique_ptr<FunctionBase<Output>> _backup;
};

}
