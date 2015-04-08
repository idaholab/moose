/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FINITESTRAINMOHRCOULOMB
#define FINITESTRAINMOHRCOULOMB

#include "FiniteStrainPlasticBase.h"

class FiniteStrainMohrCoulomb;

template<>
InputParameters validParams<FiniteStrainMohrCoulomb>();

/**
 * FiniteStrainMohrCoulomb implements rate-independent associative Mohr-Coulomb failure
 * with hardening/softening in the finite-strain framework.
 * The smoothing of the tip of the yield-surface cone is described in
 * Zienkiewicz and Prande "Some useful forms of isotropic yield surfaces for soil and rock mechanics" (1977) In G Gudehus (editor) "Finite Elements in Geomechanics" Wile, Chichester, pp 179-190.
 * The smoothing of the edges of the cone is described in
 * AJ Abbo, AV Lyamin, SW Sloan, JP Hambleton "A C2 continuous approximation to the Mohr-Coulomb yield surface" International Journal of Solids and Structures 48 (2011) 3001-3010
 */
class FiniteStrainMohrCoulomb : public FiniteStrainPlasticBase
{
public:
  FiniteStrainMohrCoulomb(const std::string & name, InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();

  /// The cohesion at zero hardening
  Real _cohesion;

  /// friction angle at zero hardening
  Real _phi;

  /// dilation angle at zero hardening
  Real _psi;

  /// The cohesion_residual
  Real _cohesion_residual;

  /// friction angle_residual
  Real _phi_residual;

  /// dilation angle_residual
  Real _psi_residual;

  /// Logarithmic rate of change of cohesion to _cohesion_residual
  Real _cohesion_rate;

  /// Logarithmic rate of change of _phi to _phi_residual
  Real _phi_rate;

  /// Logarithmic rate of change of _psi to _psi_residual
  Real _psi_rate;

  /// Square of tip smoothing parameter to smooth the cone at mean_stress = T
  Real _small_smoother2;

  /// edge smoothing parameter, in radians
  Real _tt;

  /// cos(_tt)
  Real _costt;

  /// sin(_tt)
  Real _sintt;

  /// cos(3*_tt)
  Real _cos3tt;

  /// sin(3*_tt) - useful for making comparisons with Lode angle
  Real _sin3tt;

  /// cos(6*_tt)
  Real _cos6tt;

  /// sin(6*_tt)
  Real _sin6tt;

  /// if secondInvariant < _lode_cutoff then set Lode angle to zero.  This is to guard against precision-loss
  Real _lode_cutoff;

  /// Accumulated plastic strain (the internal parameter), used in hardening relationship
  MaterialProperty<Real> & _mc_internal;

  /// Old value of accumulated plastic strain (the internal parameter), used in hardening relationship
  MaterialProperty<Real> & _mc_internal_old;

  /// maximum principal stress
  MaterialProperty<Real> & _mc_max_principal;

  /// minimum principal stress
  MaterialProperty<Real> & _mc_min_principal;

  /// Value of the yield function
  MaterialProperty<Real> & _yf;


  /**
   * The number of internal parameters (should return 0 for perfect plasticity)
   */
  virtual unsigned int numberOfInternalParameters();

  /**
   * The yield function(s)
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param f (output) the yield function (or functions in the case of multisurface plasticity)
   */
  virtual void yieldFunction(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<Real> & f);

  /**
   * The derivative of yield function(s) with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param df_dstress (output) the derivative (or derivatives in the case of multisurface plasticity).  df_dstress[alpha](i, j) = dyieldFunction[alpha]/dstress(i, j)
   */
  virtual void dyieldFunction_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & df_dstress);

