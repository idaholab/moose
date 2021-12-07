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
#include "ElementUserObject.h"

class NodalPatchRecoveryBase : public ElementUserObject
{
public:
  static InputParameters validParams();

  NodalPatchRecoveryBase(const InputParameters & parameters);

  /**
   * Solve the least-squares problem. Use the fitted coefficients to calculate  the value at the
   * requested point.
   *
   * @param p           Point at which to compute the fitted value
   * @param elem_ids    Ids of the elements in the patch
   * @return The fitted value
   */
  virtual Real nodalPatchRecovery(const Point & p, const std::vector<dof_id_type> & elem_ids) const;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject &) override {}

protected:
  /// Compute the quantity to recover using nodal patch recovery
  virtual Real computeValue() = 0;

  unsigned int _qp;

private:
  /**
   * Compute the P vector at a given point
   * i.e. given dim = 2, order = 2, polynomial P has the following terms:
   * 1
   * x
   * y
   * x^2
   * xy
   * y^2
   *
   * @param q_point point at which to evaluate the polynomial basis
   */
  RealEigenVector evaluateBasisFunctions(const Point & q_point) const;

  /// The polynomial order, default is variable order
  const unsigned int _patch_polynomial_order;

  /// The multi-index table
  const std::vector<std::vector<unsigned int>> _multi_index;

  /// Number of basis functions
  const unsigned int _q;

  /// The element-level A matrix
  std::map<dof_id_type, RealEigenMatrix> _Ae;

  /// The element-level b vector
  std::map<dof_id_type, RealEigenVector> _be;
};
