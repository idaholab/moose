/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "FEProblem.h"
#include "Factory.h"
#include "ProblemFactory.h"
#include "DisplacedProblem.h"
#include "OutputProblem.h"
#include "MaterialData.h"
#include "ComputePostprocessorsThread.h"
#include "ComputeNodalPPSThread.h"
#include "ActionWarehouse.h"
#include "Conversion.h"
#include "Moose.h"
#include "ConstantIC.h"
#include "FP.h"
#include "Parser.h"
#include "ElementH1Error.h"

unsigned int FEProblem::_n = 0;

static
std::string name_sys(const std::string & name, unsigned int n)
{
  std::ostringstream os;
  os << name << n;
  return os.str();
}

template<>
InputParameters validParams<FEProblem>()
{
  return validParams<SubProblem>();
}

FEProblem::FEProblem(const std::string & name, InputParameters parameters) :
    SubProblem(name, parameters),
    _nl(*this, name_sys("nl", _n)),
    _aux(*this, name_sys("aux", _n)),
    _coupling(Moose::COUPLING_DIAG),
    _cm(NULL),
    _quadrature_order(CONSTANT),
    _pps_output_table_max_rows(0),
    _postprocessor_screen_output(true),
    _postprocessor_csv_output(false),
    _postprocessor_gnuplot_output(false),
    _gnuplot_format("ps"),
    _ex_reader(NULL),
    _out(*this, _eq),
    _out_problem(NULL),
#ifdef LIBMESH_ENABLE_AMR
    _adaptivity(*this),
    _uniform_refine_level(0),
#endif
    _displaced_mesh(NULL),
    _displaced_problem(NULL),
    _geometric_search_data(*this, _mesh),
    _reinit_displaced_elem(false),
    _reinit_displaced_face(false),
    _output_displaced(false),
    _output_solution_history(false),
    _input_file_saved(false),
    _has_dampers(false),
    _has_constraints(false),
    _resurrector(*this),
//    _solve_only_perf_log("Solve Only"),
    _output_setup_log_early(false),
    // debugging
    _dbg_top_residuals(0)
{
  _n++;

  unsigned int n_threads = libMesh::n_threads();

  _assembly.resize(n_threads);
  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i] = new Assembly(_nl, couplingMatrix(), i);

  _functions.resize(n_threads);
  _materials.resize(n_threads);

  _material_data.resize(n_threads);
  _bnd_material_data.resize(n_threads);
  _neighbor_material_data.resize(n_threads);
  for (unsigned int i = 0; i < n_threads; i++)
  {
    _material_data[i] = new MaterialData(_material_props);
    _bnd_material_data[i] = new MaterialData(_bnd_material_props);
    _neighbor_material_data[i] = new MaterialData(_bnd_material_props);
  }

  _pps_data.resize(n_threads);
}

FEProblem::~FEProblem()
{
  delete _cm;

  bool stateful_props = _material_props.hasStatefulProperties();

  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; i++)
  {
    delete _assembly[i];

    if (!stateful_props)
    {
      delete _material_data[i];
      delete _bnd_material_data[i];
      delete _neighbor_material_data[i];
    }
  }

  if (stateful_props)
  {
    _material_props.releaseProperties();
    _bnd_material_props.releaseProperties();
  }

  delete _displaced_mesh;
  delete _displaced_problem;

  if (_out_problem)
    delete _out_problem;
}

void FEProblem::setCoordSystem(Moose::CoordinateSystemType type)
{
  SubProblem::setCoordSystem(type);
  if (_displaced_problem)
    _displaced_problem->setCoordSystem(type);
}

void FEProblem::initialSetup()
{
  if (_resurrector.isOn())
    _resurrector.restartFromFile();
  else
  {
    if (_ex_reader != NULL)
    {
      _nl.copyVars(*_ex_reader);
      _aux.copyVars(*_ex_reader);
    }
  }

  // uniform refine
  if (_uniform_refine_level > 0)
  {
    Moose::setup_perf_log.push("Uniformly Refine Mesh","Setup");
    adaptivity().uniformRefine(_uniform_refine_level);
    Moose::setup_perf_log.pop("Uniformly Refine Mesh","Setup");
  }

  if (!_resurrector.isOn())
    projectSolution();

  _eq.print_info();

  unsigned int n_threads = libMesh::n_threads();

  Moose::setup_perf_log.push("copySolutionsBackwards()","Setup");
  copySolutionsBackwards();
  Moose::setup_perf_log.pop("copySolutionsBackwards()","Setup");

  if (!_resurrector.isOn())
    _aux.compute(EXEC_INITIAL);

  if (_material_props.hasStatefulProperties())
  {
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    for (ConstElemRange::const_iterator el = elem_range.begin() ; el != elem_range.end(); ++el)
    {
      const Elem * elem = *el;
      SubdomainID blk_id = elem->subdomain_id();

      mooseAssert(_materials[0].hasMaterials(blk_id), "No materials on subdomain block " + elem->id());
      _assembly[0]->reinit(elem);
      unsigned int n_points = _assembly[0]->qRule()->n_points();
      _material_data[0]->initStatefulProps(_materials[0].getMaterials(blk_id), n_points, *elem);

      for (unsigned int side=0; side<elem->n_sides(); side++)
      {
        mooseAssert(_materials[0].hasBoundaryMaterials(blk_id), "No face materials on subdomain block " + elem->id());
        _assembly[0]->reinit(elem, side);
        unsigned int n_points = _assembly[0]->qRuleFace()->n_points();
        _bnd_material_data[0]->initStatefulProps(_materials[0].getBoundaryMaterials(blk_id), n_points, *elem, side);
        // TODO:
      }
    }
  }
  else
  {
  }

//  // RUN initial postprocessors
//  computePostprocessors(EXEC_INITIAL);

#ifdef LIBMESH_ENABLE_AMR
  Moose::setup_perf_log.push("initial adaptivity","Setup");
  for (unsigned int i = 0; i < adaptivity().getInitialSteps(); i++)
  {
    adaptMesh();
    //reproject the initial condition
//    _nl.sys().project_solution(Moose::initial_value, Moose::initial_gradient, _eq.parameters);
    _nl.projectSolution();
  }
  Moose::setup_perf_log.pop("initial adaptivity","Setup");
#endif //LIBMESH_ENABLE_AMR

  _nl.set_solution(*(_nl.sys().current_local_solution.get()));

  Moose::setup_perf_log.push("Initial updateGeomSearch()","Setup");
  //Update the geometric searches (has to be called after the problem is all set up)
  updateGeomSearch();
  Moose::setup_perf_log.pop("Initial updateGeomSearch()","Setup");

  Moose::setup_perf_log.push("Initial updateActiveSemiLocalNodeRange()","Setup");
  _mesh.updateActiveSemiLocalNodeRange(_ghosted_elems);
  if(_displaced_mesh)
    _displaced_mesh->updateActiveSemiLocalNodeRange(_ghosted_elems);
  Moose::setup_perf_log.pop("Initial updateActiveSemiLocalNodeRange()","Setup");

  // Need to see if _any_ processor has ghosted elems
  unsigned int ghosted = _ghosted_elems.size();
  Parallel::sum(ghosted);

  if(ghosted)
  {
    Moose::setup_perf_log.push("reinit() after updateGeomSearch()","Setup");
    // Call reinit to get the ghosted vectors correct now that some geometric search has been done
    _eq.reinit();

    if(_displaced_mesh)
      _displaced_problem->es().reinit();

    Moose::setup_perf_log.pop("reinit() after updateGeomSearch()","Setup");
  }

  if(_displaced_mesh)
    _displaced_problem->updateMesh(*_nl.currentSolution(), *_aux.currentSolution());

  for(unsigned int i=0; i<n_threads; i++)
  {
    _materials[i].initialSetup();
    _pps(EXEC_RESIDUAL)[i].initialSetup();
    _pps(EXEC_JACOBIAN)[i].initialSetup();
    _pps(EXEC_TIMESTEP)[i].initialSetup();
    _pps(EXEC_TIMESTEP_BEGIN)[i].initialSetup();
  }

  _aux.initialSetup();
  _aux.compute(EXEC_TIMESTEP_BEGIN);

  Moose::setup_perf_log.push("Initial computePostprocessors()","Setup");
  computePostprocessors();
  computePostprocessors(EXEC_INITIAL);
  computePostprocessors(EXEC_TIMESTEP_BEGIN);
  computePostprocessors(EXEC_RESIDUAL);
  Moose::setup_perf_log.pop("Initial computePostprocessors()","Setup");


  Moose::setup_perf_log.push("Output Initial Condition","Setup");
  if (_output_initial)
  {
    output();
    outputPostprocessors();
  }
  Moose::setup_perf_log.pop("Output Initial Condition","Setup");

  _nl.initialSetupBCs();
  _nl.initialSetupKernels();

  if (_output_setup_log_early)
    Moose::setup_perf_log.print_log();

  _nl.initialSetup();
}

