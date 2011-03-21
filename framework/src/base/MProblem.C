#include "MProblem.h"
#include "Factory.h"

namespace Moose {

MProblem::MProblem(Mesh &mesh) :
    SubProblem(mesh),
    _nl(*this, "nl"),
    _aux(*this, "aux")
{
  _sys.push_back(&_nl);
  _sys.push_back(&_aux);
}

MProblem::~MProblem()
{
}

void
MProblem::attachQuadratureRule(QBase *qrule, THREAD_ID tid)
{
  _nl.attachQuadratureRule(qrule, tid);
  _aux.attachQuadratureRule(qrule, tid);
}

void
MProblem::reinitElem(const Elem * elem, THREAD_ID tid)
{
  _nl.reinitElem(elem, tid);
  _aux.reinitElem(elem, tid);
}

void
MProblem::reinitElemFace(const Elem * elem, unsigned int side, THREAD_ID tid)
{
  _nl.reinitElemFace(elem, side, tid);
  _aux.reinitElemFace(elem, side, tid);
}

void
MProblem::reinitNode(const Node * node, THREAD_ID tid)
{
  _nl.reinitNode(node, tid);
  _aux.reinitNode(node, tid);
}

void
MProblem::init()
{
  SubProblem::init();

  _nl.init();
  _aux.init();
}

void
MProblem::addVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains/* = NULL*/)
{
  _nl.addVariable(var_name, type, active_subdomains);
}

void
MProblem::addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_subproblem") = this;
  _nl.addKernel(kernel_name, name, parameters);
}

void
MProblem::addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_subproblem") = this;
  _nl.addBoundaryCondition(bc_name, name, parameters);
}

void
MProblem::addAuxVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains/* = NULL*/)
{
  _aux.addVariable(var_name, type, active_subdomains);
}

void
MProblem::addAuxKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_subproblem") = this;
  _aux.addKernel(kernel_name, name, parameters);
}

void
MProblem::addAuxBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_subproblem") = this;
  _aux.addBoundaryCondition(bc_name, name, parameters);
}

void
MProblem::update()
{
  _nl.update();
}

void
MProblem::solve()
{
  _nl.solve();
}

bool
MProblem::converged()
{
  return _nl.converged();
}

void
MProblem::copySolutionsBackwards()
{
  _nl.copySolutionsBackwards();
  _aux.copySolutionsBackwards();
}

void
MProblem::onTimestepBegin()
{
  if (converged())
  {
    // Update backward time solution vectors
    _nl.copyOldSolutions();
    _aux.copyOldSolutions();
  }
  else
  {
    _nl.restoreSolutions();
  }
  _nl.update();

  _nl.onTimestepBegin();

  if (converged())
  {
    // Update backward material data structures
    updateMaterials();
  }

  // FIXME: fix this
//  Moose::setSolverDefaults(_moose_system, this);
}

void
MProblem::onTimestepEnd()
{
  _nl.printVarNorms();
}

void
MProblem::computeResidual(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number>& soln, NumericVector<Number>& residual)
{
  _nl.solution(soln);
  _aux.compute();
  _nl.computeResidual(residual);
}

void
MProblem::computeJacobian(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian)
{
  _nl.solution(soln);
  _aux.compute();
  _nl.computeJacobian(jacobian);
}

} // namespace
