#include "PolynomialFreeEnergy.h"

template<>
InputParameters validParams<PolynomialFreeEnergy>()
{
  InputParameters params = validParams<DerivativeParsedMaterialHelper>();
  params.addClassDescription("Polynomial free energy for single component systems");
  MooseEnum poly_order("4 6 8");
  params.addRequiredParam<MooseEnum>("polynomial_order", poly_order, "Order of polynomial free energy");
  params.addParam<MaterialPropertyName>("c_eq_name", "c_eq", "Name of material property storing the equilibrium concentration");
  params.addParam<MaterialPropertyName>("W_name", "barr_height", "Name of the material property storing the barrier height");
  params.addRequiredCoupledVar("c", "Concentration");
  return params;
}

PolynomialFreeEnergy::PolynomialFreeEnergy(const std::string & name,
                                           InputParameters parameters) :
    DerivativeParsedMaterialHelper(name, parameters),
    _c("c"),
    _a("c_eq_name"),
    _W("W_name"),
    _order(getParam<MooseEnum>("polynomial_order"))
{
  EBFunction free_energy;

  // Free energy
  switch (_order)
  {
    case 0: //4th order
      free_energy(_c, _W, _a) = pow(2.0, 4.0)*_W*pow(_c - _a, 2)*pow(1 - _c - _a, 2);
      break;
    case 1: //6th order
      free_energy(_c, _W, _a) = pow(2.0, 6.0)*_W*( 2.0*pow(_c, 6) -
                                6.0*pow(_c, 5) +
                                (3.0*_a + 27.0/4.0 - 3.0*_a*_a)*pow(_c, 4) +
                                (-6.0*_a - 7.0/2.0 + 6.0*_a*_a)*pow(_c, 3) +
                                (9.0/2.0*_a - 9.0/2.0*_a*_a + 3.0/4.0)*pow(_c, 2) +
                                (3.0/2.0*_a*_a - 3.0/2.0*_a)*_c);
      break;
    case 2: //8th order
      free_energy(_c, _W, _a) = pow(2.0, 8.0)*_W*(3.0*pow(_c, 8) -
                                12.0*pow(_c,7) +
                                ( -4.0*_a*_a + 4.0*_a + 20.0 )*pow(_c, 6) +
                                ( 12.0*_a*_a - 12.0*_a - 18.0 )*pow(_c, 5) +
                                ( 15.0*_a + 75.0/8.0 - 15.0*_a*_a )*pow(_c, 4) +
                                ( -10.0*_a - 11.0/4.0 + 10.0*_a*_a )*pow(_c, 3) +
                                ( 15.0/4.0*_a - 15.0/4.0*_a*_a + 3.0/8.0 )*pow(_c, 2) +
                                ( 3.0/4.0*_a*_a - 3.0/4.0*_a )*_c);
      break;
    default:
      mooseError("Error in PolynomialFreeEnergy: incorrect polynomial order");
  }

  std::vector<std::string> nil_str(0);
  std::vector<Real> nil_real(0);
  std::vector<std::string> mat_prop_names(2);
  mat_prop_names[0] = "W_name";
  mat_prop_names[1] = "c_eq_name";

  //Parse function
  functionParse(free_energy, nil_str, nil_str, mat_prop_names, nil_str, nil_real);
}