void FEProblem::timestepSetup()
{
  unsigned int n_threads = libMesh::n_threads();

  for(unsigned int i=0; i<n_threads; i++)
  {
    _materials[i].timestepSetup();
  }

  _aux.timestepSetup();
  _nl.timestepSetup();
  _out.timestepSetup();
  if (_out_problem)
    _out_problem->timestepSetup();
}

void
FEProblem::prepare(const Elem * elem, THREAD_ID tid)
{
  _assembly[tid]->reinit(elem);

  _nl.prepare(tid);
  _aux.prepare(tid);
  _assembly[tid]->prepare();

  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->prepare(_displaced_mesh->elem(elem->id()), tid);
}

void
FEProblem::prepare(const Elem * elem, unsigned int ivar, unsigned int jvar, const std::vector<unsigned int> & dof_indices, THREAD_ID tid)
{
  _assembly[tid]->reinit(elem);

  _nl.prepare(tid);
  _aux.prepare(tid);
  _assembly[tid]->prepareBlock(ivar, jvar, dof_indices);

  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->prepare(_displaced_mesh->elem(elem->id()), ivar, jvar, dof_indices, tid);
}

void
FEProblem::prepareAssembly(THREAD_ID tid)
{
  _assembly[tid]->prepare();

  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->prepareAssembly(tid);
}

void
FEProblem::addResidual(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->addResidual(residual);
  if(_displaced_problem)
    _displaced_problem->addResidual(residual, tid);
}

void
FEProblem::addResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->addResidualNeighbor(residual);
  if(_displaced_problem)
    _displaced_problem->addResidualNeighbor(residual, tid);
}

void
FEProblem::addResidualScalar(NumericVector<Number> & residual, THREAD_ID tid/* = 0*/)
{
  _assembly[tid]->addResidualScalar(residual);
}

void
FEProblem::cacheResidual(THREAD_ID tid)
{
  _assembly[tid]->cacheResidual();
  if(_displaced_problem)
    _displaced_problem->cacheResidual(tid);
}

void
FEProblem::cacheResidualNeighbor(THREAD_ID tid)
{
  _assembly[tid]->cacheResidualNeighbor();
  if(_displaced_problem)
    _displaced_problem->cacheResidualNeighbor(tid);
}

void
FEProblem::addCachedResidual(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->addCachedResidual(residual);
  if(_displaced_problem)
    _displaced_problem->addCachedResidual(residual, tid);
}

void
FEProblem::setResidual(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->setResidual(residual);
  if(_displaced_problem)
    _displaced_problem->setResidual(residual, tid);
}

void
FEProblem::setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->setResidualNeighbor(residual);
  if(_displaced_problem)
    _displaced_problem->setResidualNeighbor(residual, tid);
}

void
FEProblem::addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _assembly[tid]->addJacobian(jacobian);
  if(_displaced_problem)
    _displaced_problem->addJacobian(jacobian, tid);
}

void
FEProblem::addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _assembly[tid]->addJacobianNeighbor(jacobian);
  if(_displaced_problem)
    _displaced_problem->addJacobianNeighbor(jacobian, tid);
}

void
FEProblem::addJacobianScalar(SparseMatrix<Number> & jacobian, THREAD_ID tid/* = 0*/)
{
  _assembly[tid]->addJacobianScalar(jacobian);
}

void
FEProblem::cacheJacobian(THREAD_ID tid)
{
  _assembly[tid]->cacheJacobian();
  if(_displaced_problem)
    _displaced_problem->cacheJacobian(tid);
}

void
FEProblem::cacheJacobianNeighbor(THREAD_ID tid)
{
  _assembly[tid]->cacheJacobianNeighbor();
  if(_displaced_problem)
    _displaced_problem->cacheJacobianNeighbor(tid);
}

void
FEProblem::addCachedJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _assembly[tid]->addCachedJacobian(jacobian);
  if(_displaced_problem)
    _displaced_problem->addCachedJacobian(jacobian, tid);
}

void
FEProblem::addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, THREAD_ID tid)
{
  _assembly[tid]->addJacobianBlock(jacobian, ivar, jvar, dof_map, dof_indices);
  if(_displaced_problem)
    _displaced_problem->addJacobianBlock(jacobian, ivar, jvar, dof_map, dof_indices, tid);
}

void
FEProblem::addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, std::vector<unsigned int> & neighbor_dof_indices, THREAD_ID tid)
{
  _assembly[tid]->addJacobianNeighbor(jacobian, ivar, jvar, dof_map, dof_indices, neighbor_dof_indices);
  if(_displaced_problem)
    _displaced_problem->addJacobianNeighbor(jacobian, ivar, jvar, dof_map, dof_indices, neighbor_dof_indices, tid);
}

void
FEProblem::prepareShapes(unsigned int var, THREAD_ID tid)
{
  _assembly[tid]->copyShapes(var);
}

void
FEProblem::prepareFaceShapes(unsigned int var, THREAD_ID tid)
{
  _assembly[tid]->copyFaceShapes(var);
}

void
FEProblem::prepareNeighborShapes(unsigned int var, THREAD_ID tid)
{
  _assembly[tid]->copyNeighborShapes(var);
}

void
FEProblem::addGhostedElem(unsigned int elem_id)
{
  if(_mesh.elem(elem_id)->processor_id() != libMesh::processor_id())
    _ghosted_elems.insert(elem_id);
}

void
FEProblem::addGhostedBoundary(BoundaryID boundary_id)
{
  _mesh.addGhostedBoundary(boundary_id);

  if(_displaced_problem)
    _displaced_mesh->addGhostedBoundary(boundary_id);
}

