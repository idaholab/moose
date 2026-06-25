//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_LIBTORCH_ENABLED

#include <torch/torch.h>

// MOOSE includes
#include "ElementUserObject.h"

/// This user object caches assembly information (JxWxT) from MOOSE as a batched at::Tensor for the
/// Torch FEM kernels.
class TorchAssembly : public ElementUserObject
{
public:
  static InputParameters validParams();

  TorchAssembly(const InputParameters & parameters);

  /// Number of active elements on this rank
  int64_t numElem() const { return _nelem; }

  /// Number of quadrature points per element
  int64_t numQP() const { return _nqp; }

  /**
   * @brief Get the cached JxWxT for each element, each quadrature point
   *
   * The tensor has shape (nelem, nqp), where nelem is the number of elements, and nqp is the number
   * of quadrature points per element. The tensor is stored in the device memory.
   *
   * @warning The JxWxT value is only available after finalize() is called.
   */
  const at::Tensor & JxWxT() const { return _JxWxT; }

  /// Whether the current assembly cache is up to date
  bool upToDate() const { return _up_to_date; }

  /// Invalidate the cached assembly information
  void invalidate();

  void meshChanged() override { invalidate(); }

  void initialize() override;
  void execute() override;
  void finalize() override;
  void threadJoin(const UserObject &) override;

protected:
  /// Whether the current assembly cache is up to date
  bool _up_to_date = false;

  /// number of elements on this rank
  int64_t _nelem;
  /// number of quadrature points per element
  int64_t _nqp;

  /// JxWxT (product of Jacobian determinant, quadrature weight, and coordinate transformation
  /// factor) for each element, qp
  std::vector<Real> _moose_JxWxT;
  at::Tensor _JxWxT;
};

#endif // MOOSE_LIBTORCH_ENABLED
