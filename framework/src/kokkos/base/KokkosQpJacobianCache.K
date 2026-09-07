//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosQpJacobianCache.h"

namespace Moose::Kokkos
{

void
QpJacobianCache::create(const unsigned int n_subdomains, const unsigned int n_vars)
{
  if (_tensors.isAlloc())
    return;

  _tensors.create(n_subdomains, n_vars);

  _blocks.create(n_subdomains, n_vars);
  _blocks = QP_JACOBIAN_NONE;
}

void
QpJacobianCache::createTensors(const unsigned int subdomain,
                               const unsigned int var,
                               const dof_id_type n_qps)
{
  if (!_tensors(subdomain, var).isAlloc())
    _tensors(subdomain, var).createDevice(n_qps);
}

bool
QpJacobianCache::isAlloc(const unsigned int subdomain, const unsigned int var) const
{
  return _tensors.isAlloc() && _tensors(subdomain, var).isAlloc();
}

void
QpJacobianCache::addBlocks(const unsigned int subdomain,
                           const unsigned int var,
                           const unsigned int blocks)
{
  _blocks(subdomain, var) |= blocks;
}

void
QpJacobianCache::zero()
{
  QpJacobianTensor tensor;
  tensor.zero();

  for (const auto subdomain : libMesh::make_range(_tensors.n(0)))
    for (const auto var : libMesh::make_range(_tensors.n(1)))
      if (_tensors(subdomain, var).isAlloc())
        _tensors(subdomain, var) = tensor;
}

std::size_t
QpJacobianCache::localBytes() const
{
  std::size_t bytes = 0;

  for (const auto subdomain : libMesh::make_range(_tensors.n(0)))
    for (const auto var : libMesh::make_range(_tensors.n(1)))
      if (_tensors(subdomain, var).isAlloc())
        bytes += _tensors(subdomain, var).size() * sizeof(QpJacobianTensor);

  return bytes;
}

void
QpJacobianCache::copyToDevice()
{
  _tensors.copyToDevice();
  _blocks.copyToDevice();
}

} // namespace Moose::Kokkos
