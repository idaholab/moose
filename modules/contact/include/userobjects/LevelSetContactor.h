//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "libmesh/point.h"
#include "libmesh/tensor_value.h"
#include "libmesh/vector_value.h"

#include <array>

class Function;

/**
 * Base class for a rigid contactor described implicitly by a signed-distance
 * (level-set) function g_LS(x). Sign convention: g_LS > 0 outside the rigid
 * body (open gap), g_LS < 0 inside it. Outward normal n = grad(g_LS).
 *
 * The contactor may translate rigidly along each Cartesian axis by an
 * amount specified per-axis as either a MOOSE Function of time
 * (`disp_x_function`, etc.) or a coupled Scalar variable
 * (`disp_x_scalar`, etc.).  Displacement control drives one axis with a
 * Function; load control drives it with a Scalar variable whose value is
 * determined by a companion RigidBodyLoadControl scalar kernel.  Axes
 * with neither input default to zero -- so pre-existing displacement-
 * controlled inputs that leave the contactor stationary keep working
 * unchanged.
 *
 * All public query methods (signedDistance / normal / hessian / queryAt)
 * transform x -> x - translation() before delegating to the concrete
 * geometry's *Raw counterpart.
 */
class LevelSetContactor : public GeneralUserObject
{
public:
  static InputParameters validParams();
  LevelSetContactor(const InputParameters & parameters);

  /// Bundled result of all three quantities at a query point.  Callers that
  /// need more than one component at the same point should prefer queryAt()
  /// over the per-quantity accessors, because concrete contactors (notably
  /// SurfaceMeshContactor) can share expensive per-point work (KDTree lookup,
  /// point-in-solid classification) across the three quantities.
  struct Query
  {
    Real gap;
    RealVectorValue normal;
    RealTensorValue hessian;
  };

  // Public non-virtual API.  Applies the rigid-body translation and delegates
  // to the concrete *Raw method below.
  Real signedDistance(const Point & x) const { return signedDistanceRaw(x - translation()); }
  RealVectorValue normal(const Point & x) const { return normalRaw(x - translation()); }
  RealTensorValue hessian(const Point & x) const { return hessianRaw(x - translation()); }
  Query queryAt(const Point & x) const { return queryAtRaw(x - translation()); }

  /// Current rigid-body translation of the contactor.
  Point translation() const;
  /// Nonlinear system variable number for the scalar driving axis `k`, or
  /// `libMesh::invalid_uint` if that axis is Function-driven or defaulted.
  unsigned int translationScalarNumber(unsigned int k) const { return _scalar_var_num[k]; }

  virtual void initialize() override final {}
  virtual void execute() override final {}
  virtual void finalize() override final {}

protected:
  /// Raw geometry hooks -- concrete contactors implement these; they see the
  /// query point in the contactor's own (untranslated) frame.
  virtual Real signedDistanceRaw(const Point & x) const = 0;
  virtual RealVectorValue normalRaw(const Point & x) const = 0;
  virtual RealTensorValue hessianRaw(const Point &) const { return RealTensorValue(); }
  /// Default: forward to the three raw accessors.  Concrete contactors that
  /// can share per-point work should override this.
  virtual Query queryAtRaw(const Point & x) const
  {
    return {signedDistanceRaw(x), normalRaw(x), hessianRaw(x)};
  }

private:
  // Per-axis translation sources.  At most one of (_function[k], _scalar[k])
  // is non-null; both null means that axis stays at 0.  `_scalar_var_num[k]`
  // mirrors _scalar[k] but stays valid even after threading copies (the
  // VariableValue reference is what evaluates).
  std::array<const Function *, 3> _function;
  std::array<const VariableValue *, 3> _scalar;
  std::array<unsigned int, 3> _scalar_var_num;
};
