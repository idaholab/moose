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
 * MultiApp transfer between MFEM variables, via shape function evaluation. Supports transfers
 * between dissimilar meshes and finite element spaces.
 */
class MultiAppMFEMShapeEvaluationTransfer : public MFEMMultiAppTransfer
{
public:
  static InputParameters validParams();
  MultiAppMFEMShapeEvaluationTransfer(InputParameters const & params);

protected:
  /// Object to extract node positions and perform projections on destination MFEM GridFunctions.
  MFEMNodalProjector _mfem_projector;
  /// Object to perform pointwise interpolation of source MFEM GridFunctions.
  mfem::FindPointsGSLIB _mfem_interpolator;

  /// Transfer all variables from active source problem to active destination problem.
  virtual void transferVariables() override;

  /// Set current MFEM problem to fetch source variables from
  virtual MFEMProblem & getActiveFromProblem() override
  {
    MFEMProblem * mfem_from_problem =
        dynamic_cast<MFEMProblem *>(&MFEMMultiAppTransfer::getActiveFromProblem());
    if (!mfem_from_problem)
      mooseError("Transfer source problem is not an MFEM problem");
    return *mfem_from_problem;
  }
  /// Set current MFEM problem to fetch destination variables from
  virtual MFEMProblem & getActiveToProblem() override
  {
    MFEMProblem * mfem_to_problem =
        dynamic_cast<MFEMProblem *>(&MFEMMultiAppTransfer::getActiveToProblem());
    if (!mfem_to_problem)
      mooseError("Transfer destination problem is not an MFEM problem");
    return *mfem_to_problem;
  }
};

#endif
