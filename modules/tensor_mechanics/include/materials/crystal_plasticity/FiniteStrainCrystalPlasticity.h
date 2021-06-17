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

/**
 * FiniteStrainCrystalPlasticity uses the multiplicative decomposition of deformation gradient
 * and solves the PK2 stress residual equation at the intermediate configuration to evolve the
 * material state.
 * The internal variables are updated using an interative predictor-corrector algorithm.
 * Backward Euler integration rule is used for the rate equations.
 */
class FiniteStrainCrystalPlasticity : public ComputeStressBase
{
public:
  static InputParameters validParams();

  FiniteStrainCrystalPlasticity(const InputParameters & parameters);

protected:
  /**
   * This function updates the stress at a quadrature point.
   */
  virtual void computeQpStress();

  /**
   * This function updates the elasticity tensor at a quadrature point.
   * Presently void.
   */
  virtual void computeQpElasticityTensor();

  /**
   * This function initializes the stateful properties such as
   * stress, plastic deformation gradient, slip system resistances, etc.
   */
  virtual void initQpStatefulProperties();

  /**
   * This function calls the residual and jacobian functions used in the
   * stress update algorithm.
   */
  virtual void calc_resid_jacob(RankTwoTensor &, RankFourTensor &);

  /**
   * This function updates the slip increments.
   * And derivative of slip w.r.t. resolved shear stress.
   */
  virtual void getSlipIncrements();

  // Override to modify slip system resistance evolution
  /**
   * This function updates the slip system resistances.
   */
  virtual void update_slip_system_resistance();

  // Old function: Kept to avoid code break in computeQpStress
  /**
   * This function updates the slip system resistances.
   */
  virtual void updateGss();

  /**
   * This function reads slip system from file - see test.
   */
  virtual void getSlipSystems();

  /**
   * This function assign initial values of slip system resistances/internal variables
   * read from getSlipSystems().
   */
  virtual void assignSlipSysRes();

  /**
   * This function read slip system resistances from file - see test.
   */
  virtual void readFileInitSlipSysRes();

  /**
   * This function assign slip system resistances - see test.
   * .i input file format start_slip_sys_num, end_slip_sys_num, value.
   */
  virtual void getInitSlipSysRes();

  /**
   * This function read flow rate parameters from file - see test.
   */
  virtual void readFileFlowRateParams();

  /**
   * This function assign flow rate parameters - see test.
   * .i input file format start_slip_sys_num, end_slip_sys_num, value1, value2
   */
  virtual void getFlowRateParams();

  /**
   * This function read hardness parameters from file.
   */
  virtual void readFileHardnessParams();

  /**
   * This function assign flow rate parameters from .i file - see test.
   */
  virtual void getHardnessParams();

  /**
   * This function initializes slip system resistances.
   */
  virtual void initSlipSysProps();

  /**
   * This function initializes additional parameters.
   */
  virtual void initAdditionalProps();

  /**
   * This function set variables for stress and internal variable solve.
   */
  virtual void preSolveQp();

  /**
   * This function solves stress and internal variables.
   */
  virtual void solveQp();

  /**
   * This function update stress and internal variable after solve.
   */
  virtual void postSolveQp();

  /**
   * This function set variables for internal variable solve.
   */
  virtual void preSolveStatevar();

  /**
   * This function solves internal variables.
   */
  virtual void solveStatevar();

  /**
   * This function update internal variable after solve.
   */
  virtual void postSolveStatevar();

  /**
   * This function set variables for stress solve.
   */
  virtual void preSolveStress();

  /**
   * This function solves for stress, updates plastic deformation gradient.
   */
  virtual void solveStress();

  /**
   * This function update stress and plastic deformation gradient after solve.
   */
  virtual void postSolveStress();

  /**
   * This function calculate stress residual.
   */
  virtual void calcResidual(RankTwoTensor &);

  /**
   * This function calculate jacobian.
   */
  virtual void calcJacobian(RankFourTensor &);

  /**
   * This function calculate the tangent moduli for preconditioner.
   * Default is the elastic stiffness matrix.
   * Exact jacobian is currently implemented.
   * tan_mod_type can be modified to exact in .i file to turn it on.
   */
  virtual RankFourTensor calcTangentModuli();

  /**
   * This function calculate the elastic tangent moduli for preconditioner.
   */
  virtual RankFourTensor elasticTangentModuli();

  /**
   * This function calculate the exact tangent moduli for preconditioner.
   */
  virtual RankFourTensor elastoPlasticTangentModuli();

  /**
   * This function perform RU decomposition to obtain the rotation tensor.
   */
  RankTwoTensor get_current_rotation(const RankTwoTensor & a);

  ////Old function: Kept to avoid code break in computeQpStress
  /**
   * This function perform RU decomposition to obtain the rotation tensor.
   */
  RankTwoTensor getMatRot(const RankTwoTensor & a);

  /**
   * This function calculate the Schmid tensor.
   */
  void calc_schmid_tensor();

