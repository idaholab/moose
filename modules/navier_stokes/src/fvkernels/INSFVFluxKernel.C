//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVFluxKernel.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "Assembly.h"
#include "SubProblem.h"

InputParameters
INSFVFluxKernel::validParams()
{
  auto params = FVFluxKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  return params;
}

INSFVFluxKernel::INSFVFluxKernel(const InputParameters & params)
  : FVFluxKernel(params), INSFVMomentumResidualObject(*this)
{
}

void
INSFVFluxKernel::processResidual(const ADReal & residual)
{
  auto process_residual = [this](const ADReal & residual, const Elem & elem) {
    const auto dof_index = elem.dof_number(_sys.number(), _var.number(), 0);

    if (_subproblem.currentlyComputingJacobian())
      _assembly.processDerivatives(residual, dof_index, _matrix_tags);
    else
      _assembly.processResidual(residual.value(), dof_index, _vector_tags);
  };

  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    process_residual(residual, _face_info->elem());
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    process_residual(-residual, _face_info->neighbor());
}
