#include "MProblem.h"
#include "Factory.h"
#include "DisplacedProblem.h"

namespace Moose {

unsigned int MProblem::_n = 0;

static
std::string name(const std::string & name, unsigned int n)
{
  std::ostringstream os;
  os << name << n;
  return os.str();
}

MProblem::MProblem(Mesh & mesh, Problem * parent/* = NULL*/) :
    SubProblem(mesh, parent),
    _nl(*this, name("nl", _n)),
    _aux(*this, name("aux", _n)),
    _quadrature_order(CONSTANT),
    _displaced_mesh(NULL),
    _displaced_problem(NULL),
    _reinit_displaced_elem(false),
    _reinit_displaced_face(false),
    _output_displaced(true)
{
  _sys.push_back(&_nl);
  _sys.push_back(&_aux);
  _n++;

  _asm_info.resize(libMesh::n_threads());
  for (unsigned int i = 0; i < libMesh::n_threads(); ++i)
    _asm_info[i] = new AssemblyData(_mesh);
}

MProblem::~MProblem()
{
  for (unsigned int i = 0; i < libMesh::n_threads(); ++i)
    delete _asm_info[i];

//  delete _displaced_mesh;
  delete _displaced_problem;
}

void
MProblem::prepare(const Elem * elem, THREAD_ID tid)
{
  _asm_info[tid]->reinit(elem);

  _nl.prepare(tid);
  _aux.prepare(tid);
  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->prepare(_displaced_mesh->elem(elem->id()), tid);
}

void
MProblem::reinitElem(const Elem * elem, THREAD_ID tid)
{
  unsigned int n_points = _asm_info[tid]->qRule()->n_points();
  _zero[tid].resize(n_points);
  _grad_zero[tid].resize(n_points);
  _second_zero[tid].resize(n_points);

  _nl.reinitElem(elem, tid);
  _aux.reinitElem(elem, tid);

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitElem(_displaced_mesh->elem(elem->id()), tid);
}

void
MProblem::reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid)
{
  _asm_info[tid]->reinit(elem, side);
  _nl.reinitElemFace(elem, side, bnd_id, tid);
  _aux.reinitElemFace(elem, side, bnd_id, tid);

  if (_displaced_problem != NULL && _reinit_displaced_face)
    _displaced_problem->reinitElemFace(_displaced_mesh->elem(elem->id()), side, bnd_id, tid);
}

void
MProblem::reinitNode(const Node * node, THREAD_ID tid)
{
  _asm_info[tid]->reinit(node);
  _nl.reinitNode(node, tid);
  _aux.reinitNode(node, tid);

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitNode(&_displaced_mesh->node(node->id()), tid);
}

void
MProblem::reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid)
{
  _asm_info[tid]->reinit(node);
  _nl.reinitNodeFace(node, bnd_id, tid);
  _aux.reinitNodeFace(node, bnd_id, tid);

  if (_displaced_problem != NULL && _reinit_displaced_face)
    _displaced_problem->reinitNodeFace(&_displaced_mesh->node(node->id()), bnd_id, tid);
}

void
MProblem::subdomainSetup(unsigned int /*subdomain*/, THREAD_ID /*tid*/)
{
  // FIXME: call displaced_problem->subdomainSetup() ?
}

void
MProblem::addVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains/* = NULL*/)
{
  _nl.addVariable(var_name, type, active_subdomains);
  if (_displaced_problem)
    _displaced_problem->addVariable(var_name, type, active_subdomains);
}

void
MProblem::addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<System *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<System *>("_sys") = &_nl;
  }
  _nl.addKernel(kernel_name, name, parameters);
}

void
MProblem::addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_problem") = this;

  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<System *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    parameters.set<System *>("_sys") = &_nl;
  }
  _nl.addBoundaryCondition(bc_name, name, parameters);
}

void
MProblem::addAuxVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains/* = NULL*/)
{
  _aux.addVariable(var_name, type, active_subdomains);
  if (_displaced_problem)
    _displaced_problem->addAuxVariable(var_name, type, active_subdomains);
}

void
MProblem::addAuxKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<System *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<System *>("_sys") = &_aux;
  }
  _aux.addKernel(kernel_name, name, parameters);
}

void
MProblem::addAuxBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<System *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_face = true;
  }
  else
  {
    parameters.set<System *>("_sys") = &_aux;
  }
  _aux.addBoundaryCondition(bc_name, name, parameters);
}

void
MProblem::addStabilizer(const std::string & stabilizer_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_problem") = this;
  parameters.set<System *>("_sys") = &_nl;
  _nl.addStabilizer(stabilizer_name, name, parameters);
}

void
MProblem::init()
{
  SubProblem::init();
  init2();
}

void
MProblem::init2()
{
  _quadrature_order = _nl.getMinQuadratureOrder();

  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    _asm_info[tid]->createQRules(_quadrature_order);

  _nl.init();
  if (_displaced_problem)
  {
    _displaced_problem->init();
    _displaced_problem->updateMesh(_nl.solution(), _aux.solution());
  }
  _aux.init();
}

void
MProblem::update()
{
  _nl.update();
}

void
MProblem::solve()
{
  setSolverDefaults(_nl);
  Moose::perf_log.push("solve()","Solve");
  _nl.solve();
  Moose::perf_log.pop("solve()","Solve");
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
}

void
MProblem::onTimestepEnd()
{
  _nl.printVarNorms();
}

void
MProblem::computeResidual(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number>& soln, NumericVector<Number>& residual)
{
  _nl.set_solution(soln);

  if (_displaced_problem != NULL)
  {
    _displaced_problem->serializeSolution(soln, _aux.solution());
    _displaced_problem->updateMesh(soln, _aux.solution());
  }

  _aux.compute();
  _nl.computeResidual(residual);
}

void
MProblem::computeJacobian(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian)
{
  _nl.set_solution(soln);

  if (_displaced_problem != NULL)
  {
    _displaced_problem->serializeSolution(soln, _aux.solution());
    _displaced_problem->updateMesh(soln, _aux.solution());
  }

  _aux.compute();
  _nl.computeJacobian(jacobian);
}

void
MProblem::initDisplacedProblem(const std::vector<std::string> & displacements)
{
  _displaced_mesh = new Mesh(_mesh);
  _displaced_problem = new DisplacedProblem(*this, *_displaced_mesh, _mesh, displacements);
}

void
MProblem::output()
{
  SubProblem::output();
  if (_displaced_problem != NULL && _output_displaced)
    _displaced_problem->output(_time);
}

} // namespace
