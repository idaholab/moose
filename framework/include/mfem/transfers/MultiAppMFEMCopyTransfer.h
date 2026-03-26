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

#include "MFEMMultiAppTransfer.h"
#include "MFEMProblem.h"

/**
 * Transfer to copy MFEMVariables between multiapps.
 * The variables must be of the same type and dimension
 * and the MFEMMesh must be identical in both multiapps
 */
class MultiAppMFEMCopyTransfer : public MFEMMultiAppTransfer
{
public:
  static InputParameters validParams();
  MultiAppMFEMCopyTransfer(InputParameters const & params);

protected:
  /// Transfer all variables from active source problem to active destination problem.
  virtual void transferVariables(bool is_target_local) override;
  /// Check number of source and target child apps match for sibling transfer
  void checkSiblingsTransferSupported() const override;

  /// Set current MFEM problem to fetch source variables from
  virtual MFEMProblem & getActiveFromProblem() override;
  /// Set current MFEM problem to fetch destination variables from
  virtual MFEMProblem & getActiveToProblem() override;
};

#endif