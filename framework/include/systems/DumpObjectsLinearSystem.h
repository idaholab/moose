//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearSystem.h"

namespace libMesh
{
template <typename T>
class NumericVector;
}

/**
 * Linear system for dumping objects
 * TODO: consider creating a base class for LinearSystems to be able to create a more minimal linear
 * system. For now the additional code complexity is not worth it
 */
class DumpObjectsLinearSystem : public LinearSystem
{
public:
  DumpObjectsLinearSystem(FEProblemBase & problem, const std::string & name);

  virtual void solve() override {}
  virtual void stopSolve(const ExecFlagType &, const std::set<TagID> &) override {}
  virtual bool converged() override { return true; }

protected:
  NumericVector<Number> * _dummy;
};
