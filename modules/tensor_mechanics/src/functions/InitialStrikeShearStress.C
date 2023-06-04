/*
Define Function for Initial Shear Stress along Strike Direction
Problem-Specific: TPV205-2D
*/

#include "InitialStrikeShearStress.h"

registerMooseObject("TensorMechanicsApp", InitialStrikeShearStress);

InputParameters
InitialStrikeShearStress::validParams()
{
  InputParameters params = Function::validParams();
  return params;
}

InitialStrikeShearStress::InitialStrikeShearStress(const InputParameters & parameters)
  : Function(parameters)
{
}

Real
InitialStrikeShearStress::value(Real /*t*/, const Point & p) const
{

  Real x_coord = p(0);

  double T1_o = 0.0;
  if ((x_coord <= (0.0 + 1.5e3)) && (x_coord >= (0.0 - 1.5e3)))
  {
    T1_o = 81.6e6;
  }
  else if ((x_coord <= (-7.5e3 + 1.5e3)) && (x_coord >= (-7.5e3 - 1.5e3)))
  {
    T1_o = 78.0e6;
  }
  else if ((x_coord <= (7.5e3 + 1.5e3)) && (x_coord >= (7.5e3 - 1.5e3)))
  {
    T1_o = 62.0e6;
  }
  else
  {
    T1_o = 70.0e6;
  }
  return T1_o;
}
