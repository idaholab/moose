//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2Utils.h"

#ifdef NEML2_ENABLED

#include "VariadicTable.h"

namespace neml2
{

std::ostream &
operator<<(std::ostream & os, const Model & model)
{
  auto print_axis = [](std::ostream & os, const LabeledAxis & axis)
  {
    VariadicTable<std::string, Size> table({"Variable", "Storage size"});
    for (const auto & var : axis.variable_names())
      table.addRow(utils::stringify(var), axis.storage_size(var));
    table.print(os);
  };

  os << "Input:" << std::endl;
  print_axis(os, model.input_axis());

  os << std::endl;

  os << "Output:" << std::endl;
  print_axis(os, model.output_axis());

  os << std::endl;

  os << "Parameters: " << std::endl;
  VariadicTable<std::string, std::string> table({"Parameter", "Requires grad"});
  for (auto && [name, param] : model.named_parameters())
    table.addRow(name, Tensor(param).requires_grad() ? "True" : "False");
  table.print(os);

  return os;
}
} // namespace neml2

#endif // NEML2_ENABLED

namespace NEML2Utils
{

#ifdef NEML2_ENABLED

neml2::VariableName
getOldName(const neml2::VariableName & var)
{
  if (var.start_with("forces"))
    return var.slice(1).prepend("old_forces");

  if (var.start_with("state"))
    return var.slice(1).prepend("old_state");

  mooseError("An error occurred when trying to map a stateful NEML2 variable name '",
             var,
             "' onto its old counterpart. The leading sub-axis of the variable should either be "
             "'state' or 'forces'. However, we got '",
             var.slice(0, 1),
             "'");
}

template <>
neml2::Tensor
toNEML2(const Real & v)
{
  return neml2::Scalar(v, neml2::default_tensor_options());
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

static const std::string message_all =
    "To use this object, you need to have the `NEML2` library installed. Refer to the "
    "documentation for guidance on how to enable it.";
#ifdef LIBTORCH_ENABLED
static const std::string message = message_all;
#else
static const std::string message =
    message_all + " To build this library MOOSE must be configured with `LIBTORCH` support!";
#endif

void
addClassDescription(InputParameters & params, const std::string & desc)
{
#ifdef NEML2_ENABLED
  params.addClassDescription(desc);
#else
  params.addClassDescription(message + " (Original description: " + desc + ")");
#endif
}

void
libraryNotEnabledError(const InputParameters & params)
{
#ifndef NEML2_ENABLED
  mooseError(params.blockLocation() + ": " + message);
#else
  libmesh_ignore(params);
  static_assert(
      "Only place libraryNotEnabledError() in a branch that is compiled if NEML2 is not enabled!");
#endif
}

} // namespace NEML2Utils
