//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVElementalKernel.h"

InputParameters
INSFVElementalKernel::validParams()
{
  auto params = FVElementalKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  return params;
}

INSFVElementalKernel::INSFVElementalKernel(const InputParameters & params)
  : FVElementalKernel(params), INSFVMomentumResidualObject(*this)
{
}

void
INSFVElementalKernel::computeResidual()
{
  if (_rc_uo.segregated())
  {
    prepareVectorTag(_assembly, _var.number());
    _local_re(0) +=
        MetaPhysicL::raw_value(computeSegregatedContribution() * _assembly.elemVolume());
    accumulateTaggedLocalResidual();
  }
}

void
INSFVElementalKernel::computeJacobian()
{
  if (_rc_uo.segregated())
  {
    const auto r = computeSegregatedContribution() * _assembly.elemVolume();
    mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");
    addJacobian(_assembly, std::array<ADReal, 1>{{r}}, _var.dofIndices(), _var.scalingFactor());
  }
}

void
INSFVElementalKernel::computeResidualAndJacobian()
{
  if (_rc_uo.segregated())
  {
    const auto r = computeSegregatedContribution() * _assembly.elemVolume();
    addResidualsAndJacobian(
        _assembly, std::array<ADReal, 1>{{r}}, _var.dofIndices(), _var.scalingFactor());
  }
}

void
INSFVElementalKernel::addResidualAndJacobian(const ADReal & residual, const dof_id_type dof_index)
{
  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{residual}},
                          std::array<dof_id_type, 1>{{dof_index}},
                          _var.scalingFactor());
}
