/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RATEDEPSMEARCRACKMODEL_H
#define RATEDEPSMEARCRACKMODEL_H

#include "ConstitutiveModel.h"
#include "SymmElasticityTensor.h"

#include "petscsys.h"
#include "petscblaslapack.h"

#if PETSC_VERSION_LESS_THAN(3, 5, 0)
extern "C" void FORTRAN_CALL(dgetri)(...); // matrix inversion routine from LAPACK
#endif

class RateDepSmearCrackModel;

template <>
InputParameters validParams<RateDepSmearCrackModel>();

/**
 * RateDepSmearCrackModel is the base class for rate dependent continuum damage model.
 * The model is local and hence mesh sensitive.
 */

class RateDepSmearCrackModel : public ConstitutiveModel
{
public:
  RateDepSmearCrackModel(const InputParameters & parameters);

  virtual ~RateDepSmearCrackModel();

protected:
  virtual void computeStress(const Elem & current_elem,
                             unsigned qp,
                             const SymmElasticityTensor & elasticity_tensor,
                             const SymmTensor & stress_old,
                             SymmTensor & strain_increment,
                             SymmTensor & stress_new);

  virtual void initStatefulProperties(unsigned int n_points);

  virtual void initVariables();

  /**
   * This function solves the state variables.
   * In the present formulation the damaged stress (s) is related to the undamaged stress (s0) as
   * s = exp(-d) * s0 where d is a state variable describing damage.
   * d can be scalar or vector depending on the model
   * A Newton-Raphson is used.
   */
  virtual void solve();
  /**
   * This function updates the internal variables after solve.
   */
  virtual void postSolveVariables();
  /**
   * This function updates the stress after solve.
   * In the base class it is defined as s = exp ( -d) * s0.
   */
  virtual void postSolveStress();

  /**
   * This function updates variables during solve
   * a(i+1) = a(i) + da(i+1)
   **/
  virtual void updateVariables();

  /**
   * This function returns true if convergence is not achieved
   */
  bool getConvergeVar();

  /**
   * This function calculates the residual as
   * r = v - v_old - dv
   **/
  virtual void calcResidual();

  /**
   * This function calculated thw increment of the state variables (dv)
   * used to form the residual.
   */
  virtual void calcStateIncr();

  /**
   * This function calculates the Jacobian
   **/
  virtual void calcJacobian();

  int matrixInversion(std::vector<Real> & A, int n) const;

  Real _ref_damage_rate; ///reference damage rate
  unsigned int _nstate;  ///Number of state variables
  Real _exponent;
  unsigned int _maxiter; ///Maximum number of Newton Raphson iteration
  Real _tol;             ///Relative tolerance factor for convergence of the Newton Raphson solve
  Real _zero_tol;        ///Tolerance for zero.
  Real _intvar_incr_tol; ///Allowable relative increment size of state variables (dv)
  bool _input_rndm_scale_var; ///Flag to specify scaling parameter to generate random stress
  Real _rndm_scale_var;       ///Variable value

  MaterialProperty<std::vector<Real>> & _intvar;
  MaterialProperty<std::vector<Real>> & _intvar_old;

  MaterialProperty<SymmTensor> & _stress_undamaged;
  MaterialProperty<SymmTensor> & _stress_undamaged_old;

  std::vector<Real> _intvar_incr;
  std::vector<Real> _intvar_tmp, _intvar_old_tmp;
  std::vector<Real> _resid;
  std::vector<Real> _jac;
  std::vector<Real> _dvar;

  SymmElasticityTensor _elasticity;
  SymmTensor _stress_old, _dstrain, _stress_new;
  SymmTensor _stress0, _dstress0;
  bool _nconv; ///Convergence flag
  unsigned int _qp;
  bool _err_tol; ///Flag to indicate that increment size has exceeded tolerance and needs cutback

private:
};

#endif // RATEDEPSMEARCRACKMODEL
