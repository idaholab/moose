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
 * MultiApp transfer from MFEM to libMesh variables, performed via evaluation of
 * shape functions. Meshes can differ between source and destination variables.
 */
class MultiAppMFEMTolibMeshShapeEvaluationTransfer : public MFEMMultiAppTransfer
{
public:
  static InputParameters validParams();
  MultiAppMFEMTolibMeshShapeEvaluationTransfer(InputParameters const & params);

protected:
  /// Object to perform pointwise interpolation of source MFEM GridFunctions.
  mfem::FindPointsGSLIB _mfem_interpolator;

  /// Transfer all variables from active source problem to active destination problem.
  void transferVariables() override;

  /// Set libMesh variable corresponding to var_index from MFEM GridFunction
  void setlibMeshSolutionValuesFromMFEM(const unsigned int var_index, MFEMProblem & from_problem);

  /// Set current MFEM problem to fetch source variables from
  virtual MFEMProblem & getActiveFromProblem() override
  {
    return static_cast<MFEMProblem &>(MFEMMultiAppTransfer::getActiveFromProblem());
  }
};

#endif
