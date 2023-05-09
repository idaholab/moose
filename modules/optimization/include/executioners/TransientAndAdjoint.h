//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Transient.h"
#include "AdjointTransientSolve.h"

// Forward declarations
class InputParameters;

class TransientAndAdjoint : public Transient
{
public:
  static InputParameters validParams();

  TransientAndAdjoint(const InputParameters & parameters);

  /**
   * For storing the initial condition
   */
  virtual void preExecute() override;

  /**
   * For storing the converged solution
   */
  virtual void postStep() override;

  /**
   * For solving the adjoint problem
   */
  virtual void postExecute() override;

protected:
  /// The transient adjoint solve object responsible for storing forward solutions and solving the adjoint system
  AdjointTransientSolve _adjoint_solve;
  /// Cached forward time points so we can properly loop backward in time
  std::vector<Real> & _forward_times;
};
