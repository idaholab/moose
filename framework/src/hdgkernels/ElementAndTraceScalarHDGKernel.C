//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementAndTraceScalarHDGAssemblyHelper.h"
#include "ElementAndTraceScalarHDGKernel.h"
#include "TaggingInterface.h"

InputParameters
ElementAndTraceScalarHDGKernel::validParams()
{
  return HDGKernel::validParams();
}

ElementAndTraceScalarHDGKernel::ElementAndTraceScalarHDGKernel(const InputParameters & parameters)
  : HDGKernel(parameters), _cached_elem(nullptr)
{
}

void
ElementAndTraceScalarHDGKernel::compute(ElementAndTraceScalarHDGAssemblyHelper & helper)
{
  helper.resizeResiduals();
  helper.scalarVolume();
}

void
ElementAndTraceScalarHDGKernel::computeOnSide(ElementAndTraceScalarHDGAssemblyHelper & helper)
{
  helper.resizeResiduals();
  helper.scalarFace();
  helper.lmFace();
}

void
ElementAndTraceScalarHDGKernel::computeResidual()
{
  auto & helper = hdgHelper();
  compute(helper);
  for (const auto & residual_packet : helper.taggingData())
    addResiduals(_assembly, residual_packet);
}

void
ElementAndTraceScalarHDGKernel::computeJacobian()
{
  auto & helper = hdgHelper();
  compute(helper);
  for (const auto & residual_packet : helper.taggingData())
    addJacobian(_assembly, residual_packet);
}

void
ElementAndTraceScalarHDGKernel::computeResidualAndJacobian()
{
  auto & helper = hdgHelper();
  compute(helper);
  for (const auto & residual_packet : helper.taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
ElementAndTraceScalarHDGKernel::computeResidualOnSide()
{
  auto & helper = hdgHelper();
  computeOnSide(helper);
  for (const auto & residual_packet : helper.taggingData())
    addResiduals(_assembly, residual_packet);
}

void
ElementAndTraceScalarHDGKernel::computeJacobianOnSide()
{
  auto & helper = hdgHelper();
  computeOnSide(helper);
  for (const auto & residual_packet : helper.taggingData())
    addJacobian(_assembly, residual_packet);
}

void
ElementAndTraceScalarHDGKernel::computeResidualAndJacobianOnSide()
{
  auto & helper = hdgHelper();
  computeOnSide(helper);
  for (const auto & residual_packet : helper.taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
ElementAndTraceScalarHDGKernel::jacobianSetup()
{
  _cached_elem = nullptr;
}

void
ElementAndTraceScalarHDGKernel::computeOffDiagJacobian(const unsigned int)
{
  if (_cached_elem != _current_elem)
  {
    computeJacobian();
    _cached_elem = _current_elem;
  }
}

std::set<std::string>
ElementAndTraceScalarHDGKernel::additionalROVariables()
{
  return hdgHelper().additionalROVariables();
}

const std::unordered_set<unsigned int> &
ElementAndTraceScalarHDGKernel::getMatPropDependencies() const
{
  return hdgHelper().getMatPropDependencies();
}

bool
ElementAndTraceScalarHDGKernel::getMaterialPropertyCalled() const
{
  return hdgHelper().getMaterialPropertyCalled();
}
