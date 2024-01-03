/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/*                       BlackBear                              */
/*                                                              */
/*           (c) 2017 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
    VariadicTable<std::string, TorchSize> table({"Variable", "Storage size"});
    for (const auto & var : axis.variable_accessors(/*recursive=*/true))
      table.addRow(utils::stringify(var), axis.storage_size(var));
    table.print(os);
  };

  os << "Input:" << std::endl;
  print_axis(os, model.input());

  os << std::endl;

  os << "Output:" << std::endl;
  print_axis(os, model.output());

  os << std::endl;

  os << "Parameters: " << std::endl;
  VariadicTable<std::string, std::string> table({"Parameter", "Requires grad"});
  for (auto && [name, value] : model.named_parameters(/*recursive=*/true))
    table.addRow(name, value.requires_grad() ? "True" : "False");
  table.print(os);

  return os;
}
} // namespace neml2

namespace NEML2Utils
{

template <>
neml2::BatchTensor
toNEML2(const Real & v)
{
  return neml2::Scalar(v, neml2::default_tensor_options);
}

// FIXME: This is an unfortunately specialization because the models I included for testing use
// symmetric tensors everywhere. Once I tested all the models with full tensors (i.e. not in Mandel
// notation), I should be able to "fix" this specialization.
template <>
neml2::BatchTensor
toNEML2(const RankTwoTensor & r2t)
{
  return neml2::SR2::fill(r2t(0, 0), r2t(1, 1), r2t(2, 2), r2t(1, 2), r2t(0, 2), r2t(0, 1));
}

template <>
neml2::BatchTensor
toNEML2(const SymmetricRankTwoTensor & r2t)
{
  return neml2::SR2::fill(r2t(0, 0), r2t(1, 1), r2t(2, 2), r2t(1, 2), r2t(0, 2), r2t(0, 1));
}

template <>
neml2::BatchTensor
toNEML2(const std::vector<Real> & v)
{
  return neml2::BatchTensor(torch::tensor(v, neml2::default_tensor_options), 0);
}

template <>
SymmetricRankTwoTensor
toMOOSE(const neml2::BatchTensor & t)
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
toMOOSE(const neml2::BatchTensor & t)
{
  auto tc = t.contiguous();
  return std::vector<Real>(tc.data_ptr<neml2::Real>(), tc.data_ptr<neml2::Real>() + tc.numel());
}

template <>
SymmetricRankFourTensor
toMOOSE(const neml2::BatchTensor & t)
{
  // Well I don't see a good constructor for this, so let me fill out all the components.
  SymmetricRankFourTensor symsymr4t;
  for (const auto a : make_range(6))
    for (const auto b : make_range(6))
      symsymr4t(a, b) = t.base_index({a, b}).item<neml2::Real>();

  return symsymr4t;
}

} // namespace NEML2Utils

#endif

#include "MooseObject.h"
#include "Action.h"

namespace NEML2Utils
{

static const std::string message1 =
    "To use this object, you need to have the `NEML2` library installed. Refer to the "
    "documentation for guidance on how to enable it.";
static const std::string message2 =
    " To build this library MOOSE must be configured with LIBTORCH support.";

void
checkLibraryAvailability(MooseObject & self)
{
#ifndef NEML2_ENABLED
#ifdef LIBTORCH_ENABLED
  self.paramError("type", message1);
#else
  self.paramError("type", message1 + message2);
#endif
#else
  libmesh_ignore(self);
#endif
}

void
checkLibraryAvailability(Action & self)
{
#ifndef NEML2_ENABLED
#ifdef LIBTORCH_ENABLED
  mooseError(self.parameters().blockLocation() + ": " + message1);
#else
  mooseError(self.parameters().blockLocation() + ": " + message1 + message2);
#endif
#else
  libmesh_ignore(self);
#endif
}

} // namespace NEML2Utils
