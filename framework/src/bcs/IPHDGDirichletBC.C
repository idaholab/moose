//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IPHDGDirichletBC.h"
#include "IPHDGAssemblyHelper.h"

InputParameters
IPHDGDirichletBC::validParams()
{
  auto params = IntegratedBC::validParams();
  params += ADFunctorInterface::validParams();
  params.addParam<MooseFunctorName>("functor", 0, "The Dirichlet value for the primal variable");
  return params;
}

IPHDGDirichletBC::IPHDGDirichletBC(const InputParameters & params)
  : IntegratedBC(params),
    ADFunctorInterface(this),
    _dirichlet_val(getFunctor<Real>("functor")),
    _my_elem(nullptr),
    _my_side(libMesh::invalid_uint)
{
}

void
IPHDGDirichletBC::compute()
{
  auto & iphdg_helper = iphdgHelper();
  iphdg_helper.resizeResiduals();

  // u, lm_u
  iphdg_helper.scalarDirichlet(_dirichlet_val);
  iphdg_helper.lmDirichlet(_dirichlet_val);
}

void
IPHDGDirichletBC::computeResidual()
{
  compute();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addResiduals(_assembly, residual_packet);
}

void
IPHDGDirichletBC::computeJacobian()
{
  compute();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addJacobian(_assembly, residual_packet);
}

void
IPHDGDirichletBC::computeResidualAndJacobian()
{
  compute();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
IPHDGDirichletBC::jacobianSetup()
{
  _my_elem = nullptr;
  _my_side = libMesh::invalid_uint;
}

void
IPHDGDirichletBC::computeOffDiagJacobian(const unsigned int)
{
  if ((_my_elem != _current_elem) || (_my_side != _current_side))
  {
    computeJacobian();
    _my_elem = _current_elem;
    _my_side = _current_side;
  }
}
