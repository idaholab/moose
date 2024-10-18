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
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "PetscSupport.h"

// libMesh includes
#include "libmesh/sparse_matrix.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/preconditioner.h"

// Forward declarations
class LumpedPreconditioner;

/**
 * Class to that applies the lumped mass matrix preconditioner
 * in the ExplicitTimeIntegrator
 */
class LumpedPreconditioner : public libMesh::Preconditioner<Real>
{
public:
  LumpedPreconditioner(const NumericVector<Real> & diag_inverse)
    : Preconditioner(diag_inverse.comm()), _diag_inverse(diag_inverse)
  {
  }

  virtual void init() override
  {
    // No more initialization needed here
    _is_initialized = true;
  }

  virtual void apply(const NumericVector<Real> & x, NumericVector<Real> & y) override
  {
    y.pointwise_mult(_diag_inverse, x);
  }

protected:
  /// The inverse of the diagonal of the lumped matrix
  const NumericVector<Real> & _diag_inverse;
};
