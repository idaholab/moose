/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  Base class for Richards SUPG
//
#ifndef RICHARDSSUPG_H
#define RICHARDSSUPG_H

#include "GeneralUserObject.h"

class RichardsSUPG;

template <>
InputParameters validParams<RichardsSUPG>();

/**
 * base class for SUPG of the Richards equation
 * You must override all the functions below with your specific implementation
 */
class RichardsSUPG : public GeneralUserObject
{
public:
  RichardsSUPG(const InputParameters & parameters);

  void initialize();
  void execute();
  void finalize();

  /**
   * SUPG velocity
   * This points in direction of information propagation.
   * @param perm permeability tensor
   * @param gradp gradient of porepressure
   * @param density fluid density
   * @param gravity gravitational acceleration vector
   */
  virtual RealVectorValue velSUPG(RealTensorValue perm,
                                  RealVectorValue gradp,
                                  Real density,
                                  RealVectorValue gravity) const = 0;

  /**
   * derivative of SUPG velocity wrt gradient of porepressure
   * @param perm permeability tensor
   */
  virtual RealTensorValue dvelSUPG_dgradp(RealTensorValue perm) const = 0;

  /**
   * derivative of SUPG velocity wrt porepressure (keeping gradp fixed)
   * @param perm permeability tensor
   * @param density_prime derivative of fluid density wrt porepressure
   * @param gravity gravitational acceleration vector
   */
  virtual RealVectorValue
  dvelSUPG_dp(RealTensorValue perm, Real density_prime, RealVectorValue gravity) const = 0;

  /**
   * |bb| ~ 2*velocity/element_length
   * @param vel SUPG velocity
   * @param dimen dimension of problem
   * @param xi_prime spatial gradient of the isoparametric coordinate xi
   * @param eta_prime spatial gradient of the isoparametric coordinate eta
   * @param zeta_prime spatial gradient of the isoparametric coordinate zeta
   */
  virtual RealVectorValue bb(RealVectorValue vel,
                             int dimen,
                             RealVectorValue xi_prime,
                             RealVectorValue eta_prime,
                             RealVectorValue zeta_prime) const = 0;

  /**
   * derivative of bb*bb wrt gradient of porepressure
   * @param vel SUPG velocity
   * @param dvel_dgradp derivative of velocity wrt gradient of porepressure
   * @param xi_prime spatial gradient of the isoparametric coordinate xi
   * @param eta_prime spatial gradient of the isoparametric coordinate eta
   * @param zeta_prime spatial gradient of the isoparametric coordinate zeta
   */
  virtual RealVectorValue dbb2_dgradp(RealVectorValue vel,
                                      RealTensorValue dvel_dgradp,
                                      RealVectorValue xi_prime,
                                      RealVectorValue eta_prime,
                                      RealVectorValue zeta_prime) const = 0;

  /**
   * derivative of bb*bb wrt porepressure
   * @param vel SUPG velocity
   * @param dvel_dp derivative of velocity wrt porepressure
   * @param xi_prime spatial gradient of the isoparametric coordinate xi
   * @param eta_prime spatial gradient of the isoparametric coordinate eta
   * @param zeta_prime spatial gradient of the isoparametric coordinate zeta
   */
  virtual Real dbb2_dp(RealVectorValue vel,
                       RealVectorValue dvel_dp,
                       RealVectorValue xi_prime,
                       RealVectorValue eta_prime,
                       RealVectorValue zeta_prime) const = 0;

  /**
   * The SUPG tau parameter.
   * This dictates how strong the SUPG is
   * @param vel SUPG velocity
   * @param traceperm trace of the permeability tensor for the material
   * @param b the b parameter: |b| ~ 2*SUPGvelocity/element_length
   */
  virtual Real tauSUPG(RealVectorValue vel, Real traceperm, RealVectorValue b) const = 0;

  /**
   * derivative of tau wrt gradient of porepressure
   * @param vel SUPG velocity
   * @param dvel_dgradp derivative of the SUPG velocity wrt gradient of porepressure
   * @param traceperm trace of the permeability tensor for the material
   * @param b the b parameter: |b| ~ 2*SUPGvelocity/element_length
   * @param db2_dgradp derivative of b*b wrt gradient of porepressure
   */
  virtual RealVectorValue dtauSUPG_dgradp(RealVectorValue vel,
                                          RealTensorValue dvel_dgradp,
                                          Real traceperm,
                                          RealVectorValue b,
                                          RealVectorValue db2_dgradp) const = 0;

  /**
   * derivative of tau wrt porepressure (keeping gradp fixed)
   * @param vel SUPG velocity
   * @param dvel_dp derivative of the SUPG velocity wrt porepressure
   * @param traceperm trace of the permeability tensor for the material
   * @param b the b parameter: |b| ~ 2*SUPGvelocity/element_length
   * @param db2_dp derivative of b*b wrt porepressure
   */
  virtual Real dtauSUPG_dp(RealVectorValue vel,
                           RealVectorValue dvel_dp,
                           Real traceperm,
                           RealVectorValue b,
                           Real db2_dp) const = 0;

  /**
   * Returns true if SUPG is trivial.
   * This may used for optimization since typically
   * SUPG stuff is quite expensive to calculate
   */
  virtual bool SUPG_trivial() const = 0;
};

#endif // RICHARDSSUPG_H
