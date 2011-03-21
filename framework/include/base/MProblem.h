#ifndef MPROBLEM_H_
#define MPROBLEM_H_

#include "Problem.h"
#include "Mesh.h"
#include "ImplicitSystem.h"
#include "AuxiliarySystem.h"

namespace Moose {

/**
 * Specialization of Problem for solving nonlinear equations plus auxiliary equations
 *
 */
class MProblem : public Problem
{
public:
  MProblem(Mesh &mesh);
  virtual ~MProblem();

  virtual void attachQuadratureRule(QBase *qrule, THREAD_ID tid);
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);
  virtual void reinitElemFace(const Elem * elem, unsigned int side, THREAD_ID tid);
  virtual void reinitNode(const Node * node, THREAD_ID tid);

  virtual void init();

  virtual void update();

  virtual void solve();
  virtual bool converged();

  virtual void onTimestepBegin();
  virtual void onTimestepEnd();

  virtual void copySolutionsBackwards();

  // NL /////
  void addVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains = NULL);

  void addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  void addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);

  ImplicitSystem & getNonlinearSystem() { return _nl; }

  // Aux /////
  void addAuxVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains = NULL);

  void addAuxKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  void addAuxBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);

  AuxiliarySystem & getAuxiliarySystem() { return _aux; }

  ////
  virtual void computeResidual(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, NumericVector<Number> & residual);
  virtual void computeJacobian(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, SparseMatrix<Number> &  jacobian);

protected:
  ImplicitSystem _nl;
  AuxiliarySystem _aux;
};

}

#endif /* MPROBLEM_H_ */
