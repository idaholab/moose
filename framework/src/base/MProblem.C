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
    _nl(*_parent, name("nl", _n)),
    _aux(*_parent, name("aux", _n)),
    _quadrature_order(CONSTANT),
    _displaced_mesh(NULL),
    _displaced_problem(NULL),
    _output_displaced(true)
{
  _sys.push_back(&_nl);
  _sys.push_back(&_aux);
  _n++;

  unsigned int n_threads = libMesh::n_threads();
  _qrule.resize(n_threads);
  _fe.resize(n_threads);
  _points.resize(n_threads);
  for (unsigned int i = 0; i < _fe.size(); ++i)
  {
    _fe[i] = FEBase::build(_mesh.dimension(), FEType(FIRST, LAGRANGE)).release();
    _points[i] = _fe[i]->get_xyz();
  }
}

MProblem::~MProblem()
{
//  delete _displaced_mesh;
  delete _displaced_problem;
}

void
MProblem::attachQuadratureRule(QBase *qrule, THREAD_ID tid)
{
  _qrule[tid] = qrule;
  _fe[tid]->attach_quadrature_rule(qrule);

  _nl.attachQuadratureRule(qrule, tid);
  _aux.attachQuadratureRule(qrule, tid);
}

void
MProblem::reinitElem(const Elem * elem, THREAD_ID tid)
{
  _elem = elem;
  _fe[tid]->reinit(elem);
  _points[tid] = _fe[tid]->get_xyz();

  _nl.reinitElem(elem, tid);
  _aux.reinitElem(elem, tid);
}

void
MProblem::reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid)
{
  _elem = elem;
  _fe[tid]->reinit(elem, side);
  _points[tid] = _fe[tid]->get_xyz();

  _nl.reinitElemFace(elem, side, bnd_id, tid);
  _aux.reinitElemFace(elem, side, bnd_id, tid);
}

void
MProblem::reinitNode(const Node * node, THREAD_ID tid)
{
  _nl.reinitNode(node, tid);
  _aux.reinitNode(node, tid);
}

void
MProblem::reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid)
{
  _nl.reinitNodeFace(node, bnd_id, tid);
  _aux.reinitNodeFace(node, bnd_id, tid);
}

void
MProblem::subdomainSetup(unsigned int subdomain, THREAD_ID tid)
{
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
  _nl.addKernel(kernel_name, name, parameters);
}

void
MProblem::addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_problem") = this;
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
  _aux.addKernel(kernel_name, name, parameters);
}

void
MProblem::addAuxBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_problem") = this;
  _aux.addBoundaryCondition(bc_name, name, parameters);
}

void
MProblem::addMaterial(const std::string & mat_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_problem") = this;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    std::vector<unsigned int> blocks = parameters.get<std::vector<unsigned int> >("block");
    for (unsigned int i=0; i<blocks.size(); ++i)
    {
      parameters.set<unsigned int>("block_id") = blocks[i];

      // volume material
      Material *material = static_cast<Material *>(Factory::instance()->create(mat_name, name, parameters));
      mooseAssert(material != NULL, "Not a Material object");
      _materials[tid].addMaterial(blocks[i], material);
      // boundary material
      Material *bnd_material = static_cast<Material *>(Factory::instance()->create(mat_name, name, parameters));
      mooseAssert(bnd_material != NULL, "Not a Material object");
      _materials[tid].addBoundaryMaterial(blocks[i], bnd_material);
    }
  }
}

void
MProblem::init()
{
  SubProblem::init();
  _quadrature_order = _nl.getMinQuadratureOrder();

  _nl.init();
  if (_displaced_problem)
  {
    _displaced_problem->init();
    _displaced_problem->updateMesh(_nl.solution(), _aux.solution());
  }
  _aux.init();

  setSolverDefaults(_nl);
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

  Moose::setSolverDefaults(_nl);
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
  _nl.solution(soln);

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
  _displaced_problem = new DisplacedProblem(*_displaced_mesh, _mesh, displacements);
}

void
MProblem::output()
{
  SubProblem::output();
  if (_displaced_problem != NULL && _output_displaced)
    _displaced_problem->output(_time);
}

} // namespace
