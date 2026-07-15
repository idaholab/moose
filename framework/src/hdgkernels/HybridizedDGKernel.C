//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HybridizedDGAssemblyHelper.h"
#include "HybridizedDGKernel.h"

InputParameters
HybridizedDGKernel::validParams()
{
  return HDGKernel::validParams();
}

HybridizedDGKernel::HybridizedDGKernel(const InputParameters & params)
  : HDGKernel(params), _cached_elem(nullptr)
{
}

void
HybridizedDGKernel::compute()
{
  auto & helper = hybridizedDGHelper();
  helper.resizeResiduals();
  helper.scalarVolume();
}

void
HybridizedDGKernel::computeOnSide()
{
  auto & helper = hybridizedDGHelper();
  helper.resizeResiduals();
  helper.scalarFace();
  helper.lmFace();
}

void
HybridizedDGKernel::computeResidual()
{
  compute();
  for (const auto & residual_packet : hybridizedDGHelper().taggingData())
    addResiduals(_assembly, residual_packet);
}

void
HybridizedDGKernel::computeJacobian()
{
  compute();
  for (const auto & residual_packet : hybridizedDGHelper().taggingData())
    addJacobian(_assembly, residual_packet);
}

void
HybridizedDGKernel::computeResidualAndJacobian()
{
  compute();
  for (const auto & residual_packet : hybridizedDGHelper().taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
HybridizedDGKernel::computeResidualOnSide()
{
  computeOnSide();
  for (const auto & residual_packet : hybridizedDGHelper().taggingData())
    addResiduals(_assembly, residual_packet);
}

void
HybridizedDGKernel::computeJacobianOnSide()
{
  computeOnSide();
  for (const auto & residual_packet : hybridizedDGHelper().taggingData())
    addJacobian(_assembly, residual_packet);
}

void
HybridizedDGKernel::computeResidualAndJacobianOnSide()
{
  computeOnSide();
  for (const auto & residual_packet : hybridizedDGHelper().taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
HybridizedDGKernel::jacobianSetup()
{
  _cached_elem = nullptr;
}

void
HybridizedDGKernel::computeOffDiagJacobian(const unsigned int)
{
  if (_cached_elem != _current_elem)
  {
    computeJacobian();
    _cached_elem = _current_elem;
  }
}

std::set<std::string>
HybridizedDGKernel::additionalROVariables()
{
  return hybridizedDGHelper().additionalROVariables();
}

const std::unordered_set<unsigned int> &
HybridizedDGKernel::getMatPropDependencies() const
{
  return hybridizedDGHelper().getMatPropDependencies();
}

bool
HybridizedDGKernel::getMaterialPropertyCalled() const
{
  return hybridizedDGHelper().getMaterialPropertyCalled();
}