bool
FEProblem::reinitDirac(const Elem * elem, THREAD_ID tid)
{
  std::set<Point> & points_set = _dirac_kernel_info._points[elem];

  bool have_points = points_set.size();

  if(have_points)
  {
    std::vector<Point> points(points_set.size());
    std::copy(points_set.begin(), points_set.end(), points.begin());

    _assembly[tid]->reinitAtPhysical(elem, points);

    _nl.prepare(tid);
    _aux.prepare(tid);

    reinitElem(elem, tid);
  }
  _assembly[tid]->prepare();

  if (_displaced_problem != NULL && (_reinit_displaced_elem))
    have_points = have_points || _displaced_problem->reinitDirac(_displaced_mesh->elem(elem->id()), tid);

  return have_points;
}

void
FEProblem::reinitElem(const Elem * elem, THREAD_ID tid)
{
  unsigned int n_points = _assembly[tid]->qRule()->n_points();
  _zero[tid].resize(n_points, 0);
  _grad_zero[tid].resize(n_points, 0);
  _second_zero[tid].resize(n_points, 0);

  _nl.reinitElem(elem, tid);
  _aux.reinitElem(elem, tid);

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitElem(_displaced_mesh->elem(elem->id()), tid);
}

void
FEProblem::reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid)
{
  _assembly[tid]->reinit(elem, side);

  unsigned int n_points = _assembly[tid]->qRule()->n_points();
  _zero[tid].resize(n_points, 0);
  _grad_zero[tid].resize(n_points, 0);
  _second_zero[tid].resize(n_points, 0);

  _nl.reinitElemFace(elem, side, bnd_id, tid);
  _aux.reinitElemFace(elem, side, bnd_id, tid);

  if (_displaced_problem != NULL && _reinit_displaced_face)
    _displaced_problem->reinitElemFace(_displaced_mesh->elem(elem->id()), side, bnd_id, tid);
}

void
FEProblem::reinitNode(const Node * node, THREAD_ID tid)
{
  _assembly[tid]->reinit(node);

  unsigned int n_points = 1;
  _zero[tid].resize(n_points, 0);
  _grad_zero[tid].resize(n_points, 0);
  _second_zero[tid].resize(n_points, 0);

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitNode(&_displaced_mesh->node(node->id()), tid);

  _nl.reinitNode(node, tid);
  _aux.reinitNode(node, tid);
}

void
FEProblem::reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid)
{
  _assembly[tid]->reinit(node);

  unsigned int n_points = 1;
  _zero[tid].resize(n_points, 0);
  _grad_zero[tid].resize(n_points, 0);
  _second_zero[tid].resize(n_points, 0);

  if (_displaced_problem != NULL && _reinit_displaced_face)
    _displaced_problem->reinitNodeFace(&_displaced_mesh->node(node->id()), bnd_id, tid);

  _nl.reinitNodeFace(node, bnd_id, tid);
  _aux.reinitNodeFace(node, bnd_id, tid);

}

void
FEProblem::reinitNodes(const std::vector<unsigned int> & nodes, THREAD_ID tid)
{
  unsigned int n_points = nodes.size();
  _zero[tid].resize(n_points, 0);
  _grad_zero[tid].resize(n_points, 0);
  _second_zero[tid].resize(n_points, 0);

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitNodes(nodes, tid);

  _nl.reinitNodes(nodes, tid);
  _aux.reinitNodes(nodes, tid);
}

void
FEProblem::reinitNodeNeighbor(const Node * node, THREAD_ID tid)
{
  _assembly[tid]->reinitNodeNeighbor(node);

  unsigned int n_points = 1;
  _zero[tid].resize(n_points, 0);
  _grad_zero[tid].resize(n_points, 0);
  _second_zero[tid].resize(n_points, 0);

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitNodeNeighbor(&_displaced_mesh->node(node->id()), tid);

  _nl.reinitNodeNeighbor(node, tid);
  _aux.reinitNodeNeighbor(node, tid);
}

void
FEProblem::reinitScalars(THREAD_ID tid)
{
  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitScalars(tid);

  _nl.reinitScalars(tid);
  _aux.reinitScalars(tid);

  _assembly[tid]->prepareScalar();
}


void
FEProblem::reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid)
{
  const Elem * neighbor = elem->neighbor(side);
  unsigned int neighbor_side = neighbor->which_neighbor_am_i(elem);

  _assembly[tid]->reinit(elem, side, neighbor);

  _nl.prepareNeighbor(tid);
  _aux.prepareNeighbor(tid);

  _assembly[tid]->prepareNeighbor();

  BoundaryID bnd_id = 0;              // some dummy number (it is not really used for anything, right now)
  _nl.reinitElemFace(elem, side, bnd_id, tid);
  _aux.reinitElemFace(elem, side, bnd_id, tid);

  _nl.reinitNeighborFace(neighbor, neighbor_side, bnd_id, tid);
  _aux.reinitNeighborFace(neighbor, neighbor_side, bnd_id, tid);
}

void
FEProblem::reinitNeighborPhys(const Elem * neighbor, unsigned int neighbor_side, const std::vector<Point> & physical_points, THREAD_ID tid)
{
  // Reinits shape the functions at the physical points
  _assembly[tid]->reinitNeighborAtPhysical(neighbor, neighbor_side, physical_points);

  // Sets the neighbor dof indices
  _nl.prepareNeighbor(tid);
  _aux.prepareNeighbor(tid);

  // Resizes Re and Ke
  _assembly[tid]->prepareNeighbor();

  // Compute the values of each variable at the points
  _nl.reinitNeighbor(neighbor, tid);
  _aux.reinitNeighbor(neighbor, tid);

  // Do the same for the displaced problem
  if (_displaced_problem != NULL)
    _displaced_problem->reinitNeighborPhys(_displaced_mesh->elem(neighbor->id()), neighbor_side, physical_points, tid);
}

void
FEProblem::getDiracElements(std::set<const Elem *> & elems)
{
  // First add in the undisplaced elements
  elems =_dirac_kernel_info._elements;

  if(_displaced_problem)
  {
    std::set<const Elem *> displaced_elements;
    _displaced_problem->getDiracElements(displaced_elements);

    { // Use the ids from the displaced elements to get the undisplaced elements
      // and add them to the list
      std::set<const Elem *>::iterator it = displaced_elements.begin();
      std::set<const Elem *>::iterator end = displaced_elements.end();

      for(;it != end; ++it)
        elems.insert(_mesh.elem((*it)->id()));
    }
  }
}

void
FEProblem::clearDiracInfo()
{
  _dirac_kernel_info.clearPoints();

  if(_displaced_problem)
    _displaced_problem->clearDiracInfo();
}


void
FEProblem::subdomainSetup(SubdomainID subdomain, THREAD_ID tid)
{
  if (_materials[tid].hasMaterials(subdomain))
  {
    // call subdomainSetup
    for (std::vector<Material *>::const_iterator it = _materials[tid].getMaterials(subdomain).begin(); it != _materials[tid].getMaterials(subdomain).end(); ++it)
      (*it)->subdomainSetup();

    // Need to reinitialize the material properties in case subdomain setup for a Kernel needs it
    // TODO: This means we are doing one more materialReinit than is necessary.  Need to refactor this to
    // keep that from happening

    // FIXME: cannot do this b/c variables are not computed => potential NaNs will pop-up
//    reinitMaterials(subdomain, tid);
  }

  _nl.subdomainSetup(subdomain, tid);

  // FIXME: call displaced_problem->subdomainSetup() ?
  //        When adding possibility with materials being evaluated on displaced mesh
}

