/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  Utility functions for van-genuchten effective saturation as a function of pressure
//
#ifndef RICHARDSSEFFVG_H
#define RICHARDSSEFFVG_H

#include "GeneralUserObject.h"

class RichardsSeffVG
{
 public:
  RichardsSeffVG();
  static Real seff(Real p, Real al, Real m);
  static Real dseff(Real p, Real al, Real m);
  static Real d2seff(Real p, Real al, Real m);

};

#endif // RICHARDSSEFFVG_H
