//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#if 0 // NEML2 v2->v3 migration: superseded by the Python model
      // modules/solid_mechanics/data/neml2/R2IncrementToRate.py (loaded via the NEML2Action 'load'
      // parameter)

#include "R2IncrementToRate.h"

#ifdef NEML2_ENABLED

#include "neml2/tensors/R2.h"
#include "neml2/tensors/functions/imap.h"

namespace neml2
{
register_NEML2_object(R2IncrementToRate);

OptionSet
R2IncrementToRate::expected_options()
{
  OptionSet options = Model::expected_options();
  options.doc() = "Compute the rate of a rank two tensor given its increment and the time step.";
  options.add_input("increment", "Increment of the rank two tensor");
  options.add_input("time", "t", "Current time");
  options.add_output("rate", "Rate of the rank two tensor");
  return options;
}

R2IncrementToRate::R2IncrementToRate(const OptionSet & options)
  : Model(options),
    _delta(declare_input_variable<R2>("increment")),
    _t(declare_input_variable<Scalar>("t")),
    _t_n(declare_input_variable<Scalar>(history_name(_t.name(), 1))),
    _rate(declare_output_variable<R2>("rate"))
{
}

void
R2IncrementToRate::set_value(bool out, bool dout_din, bool /*d2out_din2*/)
{
  if (out)
    _rate = _delta / (_t - _t_n);

  if (dout_din)
  {
    _rate.d(_delta) = 1.0 / (_t - _t_n) * imap_v<R2>();
    _rate.d(_t) = -_delta / (_t - _t_n) / (_t - _t_n);
    _rate.d(_t_n) = _delta / (_t - _t_n) / (_t - _t_n);
  }
}
} // namespace neml2

#endif // NEML2_ENABLED

#endif // NEML2 v2->v3 migration: DEFERRED
