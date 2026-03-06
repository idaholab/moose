//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

#include "NEML2TestModel2.h"

namespace neml2
{
register_NEML2_object(NEML2TestModel2);

OptionSet
NEML2TestModel2::expected_options()
{
  auto options = Model::expected_options();
  options.set_input("u") = VariableName("state", "u");
  options.set_output("s") = VariableName("state", "s");
  return options;
}

NEML2TestModel2::NEML2TestModel2(const OptionSet & options)
  : Model(options),
    // inputs
    _u(declare_input_variable<Scalar>("u")),
    // outputs
    _s(declare_output_variable<Scalar>("s"))
{
}

void
NEML2TestModel2::set_value(bool out, bool dout_din, bool /*d2out_din2*/)
{
  if (out)
    _s = _u * _u - 0.1;

  if (dout_din)
    _s.d(_u) = 2.0 * _u;
}

} // namespace neml2

#endif // NEML2_ENABLED
