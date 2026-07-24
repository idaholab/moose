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
#include "MFEMProblem.h"

/**
 * Action for the [FESpaceHierarchies] input block. Each sub-block adds a
 * MFEMFESpaceHierarchy object to the problem.
 *
 * [FESpaceHierarchies]
 *   [my_hierarchy]
 *     type = MFEMFESpaceHierarchy
 *     fespace = H1FESpace
 *     refinements = 'h 2 4'
 *   []
 * []
 */
class AddMFEMFESpaceHierarchyAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddMFEMFESpaceHierarchyAction(const InputParameters & parameters);

  virtual void act() override;
};

#endif
