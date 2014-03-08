/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  None Richards SUPG
//
#ifndef RICHARDSSUPGnone_H
#define RICHARDSSUPGnone_H

#include "RichardsSUPG.h"

class RichardsSUPGnone;


template<>
InputParameters validParams<RichardsSUPGnone>();

class RichardsSUPGnone : public RichardsSUPG
{
 public:
  RichardsSUPGnone(const std::string & name, InputParameters parameters);

  RealVectorValue velSUPG(RealTensorValue /*perm*/, RealVectorValue /*gradp*/, Real /*density*/, RealVectorValue /*gravity*/) const;
  RealTensorValue dvelSUPG_dgradp(RealTensorValue /*perm*/) const;
  RealVectorValue dvelSUPG_dp(RealTensorValue /*perm*/, Real /*density_prime*/, RealVectorValue /*gravity*/) const;
  RealVectorValue bb(RealVectorValue /*vel*/, int /*dimen*/, RealVectorValue /*xi_prime*/, RealVectorValue /*eta_prime*/, RealVectorValue /*zeta_prime*/) const;
  RealVectorValue dbb2_dgradp(RealVectorValue /*vel*/, RealTensorValue /*dvel_dgradp*/, RealVectorValue /*xi_prime*/, RealVectorValue /*eta_prime*/, RealVectorValue /*zeta_prime*/) const;
  Real dbb2_dp(RealVectorValue /*vel*/, RealVectorValue /*dvel_dp*/, RealVectorValue /*xi_prime*/, RealVectorValue /*eta_prime*/, RealVectorValue /*zeta_prime*/) const;
  Real tauSUPG(RealVectorValue /*vel*/, Real /*traceperm*/, RealVectorValue /*b*/) const;
  RealVectorValue dtauSUPG_dgradp(RealVectorValue /*vel*/, RealTensorValue /*dvel_dgradp*/, Real /*traceperm*/, RealVectorValue /*b*/, RealVectorValue /*db2_dgradp*/) const;
  Real dtauSUPG_dp(RealVectorValue /*vel*/, RealVectorValue /*dvel_dp*/, Real /*traceperm*/, RealVectorValue /*b*/, Real /*db2_dp*/) const;

};

#endif // RICHARDSSUPGnone_H
