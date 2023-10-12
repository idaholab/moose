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
INSFVFluxKernel::computeResidual(const FaceInfo & fi)
{
  if (_rc_uo.segregated())
    FVFluxKernel::computeResidual(fi);
}

void
INSFVFluxKernel::computeJacobian(const FaceInfo & fi)
{
  if (_rc_uo.segregated())
    FVFluxKernel::computeJacobian(fi);
}

void
INSFVFluxKernel::computeResidualAndJacobian(const FaceInfo & fi)
{
  if (_rc_uo.segregated())
    FVFluxKernel::computeResidualAndJacobian(fi);
}

ADReal
INSFVFluxKernel::computeQpResidual()
{
  mooseAssert(_rc_uo.segregated(), "We should not get here if we are not segregated!");
  return computeSegregatedContribution();
}

void
INSFVFluxKernel::addResidualAndJacobian(const ADReal & residual)
{
  auto process_residual = [this](const ADReal & residual, const Elem & elem)
  {
    const auto dof_index = elem.dof_number(_sys.number(), _var.number(), 0);
    addResidualsAndJacobian(_assembly,
                            std::array<ADReal, 1>{{residual}},
                            std::array<dof_id_type, 1>{{dof_index}},
                            _var.scalingFactor());
  };

  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    process_residual(residual, _face_info->elem());
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    process_residual(-residual, _face_info->neighbor());
}
