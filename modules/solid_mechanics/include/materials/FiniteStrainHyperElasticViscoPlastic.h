//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeStressBase.h"
#include "HEVPFlowRateUOBase.h"
#include "HEVPStrengthUOBase.h"
#include "HEVPInternalVarUOBase.h"
#include "HEVPInternalVarRateUOBase.h"

/**
 * This class solves the viscoplastic flow rate equations in the total form
 * Involves 4 different types of user objects that calculates:
 * Internal variable rates - functions of internal variables and flow rates
 * Internal variables - functions of internal variables
 * Strengths - functions of internal variables
 * Flow rates - functions of strengths and PK2 stress
 * Flow directions - functions of strengths and PK2 stress
 * The associated derivatives from user objects are assembled and the system is solved using NR
 */
class FiniteStrainHyperElasticViscoPlastic : public ComputeStressBase
{
public:
  static InputParameters validParams();

  FiniteStrainHyperElasticViscoPlastic(const InputParameters & parameters);

protected:
  /**
   *  This function initializes the properties, stateful properties and user objects
   *  The properties and stateful properties associated with user objects are only initialized here
   *  The properties have the same name as the user object name
   **/
  virtual void initUOVariables();

  /// This function calculates the number of each user object type
  void initNumUserObjects(const std::vector<UserObjectName> &, unsigned int &);

  /// This function initializes properties for each user object
  template <typename T>
  void
  initProp(const std::vector<UserObjectName> &, unsigned int, std::vector<MaterialProperty<T> *> &);

  /**
   * This function initializes old for stateful properties associated with user object
   * Only user objects that update internal variables have an associated old property
   **/
  template <typename T>
  void initPropOld(const std::vector<UserObjectName> &,
                   unsigned int,
                   std::vector<const MaterialProperty<T> *> &);

  /// This function initializes user objects
  template <typename T>
  void initUserObjects(const std::vector<UserObjectName> &, unsigned int, std::vector<const T *> &);

  /// This function initialize variables required for Jacobian calculation
  virtual void initJacobianVariables();

  /// Initializes state
  virtual void initQpStatefulProperties();

  /// This function computes the Cauchy stress
  virtual void computeQpStress();

  /// This function computes the Jacobian
  virtual void computeQpJacobian();

  /// This function saves the old stateful properties that is modified during sub stepping
  virtual void saveOldState();

  /// Sets state for solve
  virtual void preSolveQp();

  /// Solve state
  virtual bool solveQp();

  /// Update state for output (Outside substepping)
  virtual void postSolveQp();

  /// This function restores the the old stateful properties after a successful solve
  virtual void recoverOldState();

  /// Sets state for solve (Inside substepping)
  virtual void preSolveFlowrate();

  /// Solve for flow rate and state
  virtual bool solveFlowrate();

  /// Update state for output (Inside substepping)
  virtual void postSolveFlowrate();

  /// Calls user objects to compute flow rates
  virtual bool computeFlowRateFunction();

  /// Calls user objects to compute flow directions
  virtual bool computeFlowDirection();

  /// Computes elastic Right Cauchy Green Tensor
  virtual void computeElasticRightCauchyGreenTensor();

  /// Computes PK2 stress and derivative w.r.t elastic Right Cauchy Green Tensor
  virtual void computePK2StressAndDerivative();

  /// Computes elastic Lagrangian strain
  virtual void computeElasticStrain();

  /// Computes derivative of elastic strain w.r.t elastic Right Cauchy Green Tensor
  virtual void computeDeeDce();

  /// Computes flow rate residual vector
  virtual bool computeFlowRateResidual();

  /// Computes flow rate Jacobian matrix
  virtual void computeFlowRateJacobian();

  /// Computes elastic and plastic deformation gradients
  virtual void computeElasticPlasticDeformGrad();

  /// Computes norm of residual vector
  virtual Real computeNorm(const std::vector<Real> &);

  /// Update flow rate
  virtual void updateFlowRate();

  /// Computes derivative of PK2 stress wrt inverse of plastic deformation gradient
  virtual void computeDpk2Dfpinv();

  /// This function call user objects to calculate rate of internal variables
  virtual bool computeIntVarRates();

  /// This function call user objects to integrate internal variables
  virtual bool computeIntVar();

  /// This function call user objects to compute strength
  virtual bool computeStrength();

  /// This function call user objects to compute dintvar_rate/dintvar and dintvarrate/dflowrate
  virtual void computeIntVarRateDerivatives();

