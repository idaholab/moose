//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

#include "NEML2TestModel.h"
#include "neml2/misc/assertions.h"

namespace neml2
{
register_NEML2_object(NEML2TestModel);

OptionSet
NEML2TestModel::expected_options()
{
  auto options = Model::expected_options();
  options.set_input("A") = VariableName("forces", "A");
  options.set_input("B") = VariableName("forces", "B");
  options.set_output("sum") = VariableName("state", "internal", "sum");
  options.set_output("product") = VariableName("state", "internal", "product");
  options.set_parameter<TensorName<Scalar>>("p1") = "1";
  options.set_parameter<TensorName<Scalar>>("p2") = "1";
  options.set<bool>("error") = false;
  options.set<bool>("ad") = true;
  return options;
}

NEML2TestModel::NEML2TestModel(const OptionSet & options)
  : Model(options),
    // inputs
    _input_a(declare_input_variable<Scalar>("A")),
    _input_b(declare_input_variable<Scalar>("B")),
    // outputs
    _sum(declare_output_variable<Scalar>("sum")),
    _product(declare_output_variable<Scalar>("product")),
    // parameters
    _p1(declare_parameter<Scalar>("p1", "p1")),
    _p2(declare_parameter<Scalar>("p2", "p2")),
    // for testing purposes
    _error(options.get<bool>("error")),
    _ad(options.get<bool>("ad"))
{
}

void
NEML2TestModel::request_AD()
{
  if (!_ad)
    return;

  std::vector<const VariableBase *> inputs = {&_input_a, &_input_b};

  // First derivatives
  _sum.request_AD(inputs);
  _product.request_AD(inputs);
}

void
NEML2TestModel::set_value(bool out, bool dout_din, bool /*d2out_din2*/)
{
  neml_assert(!_error, "Error flag set!");

  if (out)
  {
    _sum = _p1 * _input_a + _p2 * _input_b;
    _product = _p1 * _input_a * _p2 * _input_b;
  }

  if (!_ad && dout_din)
  {
    _sum.d(_input_a) = _p1;
    _sum.d(_input_b) = _p2;
    _product.d(_input_a) = _p1 * _p2 * _input_b;
    _product.d(_input_b) = _p1 * _p2 * _input_a;
  }
}

} // namespace neml2

#endif // NEML2_ENABLED
