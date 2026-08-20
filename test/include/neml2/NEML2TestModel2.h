//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

#pragma once

#include "neml2/models/Model.h"

namespace neml2
{

class NEML2TestModel2 : public Model
{
public:
  static OptionSet expected_options();

  NEML2TestModel2(const OptionSet & options);

protected:
  void set_value(bool out, bool dout_din, bool d2out_din2) override;

  /// input variables
  const Variable<Scalar> & _u;

  /// output variables
  Variable<Scalar> & _s;
};

} // namespace neml2

#endif // NEML2_ENABLED
