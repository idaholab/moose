//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Shared helpers for unit tests that build and run a full MooseApp (via Moose::createMooseApp())
// from an input file, rather than exercising a single class in isolation.

#include "libmesh/petsc_matrix.h"

#include <string>
#include <vector>

class FEProblemBase;

namespace MooseAppTestUtils
{

/// Wraps a vector of string command line arguments as argc/argv, with a fake executable name
/// prepended, for passing to Moose::createMooseApp(). Example:
///   Args args({"-i", "files/MyTest/my_input.i", "Executioner/dt=0.1"});
///   const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
struct Args
{
  Args(const std::vector<std::string> & args) : _args(args)
  {
    _args.insert(_args.begin(), "unused");
    for (auto & arg : _args)
      _argv.push_back(arg.data());
    _argv.push_back(nullptr);
  }
  int argc() const { return static_cast<int>(_argv.size()) - 1; }
  char ** argv() { return _argv.data(); }
  std::vector<std::string> _args;
  std::vector<char *> _argv;
};

/// Returns the number of stored nonzero entries in the assembled Jacobian.
/// Accesses the matrix via the libMesh NonlinearImplicitSystem directly, since
/// MOOSE's tag-based getMatrix() is not valid after run() returns.
PetscInt getJacobianNNZ(FEProblemBase & fe_problem);

} // namespace MooseAppTestUtils
