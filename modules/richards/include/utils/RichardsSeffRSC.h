/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSSEFFRSC_H
#define RICHARDSSEFFRSC_H

#include "GeneralUserObject.h"

/**
 * Rogers-Stallybrass-Clements version of effective saturation as a function of CAPILLARY pressure.
 * valid for residual saturations = 0, and viscosityOil = 2*viscosityWater.  (the "2" is important
 * here!).
 * C Rogers, MP Stallybrass and DL Clements "On two phase filtration under gravity and with boundary
 * infiltration: application of a Backlund transformation" Nonlinear Analysis Theory Methods and
 * Applications 7 (1983) 785--799.
 */
class RichardsSeffRSC
{
public:
  /**
   * effective saturation as a function of capillary pressure
   * @param pc capillary pressure
   * @param shift RSC's shift parameter
   * @param scale RSC's scale parameter
   */
  static Real seff(Real pc, Real shift, Real scale);

  /**
   * derivative of effective saturation wrt capillary pressure
   * @param pc capillary pressure
   * @param shift RSC's shift parameter
   * @param scale RSC's scale parameter
   */
  static Real dseff(Real pc, Real shift, Real scale);

  /**
   * 2nd derivative of effective saturation wrt capillary pressure
   * @param pc capillary pressure
   * @param shift RSC's shift parameter
   * @param scale RSC's scale parameter
   */
  static Real d2seff(Real pc, Real shift, Real scale);
};

#endif // RICHARDSSEFFRSC_H
