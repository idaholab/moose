//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBC.h"
#include "libmesh/numeric_vector.h"

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class SparseMatrix;
}
/**
 * Base boundary condition of a direct Dirichlet type
 */
class DirectDirichletBCBase : public NodalBC
{
public:
  static InputParameters validParams();

  DirectDirichletBCBase(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /**
   * Compute the value of the DirichletBC at the current quadrature point
   */
  virtual Real computeQpValue() = 0;

  /// The diagonal of the mass matrix
  const NumericVector<Number> & _mass_diag;

  const Real & _u_old;
  const Real & _u_dot_old;
};