void
FEProblem::subdomainSetupSide(SubdomainID subdomain, THREAD_ID tid)
{
  if (_materials[tid].hasBoundaryMaterials(subdomain))
  {
    // call subdomainSetup
    for (std::vector<Material *>::const_iterator it = _materials[tid].getBoundaryMaterials(subdomain).begin(); it != _materials[tid].getBoundaryMaterials(subdomain).end(); ++it)
      (*it)->subdomainSetup();

  }
}

void
FEProblem::addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< SubdomainID > * const active_subdomains/* = NULL*/)
{
  if (hasVariable(var_name))
    mooseError("Variable with name '" << var_name << "' already exists.");

  _nl.addVariable(var_name, type, scale_factor, active_subdomains);
  if (_displaced_problem)
    _displaced_problem->addVariable(var_name, type, scale_factor, active_subdomains);
}

void
FEProblem::addScalarVariable(const std::string & var_name, Order order, Real scale_factor)
{
  _nl.addScalarVariable(var_name, order, scale_factor);
  if (_displaced_problem)
    _displaced_problem->addScalarVariable(var_name, order, scale_factor);
}

void
FEProblem::addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addKernel(kernel_name, name, parameters);
}

void
FEProblem::addScalarKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addScalarKernel(kernel_name, name, parameters);
}

void
FEProblem::addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addBoundaryCondition(bc_name, name, parameters);
}

void
FEProblem::addConstraint(const std::string & c_name, const std::string & name, InputParameters parameters)
{
  _has_constraints = true;

  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    // It might _want_ to use a displaced mesh... but we're not so set it to false
    if(parameters.have_parameter<bool>("use_displaced_mesh"))
      parameters.set<bool>("use_displaced_mesh") = false;

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addConstraint(c_name, name, parameters);
}

void
FEProblem::addAuxVariable(const std::string & var_name, const FEType & type, const std::set< SubdomainID > * const active_subdomains/* = NULL*/)
{
  _aux.addVariable(var_name, type, 1.0, active_subdomains);
  if (_displaced_problem)
    _displaced_problem->addAuxVariable(var_name, type, active_subdomains);
}

void
FEProblem::addAuxScalarVariable(const std::string & var_name, Order order, Real scale_factor)
{
  _aux.addScalarVariable(var_name, order, scale_factor);
  if (_displaced_problem)
    _displaced_problem->addAuxScalarVariable(var_name, order, scale_factor);
}

void
FEProblem::addAuxKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    parameters.set<SystemBase *>("_nl_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
    parameters.set<SystemBase *>("_nl_sys") = &_nl;
  }
  _aux.addKernel(kernel_name, name, parameters);
}

void
FEProblem::addAuxScalarKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
  }
  _aux.addScalarKernel(kernel_name, name, parameters);
}

void
FEProblem::addAuxBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {

    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    parameters.set<SystemBase *>("_nl_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
    parameters.set<SystemBase *>("_nl_sys") = &_nl;
  }
  _aux.addBoundaryCondition(bc_name, name, parameters);
}

void
FEProblem::addDiracKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addDiracKernel(kernel_name, name, parameters);
}

// DGKernels ////

void
FEProblem::addDGKernel(const std::string & dg_kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addDGKernel(dg_kernel_name, name, parameters);
}

void
FEProblem::addInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters)
{
  const std::string & var_name = parameters.get<std::string>("variable");

  if (_nl.hasVariable(var_name))
    _nl.addInitialCondition(ic_name, name, parameters);
  else if (_nl.hasScalarVariable(var_name))
    _nl.addScalarInitialCondition(ic_name, name, parameters);
  else if (_aux.hasVariable(var_name))
    _aux.addInitialCondition(ic_name, name, parameters);
  else if (_aux.hasScalarVariable(var_name))
    _aux.addScalarInitialCondition(ic_name, name, parameters);
}

void
FEProblem::projectSolution()
{
  Moose::enableFPE();

  _aux.projectSolution();
  _nl.projectSolution();

  Moose::enableFPE(false);
}

void
FEProblem::addMaterial(const std::string & mat_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;
  parameters.set<SubProblem *>("_subproblem_displaced") = _displaced_problem;

  std::vector<SubdomainName> blocks = parameters.get<std::vector<SubdomainName> >("block");
  std::vector<SubdomainID> block_ids(blocks.size());

  for (unsigned int i=0; i<blocks.size(); ++i)
    block_ids[i] = _mesh.getSubdomainID(blocks[i]);

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    // volume material
    parameters.set<bool>("_bnd") = false;
    parameters.set<MaterialData *>("_material_data") = _material_data[tid];
    Material *volume_material = static_cast<Material *>(Factory::instance()->create(mat_name, name, parameters));
    mooseAssert(volume_material != NULL, "Not a Material object");
    _materials[tid].addMaterial(block_ids, volume_material);

    // boundary material
    parameters.set<bool>("_bnd") = true;
    parameters.set<MaterialData *>("_material_data") = _bnd_material_data[tid];
    Material *bnd_material = static_cast<Material *>(Factory::instance()->create(mat_name, name, parameters));
    mooseAssert(bnd_material != NULL, "Not a Material object");
    _materials[tid].addBoundaryMaterial(block_ids, bnd_material);

    // neighbor material
    parameters.set<bool>("_bnd") = true;
    parameters.set<MaterialData *>("_material_data") = _neighbor_material_data[tid];
    Material *neighbor_material = static_cast<Material *>(Factory::instance()->create(mat_name, name, parameters));
    mooseAssert(neighbor_material != NULL, "Not a Material object");
    _materials[tid].addNeighborMaterial(block_ids, neighbor_material);
  }
}

const std::vector<Material *> &
FEProblem::getMaterialsByName(const std::string & name, THREAD_ID tid)
{
  return _materials[tid].getMaterialsByName(name);
}

const std::vector<Material*> &
FEProblem::getMaterials(SubdomainID block_id, THREAD_ID tid)
{
  mooseAssert( tid < _materials.size(), "Requesting a material warehouse that does not exist");
  return _materials[tid].getMaterials(block_id);
}

const std::vector<Material*> &
FEProblem::getFaceMaterials(SubdomainID block_id, THREAD_ID tid)
{
  mooseAssert( tid < _materials.size(), "Requesting a material warehouse that does not exist");
  return _materials[tid].getBoundaryMaterials(block_id);
}

void
FEProblem::updateMaterials()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    _materials[tid].updateMaterialDataState();

  if (_material_props.hasStatefulProperties())
  {
    _material_props.shift();
    _bnd_material_props.shift();
  }
}

void
FEProblem::reinitMaterials(SubdomainID blk_id, THREAD_ID tid)
{
  if (_materials[tid].hasMaterials(blk_id))
  {
    const Elem * & elem = _assembly[tid]->elem();
    _material_data[tid]->reinit(_materials[tid].getMaterials(blk_id), _assembly[tid]->qRule()->n_points(), *elem, 0);
  }
}

