#include "VaporMixtureInterface.h"

template <>
InputParameters
validParams<VaporMixtureInterface<>>()
{
  InputParameters params = emptyInputParameters();

  params.addRequiredCoupledVar("x_secondary_vapors", "Mass fractions of secondary vapors");
  params.addRequiredParam<UserObjectName>("fp_vapor_mixture",
                                          "Vapor mixture fluid properties user object name");

  return params;
}
