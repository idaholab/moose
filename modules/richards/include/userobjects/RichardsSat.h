/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  saturation as a function of effective saturation, and its derivs wrt effective saturation
//
#ifndef RICHARDSSAT_H
#define RICHARDSSAT_H

#include "GeneralUserObject.h"

class RichardsSat;


template<>
InputParameters validParams<RichardsSat>();

class RichardsSat : public GeneralUserObject
{
 public:
  RichardsSat(const std::string & name, InputParameters parameters);

  void initialize();
  void execute();
  void finalize();

  Real sat(Real seff) const;
  Real dsat(Real /*seff*/) const;
  Real d2sat(Real /*seff*/) const;

 protected:

  Real _s_res;
  Real _sum_s_res;

};

#endif // RICHARDSSAT_H
