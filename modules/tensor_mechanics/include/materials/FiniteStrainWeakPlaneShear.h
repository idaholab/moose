
#ifndef FINITESTRAINWEAKPLANESHEAR
#define FINITESTRAINWEAKPLANESHEAR

#include "FiniteStrainPlasticBase.h"

class FiniteStrainWeakPlaneShear;

template<>
InputParameters validParams<FiniteStrainWeakPlaneShear>();

/**
 * FiniteStrainWeakPlaneShear implements rate-independent non-associative weak-plane shear failure
 * with hardening/softening in the finite-strain framework.
 */
class FiniteStrainWeakPlaneShear : public FiniteStrainPlasticBase
{
public:
  FiniteStrainWeakPlaneShear(const std::string & name, InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();

  /// The cohesion
  Real _cohesion;

  /// tan(friction angle)
  Real _tan_phi;

  /// tan(dilation angle)
  Real _tan_psi;

  /// The cohesion_residual
  Real _cohesion_residual;

  /// tan(friction angle_residual)
  Real _tan_phi_residual;

  /// tan(dilation angle_residual)
  Real _tan_psi_residual;

  /// Logarithmic rate of change of cohesion to _cohesion_residual
  Real _cohesion_rate;

  /// Logarithmic rate of change of tan_phi to tan_phi_residual
  Real _tan_phi_rate;

  /// Logarithmic rate of change of tan_psi to tan_psi_residual
  Real _tan_psi_rate;

  /// Unit normal inputted by user
  RealVectorValue _input_n;

  /// Whether the normal vector rotates with large deformations
  bool _normal_rotates;

  /**
   * The yield function needs to be smooth around shear-stress=0,
   * so it is modified to be
   * f = sqrt(s_xz^2 + s_yz^2 + (_small_smoother*_cohesion)^2) + s_zz*_tan_phi - _cohesion
   */
  Real _small_smoother;

  /// Unit normal vector to the weak plane
  MaterialProperty<RealVectorValue> & _n;

  /// Old value of unit normal vector to the weak plane
  MaterialProperty<RealVectorValue> & _n_old;

  /// Accumulated plastic strain (the internal parameter), used in hardening relationship
  MaterialProperty<Real> & _wps_internal;

  /// Old value of accumulated plastic strain (the internal parameter), used in hardening relationship
  MaterialProperty<Real> & _wps_internal_old;

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

  /// Function called just before doing returnMap
  virtual void preReturnMap();

  /// Function called just after doing returnMap
  virtual void postReturnMap();

  /// Function that's used in dyieldFunction_dstress and flowPotential
  RankTwoTensor df_dsig(const RankTwoTensor & stress, const Real & _tan_phi_or_psi);

  /// cohesion as a function of internal parameter
  Real cohesion(const Real internal_param);

  /// d(cohesion)/d(internal_param);
  Real dcohesion(const Real internal_param);

  /// tan_phi as a function of internal parameter
  Real tan_phi(const Real internal_param);

  /// d(tan_phi)/d(internal_param);
  Real dtan_phi(const Real internal_param);

  /// tan_psi as a function of internal parameter
  Real tan_psi(const Real internal_param);

  /// d(tan_psi)/d(internal_param);
  Real dtan_psi(const Real internal_param);

};

#endif //FINITESTRAINWEAKPLANESHEAR
