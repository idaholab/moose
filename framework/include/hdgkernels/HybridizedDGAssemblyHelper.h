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
class HybridizedDGAssemblyHelper : public ThreeMaterialPropertyInterface
{
public:
  static InputParameters validParams();

  HybridizedDGAssemblyHelper(const MooseObject * moose_obj,
                             MooseVariableDependencyInterface * mvdi,
                             const TransientInterface * ti,
                             SystemBase & sys,
                             const Assembly & assembly,
                             THREAD_ID tid,
                             const std::set<SubdomainID> & blocks_ids,
                             const std::set<BoundaryID> & boundary_ids);

  void resizeResiduals();
  virtual void scalarVolume() = 0;

  virtual void scalarFace() = 0;

  virtual void lmFace() = 0;

  virtual void scalarDirichlet(const Moose::Functor<Real> & dirichlet_value) = 0;

  void lmDirichlet(const Moose::Functor<Real> & dirichlet_value);

  void lmPrescribedFlux(const Moose::Functor<Real> & flux_value);

  /**
   * @returns The residuals and degree of freedom indices on which this helper operates
   */
  std::array<ADResidualsPacket, 2> taggingData() const;

  /**
   * @returns The facet variable as a set
   */
  std::set<std::string> additionalROVariables();

  virtual ~HybridizedDGAssemblyHelper() = default;

protected:
  const TransientInterface & _ti;
  const MooseVariableFE<Real> & _u_var;
  const MooseVariableFE<Real> & _u_face_var;

  const std::vector<dof_id_type> & _u_dof_indices;
  const std::vector<dof_id_type> & _lm_u_dof_indices;

  const MooseArray<ADReal> & _u_sol;
  const MooseArray<ADRealVectorValue> & _grad_u_sol;
  const MooseArray<ADReal> & _lm_u_sol;

  const MooseArray<std::vector<Real>> & _scalar_phi;
  const MooseArray<std::vector<RealVectorValue>> & _grad_scalar_phi;

  const MooseArray<std::vector<Real>> & _scalar_phi_face;
  const MooseArray<std::vector<RealVectorValue>> & _grad_scalar_phi_face;
  const MooseArray<std::vector<Real>> & _lm_phi_face;

  const Real & _elem_volume;
  const Real & _side_area;

  const Elem * const & _ip_current_elem;
  const unsigned int & _ip_current_side;
  const Elem * const & _ip_current_side_elem;

  const MooseArray<Real> & _ip_JxW;
  const QBase * const & _ip_qrule;
  const MooseArray<Point> & _ip_q_point;

  const MooseArray<Real> & _ip_JxW_face;
  const QBase * const & _ip_qrule_face;
  const MooseArray<Point> & _ip_q_point_face;
  const MooseArray<Point> & _ip_normals;

  DenseVector<ADReal> _scalar_re, _lm_re;
};

inline void
HybridizedDGAssemblyHelper::resizeResiduals()
{
  _scalar_re.resize(_u_dof_indices.size());
  _lm_re.resize(_lm_u_dof_indices.size());
}
