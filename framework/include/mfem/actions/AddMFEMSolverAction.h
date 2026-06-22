//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MooseObjectAction.h"

/**
 * This class allows us to configure MFEM solver objects in the input file.
 *
 * [Solvers]
 *   [nl]
 *     type = MFEMNewtonNonlinearSolver
 *   []
 * []
 */
class AddMFEMSolverAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddMFEMSolverAction(const InputParameters & parameters);

  void act() override;
};

#endif