  /// This function call user objects to compute dintvar/dintvar_rate and dintvar/dflowrate
  virtual void computeIntVarDerivatives();

  /// This function call user objects to compute dstrength/dintvar
  void computeStrengthDerivatives();

  /// Absolute tolerance for residual convergence check
  Real _resid_abs_tol;
  /// Relative tolerance for residual convergence check
  Real _resid_rel_tol;
  /// Maximum number of iterations
  unsigned int _maxiters;
  /// Maximum number of substep iterations
  unsigned int _max_substep_iter;

  /// Names of flow rate user objects
  std::vector<UserObjectName> _flow_rate_uo_names;
  /// Names of strength user objects
  std::vector<UserObjectName> _strength_uo_names;
  /// Names of internal variable user objects
  std::vector<UserObjectName> _int_var_uo_names;
  /// Names of internal variable rate user objects
  std::vector<UserObjectName> _int_var_rate_uo_names;

  /// Number of flow rate user objects
  unsigned int _num_flow_rate_uos;
  /// Number of strength user objects
  unsigned int _num_strength_uos;
  /// Number of internal variable user objects
  unsigned int _num_int_var_uos;
  /// Number of internal variable rate user objects
  unsigned int _num_int_var_rate_uos;

  /// Flow rate user objects
  std::vector<const HEVPFlowRateUOBase *> _flow_rate_uo;
  /// Strength user objects
  std::vector<const HEVPStrengthUOBase *> _strength_uo;
  /// Internal variable user objects
  std::vector<const HEVPInternalVarUOBase *> _int_var_uo;
  /// Internal variable rate user objects
  std::vector<const HEVPInternalVarRateUOBase *> _int_var_rate_uo;

  std::string _pk2_prop_name;
  MaterialProperty<RankTwoTensor> & _pk2;
  MaterialProperty<RankTwoTensor> & _fp;
  const MaterialProperty<RankTwoTensor> & _fp_old;
  MaterialProperty<RankTwoTensor> & _ce;

  /// Name of the elasticity tensor material property
  const std::string _elasticity_tensor_name;
  /// Elasticity tensor material property
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  const MaterialProperty<RankTwoTensor> & _deformation_gradient;
  const MaterialProperty<RankTwoTensor> & _deformation_gradient_old;
  const MaterialProperty<RankTwoTensor> & _rotation_increment;

  std::vector<MaterialProperty<Real> *> _flow_rate_prop;
  std::vector<MaterialProperty<Real> *> _strength_prop;
  std::vector<MaterialProperty<Real> *> _int_var_stateful_prop;
  std::vector<const MaterialProperty<Real> *> _int_var_stateful_prop_old;
  std::vector<MaterialProperty<Real> *> _int_var_rate_prop;
  std::vector<Real> _int_var_old;

  RankTwoTensor _dfgrd_tmp;
  RankTwoTensor _fp_tmp_inv, _fp_tmp_old_inv;
  RankTwoTensor _fe, _ee;
  RankTwoTensor _pk2_fet, _fe_pk2;
  RankFourTensor _dpk2_dce, _dpk2_dfe, _dfe_dfpinv, _dpk2_dfpinv;
  RankFourTensor _dee_dce, _dce_dfe, _dfe_df;
  RankFourTensor _tan_mod, _df_dstretch_inc;

  std::vector<RankTwoTensor> _flow_dirn;
  std::vector<RankTwoTensor> _dflowrate_dpk2;
  std::vector<RankTwoTensor> _dpk2_dflowrate;
  std::vector<RankTwoTensor> _dfpinv_dflowrate;

  DenseVector<Real> _dflow_rate;
  DenseVector<Real> _flow_rate;
  DenseVector<Real> _resid;

  /// Jacobian variables
  std::vector<DenseVector<Real>> _dintvarrate_dflowrate;
  std::vector<DenseVector<Real>> _dintvar_dflowrate_tmp;

  DenseMatrix<Real> _dintvarrate_dintvar;
  DenseMatrix<Real> _dintvar_dintvarrate;
  DenseMatrix<Real> _dintvar_dintvar;
  DenseMatrix<Real> _dintvar_dflowrate;
  DenseMatrix<Real> _dstrength_dintvar;
  DenseMatrix<Real> _dflowrate_dstrength;
  DenseVector<Real> _dintvar_dintvar_x;
  DenseMatrix<Real> _jac;

  Real _dt_substep;
};
