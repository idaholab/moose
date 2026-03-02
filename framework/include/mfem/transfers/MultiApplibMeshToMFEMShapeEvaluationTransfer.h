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

#include "MFEMNodalProjector.h"
#include "MFEMMultiAppTransfer.h"
#include "MFEMProblem.h"

/**
 * MultiApp transfer from libMesh to MFEM variables, performed via evaluation of
 * shape functions. Meshes can differ between source and destination variables.
 */
class MultiApplibMeshToMFEMShapeEvaluationTransfer : public MFEMMultiAppTransfer
{
public:
  static InputParameters validParams();
  MultiApplibMeshToMFEMShapeEvaluationTransfer(InputParameters const & params);

protected:
  /// Object to extract node positions and perform projections on MFEM GridFunctions.
  MFEMNodalProjector _mfem_projector;

  /// Transfer all variables from active source problem to active destination problem.
  void transferVariables() override;

  /// Set MFEM GridFunction values from the libMesh variable corresponding to var_index
  void setMFEMGridFunctionValuesFromlibMesh(const unsigned int var_index, MFEMProblem & to_problem);

  /// Set current problem to fetch destination variables from
  virtual MFEMProblem & getActiveToProblem() override
  {
    return static_cast<MFEMProblem &>(MFEMMultiAppTransfer::getActiveToProblem());
  }
};

#endif
