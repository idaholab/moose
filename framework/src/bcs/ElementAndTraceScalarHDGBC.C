//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementAndTraceScalarHDGAssemblyHelper.h"
#include "ElementAndTraceScalarHDGBC.h"
#include "TaggingInterface.h"

InputParameters
ElementAndTraceScalarHDGBC::validParams()
{
  return ADIntegratedBC::validParams();
}

ElementAndTraceScalarHDGBC::ElementAndTraceScalarHDGBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _cached_elem(nullptr), _cached_side(libMesh::invalid_uint)
{
}

void
ElementAndTraceScalarHDGBC::computeResidual()
{
  auto & helper = hdgHelper();
  compute(helper);
  for (const auto & residual_packet : helper.taggingData())
    addResiduals(_assembly, residual_packet);
}

void
ElementAndTraceScalarHDGBC::computeJacobian()
{
  auto & helper = hdgHelper();
  compute(helper);
  for (const auto & residual_packet : helper.taggingData())
    addJacobian(_assembly, residual_packet);
}

void
ElementAndTraceScalarHDGBC::computeResidualAndJacobian()
{
  auto & helper = hdgHelper();
  compute(helper);
  for (const auto & residual_packet : helper.taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
ElementAndTraceScalarHDGBC::jacobianSetup()
{
  _cached_elem = nullptr;
  _cached_side = libMesh::invalid_uint;
}

void
ElementAndTraceScalarHDGBC::computeOffDiagJacobian(const unsigned int)
{
  if ((_cached_elem != _current_elem) || (_cached_side != _current_side))
  {
    computeJacobian();
    _cached_elem = _current_elem;
    _cached_side = _current_side;
  }
}

const std::unordered_set<unsigned int> &
ElementAndTraceScalarHDGBC::getMatPropDependencies() const
{
  return hdgHelper().getMatPropDependencies();
}

bool
ElementAndTraceScalarHDGBC::getMaterialPropertyCalled() const
{
  return hdgHelper().getMaterialPropertyCalled();
}
