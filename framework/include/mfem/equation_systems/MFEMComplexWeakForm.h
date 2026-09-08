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

#include "MFEMWeakFormBase.h"
#include "ComplexEquationSystem.h"

/**
 * Constructs and stores an Moose::MFEM::ComplexEquationSystem object.
 */
class MFEMComplexWeakForm : public MFEMWeakFormBase
{
public:
  MFEMComplexWeakForm(const InputParameters & parameters);

protected:
  virtual std::shared_ptr<Moose::MFEM::EquationSystem> makeEquationSystem() override;

  virtual void addBoundaryCondition(const std::string & name,
                                    std::shared_ptr<MFEMBoundaryCondition> bc) override;

  virtual void addKernel(const std::string & name, std::shared_ptr<MFEMKernel> kernel) override;

private:
  /// The equation system as a ComplexEquationSystem, which is the type this class builds in
  /// makeEquationSystem(). Needed because the AddComplex* methods are declared only on the
  /// derived system and so cannot be reached through the base-typed _equation_system.
  Moose::MFEM::ComplexEquationSystem & complexEquationSystem();
};

#endif