void
FEProblem::reinitMaterialsFace(SubdomainID blk_id, unsigned int side, THREAD_ID tid)
{
  if (_materials[tid].hasBoundaryMaterials(blk_id))
  {
    const Elem * & elem = _assembly[tid]->elem();
    _bnd_material_data[tid]->reinit(_materials[tid].getBoundaryMaterials(blk_id), _assembly[tid]->qRuleFace()->n_points(), *elem, side);
  }
}

void
FEProblem::reinitMaterialsNeighbor(SubdomainID blk_id, unsigned int /*side*/, THREAD_ID tid)
{
  if (_materials[tid].hasNeighborMaterials(blk_id))
  {
    // NOTE: this will not work with h-adaptivity
    const Elem * & neighbor = _assembly[tid]->neighbor();
    unsigned int neighbor_side = neighbor->which_neighbor_am_i(_assembly[tid]->elem());
    _neighbor_material_data[tid]->reinit(_materials[tid].getNeighborMaterials(blk_id), _assembly[tid]->qRuleFace()->n_points(), *neighbor, neighbor_side);
  }
}

void
FEProblem::addPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters, ExecFlagType type/* = EXEC_TIMESTEP*/)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
  }

  // Parameter 'execute_on' needs to override the 'type' arg
  // TODO: remove this when we get rid of residual/jacobian sub-block in the input file
  if (parameters.wasSeenInInput("execute_on"))
    type = Moose::stringToEnum<ExecFlagType>(parameters.get<std::string>("execute_on"));

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    // distinguish between side and the rest of PPs to provide the right material object
    if(parameters.have_parameter<std::vector<BoundaryName> >("boundary"))
    {
      if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
        _reinit_displaced_face = true;

      parameters.set<MaterialData *>("_material_data") = _bnd_material_data[tid];
//      if (!_pps_data[tid].hasPostprocessor(name))
//      {
        Postprocessor * pp = static_cast<Postprocessor *>(Factory::instance()->create(pp_name, name, parameters));
        _pps(type)[tid].addPostprocessor(pp);
//      }
//      else
//        mooseError("Duplicate postprocessor name '" + name + "'");
    }
    else
    {
      if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
        _reinit_displaced_elem = true;

      parameters.set<MaterialData *>("_material_data") = _material_data[tid];
//      if (!_pps_data[tid].hasPostprocessor(name))
//      {
        Postprocessor * pp = static_cast<Postprocessor *>(Factory::instance()->create(pp_name, name, parameters));
        _pps(type)[tid].addPostprocessor(pp);
//      }
//      else
//        mooseError("Duplicate postprocessor name '" + name + "'");
    }
  }
}

Real &
FEProblem::getPostprocessorValue(const std::string & name, THREAD_ID tid)
{
  return _pps_data[tid].getPostprocessorValue(name);
}

void
FEProblem::computePostprocessorsInternal(std::vector<PostprocessorWarehouse> & pps)
{
  if (pps[0].blocks().size() > 0 || pps[0].boundaryIds().size() > 0 || pps[0].nodesetIds().size() > 0)
  {

    /* Note: The fact that we compute the aux system when we compute the postprocessors
     * is a very bad behavior that some of our applications have come to rely on.  This
     * needs to be fixed.  For now we cannot easily change this behavior without
     * affecting a number of applications.  However when I added the nodal postprocessors
     * this also changed the behavior so this hack is here to maintain saneness for now
     */
    if (!pps[0].nodesetIds().size())
    {
      serializeSolution();

      if (_displaced_problem != NULL)
        _displaced_problem->updateMesh(*_nl.currentSolution(), *_aux.currentSolution());

      _aux.compute();
    }

    // init
    bool have_nodal_pps = false;
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      for (std::set<SubdomainID>::const_iterator block_it = pps[tid].blocks().begin();
          block_it != pps[tid].blocks().end();
          ++block_it)
      {
        SubdomainID block_id = *block_it;

        for (std::vector<Postprocessor *>::const_iterator postprocessor_it = pps[tid].elementPostprocessors(block_id).begin();
            postprocessor_it != pps[tid].elementPostprocessors(block_id).end();
            ++postprocessor_it)
          (*postprocessor_it)->initialize();
      }

      for (std::set<BoundaryID>::const_iterator boundary_it = pps[tid].boundaryIds().begin();
          boundary_it != pps[tid].boundaryIds().end();
          ++boundary_it)
      {
        //note: for threaded applications where the elements get broken up it
        //may be more efficient to initialize these on demand inside the loop
        for (std::vector<Postprocessor *>::const_iterator side_postprocessor_it = pps[tid].sidePostprocessors(*boundary_it).begin();
            side_postprocessor_it != pps[tid].sidePostprocessors(*boundary_it).end();
            ++side_postprocessor_it)
          (*side_postprocessor_it)->initialize();
      }

      for (std::set<BoundaryID>::const_iterator boundary_it = pps[tid].nodesetIds().begin();
          boundary_it != pps[tid].nodesetIds().end();
          ++boundary_it)
      {
        for (std::vector<Postprocessor *>::const_iterator nodal_postprocessor_it = pps[tid].nodalPostprocessors(*boundary_it).begin();
             nodal_postprocessor_it != pps[tid].nodalPostprocessors(*boundary_it).end();
             ++nodal_postprocessor_it)
        {
          (*nodal_postprocessor_it)->initialize();
          have_nodal_pps = true;
        }
      }
    }

    // compute
    ComputePostprocessorsThread cppt(*this, getNonlinearSystem(), *getNonlinearSystem().currentSolution(), pps);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cppt);

    // Store element postprocessors values
    std::set<Postprocessor *> already_gathered;
    for (std::set<SubdomainID>::const_iterator block_ids_it = pps[0].blocks().begin();
        block_ids_it != pps[0].blocks().end();
        ++block_ids_it)
    {
      SubdomainID block_id = *block_ids_it;

      const std::vector<Postprocessor *> & element_postprocessors = pps[0].elementPostprocessors(block_id);
      // Store element postprocessors values
      for (unsigned int i = 0; i < element_postprocessors.size(); ++i)
      {
        Postprocessor *ps = element_postprocessors[i];
        std::string name = ps->name();

        // join across the threads (gather the value in thread #0)
        if (already_gathered.find(ps) == already_gathered.end())
        {
          for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
            ps->threadJoin(*pps[tid].elementPostprocessors(block_id)[i]);

          Real value = ps->getValue();
          // store the value in each thread

          for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
            _pps_data[tid].storeValue(name, value);

          already_gathered.insert(ps);
        }
      }
    }

    // Store side postprocessors values
    already_gathered.clear();
    for (std::set<BoundaryID>::const_iterator boundary_ids_it = pps[0].boundaryIds().begin();
         boundary_ids_it != pps[0].boundaryIds().end();
         ++boundary_ids_it)
    {
      BoundaryID boundary_id = *boundary_ids_it;

      const std::vector<Postprocessor *> & side_postprocessors = pps[0].sidePostprocessors(boundary_id);
      for (unsigned int i = 0; i < side_postprocessors.size(); ++i)
      {
        Postprocessor *ps = side_postprocessors[i];
        std::string name = ps->name();

        // join across the threads (gather the value in thread #0)
        if (already_gathered.find(ps) == already_gathered.end())
        {
          for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
            ps->threadJoin(*pps[tid].sidePostprocessors(boundary_id)[i]);

          Real value = ps->getValue();

          // store the value in each thread
          for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
            _pps_data[tid].storeValue(name, value);

          already_gathered.insert(ps);
        }
      }
    }

    // Don't waste time looping over nodes if there aren't any nodal postprocessors to calculate
    if (have_nodal_pps)
    {
      ComputeNodalPPSThread cnppt(*this, pps);
      Threads::parallel_reduce(*_mesh.getLocalNodeRange(), cnppt);

      // Store nodal postprocessors values
      already_gathered.clear();
      for (std::set<BoundaryID>::const_iterator boundary_ids_it = pps[0].nodesetIds().begin();
           boundary_ids_it != pps[0].nodesetIds().end();
           ++boundary_ids_it)
      {
        BoundaryID boundary_id = *boundary_ids_it;

        const std::vector<Postprocessor *> & nodal_postprocessors = pps[0].nodalPostprocessors(boundary_id);
        for (unsigned int i = 0; i < nodal_postprocessors.size(); ++i)
        {
          Postprocessor *ps = nodal_postprocessors[i];
          std::string name = ps->name();

          // join across the threads (gather the value in thread #0)
          if (already_gathered.find(ps) == already_gathered.end())
          {
            for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
              ps->threadJoin(*pps[tid].nodalPostprocessors(boundary_id)[i]);

            Real value = ps->getValue();
            // store the value in each thread

            for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
              _pps_data[tid].storeValue(name, value);

            already_gathered.insert(ps);
          }
        }
      }
    }
  }

  // Compute and store generic postprocessors values
  for (std::vector<Postprocessor *>::const_iterator generic_postprocessor_it = pps[0].genericPostprocessors().begin();
      generic_postprocessor_it != pps[0].genericPostprocessors().end();
      ++generic_postprocessor_it)
  {
    std::string name = (*generic_postprocessor_it)->name();
    (*generic_postprocessor_it)->initialize();
    (*generic_postprocessor_it)->execute();
    Real value = (*generic_postprocessor_it)->getValue();

    // store the value in each thread
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
      _pps_data[tid].storeValue(name, value);
  }
}

