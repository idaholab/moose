//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef NEML2_ENABLED

// MOOSE includes
#include "ElementUserObject.h"

#include "neml2/tensors/Tensor.h"

/// This user object caches assembly information from MOOSE.
class NEML2Assembly : public ElementUserObject
{
public:
  static InputParameters validParams();

  NEML2Assembly(const InputParameters & parameters);

  /// Number of active elements on this rank
  int64_t numElem() const { return _nelem; }

  /// Number of quadrature points per element
  int64_t numQP() const { return _nqp; }

  /**
   * @brief Get the cached JxWxT for each element, each quadrature point
   *
   * The NEML2 tensor will have shape (nelem, nqp;), where nelem is the number of elements, and nqp
   * is the number of quadrature points per element. The tensor is stored in the device memory.
   *
   * @warning The JxWxT value is only available after finalize() is called.
   */
  const neml2::Tensor & JxWxT() const { return _neml2_JxWxT; }

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

  /// JxWxT (product of Jacobian determinant, quadrature weight, and coordinate transformation factor) for each element, qp
  std::vector<Real> _moose_JxWxT;
  neml2::Tensor _neml2_JxWxT;
};

#endif
