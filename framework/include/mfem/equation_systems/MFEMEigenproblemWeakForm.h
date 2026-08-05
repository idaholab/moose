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

#include "MFEMWeakForm.h"

/**
 * Constructs and stores an Moose::MFEM::EigenproblemEquationSystem object.
 */
class MFEMEigenproblemWeakForm : public MFEMWeakForm
{
public:
  MFEMEigenproblemWeakForm(const InputParameters & parameters);

  /// Constructs the EquationSystem.
  virtual std::shared_ptr<Moose::MFEM::EquationSystem> createEquationSystem() override;
};

#endif
