//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HybridizedDGBC.h"
#include "HybridizedDGAssemblyHelper.h"

InputParameters
HybridizedDGBC::validParams()
{
  return ADIntegratedBC::validParams();
}

HybridizedDGBC::HybridizedDGBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _cached_elem(nullptr), _cached_side(libMesh::invalid_uint)
{
}

void
HybridizedDGBC::computeResidual()
{
  compute();
  for (const auto & residual_packet : hybridizedDGHelper().taggingData())
    addResiduals(_assembly, residual_packet);
}

void
HybridizedDGBC::computeJacobian()
{
  compute();
  for (const auto & residual_packet : hybridizedDGHelper().taggingData())
    addJacobian(_assembly, residual_packet);
}

void
HybridizedDGBC::computeResidualAndJacobian()
{
  compute();
  for (const auto & residual_packet : hybridizedDGHelper().taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
HybridizedDGBC::jacobianSetup()
{
  _cached_elem = nullptr;
  _cached_side = libMesh::invalid_uint;
}

void
HybridizedDGBC::computeOffDiagJacobian(const unsigned int)
{
  if ((_cached_elem != _current_elem) || (_cached_side != _current_side))
  {
    computeJacobian();
    _cached_elem = _current_elem;
    _cached_side = _current_side;
  }
}

const std::unordered_set<unsigned int> &
HybridizedDGBC::getMatPropDependencies() const
{
  return hybridizedDGHelper().getMatPropDependencies();
}

bool
HybridizedDGBC::getMaterialPropertyCalled() const
{
  return hybridizedDGHelper().getMaterialPropertyCalled();
}
