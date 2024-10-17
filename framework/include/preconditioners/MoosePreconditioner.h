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
#include "MooseObject.h"
#include "Restartable.h"
#include "PerfGraphInterface.h"

// Libmesh include
#include "libmesh/preconditioner.h"
#include "libmesh/linear_solver.h"
#include "libmesh/coupling_matrix.h"

// Forward declarations
class FEProblemBase;
class NonlinearSystemBase;
namespace libMesh
{
class MeshBase;
template <typename T>
class NumericVector;
}

/**
 * Base class for MOOSE preconditioners.
 */
class MoosePreconditioner : public MooseObject, public Restartable, public PerfGraphInterface
{
public:
  static InputParameters validParams();

  MoosePreconditioner(const InputParameters & params);
  virtual ~MoosePreconditioner() = default;

  /**
   * Helper function for copying values associated with variables in
   * vectors from two different systems.
   */
  static void copyVarValues(MeshBase & mesh,
                            const unsigned int from_system,
                            const unsigned int from_var,
                            const NumericVector<Number> & from_vector,
                            const unsigned int to_system,
                            const unsigned int to_var,
                            NumericVector<Number> & to_vector);

protected:
  /// Setup the coupling matrix on the finite element problem
  void setCouplingMatrix(std::unique_ptr<libMesh::CouplingMatrix> cm);

  /// Subproblem this preconditioner is part of
  FEProblemBase & _fe_problem;

  /// The nonlinear system number whose linearization this preconditioner should be applied to
  const unsigned int _nl_sys_num;

  /// The nonlinear system whose linearization this preconditioner should be applied to
  NonlinearSystemBase & _nl;

  friend class SetupPreconditionerAction;
};