  /**
   * The derivative of yield function(s) with respect to internal parameters
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param df_dintnl (output) the derivatives.  df_dstress[alpha][a] = dyieldFunction[alpha]/dintnl[a]
   */
  virtual void dyieldFunction_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<std::vector<Real> > & df_dintnl);

  /**
   * The flow potential(s) - one for each yield function
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param r (output) the flow potential (flow potentials in the multi-surface case)
   */
  virtual void flowPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & r);

  /**
   * The derivative of the flow potential(s) with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param dr_dstress (output) the derivative.  dr_dstress[alpha](i, j, k, l) = dr[alpha](i, j)/dstress(k, l)
   */
  virtual void dflowPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankFourTensor> & dr_dstress);

  /**
   * The derivative of the flow potentials with respect to internal parameters
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param dr_dintnl (output) the derivatives.  dr_dintnl[alpha][a](i, j) = dr[alpha](i, j)/dintnl[a]
   */
  virtual void dflowPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<std::vector<RankTwoTensor> > & dr_dintnl);

  /**
   * The hardening potentials (one for each internal parameter and for each yield function)
   * @param stress the stress at which to calculate the hardening potential
   * @param intnl vector of internal parameters
   * @param h (output) the hardening potentials.  h[a][alpha] = hardening potential for yield fcn alpha and internal param a
   */
  virtual void hardPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<std::vector<Real> > & h);

  /**
   * The derivative of the hardening potentials with respect to stress
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl vector of internal parameters
   * @param dh_dstress (output) the derivative.  dh_dstress[a][alpha](i, j) = dh[a][alpha]/dstress(k, l)
   */
  virtual void dhardPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<std::vector<RankTwoTensor> > & dh_dstress);

  /**
   * The derivative of the hardening potential with respect to internal parameters
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl vector of internal parameters
   * @param dh_dintnl (output) the derivatives.  dh_dintnl[a][alpha][b] = dh[a][alpha]/dintnl[b]
   */
  virtual void dhardPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<std::vector<std::vector<Real> > > & dh_dintnl);

  /// Function called just after doing returnMap
  virtual void postReturnMap();

  /// cohesion as a function of internal parameter
  virtual Real cohesion(const Real internal_param);

  /// d(cohesion)/d(internal_param);
  virtual Real dcohesion(const Real internal_param);

  /// friction angle as a function of internal parameter
  virtual Real phi(const Real internal_param);

  /// d(phi)/d(internal_param);
  virtual Real dphi(const Real internal_param);

  /// dilation angle as a function of internal parameter
  virtual Real psi(const Real internal_param);

  /// d(psi)/d(internal_param);
  virtual Real dpsi(const Real internal_param);


 private:

  /**
   * Computes Abbo et al's A, B and C parameters
   * @param sin3lode sin(3*(lode angle))
   * @param sin_angle sin(friction_angle) (for yield function), or sin(dilation_angle) (for potential function)
   * @param aaa (output) Abbo's A
   * @param bbb (output) Abbo's B
   * @param ccc (output) Abbo's C
   */
  void abbo(const Real sin3lode, const Real sin_angle, Real & aaa, Real & bbb, Real & ccc);

  /**
   * Computes derivatives of Abbo et al's A, B and C parameters wrt sin_angle
   * @param sin3lode sin(3*(lode angle))
   * @param sin_angle sin(friction_angle) (for yield function), or sin(dilation_angle) (for potential function)
   * @param daaa (output) d(Abbo's A)/d(sin_angle)
   * @param dbbb (output) d(Abbo's B)/d(sin_angle)
   * @param dccc (output) d(Abbo's C)/d(sin_angle)
   */
  void dabbo(const Real sin3lode, const Real sin_angle, Real & daaa, Real & dbbb, Real & dccc);

  /**
   * d(yieldFunction)/d(stress), but with the ability to put friction or dilation angle into the result
   * @param stress the stress at which to calculate
   * @param sin_angle either sin(friction angle) or sin(dilation angle)
   */
  RankTwoTensor df_dsig(const RankTwoTensor & stress, const Real sin_angle);

};

#endif //FINITESTRAINMOHRCOULOMB
