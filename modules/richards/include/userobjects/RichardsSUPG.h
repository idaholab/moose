/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  Base class for Richards SUPG
//
#ifndef RICHARDSSUPG_H
#define RICHARDSSUPG_H

#include "GeneralUserObject.h"

class RichardsSUPG;


template<>
InputParameters validParams<RichardsSUPG>();

class RichardsSUPG : public GeneralUserObject
{
 public:
  RichardsSUPG(const std::string & name, InputParameters parameters);

  void initialize();
  void execute();
  void finalize();

  // These functions must be over-ridden in the derived class
  // to provide the actual values of tauSUPG, etc
  virtual RealVectorValue velSUPG(RealTensorValue perm, RealVectorValue gradp, Real density, RealVectorValue gravity) const = 0;
  virtual RealTensorValue dvelSUPG_dgradp(RealTensorValue perm) const = 0;
  virtual RealVectorValue dvelSUPG_dp(RealTensorValue perm, Real density_prime, RealVectorValue gravity) const = 0;
  virtual RealVectorValue bb(RealVectorValue vel, int dimen, RealVectorValue xi_prime, RealVectorValue eta_prime, RealVectorValue zeta_prime) const = 0;
  virtual RealVectorValue dbb2_dgradp(RealVectorValue vel, RealTensorValue dvel_dgradp, RealVectorValue xi_prime, RealVectorValue eta_prime, RealVectorValue zeta_prime) const = 0;
  virtual Real dbb2_dp(RealVectorValue vel, RealVectorValue dvel_dp, RealVectorValue xi_prime, RealVectorValue eta_prime, RealVectorValue zeta_prime) const = 0;
  virtual Real tauSUPG(RealVectorValue vel, Real traceperm, RealVectorValue b) const = 0;
  virtual RealVectorValue dtauSUPG_dgradp(RealVectorValue vel, RealTensorValue dvel_dgradp, Real traceperm, RealVectorValue b, RealVectorValue db2_dgradp) const = 0;
  virtual Real dtauSUPG_dp(RealVectorValue vel, RealVectorValue dvel_dp, Real traceperm, RealVectorValue b, Real db2_dp) const = 0;
};

#endif // RICHARDSSUPG_H
