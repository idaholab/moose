//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/nonlinear_implicit_system.h"

class FEProblemBase;

namespace libMesh
{
template <typename>
class NumericVector;
template <typename>
class SparseMatrix;
class NonlinearImplicitSystem;
}

class ComputeResidualAndJacobian
  : public libMesh::NonlinearImplicitSystem::ComputeResidualandJacobian
{
private:
  FEProblemBase & _fe_problem;

public:
  ComputeResidualAndJacobian(FEProblemBase & fe_problem);

  void residual_and_jacobian(const libMesh::NumericVector<libMesh::Number> & u,
                             libMesh::NumericVector<libMesh::Number> * R,
                             libMesh::SparseMatrix<libMesh::Number> * J,
                             libMesh::NonlinearImplicitSystem &) override;
};
