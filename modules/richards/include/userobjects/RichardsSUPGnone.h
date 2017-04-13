/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSSUPGnone_H
#define RICHARDSSUPGnone_H

#include "RichardsSUPG.h"

class RichardsSUPGnone;

template <>
InputParameters validParams<RichardsSUPGnone>();

/**
 * no Richards SUPG.
 * Use this class if you don't want any SUPG for the Richards equations in your simulation
 */
class RichardsSUPGnone : public RichardsSUPG
{
public:
  RichardsSUPGnone(const InputParameters & parameters);

  /// SUPG velocity = zero
  RealVectorValue velSUPG(RealTensorValue /*perm*/,
                          RealVectorValue /*gradp*/,
                          Real /*density*/,
                          RealVectorValue /*gravity*/) const;

  /// derivative of SUPG velocity wrt gradient of porepressure = zero
  RealTensorValue dvelSUPG_dgradp(RealTensorValue /*perm*/) const;

  /// derivative of SUPG velocity wrt poreporessure = zero
  RealVectorValue dvelSUPG_dp(RealTensorValue /*perm*/,
                              Real /*density_prime*/,
                              RealVectorValue /*gravity*/) const;

  /// bb parameter = zero
  RealVectorValue bb(RealVectorValue /*vel*/,
                     int /*dimen*/,
                     RealVectorValue /*xi_prime*/,
                     RealVectorValue /*eta_prime*/,
                     RealVectorValue /*zeta_prime*/) const;

  /// derivative of bb*bb wrt gradient of porepressure = zero
  RealVectorValue dbb2_dgradp(RealVectorValue /*vel*/,
                              RealTensorValue /*dvel_dgradp*/,
                              RealVectorValue /*xi_prime*/,
                              RealVectorValue /*eta_prime*/,
                              RealVectorValue /*zeta_prime*/) const;

  /// derivative of bb*bb wrt porepressure = zero
  Real dbb2_dp(RealVectorValue /*vel*/,
               RealVectorValue /*dvel_dp*/,
               RealVectorValue /*xi_prime*/,
               RealVectorValue /*eta_prime*/,
               RealVectorValue /*zeta_prime*/) const;

  /// tau SUPG parameter = zero
  Real tauSUPG(RealVectorValue /*vel*/, Real /*traceperm*/, RealVectorValue /*b*/) const;

  /// derivative of tau SUPG parameter wrt gradient of porepressure = zero
  RealVectorValue dtauSUPG_dgradp(RealVectorValue /*vel*/,
                                  RealTensorValue /*dvel_dgradp*/,
                                  Real /*traceperm*/,
                                  RealVectorValue /*b*/,
                                  RealVectorValue /*db2_dgradp*/) const;

  /// derivative of tau SUPG parameter wrt porepressure = zero
  Real dtauSUPG_dp(RealVectorValue /*vel*/,
                   RealVectorValue /*dvel_dp*/,
                   Real /*traceperm*/,
                   RealVectorValue /*b*/,
                   Real /*db2_dp*/) const;

  bool SUPG_trivial() const;
};

#endif // RICHARDSSUPGnone_H
