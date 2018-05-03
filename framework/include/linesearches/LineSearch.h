//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LINESEARCH_H
#define LINESEARCH_H

#include "libmesh/parallel_object.h"
#include "ConsoleStreamInterface.h"

using namespace libMesh;

class FEProblem;

class LineSearch : public ConsoleStreamInterface, public ParallelObject
{
public:
  LineSearch(FEProblem & fe_problem, MooseApp & app);

  /**
   * zeros the nonlinear iteration count
   */
  void zeroIts() { _nl_its = 0; }

  /**
   * read-only reference to number of non-linear iterations
   */
  const size_t & nlIts() const { return _nl_its; }

  /**
   * The method that actually implements the line-search
   */
  virtual void linesearch() = 0;

protected:
  /// Reference to the finite element problem
  FEProblem & _fe_problem;

  /// number of non-linear iterations
  size_t _nl_its;
};

#endif
