//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  auto params = Kernel::validParams();
  params += ADFunctorInterface::validParams();
  return params;
}

IPHDGKernel::IPHDGKernel(const InputParameters & params)
  : Kernel(params), ADFunctorInterface(this), _current_side(_assembly.side()), _my_elem(nullptr)
{
}

void
IPHDGKernel::compute()
{
  auto & iphdg_helper = iphdgHelper();
  iphdg_helper.resizeResiduals();

  // u
  iphdg_helper.scalarVolume();

  for (const auto side : _current_elem->side_index_range())
    if (_neigh = _current_elem->neighbor_ptr(side);
        _neigh && this->hasBlocks(_neigh->subdomain_id()))
    {
      NonlinearThread::prepareFace(
          _fe_problem, _tid, _current_elem, side, Moose::INVALID_BOUNDARY_ID);
      mooseAssert(_current_side == side, "The sides should be the same");
      // u, lm_u
      iphdg_helper.scalarFace();
      iphdg_helper.lmFace();
    }
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
