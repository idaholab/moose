//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "libmesh/petsc_vector.h"

/**
 * This class is responsible for collecting solution vectors in one place. The
 * vectors are kept distributed with respect to the communicator of the application.
 * The whole solution vector is stored, which may contain multiple variables.
 * The saving frequency can be defined using the `execute_on` parameter.
 */
class SolutionContainer : public GeneralReporter
{
public:
  static InputParameters validParams();
  SolutionContainer(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

  /// Return the whole solution container
  const std::vector<std::unique_ptr<NumericVector<Number>>> & getContainer()
  {
    return _accumulated_solutions;
  }

  /**
   * Return one of the stored solution vectors
   * @param local_i The index of the locally stored solution vector
   */
  const std::unique_ptr<NumericVector<Number>> & getSolution(unsigned int local_i) const;

protected:
  /// Dynamic container for solution vectors. We store pointers to make sure that the change in size
  /// comes with little overhead. This is a reference because we need it to be restartable for
  /// stochastic runs in batch mode.
  std::vector<std::unique_ptr<NumericVector<Number>>> & _accumulated_solutions;
};
