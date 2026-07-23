//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseTypes.h"
#include "MooseArray.h"
#include "MooseFunctor.h"
#include "Function.h"
#include "Kernel.h"
#include "MooseVariableDependencyInterface.h"
#include "TaggingInterface.h"
#include "ThreeMaterialPropertyInterface.h"

#include "libmesh/vector_value.h"
#include <vector>

template <typename>
class MooseVariableFE;
class SystemBase;
class MooseObject;
class MooseVariableDependencyInterface;
class TransientInterface;

/**
 * Method-neutral assembly data and operations for a two-field hybridized DG discretization.
 *
 * The two fields are an element-interior scalar and its facet trace. Derived helpers provide the
 * method-specific volume and face weak forms.
 */
class HDGAssemblyHelper : public ThreeMaterialPropertyInterface
{
public:
  static InputParameters validParams();

  HDGAssemblyHelper(const MooseObject * moose_obj,
                    MooseVariableDependencyInterface * mvdi,
                    const TransientInterface * ti,
                    SystemBase & sys,
                    const Assembly & assembly,
                    THREAD_ID tid,
                    const std::set<SubdomainID> & blocks_ids,
                    const std::set<BoundaryID> & boundary_ids);

  /// Resizes the element-interior and facet residual vectors for the current element.
  void resizeResiduals();

  /// Assembles the element-interior equation's volume contribution.
  virtual void scalarVolume() = 0;

  /// Assembles the element-interior equation's face contribution.
  virtual void scalarFace() = 0;

  /// Assembles the facet equation's interior-face contribution.
  virtual void lmFace() = 0;

  /// Assembles the element-interior equation on a Dirichlet boundary.
  virtual void scalarDirichlet(const Moose::Functor<Real> & dirichlet_value) = 0;

  /// Enforces a Dirichlet value in the facet equation.
  void lmDirichlet(const Moose::Functor<Real> & dirichlet_value);

  /// Applies a prescribed flux in the facet equation.
  void lmPrescribedFlux(const Moose::Functor<Real> & flux_value);

  /**
   * @returns The residuals and degree of freedom indices on which this helper operates
   */
  std::array<ADResidualsPacket, 2> taggingData() const;

  /**
   * @returns The facet variable as a set
   */
  std::set<std::string> additionalROVariables();

  virtual ~HDGAssemblyHelper() = default;

protected:
  /// Transient state used to evaluate boundary functors.
  const TransientInterface & _ti;

  /// Element-interior scalar variable.
  const MooseVariableFE<Real> & _u_var;

  /// Facet trace variable.
  const MooseVariableFE<Real> & _u_face_var;

  /// Degree-of-freedom indices for the element-interior scalar.
  const std::vector<dof_id_type> & _u_dof_indices;

  /// Degree-of-freedom indices for the facet trace.
  const std::vector<dof_id_type> & _lm_u_dof_indices;

  /// Element-interior scalar values at quadrature points.
  const MooseArray<ADReal> & _u_sol;

  /// Element-interior scalar gradients at quadrature points.
  const MooseArray<ADRealVectorValue> & _grad_u_sol;

  /// Facet trace values at face quadrature points.
  const MooseArray<ADReal> & _lm_u_sol;

  /// Element-interior scalar test functions.
  const MooseArray<std::vector<Real>> & _scalar_phi;

  /// Gradients of the element-interior scalar test functions.
  const MooseArray<std::vector<RealVectorValue>> & _grad_scalar_phi;

  /// Element-interior scalar test functions evaluated on a face.
  const MooseArray<std::vector<Real>> & _scalar_phi_face;

  /// Element-interior scalar test-function gradients evaluated on a face.
  const MooseArray<std::vector<RealVectorValue>> & _grad_scalar_phi_face;

  /// Facet trace test functions evaluated on a face.
  const MooseArray<std::vector<Real>> & _lm_phi_face;

  /// Current element volume.
  const Real & _elem_volume;

  /// Current side area.
  const Real & _side_area;

  /// Current element.
  const Elem * const & _current_elem;

  /// Current element-side index.
  const unsigned int & _current_side;

  /// Current side element.
  const Elem * const & _current_side_elem;

  /// Element quadrature weights including the transformed Jacobian.
  const MooseArray<Real> & _JxW;

  /// Element quadrature rule.
  const QBase * const & _qrule;

  /// Physical element quadrature points.
  const MooseArray<Point> & _q_point;

  /// Face quadrature weights including the transformed Jacobian.
  const MooseArray<Real> & _JxW_face;

  /// Face quadrature rule.
  const QBase * const & _qrule_face;

  /// Physical face quadrature points.
  const MooseArray<Point> & _q_point_face;

  /// Outward unit normals on the current face.
  const MooseArray<Point> & _normals;

  /// Residual for the element-interior scalar equation.
  DenseVector<ADReal> _scalar_re;

  /// Residual for the facet trace equation.
  DenseVector<ADReal> _lm_re;
};

inline void
HDGAssemblyHelper::resizeResiduals()
{
  _scalar_re.resize(_u_dof_indices.size());
  _lm_re.resize(_lm_u_dof_indices.size());
}
