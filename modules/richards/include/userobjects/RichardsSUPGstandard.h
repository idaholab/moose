//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RichardsSUPG.h"

/**
 * standard SUPG relationships
 * valid for the Richards equation.
 * here i use a formula for "tau" presented in Appendix A of
 * TJR Hughes, M Mallet and A Mizukami ``A new finite element formulation for computational fluid
 * dynamics:: II. Behond SUPG'' Computer Methods in Applied Mechanics and Engineering 54 (1986)
 * 341--355
 */
class RichardsSUPGstandard : public RichardsSUPG
{
public:
  static InputParameters validParams();

  RichardsSUPGstandard(const InputParameters & parameters);

  /**
   * SUPG velocity = -perm*(gradp - density*gravity)
   * This points in direction of information propagation.
   * @param perm permeability tensor
   * @param gradp gradient of porepressure
   * @param density fluid density
   * @param gravity gravitational acceleration vector
   */
  RealVectorValue
  velSUPG(RealTensorValue perm, RealVectorValue gradp, Real density, RealVectorValue gravity) const;

  /**
   * derivative of SUPG velocity wrt gradient of porepressure
   * @param perm permeability tensor
   */
  RealTensorValue dvelSUPG_dgradp(RealTensorValue perm) const;

  /**
   * derivative of SUPG velocity wrt porepressure (keeping gradp fixed)
   * @param perm permeability tensor
   * @param density_prime derivative of fluid density wrt porepressure
   * @param gravity gravitational acceleration vector
   */
  RealVectorValue
  dvelSUPG_dp(RealTensorValue perm, Real density_prime, RealVectorValue gravity) const;

  /**
   * |bb| ~ 2*velocity/element_length
   * @param vel SUPG velocity
   * @param dimen dimension of problem
   * @param xi_prime spatial gradient of the isoparametric coordinate xi
   * @param eta_prime spatial gradient of the isoparametric coordinate eta
   * @param zeta_prime spatial gradient of the isoparametric coordinate zeta
   */
  RealVectorValue bb(RealVectorValue vel,
                     int dimen,
                     RealVectorValue xi_prime,
                     RealVectorValue eta_prime,
                     RealVectorValue zeta_prime) const;

  /**
   * derivative of bb*bb wrt gradient of porepressure
   * @param vel SUPG velocity
   * @param dvel_dgradp derivative of velocity wrt gradient of porepressure
   * @param xi_prime spatial gradient of the isoparametric coordinate xi
   * @param eta_prime spatial gradient of the isoparametric coordinate eta
   * @param zeta_prime spatial gradient of the isoparametric coordinate zeta
   */
  RealVectorValue dbb2_dgradp(RealVectorValue vel,
                              RealTensorValue dvel_dgradp,
                              RealVectorValue xi_prime,
                              RealVectorValue eta_prime,
                              RealVectorValue zeta_prime) const;

  /**
   * derivative of bb*bb wrt porepressure
   * @param vel SUPG velocity
   * @param dvel_dp derivative of velocity wrt porepressure
   * @param xi_prime spatial gradient of the isoparametric coordinate xi
   * @param eta_prime spatial gradient of the isoparametric coordinate eta
   * @param zeta_prime spatial gradient of the isoparametric coordinate zeta
   */
  Real dbb2_dp(RealVectorValue vel,
               RealVectorValue dvel_dp,
               RealVectorValue xi_prime,
               RealVectorValue eta_prime,
               RealVectorValue zeta_prime) const;

  /**
   * The SUPG tau parameter.
   * This dictates how strong the SUPG is
   * @param vel SUPG velocity
   * @param traceperm trace of the permeability tensor for the material
   * @param b the b parameter: |b| ~ 2*SUPGvelocity/element_length
   */
  Real tauSUPG(RealVectorValue vel, Real traceperm, RealVectorValue b) const;

  /**
   * derivative of tau wrt gradient of porepressure
   * @param vel SUPG velocity
   * @param dvel_dgradp derivative of the SUPG velocity wrt gradient of porepressure
   * @param traceperm trace of the permeability tensor for the material
   * @param b the b parameter: |b| ~ 2*SUPGvelocity/element_length
   * @param db2_dgradp derivative of b*b wrt gradient of porepressure
   */
  RealVectorValue dtauSUPG_dgradp(RealVectorValue vel,
                                  RealTensorValue dvel_dgradp,
                                  Real traceperm,
                                  RealVectorValue b,
                                  RealVectorValue db2_dgradp) const;

  /**
   * derivative of tau wrt porepressure (keeping gradp fixed)
   * @param vel SUPG velocity
   * @param dvel_dp derivative of the SUPG velocity wrt porepressure
   * @param traceperm trace of the permeability tensor for the material
   * @param b the b parameter: |b| ~ 2*SUPGvelocity/element_length
   * @param db2_dp derivative of b*b wrt porepressure
   */
  Real dtauSUPG_dp(RealVectorValue vel,
                   RealVectorValue dvel_dp,
                   Real traceperm,
                   RealVectorValue b,
                   Real db2_dp) const;

  /// returns false in this case since everything is trivial
  bool SUPG_trivial() const;

protected:
  /**
   * the SUPG pressure parameter
   * This dictates how strong the SUPG is
   * _p_SUPG large means only a little SUPG
   * _p_SUPG small means close to fully-upwind
   */
  Real _p_SUPG;

private:
  /// cosh(alpha)/sinh(alpha) - 1/alpha, modified at extreme values of alpha to prevent overflows
  Real cosh_relation(Real alpha) const;

  /// derivative of cosh_relation wrt alpha
  Real cosh_relation_prime(Real alpha) const;
};
