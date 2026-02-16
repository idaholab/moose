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

#include "MultiAppTransfer.h"
#include "MultiApp.h"
#include "MooseAppCoordTransform.h"
#include "MFEMProblem.h"

class MooseMesh;

//*
// Copy MFEMVariables between multiapps
// The variables must be of the same type and dimension
// but MFEMMesh may differ in each subapp
// */

class MultiAppMFEMTolibMeshGeneralFieldTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();
  MultiAppMFEMTolibMeshGeneralFieldTransfer(InputParameters const & params);
  void execute() override;
  auto const & getFromVarName(int i) { return _from_var_names.at(i); };
  auto const & getToVarName(int i) { return _to_var_names.at(i); };
  auto numFromVar() { return _from_var_names.size(); }
  auto numToVar() { return _to_var_names.size(); }

protected:
  mfem::FindPointsGSLIB _mfem_interpolator;
  std::vector<VariableName> _from_var_names;
  std::vector<AuxVariableName> _to_var_names;

  void transfer(FEProblemBase & to_problem, MFEMProblem & from_problem);
  void setlibMeshSolutionValuesFromMFEM(const unsigned int var_index, MFEMProblem & from_problem);
  void checkSiblingsTransferSupported() const override;
};

#endif
