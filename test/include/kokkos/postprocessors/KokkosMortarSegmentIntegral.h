//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosGeneralPostprocessor.h"

class AutomaticMortarGeneration;

/**
 * Integrates the product of a variable evaluated on the secondary and on the primary side of a
 * mortar interface over its mortar segments. The variable is evaluated on the device at the
 * reference points of each segment quadrature point on the lower-dimensional secondary and
 * primary elements, reading the primary element across processes, and the integral is checked
 * against the same evaluation through libMesh on the host. The dual basis coefficients of the
 * variable on each secondary element are computed on the device from the segment quadrature and
 * checked against those libMesh computes on the host.
 */
class KokkosMortarSegmentIntegral : public Moose::Kokkos::GeneralPostprocessor
{
public:
  static InputParameters validParams();

  KokkosMortarSegmentIntegral(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void compute() override;
  virtual void finalize() override;
  virtual PostprocessorValue getValue() const override { return _integral; }

  KOKKOS_FUNCTION void operator()(const Moose::Kokkos::ThreadID qp, Real & sum) const;

  /// Kokkos function tag of the dual coefficient computation
  struct DualCoeffLoop
  {
  };

  KOKKOS_FUNCTION void operator()(DualCoeffLoop, const Moose::Kokkos::ThreadID secondary) const;

private:
  /// The mortar interface
  const AutomaticMortarGeneration & _amg;
  /// The variable, defined on the lower-dimensional secondary and primary blocks
  const MooseVariableFieldBase & _moose_var;
  /// The variable
  Moose::Kokkos::Variable _var;
  /// The libMesh FE type of the variable
  const libMesh::FEType _fe_type;
  /// The Kokkos FE type ID of the variable
  unsigned int _fe_type_id;

  /**
   * Segment quadrature point data, indexed by the segment quadrature point
   */
  ///@{
  Moose::Kokkos::Array<ContiguousElementID> _secondary_elem;
  Moose::Kokkos::Array<ContiguousElementID> _primary_elem;
  Moose::Kokkos::Array<libMesh::ElemType> _secondary_type;
  Moose::Kokkos::Array<libMesh::ElemType> _primary_type;
  Moose::Kokkos::Array<Moose::Kokkos::Real3> _secondary_xi;
  Moose::Kokkos::Array<Moose::Kokkos::Real3> _primary_xi;
  Moose::Kokkos::Array<Real> _JxW;
  ///@}

  /**
   * Secondary element data for the dual basis, indexed by the local secondary element
   */
  ///@{
  /// Range of the segment quadrature points of each secondary element
  Moose::Kokkos::Array<dof_id_type> _secondary_qp_begin;
  /// Dual coefficients computed on the device, (i, j) of each secondary element flattened
  Moose::Kokkos::Array<Real> _dual_coeff;
  /// Dual coefficients computed by libMesh on the host, flattened like _dual_coeff
  std::vector<Real> _host_dual_coeff;
  ///@}

  /// Maximum number of shape functions of the variable on a secondary element
  static constexpr unsigned int max_shapes = 9;

  /// The integral evaluated on the device
  Real _integral = 0;
  /// The integral evaluated through libMesh on the host
  Real _host_integral = 0;
};
