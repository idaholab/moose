//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IPHDGBC.h"
#include "IPHDGAssemblyHelper.h"

InputParameters
IPHDGBC::validParams()
{
  return ADIntegratedBC::validParams();
}

IPHDGBC::IPHDGBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _my_elem(nullptr), _my_side(libMesh::invalid_uint)
{
}

void
IPHDGBC::computeResidual()
{
  compute();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addResiduals(_assembly, residual_packet);
}

void
IPHDGBC::computeJacobian()
{
  compute();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addJacobian(_assembly, residual_packet);
}

void
IPHDGBC::computeResidualAndJacobian()
{
  compute();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
IPHDGBC::jacobianSetup()
{
  _my_elem = nullptr;
  _my_side = libMesh::invalid_uint;
}

void
IPHDGBC::computeOffDiagJacobian(const unsigned int)
{
  if ((_my_elem != _current_elem) || (_my_side != _current_side))
  {
    computeJacobian();
    _my_elem = _current_elem;
    _my_side = _current_side;
  }
}

const std::unordered_set<unsigned int> &
IPHDGBC::getMatPropDependencies() const
{
  return iphdgHelper().getMatPropDependencies();
}
