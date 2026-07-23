//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoFieldScalarHDGAssemblyHelper.h"
#include "TwoFieldScalarHDGKernel.h"
#include "TaggingInterface.h"

InputParameters
TwoFieldScalarHDGKernel::validParams()
{
  return HDGKernel::validParams();
}

TwoFieldScalarHDGKernel::TwoFieldScalarHDGKernel(const InputParameters & parameters)
  : HDGKernel(parameters), _cached_elem(nullptr)
{
}

void
TwoFieldScalarHDGKernel::compute(TwoFieldScalarHDGAssemblyHelper & helper)
{
  helper.resizeResiduals();
  helper.scalarVolume();
}

void
TwoFieldScalarHDGKernel::computeOnSide(TwoFieldScalarHDGAssemblyHelper & helper)
{
  helper.resizeResiduals();
  helper.scalarFace();
  helper.lmFace();
}

void
TwoFieldScalarHDGKernel::computeResidual()
{
  auto & helper = hdgHelper();
  compute(helper);
  for (const auto & residual_packet : helper.taggingData())
    addResiduals(_assembly, residual_packet);
}

void
TwoFieldScalarHDGKernel::computeJacobian()
{
  auto & helper = hdgHelper();
  compute(helper);
  for (const auto & residual_packet : helper.taggingData())
    addJacobian(_assembly, residual_packet);
}

void
TwoFieldScalarHDGKernel::computeResidualAndJacobian()
{
  auto & helper = hdgHelper();
  compute(helper);
  for (const auto & residual_packet : helper.taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
TwoFieldScalarHDGKernel::computeResidualOnSide()
{
  auto & helper = hdgHelper();
  computeOnSide(helper);
  for (const auto & residual_packet : helper.taggingData())
    addResiduals(_assembly, residual_packet);
}

void
TwoFieldScalarHDGKernel::computeJacobianOnSide()
{
  auto & helper = hdgHelper();
  computeOnSide(helper);
  for (const auto & residual_packet : helper.taggingData())
    addJacobian(_assembly, residual_packet);
}

void
TwoFieldScalarHDGKernel::computeResidualAndJacobianOnSide()
{
  auto & helper = hdgHelper();
  computeOnSide(helper);
  for (const auto & residual_packet : helper.taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
TwoFieldScalarHDGKernel::jacobianSetup()
{
  _cached_elem = nullptr;
}

void
TwoFieldScalarHDGKernel::computeOffDiagJacobian(const unsigned int)
{
  if (_cached_elem != _current_elem)
  {
    computeJacobian();
    _cached_elem = _current_elem;
  }
}

std::set<std::string>
TwoFieldScalarHDGKernel::additionalROVariables()
{
  return hdgHelper().additionalROVariables();
}

const std::unordered_set<unsigned int> &
TwoFieldScalarHDGKernel::getMatPropDependencies() const
{
  return hdgHelper().getMatPropDependencies();
}

bool
TwoFieldScalarHDGKernel::getMaterialPropertyCalled() const
{
  return hdgHelper().getMaterialPropertyCalled();
}
