#include "Constant.h"

template<>
Parameters valid_params<Constant>()
{
  Parameters params;
  params.set<Real>("thermal_conductivity")=1.0;
  params.set<Real>("thermal_expansion")=1.0;
  params.set<Real>("specific_heat")=1.0;
  params.set<Real>("density")=1.0;
  params.set<Real>("youngs_modulus")=1.0;
  params.set<Real>("poissons_ratio")=1.0;
  return params;
}

void
Constant::computeProperties()
{
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
    _density[qp] = _my_density;
    _thermal_conductivity[qp] = _my_thermal_conductivity;
    _specific_heat[qp] = _my_specific_heat;
    _thermal_expansion[qp] = _my_thermal_expansion;
    _youngs_modulus[qp]  = _my_youngs_modulus;
    _poissons_ratio[qp] = _my_poissons_ratio;
  }
}