void
FEProblem::computePostprocessors(ExecFlagType type/* = EXEC_TIMESTEP*/)
{
  Moose::perf_log.push("compute_postprocessors()","Solve");

  switch (type)
  {
  case EXEC_RESIDUAL:
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
      _pps(type)[tid].residualSetup();
    break;

  case EXEC_JACOBIAN:
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
      _pps(type)[tid].jacobianSetup();
    break;

  case EXEC_TIMESTEP:
  case EXEC_TIMESTEP_BEGIN:
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
      _pps(type)[tid].timestepSetup();
    break;

  case EXEC_INITIAL:
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
      _pps(type)[tid].initialSetup();
    break;
  }
  computePostprocessorsInternal(_pps(type));

  Moose::perf_log.pop("compute_postprocessors()","Solve");
}

void
FEProblem::addPPSValuesToTable(ExecFlagType type)
{
  // Store values into table
  for (std::vector<Postprocessor *>::const_iterator postprocessor_it = _pps(type)[0].all().begin();
      postprocessor_it != _pps(type)[0].all().end();
      ++postprocessor_it)
  {
    Postprocessor *pps = *postprocessor_it;

    Moose::PPSOutputType out_type = pps->getOutput();
    if (out_type != Moose::PPS_OUTPUT_NONE)
    {
      std::string name = pps->name();
      Real value = _pps_data[0].getPostprocessorValue(name);
      switch (out_type)
      {
      case Moose::PPS_OUTPUT_FILE:
        _pps_output_table_file.addData(name, value, _time);
        break;

      case Moose::PPS_OUTPUT_SCREEN:
        _pps_output_table_screen.addData(name, value, _time);
        break;

      case Moose::PPS_OUTPUT_BOTH:
        _pps_output_table_file.addData(name, value, _time);
        _pps_output_table_screen.addData(name, value, _time);
        break;

      default:
        break;
      }
    }
  }
}

void
FEProblem::outputPostprocessors(bool force/* = false*/)
{
  ExecFlagType types[] = { EXEC_TIMESTEP, EXEC_TIMESTEP_BEGIN, EXEC_INITIAL, EXEC_JACOBIAN, EXEC_RESIDUAL };
  for (unsigned int i = 0; i < LENGTHOF(types); i++)
    addPPSValuesToTable(types[i]);

  if (!_pps_output_table_screen.empty())
  {
    if (force || (_postprocessor_screen_output && (_t_step % out().screen_interval() == 0)))
    {
      std::cout<<std::endl<<"Postprocessor Values:"<<std::endl;
      _pps_output_table_screen.printTable(std::cout, _pps_output_table_max_rows);
      std::cout<<std::endl;
    }
  }

  if (!_pps_output_table_file.empty())
  {
    if (force || (_t_step % out().interval() == 0))
    {
      // FIXME: if exodus output is enabled?
      _out.outputPps(_pps_output_table_file);
      if (_displaced_problem)
        _displaced_problem->outputPps(_pps_output_table_file);
      if (_out_problem)
        _out_problem->outputPps(_pps_output_table_file);

      if (_postprocessor_csv_output)
        _pps_output_table_file.printCSV(_out.fileBase() + ".csv", out().screen_interval());

      if (_postprocessor_gnuplot_output)
        _pps_output_table_file.makeGnuplot(_out.fileBase(), _gnuplot_format);
    }
  }
}

void
FEProblem::addDamper(std::string damper_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;
  parameters.set<SystemBase *>("_sys") = &_nl;

  _has_dampers = true;
  _nl.addDamper(damper_name, name, parameters);
}

void
FEProblem::setupDampers()
{
  _nl.setupDampers();
}

bool
FEProblem::hasVariable(const std::string & var_name)
{
  if (_nl.hasVariable(var_name))
    return true;
  else if (_aux.hasVariable(var_name))
    return true;
  else
    return false;
}

MooseVariable &
FEProblem::getVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_nl.hasVariable(var_name))
    return _nl.getVariable(tid, var_name);
  else if (!_aux.hasVariable(var_name))
    mooseError("Unknown variable " + var_name);

  return _aux.getVariable(tid, var_name);
}

bool
FEProblem::hasScalarVariable(const std::string & var_name)
{
  if (_nl.hasScalarVariable(var_name))
    return true;
  else if (_aux.hasScalarVariable(var_name))
    return true;
  else
    return false;
}

MooseVariableScalar &
FEProblem::getScalarVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_nl.hasScalarVariable(var_name))
    return _nl.getScalarVariable(tid, var_name);
  else if (_aux.hasScalarVariable(var_name))
    return _aux.getScalarVariable(tid, var_name);
  else
    mooseError("Unknown variable " + var_name);
}

void
FEProblem::createQRules(QuadratureType type, Order order)
{
  if (order == INVALID_ORDER)
  {
    // automatically determine the integration order
    Moose::setup_perf_log.push("getMinQuadratureOrder()","Setup");
    _quadrature_order = _nl.getMinQuadratureOrder();
    Moose::setup_perf_log.pop("getMinQuadratureOrder()","Setup");
  }
  else
    _quadrature_order = order;

  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    _assembly[tid]->createQRules(type, _quadrature_order);

  if (_displaced_problem)
    _displaced_problem->createQRules(type, _quadrature_order);
}

