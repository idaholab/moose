/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FINITESTRAINHYPERELASTICVISCOPLASTIC_H
#define FINITESTRAINHYPERELASTICVISCOPLASTIC_H

#include "ComputeStressBase.h"
#include "HyperElasticStress.h"
#include "FlowRateModel.h"

class FiniteStrainHyperElasticViscoPlastic;

template<>
InputParameters validParams<FiniteStrainHyperElasticViscoPlastic>();

/**
 * This class is for Hyperelastic Visco-Plastic material
 * Multiple flow models are solved simultaneously
 * The flow models are passed as user objects
 * Residual equations are set for flow rate equations
 * Solved using NR with substepping
 * Stress is calculated using user objects (Present PK2 stress)
 */

class FiniteStrainHyperElasticViscoPlastic : public ComputeStressBase
{
 public:
  FiniteStrainHyperElasticViscoPlastic (const InputParameters & parameters);
  FiniteStrainHyperElasticViscoPlastic (const std::string & name, InputParameters parameters); // DEPRECATED

 protected:

  ///This function computes the Cauchy stress
  virtual void computeQpStress();

  ///This function computes the Jacobian
  void computeQpJacobian();

  ///Initializes state
  virtual void initQpStatefulProperties();

  ///Sets state for solve (Outside substepping)
  virtual void preSolveQp();

  ///Solve state
  virtual bool solveQp();

  ///Update state for output (Outside substepping)
  virtual void postSolveQp();

  ///Sets state for solve (Inside substepping)
  virtual void preSolveStress();

  ///Solve for flow rate and state
  virtual bool solveStress();

  ///Update state for output (Inside substepping)
  virtual void postSolveStress();

  ///Calls user objects to compute flow rates
  virtual bool computeFlowRate();

  ///Calls user objects to compute flow directions
  virtual bool computeFlowDirection();

  ///Calls user objects to update internal variables
  virtual bool updateInternalVar();

  ///Computes elastic Right Cauchy Green Tensor
  virtual void computeElasticRightCauchyGreenTensor();

  ///Calls user object to compute PK2 stress
  virtual void computePK2Stress();

  ///Computes flow rate residual vector
  virtual bool computeFlowRateResidual();

  ///Computes flow rate Jacobian matrix
  virtual void computeFlowRateJacobian();

  ///Computes elastic and plastic deformation gradients
  virtual void computeElasticPlasticDeformGrad();

  ///Computes norm of residual vector
  virtual Real computeNorm(Real*);

  ///Update flow rate
  void updateFlowRate();

  ///Computes derivative of PK2 stress wrt inverse of plastic deformation gradient
  void computeDpk2Dfpinv();

  ///Absolute tolerance for residual convergence check
  Real _resid_abs_tol;
  ///Relative tolerance for residual convergence check
  Real _resid_rel_tol;
  ///Maximum number of iterations
  unsigned int _maxiters;
  ///Maximum number of substep iterations
  unsigned int _max_substep_iter;
  ///Stress user object
  const HyperElasticStress & _stress_uo;
  ///Flow model user objects
  std::vector<const FlowRateModel *> _flow_rate_uo;
  ///Stores the end indices of internal variable vector associated with each flow model
  std::vector<unsigned int> _q_end_index;
  ///Number of flow rate user objects
  unsigned int _num_flow_rate_uos;
  ///Total number of internal variables
  unsigned int _num_internal_var;

  MaterialProperty<RankTwoTensor> & _fp;
  MaterialProperty<RankTwoTensor> & _fp_old;
  MaterialProperty< std::vector<Real> > & _q;
  MaterialProperty< std::vector<Real> > & _q_old;
  const MaterialProperty<RankTwoTensor> & _deformation_gradient;
  const MaterialProperty<RankTwoTensor> & _deformation_gradient_old;

  RankFourTensor _dpk2_dce, _dpk2_dfpinv;
  RankTwoTensor _pk2_tmp, _fe, _ce, _ee, _pk2_dev;
  RankTwoTensor _dfgrd_tmp, _fp_tmp_inv, _fp_tmp_old_inv;
  std::vector<Real> _q_tmp, _q_tmp_old;
  std::vector<Real> _flow_rate, _flow_rate_func;
  std::vector<Real> _dflowrate_dq, _dq_dflowrate;
  std::vector<RankTwoTensor> _dflowrate_dpk2, _flow_dirn, _dpk2_dflowrate;
  std::vector<Real> _resid, _jac;
  Real _dt_substep;
};

#endif //FINITESTRAINHYPERELASTICVISCOPLASTIC_H
