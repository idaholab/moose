//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolynomialFreeEnergy.h"

registerMooseObject("PhaseFieldApp", PolynomialFreeEnergy);

InputParameters
PolynomialFreeEnergy::validParams()
{
  InputParameters params = DerivativeParsedMaterialHelper::validParams();
  params.addClassDescription("Polynomial free energy for single component systems");
  MooseEnum poly_order("4 6 8");
  params.addRequiredParam<MooseEnum>(
      "polynomial_order", poly_order, "Order of polynomial free energy");
  params.addParam<MaterialPropertyName>(
      "c_eq_name", "c_eq", "Name of material property storing the equilibrium concentration");
  params.addParam<MaterialPropertyName>(
      "W_name", "barr_height", "Name of the material property storing the barrier height");
  params.addRequiredCoupledVar("c", "Concentration");
  return params;
}

PolynomialFreeEnergy::PolynomialFreeEnergy(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper(parameters),
    _c("c"),
    _a("c_eq_name"),
    _W("W_name"),
    _order(getParam<MooseEnum>("polynomial_order"))
{
  EBFunction free_energy;

  // Free energy
  switch (_order)
  {
    case 0: // 4th order
      free_energy(_c, _W, _a) = pow(2.0, 4.0) * _W * pow(_c - _a, 2) * pow(1 - _c - _a, 2);
      break;
    case 1: // 6th order
      free_energy(_c, _W, _a) = pow(2.0, 6.0) * _W *
                                (2.0 * pow(_c, 6) - 6.0 * pow(_c, 5) +
                                 (3.0 * _a + 27.0 / 4.0 - 3.0 * _a * _a) * pow(_c, 4) +
                                 (-6.0 * _a - 7.0 / 2.0 + 6.0 * _a * _a) * pow(_c, 3) +
                                 (9.0 / 2.0 * _a - 9.0 / 2.0 * _a * _a + 3.0 / 4.0) * pow(_c, 2) +
                                 (3.0 / 2.0 * _a * _a - 3.0 / 2.0 * _a) * _c);
      break;
    case 2: // 8th order
      free_energy(_c, _W, _a) =
          pow(2.0, 8.0) * _W *
          (3.0 * pow(_c, 8) - 12.0 * pow(_c, 7) + (-4.0 * _a * _a + 4.0 * _a + 20.0) * pow(_c, 6) +
           (12.0 * _a * _a - 12.0 * _a - 18.0) * pow(_c, 5) +
           (15.0 * _a + 75.0 / 8.0 - 15.0 * _a * _a) * pow(_c, 4) +
           (-10.0 * _a - 11.0 / 4.0 + 10.0 * _a * _a) * pow(_c, 3) +
           (15.0 / 4.0 * _a - 15.0 / 4.0 * _a * _a + 3.0 / 8.0) * pow(_c, 2) +
           (3.0 / 4.0 * _a * _a - 3.0 / 4.0 * _a) * _c);
      break;
    default:
      mooseError("Error in PolynomialFreeEnergy: incorrect polynomial order");
  }

  // Parse function
  functionParse(free_energy, {}, {}, {"W_name", "c_eq_name"}, {}, {});
}
