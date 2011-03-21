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


class NonlinearSystem : public SystemTempl<TransientNonlinearImplicitSystem>
{
public:
  NonlinearSystem(SubProblem & problem, const std::string & name);
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

  virtual void init();

  void setPreconditioner(Preconditioner<Real> *pc);

public:
  SubProblem & _subproblem;
  // FIXME: make these protected and create getters/setters
  Real _last_rnorm;
  Real _l_abs_step_tol;
  Real _initial_residual;

protected:
  void computeTimeDeriv();
  void computeResidualInternal(NumericVector<Number> & residual);
  void finishResidual(NumericVector<Number> & residual);

  NumericVector<Number> & _nl_solution;

  Real & _t;
  Real & _dt;
  Real & _dt_old;
  int & _t_step;
  std::vector<Real> _time_weight;                       /// Coefficients (weights) for the time discretization
  Moose::TimeSteppingScheme _time_stepping_scheme;             /// Time stepping scheme used for time discretization
  Real _time_stepping_order;                            /// The order of the time stepping scheme

  // holders
  std::vector<KernelWarehouse> _kernels;
  std::vector<BCWarehouse> _bcs;
  std::vector<StabilizerWarehouse> _stabilizers;

  Preconditioner<Real> * _preconditioner;               /// Preconditioner

  friend class ComputeResidualThread;
  friend class ComputeJacobianThread;
};

#endif /* NONLINEARSYSTEM_H */
