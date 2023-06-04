/*
Define Function for Spatial Distribution of Static Friction Coefficient Mu_s
Problem-Specific: TPV205-2D
*/

#include "StaticFricCoeffMus.h"

registerMooseObject("TensorMechanicsApp", StaticFricCoeffMus);

InputParameters
StaticFricCoeffMus::validParams()
{
  InputParameters params = Function::validParams();
  return params;
}

StaticFricCoeffMus::StaticFricCoeffMus(const InputParameters & parameters) : Function(parameters) {}

Real
StaticFricCoeffMus::value(Real /*t*/, const Point & p) const
{

  Real x_coord = p(0);

  double mu_s = 0.0;
  if (x_coord >= -15.0e3 && x_coord <= 15.0e3)
  {
    mu_s = 0.677;
  }
  else
  {
    mu_s = 10000.0;
  }
  return mu_s;
}
