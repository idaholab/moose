//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2Utils.h"
#include "SubProblem.h"

#ifdef NEML2_ENABLED

#include "VariadicTable.h"

namespace neml2
{

std::ostream &
operator<<(std::ostream & os, const Model & model)
{
  VariadicTable<std::string, std::string, std::string, Size> table(
      {"Type", "Name", "Tensor type", "Storage size"});

  for (const auto && [name, var] : model.input_variables())
    table.addRow("input", utils::stringify(name), utils::stringify(var.type()), var.base_storage());

  for (const auto && [name, var] : model.output_variables())
    table.addRow(
        "output", utils::stringify(name), utils::stringify(var.type()), var.base_storage());

  for (auto && [name, param] : model.named_parameters())
    table.addRow("parameter", name, utils::stringify(param.type()), Tensor(param).base_storage());

  for (auto && [name, buffer] : model.named_buffers())
    table.addRow("buffer", name, utils::stringify(buffer.type()), Tensor(buffer).base_storage());

  table.print(os);

  return os;
}
} // namespace neml2

#endif // NEML2_ENABLED

namespace NEML2Utils
{

#ifdef NEML2_ENABLED
void
assertVariable(const neml2::VariableName & v)
{
  if (v.empty())
    mooseError("Empty NEML2 variable");

  if (!v.start_with("forces") && !v.start_with("state"))
    mooseError("The NEML2 variable '",
               v,
               "' should be defined on the forces or the state sub-axis, got ",
               v.slice(0, 1),
               " instead");
}

void
assertOldVariable(const neml2::VariableName & v)
{
  if (v.empty())
    mooseError("Empty NEML2 variable");

  if (!v.start_with("old_forces") && !v.start_with("old_state"))
    mooseError("The NEML2 variable '",
               v,
               "' should be defined on the old_forces or the old_state sub-axis, got ",
               v.slice(0, 1),
               " instead");
}

template <>
neml2::Tensor
toNEML2(const Real & v)
{
  return neml2::Scalar::full(v);
}

// FIXME: This is an unfortunately specialization because the models I included for testing use
// symmetric tensors everywhere. Once I tested all the models with full tensors (i.e. not in Mandel
// notation), I should be able to "fix" this specialization.
template <>
neml2::Tensor
toNEML2(const RankTwoTensor & r2t)
{
  return neml2::SR2::fill(r2t(0, 0), r2t(1, 1), r2t(2, 2), r2t(1, 2), r2t(0, 2), r2t(0, 1));
}

template <>
neml2::Tensor
toNEML2(const SymmetricRankTwoTensor & r2t)
{
  return neml2::SR2::fill(r2t(0, 0), r2t(1, 1), r2t(2, 2), r2t(1, 2), r2t(0, 2), r2t(0, 1));
}

template <>
neml2::Tensor
toNEML2(const std::vector<Real> & v)
{
  return neml2::Tensor(torch::tensor(v, neml2::default_tensor_options()), 0);
}

template <>
Real
toMOOSE(const neml2::Tensor & t)
{
  return t.item<Real>();
}

template <>
SymmetricRankTwoTensor
toMOOSE(const neml2::Tensor & t)
{
  using symr2t = SymmetricRankTwoTensor;
  return symr2t(t.base_index({0}).item<neml2::Real>() / symr2t::mandelFactor(0),
                t.base_index({1}).item<neml2::Real>() / symr2t::mandelFactor(1),
                t.base_index({2}).item<neml2::Real>() / symr2t::mandelFactor(2),
                t.base_index({3}).item<neml2::Real>() / symr2t::mandelFactor(3),
                t.base_index({4}).item<neml2::Real>() / symr2t::mandelFactor(4),
                t.base_index({5}).item<neml2::Real>() / symr2t::mandelFactor(5));
}

template <>
std::vector<Real>
toMOOSE(const neml2::Tensor & t)
{
  auto tc = t.contiguous();
  return std::vector<Real>(tc.data_ptr<neml2::Real>(), tc.data_ptr<neml2::Real>() + tc.numel());
}

template <>
SymmetricRankFourTensor
toMOOSE(const neml2::Tensor & t)
{
  // Well I don't see a good constructor for this, so let me fill out all the components.
  SymmetricRankFourTensor symsymr4t;
  for (const auto a : make_range(6))
    for (const auto b : make_range(6))
      symsymr4t(a, b) = t.base_index({a, b}).item<neml2::Real>();

  return symsymr4t;
}

#endif // NEML2_ENABLED

static const std::string missing_neml2 = "The `NEML2` library is required but not enabled. Refer "
                                         "to the documentation for guidance on how to enable it.";

bool
shouldCompute(const SubProblem & problem)
{
  // NEML2 computes residual and Jacobian together at EXEC_LINEAR
  // There is no work to be done at EXEC_NONLINEAR **UNLESS** we are computing the Jacobian for
  // automatic scaling.
  if (problem.computingScalingJacobian())
    return true;

  if (problem.currentlyComputingResidualAndJacobian())
    return true;

  if (problem.currentlyComputingJacobian())
    return false;

  return true;
}

std::string
docstring(const std::string & desc)
{
#ifndef NEML2_ENABLED
  return missing_neml2 + " (Original description: " + desc + ")";
#else
  return desc;
#endif
}

void
assertNEML2Enabled()
{
#ifndef NEML2_ENABLED
  mooseError(missing_neml2);
#endif
}

} // namespace NEML2Utils
