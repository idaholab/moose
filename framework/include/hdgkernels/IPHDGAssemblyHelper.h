//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
#include "ADFunctorInterface.h"

#include "libmesh/vector_value.h"
#include <vector>

template <typename>
class MooseVariableFE;
class MooseVariableScalar;
template <typename>
class MooseArray;
class SystemBase;
class MooseMesh;
class MooseObject;
class MooseVariableDependencyInterface;
class TransientInterface;

/**
 * Implements all the methods for assembling a hybridized interior penalty discontinuous Galerkin
 * (IPDG-H), which is a type of HDG method, discretization of the diffusion equation. These routines
 * may be called by both HDG kernels and integrated boundary conditions.
 */
class IPHDGAssemblyHelper : public ThreeMaterialPropertyInterface, public ADFunctorInterface
{
public:
  static InputParameters validParams();

  IPHDGAssemblyHelper(const MooseObject * const moose_obj,
                      MooseVariableDependencyInterface * const mvdi,
                      const TransientInterface * const ti,
                      SystemBase & sys,
                      const Assembly & assembly,
                      const THREAD_ID tid,
                      const std::set<SubdomainID> & blocks_ids,
                      const std::set<BoundaryID> & boundary_ids);

  void resizeResiduals();
  virtual void scalarVolume() = 0;

  //
  // Methods which can be leveraged both on internal sides in the kernel and by boundary conditions
  //

  virtual void scalarFace() = 0;

  virtual void lmFace() = 0;

  virtual void scalarDirichlet(const Moose::Functor<Real> & dirichlet_value) = 0;

  void lmDirichlet(const Moose::Functor<Real> & dirichlet_value);

  void lmPrescribedFlux(const Moose::Functor<Real> & flux_value);

  /**
   * @returns The various residuals and degree of freedom indices this helper operators on
   */
  std::array<ADResidualsPacket, 2> taggingData() const;

  /**
   * @returns The names of the primal interior and LM facet variable as a set
   */
  std::set<std::string> variablesCovered();

  virtual ~IPHDGAssemblyHelper() = default;

protected:
  const TransientInterface & _ti;
  const MooseVariableFE<Real> & _u_var;
  const MooseVariableFE<Real> & _u_face_var;

  // Containers for dof indices
  const std::vector<dof_id_type> & _u_dof_indices;
  const std::vector<dof_id_type> & _lm_u_dof_indices;

  // local solutions at quadrature points
  const MooseArray<ADReal> & _u_sol;
  const MooseArray<ADRealVectorValue> & _grad_u_sol;
  const MooseArray<ADReal> & _lm_u_sol;

  // Element shape functions
  const MooseArray<std::vector<Real>> & _scalar_phi;
  const MooseArray<std::vector<RealVectorValue>> & _grad_scalar_phi;

  // Face shape functions
  const MooseArray<std::vector<Real>> & _scalar_phi_face;
  const MooseArray<std::vector<RealVectorValue>> & _grad_scalar_phi_face;
  const MooseArray<std::vector<Real>> & _lm_phi_face;

  /// The current element volume
  const Real & _elem_volume;

  /// The current element side area
  const Real & _side_area;

  /// The current element
  const Elem * const & _ip_current_elem;

  /// The current element side
  const unsigned int & _ip_current_side;

  /// The element JxW
  const MooseArray<Real> & _ip_JxW;

  /// The element qrule
  const QBase * const & _ip_qrule;

  /// The physical quadrature point locations in the element volume
  const MooseArray<Point> & _ip_q_point;

  /// The face JxW
  const MooseArray<Real> & _ip_JxW_face;

  /// The face qrule
  const QBase * const & _ip_qrule_face;

  /// The physical quadrature point locations on the face
  const MooseArray<Point> & _ip_q_point_face;

  /// The normal vector on the face
  const MooseArray<Point> & _ip_normals;

  // Local residual vectors
  DenseVector<ADReal> _scalar_re, _lm_re;
};

inline void
IPHDGAssemblyHelper::resizeResiduals()
{
  _scalar_re.resize(_u_dof_indices.size());
  _lm_re.resize(_lm_u_dof_indices.size());
}
