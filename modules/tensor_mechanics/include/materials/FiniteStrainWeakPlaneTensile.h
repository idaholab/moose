
#ifndef FINITESTRAINWEAKPLANETENSILE
#define FINITESTRAINWEAKPLANETENSILE

#include "FiniteStrainPlasticBase.h"

class FiniteStrainWeakPlaneTensile;

template<>
InputParameters validParams<FiniteStrainWeakPlaneTensile>();

/**
 * FiniteStrainWeakPlaneTensile implements rate-independent associative weak-plane tensile failure
 * with hardening/softening in the finite-strain framework.
 */
class FiniteStrainWeakPlaneTensile : public FiniteStrainPlasticBase
{
public:
  FiniteStrainWeakPlaneTensile(const std::string & name, InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();

  /// tension cutoff
  Real _tension_cutoff;

  /// tension cutoff at infinite hardening/softening
  Real _tension_cutoff_residual;

  /// Tensile strength = wpt_tensile_strenght_residual + (wpt_tensile_strength - wpt_tensile_strength_residual)*exp(-wpt_tensile_rate*plasticstrain).
  Real _tension_cutoff_rate;

  /// Unit normal inputted by user
  RealVectorValue _input_n;

  /// Whether the normal vector rotates with large deformations
  bool _normal_rotates;

  /// Unit normal vector to the weak plane
  MaterialProperty<RealVectorValue> & _n;

  /// Old value of unit normal vector to the weak plane
  MaterialProperty<RealVectorValue> & _n_old;

  /// Accumulated plastic strain (the internal parameter), used in hardening relationship
  MaterialProperty<Real> & _wpt_internal;

  /// Old value of accumulated plastic strain (the internal parameter), used in hardening relationship
  MaterialProperty<Real> & _wpt_internal_old;

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

  /// tensile strength as a function of residual value, rate, and internal_param
  virtual Real tensile_strength(const Real internal_param);

  /// d(tensile strength)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dtensile_strength(const Real internal_param);
};

#endif //FINITESTRAINWEAKPLANETENSILE
