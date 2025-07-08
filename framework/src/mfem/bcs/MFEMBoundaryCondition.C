//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMBoundaryCondition.h"
#include "MFEMProblem.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mesh_extras.hpp"
#include "libmesh/restore_warnings.h"

InputParameters
MFEMBoundaryCondition::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params += MFEMBoundaryRestrictable::validParams();

  params.addClassDescription("Base class for applying boundary conditions to MFEM problems.");
  params.registerBase("BoundaryCondition");
  params.addParam<VariableName>("variable", "Variable on which to apply the boundary condition");
  return params;
}

MFEMBoundaryCondition::MFEMBoundaryCondition(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    MFEMBoundaryRestrictable(parameters,
                             *getMFEMProblem()
                                  .getProblemData()
                                  .gridfunctions.GetRef(getParam<VariableName>("variable"))
                                  .ParFESpace()
                                  ->GetParMesh()),
    _test_var_name(getParam<VariableName>("variable"))
{
}

#endif
