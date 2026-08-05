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

/**
 * Constructs and stores an Moose::MFEM::ComplexEquationSystem object.
 */
class MFEMComplexWeakForm : public MFEMWeakFormBase
{
public:
  MFEMComplexWeakForm(const InputParameters & parameters);

  /// Constructs the EquationSystem.
  virtual std::shared_ptr<Moose::MFEM::EquationSystem> createEquationSystem() override;

protected:
  virtual void addBoundaryCondition(const std::string & name,
                                    std::shared_ptr<MFEMBoundaryCondition> bc) override;

  virtual void addKernel(const std::string & name, std::shared_ptr<MFEMKernel> kernel) override;

private:
  /// Stores the constructed ComplexEquationSystem. Intentionally marked private to ensure
  /// other objects in the problem do not use it prior to full initialisation.
  mutable std::shared_ptr<Moose::MFEM::ComplexEquationSystem> _equation_system{nullptr};
};

#endif
