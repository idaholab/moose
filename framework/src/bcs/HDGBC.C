//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HDGBC.h"
#include "HDGAssemblyHelper.h"

InputParameters
HDGBC::validParams()
{
  return ADIntegratedBC::validParams();
}

HDGBC::HDGBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _cached_elem(nullptr), _cached_side(libMesh::invalid_uint)
{
}

void
HDGBC::computeResidual()
{
  compute();
  for (const auto & residual_packet : hdgHelper().taggingData())
    addResiduals(_assembly, residual_packet);
}

void
HDGBC::computeJacobian()
{
  compute();
  for (const auto & residual_packet : hdgHelper().taggingData())
    addJacobian(_assembly, residual_packet);
}

void
HDGBC::computeResidualAndJacobian()
{
  compute();
  for (const auto & residual_packet : hdgHelper().taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
HDGBC::jacobianSetup()
{
  _cached_elem = nullptr;
  _cached_side = libMesh::invalid_uint;
}

void
HDGBC::computeOffDiagJacobian(const unsigned int)
{
  if ((_cached_elem != _current_elem) || (_cached_side != _current_side))
  {
    computeJacobian();
    _cached_elem = _current_elem;
    _cached_side = _current_side;
  }
}

const std::unordered_set<unsigned int> &
HDGBC::getMatPropDependencies() const
{
  return hdgHelper().getMatPropDependencies();
}

bool
HDGBC::getMaterialPropertyCalled() const
{
  return hdgHelper().getMaterialPropertyCalled();
}