void
FEProblem::setCoupling(Moose::CouplingType type)
{
  _coupling = type;
}

void
FEProblem::setCouplingMatrix(CouplingMatrix * cm)
{
  _coupling = Moose::COUPLING_CUSTOM;
  delete _cm;
  _cm = cm;
}

void
FEProblem::useFECache(bool fe_cache)
{
  if(fe_cache)
    std::cout<<std::endl<<"Utilizing FE Shape Function Caching"<<std::endl<<std::endl;

  unsigned int n_threads = libMesh::n_threads();

  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i]->useFECache(fe_cache); //fe_cache);
}

void
FEProblem::init()
{
  unsigned int n_vars = _nl.nVariables();
  unsigned int n_scalar_vars = _nl.nScalarVariables();
  switch (_coupling)
  {
  case Moose::COUPLING_DIAG:
    _cm = new CouplingMatrix(n_vars + n_scalar_vars);
    for (unsigned int i = 0; i < n_vars + n_scalar_vars; i++)
      for (unsigned int j = 0; j < n_vars + n_scalar_vars; j++)
        (*_cm)(i, j) = (i == j ? 1 : 0);
    break;

  // for full jacobian
  case Moose::COUPLING_FULL:
    _cm = new CouplingMatrix(n_vars + n_scalar_vars);
    for (unsigned int i = 0; i < n_vars + n_scalar_vars; i++)
      for (unsigned int j = 0; j < n_vars + n_scalar_vars; j++)
        (*_cm)(i, j) = 1;
    break;

  case Moose::COUPLING_CUSTOM:
    // do nothing, _cm was already set through couplingMatrix() call
    break;
  }

  _nl.dofMap()._dof_coupling = _cm;
  _nl.dofMap().attach_extra_sparsity_function(&extraSparsity, &_nl);


  Moose::setup_perf_log.push("eq.init()","Setup");
  SubProblem::init();
  Moose::setup_perf_log.pop("eq.init()","Setup");

  Moose::setup_perf_log.push("mesh.applyMeshModifications()","Setup");
  _mesh.applyMeshModifications();
  Moose::setup_perf_log.pop("mesh.applyMeshModifications()","Setup");

  Moose::setup_perf_log.push("FEProblem::init::meshChanged()","Setup");
  _mesh.meshChanged();
  Moose::setup_perf_log.pop("FEProblem::init::meshChanged()","Setup");

  init2();

  _out.init();
}

void
FEProblem::init2()
{
  Moose::setup_perf_log.push("NonlinearSystem::update()","Setup");
  _nl.update();
  Moose::setup_perf_log.pop("NonlinearSystem::update()","Setup");

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    _assembly[tid]->init();

  _nl.init();

  if (_displaced_problem)
    _displaced_problem->init();

  _aux.init();
}

void
FEProblem::solve()
{
  Moose::setSolverDefaults(*this);
  Moose::perf_log.push("solve()","Solve");
//  _solve_only_perf_log.push("solve");
  _nl.solve();
//  _solve_only_perf_log.pop("solve");
  Moose::perf_log.pop("solve()","Solve");
  _nl.update();

  // sync solutions in displaced problem
  if (_displaced_problem)
    _displaced_problem->syncSolutions(*_nl.currentSolution(), *_aux.currentSolution());
}

bool
FEProblem::converged()
{
  return _nl.converged();
}

void
FEProblem::copySolutionsBackwards()
{
  _nl.copySolutionsBackwards();
  _aux.copySolutionsBackwards();
}

void
FEProblem::copyOldSolutions()
{
  _nl.copyOldSolutions();
  _aux.copyOldSolutions();
}

void FEProblem::restoreSolutions()
{
  _nl.restoreSolutions();
  _aux.restoreSolutions();

  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh(*_nl.currentSolution(), *_aux.currentSolution());
}

void
FEProblem::onTimestepBegin()
{
  _nl.onTimestepBegin();
}

void
FEProblem::onTimestepEnd()
{
  _aux.compute(EXEC_TIMESTEP);
  _nl.printVarNorms();
}

void
FEProblem::computeAuxiliaryKernels(ExecFlagType type)
{
  _aux.compute(type);
}

void
FEProblem::computeResidual(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number>& soln, NumericVector<Number>& residual)
{
  _nl.set_solution(soln);
  computePostprocessors(EXEC_RESIDUAL);

  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh(soln, *_aux.currentSolution());

  unsigned int n_threads = libMesh::n_threads();

  for(unsigned int i=0; i<n_threads; i++)
  {
    _materials[i].residualSetup();
  }
  _aux.residualSetup();

  _aux.compute();
  _nl.computeResidual(residual);

  if (_dbg_top_residuals)
    _nl.printTopResiduals(residual, _dbg_top_residuals);
}

void
FEProblem::computeJacobian(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian)
{
  _nl.set_solution(soln);
  computePostprocessors(EXEC_JACOBIAN);

  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh(soln, *_aux.currentSolution());

  unsigned int n_threads = libMesh::n_threads();

  for(unsigned int i=0; i<n_threads; i++)
  {
    _materials[i].jacobianSetup();
  }

  _aux.jacobianSetup();
  _aux.compute();
  _nl.computeJacobian(jacobian);
}

void
FEProblem::computeJacobianBlock(SparseMatrix<Number> & jacobian, libMesh::System & precond_system, unsigned int ivar, unsigned int jvar)
{
  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh(*_nl.currentSolution(), *_aux.currentSolution());

  _aux.compute();
  _nl.computeJacobianBlock(jacobian, precond_system, ivar, jvar);
}

void
FEProblem::computeBounds(NonlinearImplicitSystem & /*sys*/, NumericVector<Number>& lower, NumericVector<Number>& upper)
{
  NumericVector<Number>& _lower = _nl.getVector("lower_bound");
  NumericVector<Number>& _upper = _nl.getVector("upper_bound");
  _lower.swap(lower);
  _upper.swap(upper);
  unsigned int n_threads = libMesh::n_threads();
  for(unsigned int i=0; i<n_threads; i++)
  {
    _materials[i].residualSetup();
  }
  _aux.residualSetup();
  _aux.compute();
  _lower.swap(lower);
  _upper.swap(upper);
}

Real
FEProblem::computeDamping(const NumericVector<Number>& soln, const NumericVector<Number>& update)
{
  Moose::perf_log.push("compute_dampers()","Solve");

  // Default to no damping
  Real damping = 1.0;

  if (_has_dampers)
  {
    _nl.set_solution(soln);
    _aux.compute();
    damping = _nl.computeDamping(update);
  }

  Moose::perf_log.pop("compute_dampers()","Solve");

  return damping;
}

void
FEProblem::initDisplacedProblem(MooseMesh * displaced_mesh, InputParameters params)
{
  _displaced_mesh = displaced_mesh;

  Moose::setup_perf_log.push("Create DisplacedProblem","Setup");
  params += parameters();
  _displaced_problem = new DisplacedProblem(*this, *_displaced_mesh, params);
  Moose::setup_perf_log.pop("Create DisplacedProblem","Setup");
}