  /**
   * This function performs the line search update
   */
  bool line_search_update(const Real rnorm_prev, const RankTwoTensor);

  /**
   * This function updates internal variables after each NewTon Raphson iteration (_fp_inv)
   */
  void internalVariableUpdateNRiteration();

  /// Number of slip system resistance
  const unsigned int _nss;

  std::vector<Real> _gprops;
  std::vector<Real> _hprops;
  std::vector<Real> _flowprops;

  ///File should contain slip plane normal and direction. See test.
  std::string _slip_sys_file_name;

  ///File should contain initial values of the slip system resistances.
  std::string _slip_sys_res_prop_file_name;

  ///File should contain values of the flow rate equation parameters.
  ///Values for every slip system must be provided.
  ///Should have the same order of slip systens as in slip_sys_file. See test.
  ///The option of reading all the properties from .i is still present.
  std::string _slip_sys_flow_prop_file_name;

  ///The hardening parameters in this class are read from .i file. The user can override to read from file.
  std::string _slip_sys_hard_prop_file_name;

  ///Stress residual equation relative tolerance
  Real _rtol;
  ///Stress residual equation absolute tolerance
  Real _abs_tol;
  ///Internal variable update equation tolerance
  Real _gtol;
  ///Slip increment tolerance
  Real _slip_incr_tol;

  ///Maximum number of iterations for stress update
  unsigned int _maxiter;
  ///Maximum number of iterations for internal variable update
  unsigned int _maxiterg;

  ///Number of slip system flow rate parameters
  unsigned int _num_slip_sys_flowrate_props;

  ///Type of tangent moduli calculation
  MooseEnum _tan_mod_type;

  ///Read from options for initial values of internal variables
  MooseEnum _intvar_read_type;

  ///Number of slip system specific properties provided in the file containing slip system normals and directions
  unsigned int _num_slip_sys_props;

  bool _gen_rndm_stress_flag;

  ///Input option for scaling variable to generate random stress when convergence fails
  bool _input_rndm_scale_var;

  ///Scaling value
  Real _rndm_scale_var;

  ///Seed value
  unsigned int _rndm_seed;

  ///Maximum number of substep iterations
  unsigned int _max_substep_iter;

  ///Flag to activate line serach
  bool _use_line_search;

  ///Minimum line search step size
  Real _min_lsrch_step;

  ///Line search bisection method tolerance
  Real _lsrch_tol;

  ///Line search bisection method maximum iteration number
  unsigned int _lsrch_max_iter;

  // Line search method
  MooseEnum _lsrch_method;

  MaterialProperty<RankTwoTensor> & _fp;
  const MaterialProperty<RankTwoTensor> & _fp_old;
  MaterialProperty<RankTwoTensor> & _pk2;
  const MaterialProperty<RankTwoTensor> & _pk2_old;
  MaterialProperty<RankTwoTensor> & _lag_e;
  const MaterialProperty<RankTwoTensor> & _lag_e_old;
  MaterialProperty<std::vector<Real>> & _gss;
  const MaterialProperty<std::vector<Real>> & _gss_old;
  MaterialProperty<Real> & _acc_slip;
  const MaterialProperty<Real> & _acc_slip_old;
  MaterialProperty<RankTwoTensor> & _update_rot;

  const MaterialProperty<RankTwoTensor> & _deformation_gradient;
  const MaterialProperty<RankTwoTensor> & _deformation_gradient_old;
  /// Name of the elasticity tensor material property
  const std::string _elasticity_tensor_name;
  /// Elasticity tensor material property
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  const MaterialProperty<RankTwoTensor> & _crysrot;

  DenseVector<Real> _mo;
  DenseVector<Real> _no;

  DenseVector<Real> _a0;
  DenseVector<Real> _xm;

  Real _h0;
  Real _tau_sat;
  Real _tau_init;
  Real _r;

  RankTwoTensor _dfgrd_tmp;
  RankTwoTensor _fe, _fp_old_inv, _fp_inv, _fp_prev_inv;
  DenseVector<Real> _slip_incr, _tau, _dslipdtau;
  std::vector<RankTwoTensor> _s0;

  RankTwoTensor _pk2_tmp, _pk2_tmp_old;
  Real _accslip_tmp, _accslip_tmp_old;
  std::vector<Real> _gss_tmp;
  std::vector<Real> _gss_tmp_old;

  DenseVector<Real> _slip_sys_props;

  DenseMatrix<Real> _dgss_dsliprate;

  bool _read_from_slip_sys_file;

  bool _err_tol; ///Flag to check whether convergence is achieved

  ///Used for substepping; Uniformly divides the increment in deformation gradient
  RankTwoTensor _delta_dfgrd, _dfgrd_tmp_old;
  ///Scales the substepping increment to obtain deformation gradient at a substep iteration
  Real _dfgrd_scale_factor;
  ///Flags to reset variables and reinitialize variables
  bool _first_step_iter, _last_step_iter, _first_substep;
};
