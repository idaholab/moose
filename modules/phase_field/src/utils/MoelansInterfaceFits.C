//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MoelansInterfaceFits.h"
#include "InputParameters.h"
#include "MooseEnum.h"
#include "MathUtils.h"

#include <array>

namespace MoelansInterfaceFits
{
InputParameters
validParams()
{
  InputParameters params = emptyInputParameters();
  // the enum values match the Fit enum class
  MooseEnum fit("moelans2022=0 moelans2009=1", "moelans2022");
  params.addParam<MooseEnum>(
      "interface_fit",
      fit,
      "Polynomial fits relating gamma to the interfacial free energy and diffuse interface width: "
      "'moelans2022' (N. Moelans, Mater. Des. 217, 110592, 2022, valid for 0.53 <= gamma <= 40) "
      "or the earlier 'moelans2009' fits");
  return params;
}

Real
inverseGamma(Real g2, Fit fit)
{
  // coefficients in ascending powers of g^2
  static constexpr std::array<Real, 5> c2022{{2.0033, -8.1819, 10.323, -1.8169, -3.0944}};
  static constexpr std::array<Real, 5> c2009{{2.007, -8.183, 9.965, -0.09364, -5.288}};
  return MathUtils::polynomial(fit == Fit::MOELANS2022 ? c2022 : c2009, g2);
}

Real
f0Interf(Real inverse_gamma, Fit fit)
{
  // coefficients in ascending powers of 1/gamma
  static constexpr std::array<Real, 7> c2022{
      {0.2907, -0.5563, 1.0686, -1.5281, 1.2244, -0.4955, 0.0788}};
  static constexpr std::array<Real, 7> c2009{
      {0.2792, -0.4324, 0.6107, -0.7749, 0.6367, -0.2924, 0.05676}};
  return MathUtils::polynomial(fit == Fit::MOELANS2022 ? c2022 : c2009, inverse_gamma);
}

bool
inRange(Real g2, Fit fit)
{
  // the 2022 fit covers 0.098 <= g <= 0.766, i.e. 0.53 <= gamma <= 40 (Moelans 2022, Sec. 5)
  if (fit == Fit::MOELANS2022)
    return g2 >= 0.098 * 0.098 && g2 <= 0.766 * 0.766;

  // no range is documented for the 2009 fit; it is not checked so earlier results are reproduced
  return true;
}
}
