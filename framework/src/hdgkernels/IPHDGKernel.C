//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IPHDGAssemblyHelper.h"
#include "IPHDGKernel.h"

using namespace libMesh;

InputParameters
IPHDGKernel::validParams()
{
  auto params = HDGKernel::validParams();
  return params;
}

IPHDGKernel::IPHDGKernel(const InputParameters & params) : HDGKernel(params), _my_elem(nullptr) {}

void
IPHDGKernel::compute()
{
  auto & iphdg_helper = iphdgHelper();
  iphdg_helper.resizeResiduals();
  iphdg_helper.scalarVolume();
}

void
IPHDGKernel::computeOnSide()
{
  auto & iphdg_helper = iphdgHelper();
  iphdg_helper.resizeResiduals();
  iphdg_helper.scalarFace();
  iphdg_helper.lmFace();
}

void
IPHDGKernel::computeResidual()
{
  compute();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addResiduals(_assembly, residual_packet);
}

void
IPHDGKernel::computeJacobian()
{
  compute();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addJacobian(_assembly, residual_packet);
}

void
IPHDGKernel::computeResidualAndJacobian()
{
  compute();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
IPHDGKernel::computeResidualOnSide()
{
  computeOnSide();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addResiduals(_assembly, residual_packet);
}

void
IPHDGKernel::computeJacobianOnSide()
{
  computeOnSide();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addJacobian(_assembly, residual_packet);
}

void
IPHDGKernel::computeResidualAndJacobianOnSide()
{
  computeOnSide();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
IPHDGKernel::jacobianSetup()
{
  _my_elem = nullptr;
}

void
IPHDGKernel::computeOffDiagJacobian(const unsigned int)
{
  if (_my_elem != _current_elem)
  {
    computeJacobian();
    _my_elem = _current_elem;
  }
}

std::set<std::string>
IPHDGKernel::additionalVariablesCovered()
{
  return iphdgHelper().variablesCovered();
}

const std::unordered_set<unsigned int> &
IPHDGKernel::getMatPropDependencies() const
{
  return iphdgHelper().getMatPropDependencies();
}
