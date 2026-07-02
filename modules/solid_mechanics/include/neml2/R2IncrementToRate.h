//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#if 0 // NEML2 v2->v3 migration: superseded by the Python model modules/solid_mechanics/data/neml2/R2IncrementToRate.py (loaded via the NEML2Action 'load' parameter)

#pragma once

#ifdef NEML2_ENABLED

#include "neml2/models/Model.h"

namespace neml2
{
class R2;

/// Get the rate of a rank two tensor given its increment
class R2IncrementToRate : public Model
{
public:
  static OptionSet expected_options();

  R2IncrementToRate(const OptionSet & options);

protected:
  void set_value(bool out, bool dout_din, bool d2out_din2) override;

  /// Increment of the rank two tensor
  const Variable<R2> & _delta;

  /// Time
  const Variable<Scalar> & _t;
  const Variable<Scalar> & _t_n;

  /// Rate
  Variable<R2> & _rate;
};
} // namespace neml2

#endif // NEML2_ENABLED

#endif // NEML2 v2->v3 migration: DEFERRED
