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


protected:
  void computeTimeDeriv();
  void computeResidualInternal(NumericVector<Number> & residual);
  void finishResidual(NumericVector<Number> & residual);

  // holders
  std::vector<std::vector<Kernel *> > _kernels;

  std::vector<std::map<unsigned int, std::vector<IntegratedBC *> > > _bcs;
  std::vector<std::map<unsigned int, std::vector<NodalBC *> > > _nodal_bcs;

  friend class ComputeResidualThread;
  friend class ComputeJacobianThread;
};

}

#endif /* IMPLICITSYSTEM_H_ */
