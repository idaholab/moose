//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMKernel.h"
#include "MFEMProblem.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mesh_extras.hpp"
#include "libmesh/restore_warnings.h"

InputParameters
MFEMKernel::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params += MFEMBlockRestrictable::validParams();
  params.registerBase("Kernel");
  params.addParam<VariableName>("variable",
                                "Variable labelling the weak form this kernel is added to");
  return params;
}

MFEMKernel::MFEMKernel(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    MFEMBlockRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh()),
    _test_var_name(getParam<VariableName>("variable"))
{
}

#endif
