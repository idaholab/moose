//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchRandomUtils.h"

#include <ATen/CPUGeneratorImpl.h>
#include <ATen/ops/linalg_qr.h>
#include <ATen/ops/normal.h>

#include "MooseError.h"

namespace Moose
{

at::Generator
makeLibtorchCPUGenerator()
{
  return at::detail::createCPUGenerator();
}

at::Generator
makeLibtorchCPUGenerator(const uint64_t seed)
{
  return at::detail::createCPUGenerator(seed);
}

void
orthogonalInitializeTensor(torch::Tensor & tensor,
                           const Real gain,
                           const c10::optional<at::Generator> generator)
{
  if (tensor.ndimension() < 2)
    mooseError("Only tensors with 2 or more dimensions are supported for orthogonal "
               "initialization.");

  if (!tensor.numel())
    return;

  torch::NoGradGuard no_grad;

  const auto rows = tensor.size(0);
  const auto cols = tensor.numel() / rows;
  auto flattened = torch::empty({rows, cols}, tensor.options());
  at::normal_out(flattened, 0.0, 1.0, {rows, cols}, generator);

  if (rows < cols)
    flattened = flattened.transpose(0, 1);

  auto qr = at::linalg_qr(flattened, "reduced");
  auto q = std::get<0>(qr);
  const auto phases = torch::diag(std::get<1>(qr), 0).sign();
  q = q * phases;

  if (rows < cols)
    q = q.transpose(0, 1);

  tensor.view_as(q).copy_(q);
  tensor.mul_(gain);
}

} // namespace Moose

#endif