void
FEProblem::updateGeomSearch()
{
  _geometric_search_data.update();

  if(_displaced_problem)
    _displaced_problem->updateGeomSearch();
}

void
FEProblem::output(bool force/*= false*/)
{
  if ((_t_step % out().interval() == 0) || force)
  {
    _out.output();

    // if the OverSample problem is setup, output it's solution
    if (_out_problem)
    {
      _out_problem->init();
      _out_problem->output(force);
    }

    if(_output_solution_history)
      _out.outputSolutionHistory();

    if (_displaced_problem != NULL && _output_displaced)
      _displaced_problem->output();

    // save the input file if we did not do so already
    if (!_input_file_saved)
    {
      _out.outputInput();
      if (_out_problem)
        _out_problem->outputInput();
      _input_file_saved = true;
    }
  }

  _resurrector.write();
}

void
FEProblem::setOutputVariables(std::vector<std::string> output_variables)
{
  _out.setOutputVariables(output_variables);
  if(_displaced_problem)
    _displaced_problem->setOutputVariables(output_variables);
}

OutputProblem &
FEProblem::getOutputProblem(unsigned int refinements)
{
  // TODO: When do we build this?
  if (!_out_problem)
  {
    InputParameters params = validParams<OutputProblem>();
    params.set<FEProblem *>("mproblem") = this;
    params.set<unsigned int>("refinements") = refinements;
    params.set<MooseMesh *>("mesh") = &_mesh;
    _out_problem = static_cast<OutputProblem *>(ProblemFactory::instance()->create("OutputProblem", "Output Problem", params));
  }
  return *_out_problem;
}

#ifdef LIBMESH_ENABLE_AMR
void
FEProblem::adaptMesh()
{
  unsigned int cycles_per_step = _adaptivity.getCyclesPerStep();
  for (unsigned int i=0; i < cycles_per_step; ++i)
  {
    _adaptivity.adaptMesh();
    meshChanged();
  }
}
#endif //LIBMESH_ENABLE_AMR

void
FEProblem::meshChanged()
{
  // mesh changed
  _eq.reinit();
  _mesh.meshChanged();

  unsigned int n_threads = libMesh::n_threads();

  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i]->invalidateCache();

  _geometric_search_data.update();
  _mesh.updateActiveSemiLocalNodeRange(_ghosted_elems);

  if(_displaced_problem != NULL)
  {
    _displaced_problem->meshChanged();
    _displaced_mesh->updateActiveSemiLocalNodeRange(_ghosted_elems);
  }
}

void
FEProblem::checkProblemIntegrity()
{
  // Check for unsatisfied actions
  const std::set<SubdomainID> & mesh_subdomains = _mesh.meshSubdomains();
  Moose::action_warehouse.checkUnsatisfiedActions();

  // Check kernel coverage of subdomains (blocks) in the mesh
  _nl.checkKernelCoverage(mesh_subdomains);

  // Check materials
  {
#ifdef LIBMESH_ENABLE_AMR
    if (_material_props.hasStatefulProperties() && _adaptivity.isOn())
      mooseError("Cannot use Material classes with stateful properties while utilizing adaptivity!");
#endif

    std::set<SubdomainID> local_mesh_subs(mesh_subdomains);
    /**
     * If a material is specified for any block in the simulation, then all blocks must
     * have a material specified.
     */
    bool check_material_coverage = false;
    for (std::set<SubdomainID>::const_iterator i = _materials[0].blocks().begin(); i != _materials[0].blocks().end(); ++i)
    {
      if (mesh_subdomains.find(*i) == mesh_subdomains.end())
      {
        std::stringstream oss;
        oss << "Material block \"" << *i << "\" specified in the input file does not exist";
        mooseError (oss.str());
      }

      local_mesh_subs.erase(*i);
      check_material_coverage = true;
    }

    // Check Material Coverage
    if (check_material_coverage && !local_mesh_subs.empty())
    {
      std::stringstream extra_subdomain_ids;
      /// <unsigned int> is necessary to print SubdomainIDs in the statement below
      std::copy (local_mesh_subs.begin(), local_mesh_subs.end(), std::ostream_iterator<unsigned int>(extra_subdomain_ids, " "));

      mooseError("The following blocks from your input mesh do not contain on active material: " + extra_subdomain_ids.str() + "\nWhen ANY mesh block contains a Material object, all blocks must contain a Material object.\n");
    }
  }

  // Check that BCs used in your simulation exist in your mesh
  _nl.checkBCCoverage();

  checkPPSs();
}

void
FEProblem::checkPPSs()
{
  // Check pps block coverage
  std::set<SubdomainID> mesh_subdomains = _mesh.meshSubdomains();
  std::set<SubdomainID> pps_blocks;

  // gather names of all postprocessors that were defined in the input file
  // and the blocks that they are defined on
  std::set<std::string> names;
  ExecFlagType types[] = { EXEC_INITIAL, EXEC_RESIDUAL, EXEC_JACOBIAN, EXEC_TIMESTEP, EXEC_TIMESTEP_BEGIN };
  for (unsigned int i = 0; i < LENGTHOF(types); i++)
  {
    for (std::vector<Postprocessor *>::const_iterator it = _pps(types[i])[0].all().begin(); it != _pps(types[i])[0].all().end(); ++it)
      names.insert((*it)->name());

    pps_blocks.insert(_pps(types[i])[0].blocks().begin(), _pps(types[i])[0].blocks().end());
  }

  // See if all referenced blocks are covered
  mesh_subdomains.insert(Moose::ANY_BLOCK_ID);
  std::set<SubdomainID> difference;
  std::set_difference(pps_blocks.begin(), pps_blocks.end(), mesh_subdomains.begin(), mesh_subdomains.end(),
                      std::inserter(difference, difference.end()));

  if (!difference.empty())
  {
    std::ostringstream oss;
    oss << "One or more Postprocessors is referencing a nonexistent block:\n";
    for (std::set<SubdomainID>::iterator i = difference.begin(); i != difference.end();  ++i)
      oss << *i << "\n";
    mooseError(oss.str());
  }

  // check that all requested PPS were defined in the input file
  for (std::map<std::string, PostprocessorValue>::const_iterator it = _pps_data[0].values().begin(); it != _pps_data[0].values().end(); ++it)
  {
    if (names.find(it->first) == names.end())
      mooseError("Postprocessor '" + it->first + "' requested but not specified in the input file.");
  }
}

void
FEProblem::serializeSolution()
{
  _nl.serializeSolution();
  _aux.serializeSolution();
}

void
FEProblem::setRestartFile(const std::string & file_name)
{
  _resurrector.setRestartFile(file_name);
}

void
FEProblem::setNumRestartFiles(unsigned int num_files)
{
  _resurrector.setNumRestartFiles(num_files);
}


std::vector<std::string>
FEProblem::getVariableNames()
{
  std::vector<std::string> names;

  std::vector<std::string> & nl_var_names = _nl.getVariableNames();
  names.insert(names.end(), nl_var_names.begin(), nl_var_names.end());

  std::vector<std::string> & aux_var_names = _aux.getVariableNames();
  names.insert(names.end(), aux_var_names.begin(), aux_var_names.end());

  return names;
}
