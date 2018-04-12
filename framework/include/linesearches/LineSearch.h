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

class FEProblemBase;

class LineSearch : public ConsoleStreamInterface, public ParallelObject
{
public:
  LineSearch(FEProblemBase & fe_problem, MooseApp & app);

  // A dummy pure virtual destructor because this must be a base class; the derived class must
  // implement the line searching capabilities
  virtual ~LineSearch() = 0;

  /**
   * zeros the nonlinear iteration count
   */
  void zero_its() { _nl_its = 0; }

  /**
   * read-only reference to number of non-linear iterations
   */
  const size_t & nl_its() const { return _nl_its; }

protected:
  /// Reference to the finite element problem
  FEProblemBase & _fe_problem;

  /// number of non-linear iterations
  size_t _nl_its;
};

#endif
