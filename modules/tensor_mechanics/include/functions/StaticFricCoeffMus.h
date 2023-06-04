/*
Define Function for Spatial Distribution of Static Friction Coefficient Mu_s
*/

#pragma once

#include "Function.h"

class StaticFricCoeffMus : public Function
{
public:
  StaticFricCoeffMus(const InputParameters & parameters);

  static InputParameters validParams();

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;
};
