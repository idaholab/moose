#include "UO2.h"


void
UO2::computeProperties()
{
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
    double a, b, c, d;
    if( 923.0 > _temp[qp] )
    {
      a =  0.997      ;
      b =  9.082e-6   ;
      c = -2.705e-10 ;
      d =  4.391e-13  ;
    }
    else
    {
      a =  0.997      ;
      b =  1.179e-5   ;
      c = -2.429e-9   ;
      d =  1.219e-12  ;
    }

    double term     = a + b*_temp[qp] + c*_temp[qp]*_temp[qp] + d*_temp[qp]*_temp[qp]*_temp[qp];
    _density[qp]    = 10960.0 / pow(term, 3);

    double lambda0  = 1.0 / (3.24e-2 + 2.51e-4*_temp[qp]);
    double theta    = 3.67*exp(-4.73e-4*_temp[qp])*sqrt(2.0*_oxygen[qp]*lambda0);
    _thermal_conductivity[qp] = lambda0*atan(theta)/theta + 5.95e-11*_temp[qp]*_temp[qp]*_temp[qp];

    _specific_heat[qp] = 264256.0 + 47.0*_temp[qp];

    // Page 2-46 in MA_temp[qp]PRO, pg 119 Olander (1/K)
    _thermal_expansion[qp] = 1.0e-5 * (1.0 - 5.1 * _oxygen[qp]);
    
    // Page 2-55 in MATPRO Volume 4 (0.05 is fuel porosity value - need a better way to do this)
    _youngs_modulus[qp]  = 2.334e11 * (1.0 - 2.752 * (0.05)) * (1.0 - 1.0915e-4 * _temp[qp]) * exp(-1.75 * _oxygen[qp]); // (Pa)

    // Made up by Derek
    _poissons_ratio[qp] = .3;
  }
}

