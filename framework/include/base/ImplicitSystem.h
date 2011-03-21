#ifndef IMPLICITSYSTEM_H_
#define IMPLICITSYSTEM_H_

#include "System.h"
#include "KernelWarehouse.h"
#include "BCWarehouse.h"

#include "transient_system.h"
#include "nonlinear_implicit_system.h"
#include "numeric_vector.h"
#include "sparse_matrix.h"

namespace Moose {

class ImplicitSystem : public SystemTempl<TransientNonlinearImplicitSystem>
{
public:
  ImplicitSystem(Problem & problem, const std::string & name);
  virtual ~ImplicitSystem();

  virtual bool converged();

  void addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  void addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);

  void computeResidual(NumericVector<Number> & residual);
  void computeJacobian(SparseMatrix<Number> &  jacobian);

  void printVarNorms();

  void timeSteppingScheme(TimeSteppingScheme scheme);

  void onTimestepBegin();

public:
  // FIXME: make these protected and create getters/setters
  Real _last_rnorm;
  Real _l_abs_step_tol;
  Real _initial_residual;

protected:
  void computeTimeDeriv();
  void computeResidualInternal(NumericVector<Number> & residual);
  void finishResidual(NumericVector<Number> & residual);

  Real & _dt;
  Real & _dt_old;
  int & _t_step;
  std::vector<Real> _time_weight;                       /// Coefficients (weights) for the time discretization
  TimeSteppingScheme _time_stepping_scheme;             /// Time stepping scheme used for time discretization
  Real _time_stepping_order;                            /// The order of the time stepping scheme

  // holders
  std::vector<KernelWarehouse> _kernels;
  std::vector<BCWarehouse> _bcs;

  friend class ComputeResidualThread;
  friend class ComputeJacobianThread;
};

} // namespace

#endif /* IMPLICITSYSTEM_H_ */
