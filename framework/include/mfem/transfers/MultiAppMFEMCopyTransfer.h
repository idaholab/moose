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

#include "MultiAppMFEMGeneralFieldTransferBase.h"

class MooseMesh;

//*
// Copy MFEMVariables between multiapps
// The variables must be of the same type and dimension
// and the MFEMMesh must be identical in both multiapps
// */

class MultiAppMFEMCopyTransfer : public MultiAppMFEMGeneralFieldTransferBase
{
public:
  static InputParameters validParams();
  MultiAppMFEMCopyTransfer(InputParameters const & params);

protected:
  virtual MFEMProblem & getActiveToProblem() override {return static_cast<MFEMProblem &>(*_active_to_problem);};
  virtual MFEMProblem & getActiveFromProblem() override {return static_cast<MFEMProblem &>(*_active_from_problem);};
  void transferVariables() override;
};

#endif
