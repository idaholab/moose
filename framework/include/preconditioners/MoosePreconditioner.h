//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSEPRECONDITIONER_H
#define MOOSEPRECONDITIONER_H

// MOOSE includes
#include "MooseObject.h"
#include "Restartable.h"

// Forward declarations
class FEProblemBase;
class MoosePreconditioner;

namespace libMesh
{
class MeshBase;
template <typename T>
class NumericVector;
}

template <>
InputParameters validParams<MoosePreconditioner>();

/**
 * Base class for MOOSE preconditioners.
 */
class MoosePreconditioner : public MooseObject, public Restartable
{
public:
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
  /// Subproblem this preconditioner is part of
  FEProblemBase & _fe_problem;
};

#endif /* MOOSEPRECONDITIONER_H */
