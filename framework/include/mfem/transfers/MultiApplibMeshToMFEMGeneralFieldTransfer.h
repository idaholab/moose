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

#include <vector>

#include "MultiAppMFEMGeneralFieldTransferBase.h"

class MooseMesh;

//*
// Copy MFEMVariables between multiapps
// The variables must be of the same type and dimension
// but MFEMMesh may differ in each subapp
// */

class MultiApplibMeshToMFEMGeneralFieldTransfer : public MultiAppMFEMGeneralFieldTransferBase, public MFEMTransferProjector
{
public:
  static InputParameters validParams();
  MultiApplibMeshToMFEMGeneralFieldTransfer(InputParameters const & params);

protected:
  mfem::FindPointsGSLIB _mfem_interpolator;
  virtual MFEMProblem & getActiveToProblem() override {return static_cast<MFEMProblem &>(*_active_to_problem);};
  void transferVariables() override;
  void setMFEMGridFunctionValuesFromlibMesh(const unsigned int var_index, MFEMProblem & to_problem);
};

#endif
