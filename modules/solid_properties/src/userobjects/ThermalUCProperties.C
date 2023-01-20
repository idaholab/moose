#include "ThermalUCProperties.h"
#include "libmesh/utility.h"

registerMooseObject("SolidPropertiesApp", ThermalUCProperties);

InputParameters
ThermalUCProperties::validParams()

{
  InputParameters params = ThermalSolidProperties::validParams();

  params.addRangeCheckedParam<Real>("density", 13824.7, "density > 0.0", "(Constant) density");
  params.addClassDescription("Uranium Carbide (UC) thermal properties.");
  return params;
}

ThermalUCProperties::ThermalUCProperties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters), _rho_const(getParam<Real>("density"))
{
}

Real
ThermalUCProperties::cp_from_T(const Real & T) const
{
  return 239.7 - 5.068e-3 * T + 1.7604e-5 * Utility::pow<2>(T) - 3488100 / Utility::pow<2>(T);
}

void
ThermalUCProperties::cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const
{
  cp = cp_from_T(T);
  dcp_dT = - 5.068e-3 - 3.5208e-5 * T + 6976200 / Utility::pow<3>(T);
}

Real
ThermalUCProperties::k_from_T(const Real & T) const
{

 if (323 < T < 923){
                   return 21.7 - 3.04e-3 * (T-273) + 3.61e-6 * Utility::pow<2>(T-273);
         }
 if (924 < T < 2573){
                    return 20.2 + 1.48e-3 * (T-273);
         }

}

void
ThermalUCProperties::k_from_T(const Real & T, Real & k, Real & dk_dT) const
{

 if (323 < T < 923){
                   k = k_from_T(T);
                   dk_dT = - 3.04e-3 + 6.32e-6 * (T-273);
         }
 if (924 < T < 2573){
                    k = k_from_T(T);
                    dk_dT = 1.48e-3;
         }
}


Real
ThermalUCProperties::rho_from_T(const Real & /* T */) const
{
  return _rho_const;
}

void
ThermalUCProperties::rho_from_T(const Real & T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);
  drho_dT = 0.0;
}
