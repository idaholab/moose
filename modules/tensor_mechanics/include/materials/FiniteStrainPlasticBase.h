#ifndef FINITESTRAINPLASTICBASE
#define FINITESTRAINPLASTICBASE

#include "FiniteStrainMaterial.h"

class FiniteStrainPlasticBase;

template<>
InputParameters validParams<FiniteStrainPlasticBase>();

/**
 * FiniteStrainPlasticBase is base class for plasticity with
 * finite strain, associative or non-associative,
 * perfect plasticity or with hardening/softening.
 */
class FiniteStrainPlasticBase : public FiniteStrainMaterial
{
public:
  FiniteStrainPlasticBase(const std::string & name, InputParameters parameters);

protected:
  virtual void computeQpStress();
  virtual void initQpStatefulProperties();

  /// Maximum number of Newton-Raphson iterations allowed
  unsigned int _max_iter;

  /// Tolerance(s) on yield function(s)
  std::vector<Real> _f_tol;

  /// Tolerance(s) on the internal constraint(s)
  std::vector<Real> _ic_tol;

  /// Tolerance on the direction constraint
  Real _dirn_tol;

  /// Debug parameter - useful for coders, not for users (hopefully!)
  int _fspb_debug;

  /// plastic strain
  MaterialProperty<RankTwoTensor> & _plastic_strain;

  /// Old value of plastic strain
  MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  /// internal parameters
  MaterialProperty<std::vector<Real> > & _intnl;

  /// old values of internal parameters
  MaterialProperty<std::vector<Real> > & _intnl_old;

  /// yield functions
  MaterialProperty<std::vector<Real> > & _f;



  /**
   * The number of yield functions (should return 1 for single-surface plasticity)
   */
  virtual unsigned int numberOfYieldFunctions();

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

  /**
   * The constraints.  These are set to zero (or <=0 in the case of the yield functions)
   * by the Newton-Raphson process
   * @param stress The stress
   * @param intnl_old old values of the internal parameters
   * @param intnl internal parameters
   * @param ga Current value(s) of the plasticity multiplier(s) (consistency parameters)
   * @param delta_dp Change in plastic strain incurred so far during the return
   * @param f (output) Yield function(s)
   * @param dirn (output) Direction constraint
   * @param ic (output) Internal-parameter constraint
   */
  virtual void calculateConstraints(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & ga, const RankTwoTensor & delta_dp, std::vector<Real> & f, RankTwoTensor & dirn, std::vector<Real> & ic);

  /**
   * The residual-squared
   * @param f the yield function(s)
   * @param dirn the direction constraint
   * @param ic the internal constraint(s)
   */
  virtual Real residual2(const std::vector<Real> & f, const RankTwoTensor & dirn, const std::vector<Real> & ic);


  /**
   * Implements the return map
   * @param stress_old  The stress at the previous "time" step
   * @param plastic_strain_old  The value of plastic strain at the previous "time" step
   * @param intnl_old The internal variables at the previous "time" step
   * @param delta_d  The total strain increment for this "time" step
   * @param E_ijkl   The elasticity tensor.  If no plasiticity then sig_new = sig_old + E_ijkl*delta_d
   * @param sress    (output) The stress after returning to the yield surface
   * @param plastic_strain   (output) The value of plastic strain after returning to the yield surface
   * @param intnl    (output) The internal variables after returning to the yield surface
   * @param f  (output) The yield functions after returning to the yield surface
   * Note that this algorithm doesn't do any rotations.  In order to find the
   * final stress and plastic_strain must be rotated using _rotation_increment.
   * This is usually done in computeQpStress
   */
  virtual void returnMap(const RankTwoTensor & stress_old, const RankTwoTensor & plastic_strain_old, const std::vector<Real> & intnl_old, const RankTwoTensor & delta_d, const RankFourTensor & E_ijkl, RankTwoTensor & stress, RankTwoTensor & plastic_strain, std::vector<Real> & intnl, std::vector<Real> & f);

  /**
   * Performs one Newton-Raphson step.  The purpose here is to find the
   * changes, dstress, dpm and dintnl according to the Newton-Raphson procedure
   * @param stress Current value of stress
   * @param intnl_old The internal variables at the previous "time" step
   * @param intnl    Current value of the internal variables
   * @param ga  Current value of the plasticity multipliers (consistency parameters)
   * @param E_inv inverse of the elasticity tensor
   * @param delta_dp  Current value of the plastic-strain increment (ie plastic_strain - plastic_strain_old)
   * @param f The yield function
   * @param dstress (output) The change in stress for a full Newton step
   * @param dpm (output) The change in plasticity multiplier for a full Newton step
   * @param dintnl (output) The change in internal variable(s) for a full Newton step
   */
  virtual void nrStep(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & ga, const RankFourTensor & E_inv, const RankTwoTensor & delta_dp, const std::vector<Real> & f, RankTwoTensor & dstress, std::vector<Real> & dpm, std::vector<Real> & dintnl);

  /**
   * Performs a line search.  Algorithm is taken straight from
   * "Numerical Recipes".  Given the changes dstress, dpm and dintnl
   * provided by the nrStep routine, a line-search looks for an appropriate
   * under-relaxation that reduces the residual-squared (nr_res2).
   *
   * Most variables are input/output variables: they enter the function
   * with their values at the start of the Newton step, and they exit
   * the function with values attained after applying the under-relaxation
   *
   * @param nr_res2 (input/output) The residual-squared
   * @param intnl_old  The internal variables at the previous "time" step
   * @param intnl (input/output) The internal variables
   * @param ga (input/output) The plasticity multiplier(s) (consistency parameter(s))
   * @param E_inv inverse of the elasticity tensor
   * @param delta_dp (input/output) Change in plastic strain from start of "time" step to current configuration (plastic_strain - plastic_strain_old)
   * @param dstress Change in stress for a full Newton step
   * @param dpm Change in plasticity multiplier for a full Newton step
   * @param dintnl change in internal parameter(s) for a full Newton step
   * @param f (input/output) Yield function(s)
   * @param dirn (input/output) Direction constraint
   * @param ic (input/output) Internal constraint
   */
  virtual void lineSearch(Real & nr_res2, RankTwoTensor & stress, const std::vector<Real> & intnl_old, std::vector<Real> & intnl, std::vector<Real> & ga, const RankFourTensor & E_inv, RankTwoTensor & delta_dp, const RankTwoTensor & dstress, const std::vector<Real> & dpm, const std::vector<Real> & dintnl, std::vector<Real> & f, RankTwoTensor & dirn, std::vector<Real> & ic);



};

#endif //FINITESTRAINPLASTICBASE
