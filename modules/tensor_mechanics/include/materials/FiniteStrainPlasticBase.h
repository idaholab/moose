/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FINITESTRAINPLASTICBASE
#define FINITESTRAINPLASTICBASE

#include "FiniteStrainMaterial.h"
#include "Function.h"

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
  FiniteStrainPlasticBase(const InputParameters & parameters);

protected:
  virtual void computeQpStress();
  virtual void computeQpElasticityTensor();
  virtual void initQpStatefulProperties();

  /// Maximum number of Newton-Raphson iterations allowed
  unsigned int _max_iter;

  /// Maximum number of subdivisions allowed
  unsigned int _max_subdivisions;

  /// Tolerance(s) on yield function(s)
  std::vector<Real> _f_tol;

  /// Tolerance(s) on the internal constraint(s)
  std::vector<Real> _ic_tol;

  /// Tolerance on the plastic strain increment ("direction") constraint
  Real _epp_tol;

  /// Debug parameter - useful for coders, not for users (hopefully!)
  int _fspb_debug;

  /// Debug the Jacobian entries at this stress
  RankTwoTensor _fspb_debug_stress;

  /// Debug the Jacobian entires at these plastic multipliers
  std::vector<Real> _fspb_debug_pm;

  /// Debug the Jacobian entires at these internal parameters
  std::vector<Real> _fspb_debug_intnl;

  /// Debug finite-differencing parameter for the stress
  Real _fspb_debug_stress_change;

  /// Debug finite-differencing parameters for the plastic multipliers
  std::vector<Real> _fspb_debug_pm_change;

  /// Debug finite-differencing parameters for the internal parameters
  std::vector<Real> _fspb_debug_intnl_change;

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

  /// Number of Newton-Raphson iterations used in the return-map
  MaterialProperty<unsigned int> & _iter;



  // *****************************************************************
  // *                                                               *
  // *  FOLLOWING ARE DESIGNED TO BE OVERRIDDEN BY DERIVED CLASSES   *
  // *                                                               *
  // *****************************************************************

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
   * @param[out] f  the yield function (or functions in the case of multisurface plasticity)
   */
  virtual void yieldFunction(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<Real> & f);

  /**
   * The derivative of yield function(s) with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param[out] df_dstress  the derivative (or derivatives in the case of multisurface plasticity).  df_dstress[alpha](i, j) = dyieldFunction[alpha]/dstress(i, j)
   */
  virtual void dyieldFunction_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & df_dstress);

  /**
   * The derivative of yield function(s) with respect to internal parameters
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param[out] df_dintnl  the derivatives.  df_dstress[alpha][a] = dyieldFunction[alpha]/dintnl[a]
   */
  virtual void dyieldFunction_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<std::vector<Real> > & df_dintnl);

  /**
   * The flow potential(s) - one for each yield function
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param[out] r  the flow potential (flow potentials in the multi-surface case)
   */
  virtual void flowPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & r);

  /**
   * The derivative of the flow potential(s) with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param[out] dr_dstress  the derivative.  dr_dstress[alpha](i, j, k, l) = dr[alpha](i, j)/dstress(k, l)
   */
  virtual void dflowPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankFourTensor> & dr_dstress);

  /**
   * The derivative of the flow potentials with respect to internal parameters
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param[out] dr_dintnl  the derivatives.  dr_dintnl[alpha][a](i, j) = dr[alpha](i, j)/dintnl[a]
   */
  virtual void dflowPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<std::vector<RankTwoTensor> > & dr_dintnl);

  /**
   * The hardening potentials (one for each internal parameter and for each yield function)
   * @param stress the stress at which to calculate the hardening potential
   * @param intnl vector of internal parameters
   * @param[out] h  the hardening potentials.  h[a][alpha] = hardening potential for yield fcn alpha and internal param a
   */
  virtual void hardPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<std::vector<Real> > & h);

  /**
   * The derivative of the hardening potentials with respect to stress
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl vector of internal parameters
   * @param[out] dh_dstress  the derivative.  dh_dstress[a][alpha](i, j) = dh[a][alpha]/dstress(k, l)
   */
  virtual void dhardPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<std::vector<RankTwoTensor> > & dh_dstress);

  /**
   * The derivative of the hardening potential with respect to internal parameters
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl vector of internal parameters
   * @param[out] dh_dintnl  the derivatives.  dh_dintnl[a][alpha][b] = dh[a][alpha]/dintnl[b]
   */
  virtual void dhardPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<std::vector<std::vector<Real> > > & dh_dintnl);

  /// Function called just before doing returnMap
  virtual void preReturnMap();

  /// Function called just after doing returnMap
  virtual void postReturnMap();


  // **********************************************************************
  // *                                                                    *
  // *  USUALLY IT IS NOT NECESSARY TO OVERRIDE ANYTHING ELSE BELOW HERE  *
  // *                                                                    *
  // **********************************************************************

  /**
   * The constraints.  These are set to zero (or <=0 in the case of the yield functions)
   * by the Newton-Raphson process
   * @param stress The stress
   * @param intnl_old old values of the internal parameters
   * @param intnl internal parameters
   * @param pm Current value(s) of the plasticity multiplier(s) (consistency parameters)
   * @param delta_dp Change in plastic strain incurred so far during the return
   * @param[out] f  Yield function(s)
   * @param[out] epp  Plastic-strain increment constraint
   * @param[out] ic  Internal-parameter constraint
   */
  virtual void calculateConstraints(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankTwoTensor & delta_dp, std::vector<Real> & f, RankTwoTensor & epp, std::vector<Real> & ic);

  /**
   * Given the constraints, calculate the RHS which is
   * rhs = -(epp(0,0), epp(1,0), epp(1,1), epp(2,0), epp(2,1), epp(2,2), f[0], f[1], ..., f[num_f], ic[0], ic[1], ..., ic[num_ic])
   *
   * @param stress The stress
   * @param intnl_old old values of the internal parameters
   * @param intnl internal parameters
   * @param pm Current value(s) of the plasticity multiplier(s) (consistency parameters)
   * @param delta_dp Change in plastic strain incurred so far during the return
   * @param[out] rhs  Right-Hand-Side
   */
  virtual void calculateRHS(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankTwoTensor & delta_dp, std::vector<Real> & rhs);

  /**
   * The residual-squared
   * @param f the yield function(s)
   * @param epp the plastic strain increment constraint
   * @param ic the internal constraint(s)
   */
  virtual Real residual2(const std::vector<Real> & f, const RankTwoTensor & epp, const std::vector<Real> & ic);

  virtual void calculateJacobian(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankFourTensor & E_inv, std::vector<std::vector<Real> > & jac);

  /**
   * Implements the return map
   * @param stress_old  The stress at the previous "time" step
   * @param plastic_strain_old  The value of plastic strain at the previous "time" step
   * @param intnl_old The internal variables at the previous "time" step
   * @param delta_d  The total strain increment for this "time" step
   * @param E_ijkl   The elasticity tensor.  If no plasiticity then sig_new = sig_old + E_ijkl*delta_d
   * @param[out] stress     The stress after returning to the yield surface
   * @param[out] plastic_strain    The value of plastic strain after returning to the yield surface
   * @param[out] intnl     The internal variables after returning to the yield surface
   * @param[out] f   The yield functions after returning to the yield surface
   * @param[out] iter  The number of Newton-Raphson iterations used
   * @return True if the stress was successfully returned to the yield surface
   * Note that this algorithm doesn't do any rotations.  In order to find the
   * final stress and plastic_strain must be rotated using _rotation_increment.
   * This is usually done in computeQpStress
   */
  virtual bool returnMap(const RankTwoTensor & stress_old, const RankTwoTensor & plastic_strain_old, const std::vector<Real> & intnl_old, const RankTwoTensor & delta_d, const RankFourTensor & E_ijkl, RankTwoTensor & stress, RankTwoTensor & plastic_strain, std::vector<Real> & intnl, std::vector<Real> & f, unsigned int & iter);

  /**
   * Performs one Newton-Raphson step.  The purpose here is to find the
   * changes, dstress, dpm and dintnl according to the Newton-Raphson procedure
   * @param stress Current value of stress
   * @param intnl_old The internal variables at the previous "time" step
   * @param intnl    Current value of the internal variables
   * @param pm  Current value of the plasticity multipliers (consistency parameters)
   * @param E_inv inverse of the elasticity tensor
   * @param delta_dp  Current value of the plastic-strain increment (ie plastic_strain - plastic_strain_old)
   * @param[out] dstress  The change in stress for a full Newton step
   * @param[out] dpm  The change in plasticity multiplier for a full Newton step
   * @param[out] dintnl  The change in internal variable(s) for a full Newton step
   */
  virtual void nrStep(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankFourTensor & E_inv, const RankTwoTensor & delta_dp, RankTwoTensor & dstress, std::vector<Real> & dpm, std::vector<Real> & dintnl);

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
   * @param[in,out] nr_res2 The residual-squared
   * @param[in,out] stress Current value of stress
   * @param intnl_old  The internal variables at the previous "time" step
   * @param[in,out] intnl The internal variables
   * @param[in,out] pm The plasticity multiplier(s) (consistency parameter(s))
   * @param E_inv inverse of the elasticity tensor
   * @param[in,out] delta_dp Change in plastic strain from start of "time" step to current configuration (plastic_strain - plastic_strain_old)
   * @param dstress Change in stress for a full Newton step
   * @param dpm Change in plasticity multiplier for a full Newton step
   * @param dintnl change in internal parameter(s) for a full Newton step
   * @param[in,out] f Yield function(s)
   * @param[in,out] epp Plastic strain increment constraint
   * @param[in,out] ic Internal constraint
   * @return true if successfully found a step that reduces the residual-squared
   */
  virtual bool lineSearch(Real & nr_res2, RankTwoTensor & stress, const std::vector<Real> & intnl_old, std::vector<Real> & intnl, std::vector<Real> & pm, const RankFourTensor & E_inv, RankTwoTensor & delta_dp, const RankTwoTensor & dstress, const std::vector<Real> & dpm, const std::vector<Real> & dintnl, std::vector<Real> & f, RankTwoTensor & epp, std::vector<Real> & ic);

  ElasticityTensorR4 _Cijkl;
  Function * const _prefactor_function;

 private:

  // Following are used for checking derivatives

  /**
   * Checks the derivatives, eg dyieldFunction_dstress by using
   * finite difference approximations.
   */
  void checkDerivatives();

  /**
   * The finite-difference derivative of yield function(s) with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param[out] df_dstress  the derivative (or derivatives in the case of multisurface plasticity).  df_dstress[alpha](i, j) = dyieldFunction[alpha]/dstress(i, j)
   */
  void fddyieldFunction_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & df_dstress);

  /**
   * The finite-difference derivative of the flow potential(s) with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param[out] dr_dstress  the derivative.  dr_dstress[alpha](i, j, k, l) = dr[alpha](i, j)/dstress(k, l)
   */
  virtual void fddflowPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankFourTensor> & dr_dstress);

  /**
   * The finite-difference derivative of the flow potentials with respect to internal parameters
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param[out] dr_dintnl  the derivatives.  dr_dintnl[alpha][a](i, j) = dr[alpha](i, j)/dintnl[a]
   */
  virtual void fddflowPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<std::vector<RankTwoTensor> > & dr_dintnl);

  /**
   * Checks the full Jacobian, which is just certain
   * linear combinations of the dyieldFunction_dstress, etc,
   * by using finite difference approximations
   */
  void checkJacobian();

  /**
   * The Jacobian calculated using finite differences.  The output
   * should be equal to calculateJacobian(...) if everything is
   * coded correctly.
   * @param stress the stress at which to calculate the Jacobian
   * @param intnl_old the old values of internal variables (jacobian is inependent of these, but they are needed to do the finite-differencing cleanly)
   * @param intnl the vector of internal parameters at which to calculate the Jacobian
   * @param pm the plasticity multipliers at which to calculate the Jacobian
   * @param delta_dp plastic_strain - plastic_strain_old (Jacobian is independent of this, but it is needed to do the finite-differencing cleanly)
   * @param E_inv inverse of the elasticity tensor
   * @param[out] jac  the finite-difference Jacobian
   */
  virtual void fdJacobian(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankTwoTensor & delta_dp, const RankFourTensor & E_inv, std::vector<std::vector<Real> > & jac);

  /**
   * Outputs the debug parameters: _fspb_debug_stress, _fspd_debug_pm, etc
   * and checks that they are sized correctly
   */
  void outputAndCheckDebugParameters();

};

#endif //FINITESTRAINPLASTICBASE
