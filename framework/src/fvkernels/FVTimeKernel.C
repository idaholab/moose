//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVTimeKernel.h"

#include "SystemBase.h"

registerADMooseObject("MooseApp", FVTimeKernel);

InputParameters
FVTimeKernel::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";
  return params;
}

FVTimeKernel::FVTimeKernel(const InputParameters & parameters)
  : FVElementalKernel(parameters), _u_dot(_var.adUDot())
{
}

void
FVTimeKernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());
  _local_re(0) += MetaPhysicL::raw_value(computeQpResidual() * _assembly.elem()->volume());
  accumulateTaggedLocalResidual();
}

ADReal
FVTimeKernel::computeQpResidual()
{
  return _u_dot[_qp];
}

void
FVTimeKernel::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());
  auto dofs_per_elem = _subproblem.systemBaseNonlinear().getMaxVarNDofsPerElem();
  size_t ad_offset = _var.number() * dofs_per_elem;
  auto vol = _assembly.elem()->volume();
  const auto r = computeQpResidual() * vol;
  _local_ke(0, 0) += r.derivatives()[ad_offset];
  accumulateTaggedLocalMatrix();
}
