//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "neml2/models/Model.h"

namespace neml2
{

class NEML2TestModel : public Model
{
public:
  static OptionSet expected_options();

  NEML2TestModel(const OptionSet & options);

  void request_AD() override;

protected:
  // input variables
  const Variable<Scalar> & _input_a;
  const Variable<Scalar> & _input_b;

  // output variables
  Variable<Scalar> & _sum;
  Variable<Scalar> & _product;

  /// Compute
  virtual void set_value(bool out, bool dout_din, bool d2out_din2) override;
};

} // namespace neml2
