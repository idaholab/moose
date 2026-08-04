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

#include "MFEMObject.h"
#include "EquationSystem.h"

class MFEMBoundaryCondition;
class MFEMKernel;

/**
 * Constructs and stores an Moose::MFEM::EquationSystem object. Access using the
 * getFESpace() accessor.
 */
class MFEMWeakForm : public MFEMObject
{
public:
  static InputParameters validParams();

  MFEMWeakForm(const InputParameters & parameters);

  /// Constructs the EquationSystem.
  std::shared_ptr<Moose::MFEM::EquationSystem> createEquationSystem();

  void addBoundaryCondition(std::shared_ptr<MFEMBoundaryCondition> bc);

  void addKernel(std::shared_ptr<MFEMKernel> kernel);

private:
  /// Stores the constructed EquationSystem.
  mutable std::shared_ptr<Moose::MFEM::EquationSystem> _equation_system{nullptr};
};

#endif
