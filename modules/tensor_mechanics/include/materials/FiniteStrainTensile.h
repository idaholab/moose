
#ifndef FINITESTRAINTENSILE
#define FINITESTRAINTENSILE

#include "FiniteStrainPlasticBase.h"

class FiniteStrainTensile;

template<>
InputParameters validParams<FiniteStrainTensile>();

/**
 * FiniteStrainTensile implements rate-independent associative tensile failure
 * with hardening/softening in the finite-strain framework.
 * The smoothing of the tip of the yield-surface cone is described in
 * Zienkiewicz and Prande "Some useful forms of isotropic yield surfaces for soil and rock mechanics" (1977) In G Gudehus (editor) "Finite Elements in Geomechanics" Wile, Chichester, pp 179-190.
 * The smoothing of the edges of the cone is described in
 * AJ Abbo, AV Lyamin, SW Sloan, JP Hambleton "A C2 continuous approximation to the Mohr-Coulomb yield surface" International Journal of Solids and Structures 48 (2011) 3001-3010
 */
class FiniteStrainTensile : public FiniteStrainPlasticBase
{
public:
  FiniteStrainTensile(const std::string & name, InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();

  /// tensile strength at zero hardening/softening
  Real _tensile_strength0;

  /// tensile strength at infinite hardening/softening
  Real _tensile_strength_residual;

  /// Tensile strength = tensile_strength_residual + (tensile_strength - tensile_strength_residual)*exp(-tensile_strength_rate*plasticstrain).
  Real _tensile_strength_rate;

  /// Square of tip smoothing parameter to smooth the cone at mean_stress = T
  Real _small_smoother2;

  /// edge smoothing parameter, in radians
  Real _tt;

  /// sin(3*_tt) - useful for making comparisons with Lode angle
  Real _sin3tt;

  /// Accumulated plastic strain (the internal parameter), used in hardening relationship
  MaterialProperty<Real> & _tensile_internal;

  /// Old value of accumulated plastic strain (the internal parameter), used in hardening relationship
  MaterialProperty<Real> & _tensile_internal_old;

  /// maximum tensile stress
  MaterialProperty<Real> & _tensile_max_principal;

  /// Value of the yield function
  MaterialProperty<Real> & _yf;

  /// Abbo et al's C parameter
  Real _ccc;

  /// Abbo et al's B parameter
  Real _bbb;

  /// Abbo et al's A parameter
  Real _aaa;


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

  /// tensile strength as a function of residual value, rate, and internal_param
  virtual Real tensile_strength(const Real internal_param);

  /// d(tensile strength)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dtensile_strength(const Real internal_param);
};

#endif //FINITESTRAINTENSILE
