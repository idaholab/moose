/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Moose.h"

namespace Water_Steam_EOS
{
extern "C" {

// subroutine TSAT(p, 100D0, Ts)
// double TSAT_( double&, double&, double&);

// subroutine water_steam_prop_PH(p, h, Ts,T, Sw, Den, dTdp, dTdh, dDendp, dDendh,  ierr, dhwdp,
// dhsdp, dTdp)
void FORTRAN_CALL(water_steam_prop_ph)(double &,
                                       double &,
                                       double &,
                                       double &,
                                       double &,
                                       double &,
                                       double &,
                                       double &,
                                       double &,
                                       double &,
                                       double &,
                                       double &,
                                       double &,
                                       double &,
                                       double &,
                                       int &,
                                       double &,
                                       double &,
                                       double &);

// subroutine water_steam_prop_PH_ex(p, h, Ts,T, Sw, Den, dTdp, dTdh, dDendp, dDendh,  ierr, dhwdp,
// dhsdp, dTdp, arg1, arg2)
void FORTRAN_CALL(water_steam_prop_ph_ex)(double &,
                                          double &,
                                          double &,
                                          double &,
                                          double &,
                                          double &,
                                          double &,
                                          double &,
                                          double &,
                                          double &,
                                          double &,
                                          double &,
                                          double &,
                                          double &,
                                          double &,
                                          int &,
                                          double &,
                                          double &,
                                          double &,
                                          double &,
                                          double &);

// subroutine water_steam_prop_PH_noderiv(p, h, T, Sw, Den,Denw, Dens, hw, hs,visw,viss,ierror)
void FORTRAN_CALL(water_steam_prop_ph_noderiv)(double &,
                                               double &,
                                               double &,
                                               double &,
                                               double &,
                                               double &,
                                               double &,
                                               double &,
                                               double &,
                                               double &,
                                               double &,
                                               int &);

// subroutine  wateos_noderiv(T, p, dw, dwmol, hw, energyscale, ierr)
// extern "C" double watereos_( double&, double&, double&, double&, double&, double&, double&);
// void water_eos1_( double&, double&, double&);

// subroutine  wateos_noderiv(T, p, dw)  this one is faster, no derivatives calculated
void FORTRAN_CALL(wateos_noderiv1)(double &, double &, double &);

// subroutine  VISW_noderiv1 (rho_s,tc,vs)
void FORTRAN_CALL(viss_noderiv1)(double &, double &, double &);

//================================================================================
//
//
// Following function/subroutines definitions are in source water_steam_functions.f90
//
//--------------------------------------------------------------------------------

void FORTRAN_CALL(boundary_23)(double & P, double & T, int & N);

void FORTRAN_CALL(saturation)(double & P, double & T, int & N, int & nerr);

void FORTRAN_CALL(enthalpy_density_pt)(
    double & H, double & D, double & P, double & T, int & nerr, int & nRegion);

double FORTRAN_CALL(viscosity)(double & rho, double & T);

//////////////////////////////////////////////////////////////////////////////////
//
//   A  list of functions used for BISON coolant channel model
//
//
//   | Function name             | source code file           |
//   |---------------------------+----------------------------|
//   | void water_steam_prop_ph_ | water_steam_phase_prop.f90 |
//   | void viss_noderiv1_       | VISS_noderiv1.f90          |
//   | void boundary_23_         | water_steam_functions.f90  |
//   | void saturation_          | water_steam_functions.f90  |
//   | void enthalpy_density_pt_ | water_steam_functions.f90  |
//   | double viscosity_         | water_steam_functions.f90  |
//   |                           |                            |
//
//
//////////////////////////////////////////////////////////////////////////////////
}
}
