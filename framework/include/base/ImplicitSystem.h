#ifndef IMPLICITSYSTEM_H_
#define IMPLICITSYSTEM_H_

#include "SubProblem.h"

#include "transient_system.h"
#include "nonlinear_implicit_system.h"
#include "numeric_vector.h"
#include "sparse_matrix.h"

namespace Moose {

class ImplicitSystem : public SubProblemTempl<TransientNonlinearImplicitSystem>
{
public:
  ImplicitSystem(Problem & problem, const std::string & name);
  virtual ~ImplicitSystem();

  virtual bool converged();

  void addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  void addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);

  void computeResidual(NumericVector<Number> & residual);
  void computeJacobian(SparseMatrix<Number> &  jacobian);

  std::map<std::string, Moose::Variable *>::iterator varsBegin(THREAD_ID tid = 0) { return _vars[tid].begin(); }
  std::map<std::string, Moose::Variable *>::iterator varsEnd(THREAD_ID tid = 0) { return _vars[tid].end(); }

  void printVarNorms();

  void timeSteppingScheme(TimeSteppingScheme scheme);

  void onTimestepBegin();

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
  std::vector<std::vector<Kernel *> > _kernels;

  std::vector<std::map<unsigned int, std::vector<IntegratedBC *> > > _bcs;
  std::vector<std::map<unsigned int, std::vector<NodalBC *> > > _nodal_bcs;

  friend class ComputeResidualThread;
  friend class ComputeJacobianThread;
};

}

#endif /* IMPLICITSYSTEM_H_ */
