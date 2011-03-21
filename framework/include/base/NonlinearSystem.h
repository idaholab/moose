#ifndef NONLINEARSYSTEM_H
#define NONLINEARSYSTEM_H

#include "SystemBase.h"
#include "KernelWarehouse.h"
#include "BCWarehouse.h"
#include "StabilizerWarehouse.h"

#include "transient_system.h"
#include "nonlinear_implicit_system.h"
#include "numeric_vector.h"
#include "sparse_matrix.h"
#include "preconditioner.h"

class MProblem;

/**
 * Nonlinear system to be solved
 *
 * It is a part of MProblem ;-)
 */
class NonlinearSystem : public SystemTempl<TransientNonlinearImplicitSystem>
{
public:
  NonlinearSystem(MProblem & problem, const std::string & name);
  virtual ~NonlinearSystem();

  virtual void init();
  virtual bool converged();

  void addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);
  void addStabilizer(const std::string & stabilizer_name, const std::string & name, InputParameters parameters);

  void computeResidual(NumericVector<Number> & residual);
  void computeJacobian(SparseMatrix<Number> &  jacobian);
  void computeJacobianBlock(SparseMatrix<Number> & jacobian, libMesh::System & precond_system, unsigned int ivar, unsigned int jvar);

  void printVarNorms();

  void timeSteppingScheme(Moose::TimeSteppingScheme scheme);

  void onTimestepBegin();

  void setVarScaling(std::vector<Real> scaling);
  void setScaling();

  virtual void set_solution(const NumericVector<Number> & soln) { _nl_solution = soln; }
  virtual NumericVector<Number> & solution() { return _nl_solution; }

  void setPreconditioner(Preconditioner<Real> *pc);

public:
  MProblem & _mproblem;
  // FIXME: make these protected and create getters/setters
  Real _last_rnorm;
  Real _l_abs_step_tol;
  Real _initial_residual;

protected:
  void computeTimeDeriv();
  void computeResidualInternal(NumericVector<Number> & residual);
  void finishResidual(NumericVector<Number> & residual);

  NumericVector<Number> & _nl_solution;                 /// solution vector from nonlinear solver

  Real & _t;                                            /// time
  Real & _dt;                                           /// size of the time step
  Real & _dt_old;                                       /// previous time step size
  int & _t_step;                                        /// time step (number)
  std::vector<Real> _time_weight;                       /// Coefficients (weights) for the time discretization
  Moose::TimeSteppingScheme _time_stepping_scheme;      /// Time stepping scheme used for time discretization
  Real _time_stepping_order;                            /// The order of the time stepping scheme

  // holders
  std::vector<KernelWarehouse> _kernels;                /// Kernel storage for each thread
  std::vector<BCWarehouse> _bcs;                        /// BC storage for each thread
  std::vector<StabilizerWarehouse> _stabilizers;        /// Stabilizers storage for each thread

  Preconditioner<Real> * _preconditioner;               /// Preconditioner

  friend class ComputeResidualThread;
  friend class ComputeJacobianThread;
};

#endif /* NONLINEARSYSTEM_H */
