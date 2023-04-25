//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolveObject.h"

class NonlinearSystemBase;

namespace libMesh
{
template <typename T>
class SparseMatrix;
template <typename T>
class NumericVector;
}

class AdjointSolve : public SolveObject
{
public:
  AdjointSolve(Executioner & ex);

  static InputParameters validParams();

  virtual bool solve() override;

protected:
  virtual void assembleAdjointSystem(SparseMatrix<Number> & matrix,
                                     NumericVector<Number> & solution,
                                     NumericVector<Number> & rhs);
  void applyNodalBCs(SparseMatrix<Number> & matrix,
                     NumericVector<Number> & solution,
                     NumericVector<Number> & rhs);

  const unsigned int _forward_sys_num;
  const unsigned int _adjoint_sys_num;
  NonlinearSystemBase & _nl_forward;
  NonlinearSystemBase & _nl_adjoint;
};
