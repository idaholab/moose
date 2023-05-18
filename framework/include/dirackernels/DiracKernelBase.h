//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "DiracKernelInfo.h"
#include "ResidualObject.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MaterialPropertyInterface.h"
#include "GeometricSearchInterface.h"
#include "MooseVariableField.h"
#include "MooseVariableInterface.h"
#include "BlockRestrictable.h"

/**
 * DiracKernelBase is the base class for all DiracKernel type classes.
 */
class DiracKernelBase : public ResidualObject,
                        public CoupleableMooseVariableDependencyIntermediateInterface,
                        public MaterialPropertyInterface,
                        protected GeometricSearchInterface,
                        public BlockRestrictable
{
public:
  static InputParameters validParams();

  DiracKernelBase(const InputParameters & parameters);

  /**
   * This gets called by computeOffDiagJacobian() at each quadrature point.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /**
   * Computes the off-diagonal Jacobian for variable jvar.
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override = 0;

  /**
   * This is where the DiracKernel should call addPoint() for each point it needs to have a
   * value distributed at.
   */
  virtual void addPoints() = 0;

  /**
   * Whether or not this DiracKernel has something to distribute on this element.
   */
  bool hasPointsOnElem(const Elem * elem);

  /**
   * Whether or not this DiracKernel has something to distribute at this Point.
   */
  bool isActiveAtPoint(const Elem * elem, const Point & p);

  /**
   * Remove all of the current points and elements.
   * NOTE: The points are still cached by id to find them fast
   */
  void clearPoints();

  /**
   * Clear the cache of points because the points may have moved
   */
  void clearPointsCaches();

  /**
   * Clear point cache when the mesh changes, so that element
   * coarsening, element deletion, and distributed mesh repartitioning
   * don't leave this with an invalid cache.
   */
  virtual void meshChanged() override { clearPointsCaches(); };

protected:
  /**
   * Add the physical x,y,z point located in the element "elem" to the list of points
   * this DiracKernel will be asked to evaluate a value at.
   */
  void addPoint(const Elem * elem, Point p, unsigned id = libMesh::invalid_uint);

  /**
   * This is a highly inefficient way to add a point where this DiracKernel needs to be
   * evaluated.
   *
   * This spawns a search for the element containing that point!
   */
  const Elem * addPoint(Point p, unsigned id = libMesh::invalid_uint);

  /**
   * Returns the user-assigned ID of the current Dirac point if it
   * exits, and libMesh::invalid_uint otherwise.  Can be used e.g. in
   * the computeQpResidual() function to determine the cached ID of
   * the current point, in case this information is relevant.
   */
  unsigned currentPointCachedID();

  ///< Current element
  const Elem * const & _current_elem;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  /// Place for storing Point/Elem information shared across all
  /// DiracKernel objects.
  DiracKernelInfo & _dirac_kernel_info;

  /// Place for storing Point/Elem information only for this DiracKernel
  DiracKernelInfo _local_dirac_kernel_info;

  /// The current point
  Point _current_point;

  /// Quadrature point index
  unsigned int _qp;
  /// Quadrature points
  const MooseArray<Point> & _q_point;
  /// Physical points
  const MooseArray<Point> & _physical_point;
  /// Quadrature rule
  const QBase * const & _qrule;
  /// Transformed Jacobian weights
  const MooseArray<Real> & _JxW;

  /// i-th, j-th index for enumerating shape and test functions
  unsigned int _i, _j;

  /// drop duplicate points or consider them in residual and Jacobian
  const bool _drop_duplicate_points;

  // @{ Point-not-found behavior
  enum class PointNotFoundBehavior
  {
    ERROR,
    WARNING,
    IGNORE
  };
  const PointNotFoundBehavior _point_not_found_behavior;
  // @}

  /// Whether Dirac sources can move during the simulation
  const bool _allow_moving_sources;

  /// Data structure for caching user-defined IDs which can be mapped to
  /// specific std::pair<const Elem*, Point> and avoid the PointLocator Elem lookup.
  typedef std::map<unsigned, std::pair<const Elem *, Point>> point_cache_t;
  point_cache_t _point_cache;

  /// Map from Elem* to a list of (Dirac point, id) pairs which can be used
  /// in a user's computeQpResidual() routine to determine the user-defined ID for
  /// the current Dirac point, if one exists.
  typedef std::map<const Elem *, std::vector<std::pair<Point, unsigned>>> reverse_cache_t;
  reverse_cache_t _reverse_point_cache;

private:
  /// This function is used internally when the Elem for a
  /// locally-cached point needs to be updated.  You must pass in a
  /// pointer to the old_elem whose data is to be updated, the
  /// new_elem to which the Point belongs, and the Point and id
  /// information.
  void updateCaches(const Elem * old_elem, const Elem * new_elem, Point p, unsigned id);

  /// A helper function for addPoint(Point, id) for when
  /// id != invalid_uint.
  const Elem * addPointWithValidId(Point p, unsigned id);
};
