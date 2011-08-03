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

#include "MProblem.h"
#include "Factory.h"
#include "DisplacedProblem.h"
#include "MaterialData.h"
#include "ComputePostprocessorsThread.h"
#include "ActionWarehouse.h"
#include "Conversion.h"

#include "ElementH1Error.h"

unsigned int MProblem::_n = 0;

namespace Moose
{

Number initial_value (const Point & p,
                      const Parameters & parameters,
                      const std::string & sys_name,
                      const std::string & var_name)
{
  Problem * problem = parameters.get<Problem *>("_problem");
  mooseAssert(problem != NULL, "Internal pointer to _problem was not set");
  return problem->initialValue(p, parameters, sys_name, var_name);
}

Gradient initial_gradient (const Point & p,
                           const Parameters & parameters,
                           const std::string & sys_name,
                           const std::string & var_name)
{
  Problem * problem = parameters.get<Problem *>("_problem");
  mooseAssert(problem != NULL, "Internal pointer to _problem was not set");
  return problem->initialGradient(p, parameters, sys_name, var_name);
}

void initial_condition(EquationSystems & es, const std::string & system_name)
{
  Problem * problem = es.parameters.get<Problem *>("_problem");
  mooseAssert(problem != NULL, "Internal pointer to MooseSystem was not set");
  problem->initialCondition(es, system_name);
}

} // namespace Moose


static
std::string name(const std::string & name, unsigned int n)
{
  std::ostringstream os;
  os << name << n;
  return os.str();
}

MProblem::MProblem(MooseMesh & mesh, Problem * parent/* = NULL*/) :
    SubProblem(mesh, parent),
    _nl(*this, name("nl", _n)),
    _aux(*this, name("aux", _n)),
    _quadrature_order(CONSTANT),
    _postprocessor_screen_output(true),
    _postprocessor_csv_output(false),
    _postprocessor_ensight_output(false),
    _postprocessor_gnuplot_output(false),
    _gnuplot_format("ps"),
    _out(*this),
#ifdef LIBMESH_ENABLE_AMR
    _adaptivity(*this),
#endif
    _displaced_mesh(NULL),
    _displaced_problem(NULL),
    _geometric_search_data(*this, _mesh),
    _reinit_displaced_elem(false),
    _reinit_displaced_face(false),
    _output_displaced(false),
    _input_file_saved(false),
    _has_dampers(false),
    _restart(false),
    _solve_only_perf_log("Solve Only"),
    _output_setup_log_early(false),
    // debugging
    _dbg_top_residuals(0)
{
  _n++;

  unsigned int n_threads = libMesh::n_threads();

  _asm_info.resize(n_threads);
  for (unsigned int i = 0; i < n_threads; ++i)
    _asm_info[i] = new AssemblyData(_mesh);

  _functions.resize(n_threads);
  _materials.resize(n_threads);

  _material_data.resize(n_threads);
  _bnd_material_data.resize(n_threads);
  for (unsigned int i = 0; i < n_threads; i++)
  {
    _material_data[i] = new MaterialData(_material_props);
    _bnd_material_data[i] = new MaterialData(_bnd_material_props);
  }

  _pps_data.resize(n_threads);
}

MProblem::~MProblem()
{
  bool stateful_props = _material_props.hasStatefulProperties();

  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; i++)
  {
    delete _asm_info[i];

    if (!stateful_props)
    {
      delete _material_data[i];
      delete _bnd_material_data[i];
    }
  }

  if (stateful_props)
  {
    _material_props.releaseProperties();
    _bnd_material_props.releaseProperties();
  }

  // ICS
  for (std::map<std::string, InitialCondition *>::iterator it = _ics.begin(); it != _ics.end(); ++it)
    delete it->second;

  delete _displaced_mesh;
  delete _displaced_problem;
}


void MProblem::initialSetup()
{
  if (_restart)
    restartFromFile();

  unsigned int n_threads = libMesh::n_threads();

  Moose::setup_perf_log.push("copySolutionsBackwards()","Setup");
  copySolutionsBackwards();
  Moose::setup_perf_log.pop("copySolutionsBackwards()","Setup");

  if (_material_props.hasStatefulProperties())
  {
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    for (ConstElemRange::const_iterator el = elem_range.begin() ; el != elem_range.end(); ++el)
    {
      const Elem * elem = *el;
      subdomain_id_type blk_id = elem->subdomain_id();

      mooseAssert(_materials[0].hasMaterials(blk_id), "No materials on subdomain block " + elem->id());
      _asm_info[0]->reinit(elem);
      unsigned int n_points = _asm_info[0]->qRule()->n_points();
      _material_data[0]->initStatefulProps(_materials[0].getMaterials(blk_id), n_points, *elem);

      for (unsigned int side=0; side<elem->n_sides(); side++)
      {
        mooseAssert(_materials[0].hasBoundaryMaterials(blk_id), "No face materials on subdomain block " + elem->id());
        _asm_info[0]->reinit(elem, side);
        unsigned int n_points = _asm_info[0]->qRuleFace()->n_points();
        _bnd_material_data[0]->initStatefulProps(_materials[0].getBoundaryMaterials(blk_id), n_points, *elem, side);
      }
    }
  }
  else
  {
  }

  // RUN initial postprocessors
  computePostprocessors(EXEC_INITIAL);

#ifdef LIBMESH_ENABLE_AMR
  Moose::setup_perf_log.push("adaptivity().initial()","Setup");
  adaptivity().initial();
  Moose::setup_perf_log.pop("adaptivity().initial()","Setup");
#endif //LIBMESH_ENABLE_AMR

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
  }

  _aux.initialSetup();

  Moose::setup_perf_log.push("Initial computePostprocessors()","Setup");
  computePostprocessors();
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

void MProblem::timestepSetup()
{
  unsigned int n_threads = libMesh::n_threads();

  for(unsigned int i=0; i<n_threads; i++)
  {
    _materials[i].timestepSetup();
  }

  _aux.timestepSetup();
  _nl.timestepSetup();
  _out.timestepSetup();
}

void
MProblem::prepare(const Elem * elem, THREAD_ID tid)
{
  _asm_info[tid]->reinit(elem);

  _nl.prepare(tid);
  _aux.prepare(tid);
  _nl.prepareAssembly(tid);

  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->prepare(_displaced_mesh->elem(elem->id()), tid);
}

void
MProblem::prepare(const Elem * elem, unsigned int ivar, unsigned int jvar, const std::vector<unsigned int> & dof_indices, THREAD_ID tid)
{
  _asm_info[tid]->reinit(elem);

  _nl.prepare(tid);
  _aux.prepare(tid);
  _nl.prepareAssembly(ivar, jvar, dof_indices, tid);

  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->prepare(_displaced_mesh->elem(elem->id()), ivar, jvar, dof_indices, tid);
}

void
MProblem::addResidual(NumericVector<Number> & residual, THREAD_ID tid)
{
  _nl.addResidual(residual, tid);
  if(_displaced_problem)
    _displaced_problem->addResidual(residual, tid);
}

void
MProblem::addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _nl.addJacobian(jacobian, tid);
  if(_displaced_problem)
    _displaced_problem->addJacobian(jacobian, tid);
}

void
MProblem::addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, THREAD_ID tid)
{
  _nl.addJacobianBlock(jacobian, ivar, jvar, dof_map, dof_indices, tid);
  if(_displaced_problem)
    _displaced_problem->addJacobianBlock(jacobian, ivar, jvar, dof_map, dof_indices, tid);
}

void
MProblem::prepareShapes(unsigned int var, THREAD_ID tid)
{
  _nl.asmBlock(tid).copyShapes(var);
}

void
MProblem::prepareFaceShapes(unsigned int var, THREAD_ID tid)
{
  _nl.asmBlock(tid).copyFaceShapes(var);
}

void
MProblem::addGhostedElem(unsigned int elem_id)
{
  if(_mesh.elem(elem_id)->processor_id() != libMesh::processor_id())
    _ghosted_elems.insert(elem_id);
}

void
MProblem::addGhostedBoundary(unsigned int boundary_id)
{
  _mesh.addGhostedBoundary(boundary_id);

  if(_displaced_problem)
    _displaced_mesh->addGhostedBoundary(boundary_id);
}

bool
MProblem::reinitDirac(const Elem * elem, THREAD_ID tid)
{
  std::set<Point> & points_set = _dirac_kernel_info._points[elem];

  bool have_points = points_set.size();

  if(have_points)
  {
    std::vector<Point> points(points_set.size());
    std::copy(points_set.begin(), points_set.end(), points.begin());

    _asm_info[tid]->reinitAtPhysical(elem, points);

    _nl.prepare(tid);
    _aux.prepare(tid);

    reinitElem(elem, tid);
  }
  _nl.prepareAssembly(tid);

  if (_displaced_problem != NULL && (_reinit_displaced_elem))
    have_points = have_points || _displaced_problem->reinitDirac(_displaced_mesh->elem(elem->id()), tid);

  return have_points;
}

void
MProblem::reinitElem(const Elem * elem, THREAD_ID tid)
{
  unsigned int n_points = _asm_info[tid]->qRule()->n_points();
  _zero[tid].resize(n_points, 0);
  _grad_zero[tid].resize(n_points, 0);
  _second_zero[tid].resize(n_points, 0);

  _nl.reinitElem(elem, tid);
  _aux.reinitElem(elem, tid);

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitElem(_displaced_mesh->elem(elem->id()), tid);
}

void
MProblem::reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid)
{
  _asm_info[tid]->reinit(elem, side);

  unsigned int n_points = _asm_info[tid]->qRule()->n_points();
  _zero[tid].resize(n_points, 0);
  _grad_zero[tid].resize(n_points, 0);
  _second_zero[tid].resize(n_points, 0);

  _nl.reinitElemFace(elem, side, bnd_id, tid);
  _aux.reinitElemFace(elem, side, bnd_id, tid);

  if (_displaced_problem != NULL && _reinit_displaced_face)
    _displaced_problem->reinitElemFace(_displaced_mesh->elem(elem->id()), side, bnd_id, tid);
}

void
MProblem::reinitNode(const Node * node, THREAD_ID tid)
{
  _asm_info[tid]->reinit(node);

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
MProblem::reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid)
{
  _asm_info[tid]->reinit(node);

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
MProblem::getDiracElements(std::set<const Elem *> & elems)
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
MProblem::clearDiracInfo()
{
  _dirac_kernel_info.clearPoints();

  if(_displaced_problem)
    _displaced_problem->clearDiracInfo();
}


void
MProblem::subdomainSetup(unsigned int subdomain, THREAD_ID tid)
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
MProblem::subdomainSetupSide(unsigned int subdomain, THREAD_ID tid)
{
  if (_materials[tid].hasBoundaryMaterials(subdomain))
  {
    // call subdomainSetup
    for (std::vector<Material *>::const_iterator it = _materials[tid].getBoundaryMaterials(subdomain).begin(); it != _materials[tid].getBoundaryMaterials(subdomain).end(); ++it)
      (*it)->subdomainSetup();

  }
}

void
MProblem::addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< subdomain_id_type > * const active_subdomains/* = NULL*/)
{
  _nl.addVariable(var_name, type, scale_factor, active_subdomains);
  if (_displaced_problem)
    _displaced_problem->addVariable(var_name, type, scale_factor, active_subdomains);
}

void
MProblem::addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblemInterface *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblemInterface *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addKernel(kernel_name, name, parameters);
}

void
MProblem::addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblemInterface *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    parameters.set<SubProblemInterface *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addBoundaryCondition(bc_name, name, parameters);
}

void
MProblem::addAuxVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains/* = NULL*/)
{
  _aux.addVariable(var_name, type, 1.0, active_subdomains);
  if (_displaced_problem)
    _displaced_problem->addAuxVariable(var_name, type, active_subdomains);
}

void
MProblem::addAuxKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblemInterface *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    parameters.set<SystemBase *>("_nl_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblemInterface *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
    parameters.set<SystemBase *>("_nl_sys") = &_nl;
  }
  _aux.addKernel(kernel_name, name, parameters);
}

void
MProblem::addAuxBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {

    parameters.set<SubProblemInterface *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    parameters.set<SystemBase *>("_nl_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    parameters.set<SubProblemInterface *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
    parameters.set<SystemBase *>("_nl_sys") = &_nl;
  }
  _aux.addBoundaryCondition(bc_name, name, parameters);
}

void
MProblem::addDiracKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblemInterface *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblemInterface *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addDiracKernel(kernel_name, name, parameters);
}

// Initial Conditions /////
void
MProblem::addInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters, std::string var_name)
{
  parameters.set<Problem *>("_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;
  parameters.set<std::string>("var_name") = var_name;
  _ics[var_name] = static_cast<InitialCondition *>(Factory::instance()->create(ic_name, name, parameters));
}

void
MProblem::addInitialCondition(const std::string & var_name, Real value)
{
  _pars.set<Real>("initial_" + var_name) = value;
}

Number
MProblem::initialValue (const Point& p,
                        const Parameters& /*parameters*/,
                        const std::string& /*sys_name*/,
                        const std::string& var_name)
{
//  ParallelUniqueId puid;
//  unsigned int tid = puid.id;

  // Try to grab an InitialCondition object for this variable.
  if (_ics.find(var_name) != _ics.end())
    return _ics[var_name]->value(p);

  if (_pars.have_parameter<Real>("initial_"+var_name))
    return _pars.get<Real>("initial_"+var_name);

  return 0;
}

Gradient
MProblem::initialGradient (const Point& p,
                           const Parameters& /*parameters*/,
                           const std::string& /*sys_name*/,
                           const std::string& var_name)
{
//  ParallelUniqueId puid;
//  unsigned int tid = puid.id;

  // Try to grab an InitialCondition object for this variable.
  if (_ics.find(var_name) != _ics.end())
    return _ics[var_name]->gradient(p);

  return RealGradient();
}

void
MProblem::initialCondition(EquationSystems& es, const std::string& system_name)
{
  if (!_restart)
  {
    // do not project initial condition if we are restarting from a file
    ExplicitSystem & system = es.get_system<ExplicitSystem>(system_name);
    system.project_solution(Moose::initial_value, Moose::initial_gradient, es.parameters);
  }
}

void
MProblem::addMaterial(const std::string & mat_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  parameters.set<SubProblemInterface *>("_subproblem") = this;
  parameters.set<SubProblemInterface *>("_subproblem_displaced") = _displaced_problem;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    std::vector<unsigned int> blocks = parameters.get<std::vector<unsigned int> >("block");
    for (unsigned int i=0; i<blocks.size(); ++i)
    {
      parameters.set<unsigned int>("block_id") = blocks[i];

      // volume material
      parameters.set<bool>("_bnd") = false;
      parameters.set<MaterialData *>("_material_data") = _material_data[tid];
      Material *material = static_cast<Material *>(Factory::instance()->create(mat_name, name, parameters));
      mooseAssert(material != NULL, "Not a Material object");
      _materials[tid].addMaterial(blocks[i], material);

      // boundary material
      parameters.set<bool>("_bnd") = true;
      parameters.set<MaterialData *>("_material_data") = _bnd_material_data[tid];
      Material *bnd_material = static_cast<Material *>(Factory::instance()->create(mat_name, name, parameters));
      mooseAssert(bnd_material != NULL, "Not a Material object");
      _materials[tid].addBoundaryMaterial(blocks[i], bnd_material);

//      _vars[tid].addBoundaryVars(blocks[i], bnd_material->getCoupledVars());
    }
  }
}

const std::vector<Material*> &
MProblem::getMaterials(unsigned int block_id, THREAD_ID tid)
{
  mooseAssert( tid < _materials.size(), "Requesting a material warehouse that does not exist");
  return _materials[tid].getMaterials(block_id);
}

const std::vector<Material*> &
MProblem::getFaceMaterials(unsigned int block_id, THREAD_ID tid)
{
  mooseAssert( tid < _materials.size(), "Requesting a material warehouse that does not exist");
  return _materials[tid].getBoundaryMaterials(block_id);
}

void
MProblem::updateMaterials()
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
MProblem::reinitMaterials(unsigned int blk_id, THREAD_ID tid)
{
  if (_materials[tid].hasMaterials(blk_id))
  {
    const Elem * & elem = _asm_info[tid]->elem();
    _material_data[tid]->reinit(_materials[tid].getMaterials(blk_id), _asm_info[tid]->qRule()->n_points(), *elem, 0);
  }
}

void
MProblem::reinitMaterialsFace(unsigned int blk_id, unsigned int side, THREAD_ID tid)
{
  if (_materials[tid].hasBoundaryMaterials(blk_id))
  {
    const Elem * & elem = _asm_info[tid]->elem();
    _bnd_material_data[tid]->reinit(_materials[tid].getBoundaryMaterials(blk_id), _asm_info[tid]->qRuleFace()->n_points(), *elem, side);
  }
}

void
MProblem::addPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters, ExecFlagType type/* = EXEC_TIMESTEP*/)
{
  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblemInterface *>("_subproblem") = _displaced_problem;
  }
  else
  {
    parameters.set<SubProblemInterface *>("_subproblem") = this;
  }

  // Parameter 'execute_on' needs to override the 'type' arg
  // TODO: remove this when we get rid of residual/jacobian sub-block in the input file
  if (parameters.wasSeenInInput("execute_on"))
    type = Moose::stringToEnum<ExecFlagType>(parameters.get<std::string>("execute_on"));

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    if(parameters.have_parameter<std::vector<unsigned int> >("boundary"))
    {
      if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
        _reinit_displaced_face = true;

      parameters.set<MaterialData *>("_material_data") = _bnd_material_data[tid];

      const std::vector<unsigned int> & boundaries = parameters.get<std::vector<unsigned int> >("boundary");

//      if (!_pps_data[tid].hasPostprocessor(name))
//      {
        for (unsigned int i=0; i<boundaries.size(); ++i)
        {
          parameters.set<unsigned int>("_boundary_id") = boundaries[i];
          Postprocessor * pp = static_cast<Postprocessor *>(Factory::instance()->create(pp_name, name, parameters));
          _pps(type)[tid].addPostprocessor(pp);
        }
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
MProblem::getPostprocessorValue(const std::string & name, THREAD_ID tid)
{
  return _pps_data[tid].getPostprocessorValue(name);
}

void
MProblem::computePostprocessorsInternal(std::vector<PostprocessorWarehouse> & pps)
{
  if (pps[0].blocks().size() > 0 || pps[0].boundaryIds().size() > 0)
  {
    serializeSolution();

    if (_displaced_problem != NULL)
      _displaced_problem->updateMesh(*_nl.currentSolution(), *_aux.currentSolution());

    _aux.compute();

    // init
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      for (std::set<unsigned int>::const_iterator block_it = pps[tid].blocks().begin();
          block_it != pps[tid].blocks().end();
          ++block_it)
      {
        unsigned int block_id = *block_it;

        for (std::vector<Postprocessor *>::const_iterator postprocessor_it = pps[tid].elementPostprocessors(block_id).begin();
            postprocessor_it != pps[tid].elementPostprocessors(block_id).end();
            ++postprocessor_it)
          (*postprocessor_it)->initialize();
      }

      for (std::set<unsigned int>::const_iterator boundary_it = pps[tid].boundaryIds().begin();
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
    }

    // compute
    ComputePostprocessorsThread cppt(*this, getNonlinearSystem(), *getNonlinearSystem().currentSolution(), pps);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cppt);

    // Store element postprocessors values
    for (std::set<unsigned int>::const_iterator block_ids_it = pps[0].blocks().begin();
        block_ids_it != pps[0].blocks().end();
        ++block_ids_it)
    {
      unsigned int block_id = *block_ids_it;

      const std::vector<Postprocessor *> & element_postprocessors = pps[0].elementPostprocessors(block_id);
      // Store element postprocessors values
      for (unsigned int i = 0; i < element_postprocessors.size(); ++i)
      {
        Postprocessor *ps = element_postprocessors[i];
        std::string name = ps->name();

        // join across the threads (gather the value in thread #0)
        for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
          ps->threadJoin(*pps[tid].elementPostprocessors(block_id)[i]);

        Real value = ps->getValue();
        // store the value in each thread
        for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
          _pps_data[tid].storeValue(name, value);
      }
    }

    // Store side postprocessors values
    for (std::set<unsigned int>::const_iterator boundary_ids_it = pps[0].boundaryIds().begin();
        boundary_ids_it != pps[0].boundaryIds().end();
        ++boundary_ids_it)
    {
      unsigned int boundary_id = *boundary_ids_it;

      const std::vector<Postprocessor *> & side_postprocessors = pps[0].sidePostprocessors(boundary_id);
      for (unsigned int i = 0; i < side_postprocessors.size(); ++i)
      {
        Postprocessor *ps = side_postprocessors[i];
        std::string name = ps->name();

        // join across the threads (gather the value in thread #0)
        for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
          ps->threadJoin(*pps[tid].sidePostprocessors(boundary_id)[i]);

        Real value = ps->getValue();

        // store the value in each thread
        for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
          _pps_data[tid].storeValue(name, value);
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
MProblem::computePostprocessors(ExecFlagType type/* = EXEC_TIMESTEP*/)
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
MProblem::outputPostprocessors()
{
  // Store values into table
  for (std::vector<Postprocessor *>::const_iterator postprocessor_it = _pps(EXEC_TIMESTEP)[0].all().begin();
      postprocessor_it != _pps(EXEC_TIMESTEP)[0].all().end();
      ++postprocessor_it)
  {
    Postprocessor *pps = *postprocessor_it;

    if (pps->getOutput())
    {
      std::string name = pps->name();
      Real value = _pps_data[0].getPostprocessorValue(name);

      _pps_output_table.addData(name, value, _time);
    }
  }
  // Add values of initial PPSes
  for (std::vector<Postprocessor *>::const_iterator postprocessor_it = _pps(EXEC_INITIAL)[0].all().begin();
      postprocessor_it != _pps(EXEC_INITIAL)[0].all().end();
      ++postprocessor_it)
  {
    Postprocessor *pps = *postprocessor_it;

    if (pps->getOutput())
    {
      std::string name = pps->name();
      Real value = _pps_data[0].getPostprocessorValue(name);

      _pps_output_table.addData(name, value, _time);
    }
  }

  if (_pps_output_table.empty())
    return;

  if (_postprocessor_screen_output)
  {
    std::cout<<std::endl<<"Postprocessor Values:"<<std::endl;
    _pps_output_table.printTable(std::cout);
    std::cout<<std::endl;
  }

  // FIXME: if exodus output is enabled?
  _out.outputPps(_pps_output_table);

  if (_postprocessor_csv_output)
    _pps_output_table.printCSV(_out.fileBase() + ".csv");

  if (_postprocessor_ensight_output)
    _pps_output_table.printEnsight(_out.fileBase());

  if (_postprocessor_gnuplot_output)
    _pps_output_table.makeGnuplot(_out.fileBase(), _gnuplot_format);
}

void
MProblem::addDamper(std::string damper_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  parameters.set<SubProblemInterface *>("_subproblem") = this;
  parameters.set<SystemBase *>("_sys") = &_nl;

  _has_dampers = true;
  _nl.addDamper(damper_name, name, parameters);
}

void
MProblem::setupDampers()
{
  _nl.setupDampers();
}

bool
MProblem::hasVariable(const std::string & var_name)
{
  if (_nl.hasVariable(var_name))
    return true;
  else if (_aux.hasVariable(var_name))
    return true;
  else
    return false;
}

MooseVariable &
MProblem::getVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_nl.hasVariable(var_name))
    return _nl.getVariable(tid, var_name);
  else if (_aux.hasVariable(var_name))
    return _aux.getVariable(tid, var_name);
  else
    mooseError("Unknown variable " + var_name);
}

void
MProblem::createQRules(QuadratureType type, Order order)
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
    _asm_info[tid]->createQRules(type, _quadrature_order);

  if (_displaced_problem)
    _displaced_problem->createQRules(type, _quadrature_order);
}

AsmBlock &
MProblem::asmBlock(THREAD_ID tid)
{
  return _nl.asmBlock(tid);
}

void
MProblem::init()
{
  _nl.preInit();

  Moose::setup_perf_log.push("eq.init()","Setup");
  SubProblem::init();
  Moose::setup_perf_log.pop("eq.init()","Setup");

  Moose::setup_perf_log.push("mesh.applyMeshModifications()","Setup");
  _mesh.applyMeshModifications();
  Moose::setup_perf_log.pop("mesh.applyMeshModifications()","Setup");

  Moose::setup_perf_log.push("MProblem::init::meshChanged()","Setup");
  _mesh.meshChanged();
  Moose::setup_perf_log.pop("MProblem::init::meshChanged()","Setup");

  init2();
}

void
MProblem::init2()
{
  Moose::setup_perf_log.push("NonlinearSystem::update()","Setup");
  _nl.update();
  Moose::setup_perf_log.pop("NonlinearSystem::update()","Setup");

  _nl.init();

  if (_displaced_problem)
    _displaced_problem->init();

  _aux.init();
}

void
MProblem::solve()
{
  Moose::setSolverDefaults(*this);
  Moose::perf_log.push("solve()","Solve");
  _solve_only_perf_log.push("solve");
  _nl.solve();
  _solve_only_perf_log.pop("solve");
  Moose::perf_log.pop("solve()","Solve");
  _nl.update();
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
MProblem::copyOldSolutions()
{
  _nl.copyOldSolutions();
  _aux.copyOldSolutions();
}

void
MProblem::onTimestepBegin()
{
  _nl.onTimestepBegin();
}

void
MProblem::onTimestepEnd()
{
  _aux.compute(EXEC_TIMESTEP);
  _nl.printVarNorms();
}

void
MProblem::computeResidual(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number>& soln, NumericVector<Number>& residual)
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
MProblem::computeJacobian(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian)
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
MProblem::computeJacobianBlock(SparseMatrix<Number> & jacobian, libMesh::System & precond_system, unsigned int ivar, unsigned int jvar)
{
  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh(*_nl.currentSolution(), *_aux.currentSolution());

  _aux.compute();
  _nl.computeJacobianBlock(jacobian, precond_system, ivar, jvar);
}

Real
MProblem::computeDamping(const NumericVector<Number>& soln, const NumericVector<Number>& update)
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
MProblem::initDisplacedProblem(MooseMesh * displaced_mesh, const std::vector<std::string> & displacements)
{
  _displaced_mesh = displaced_mesh;

  Moose::setup_perf_log.push("Create DisplacedProblem","Setup");
  _displaced_problem = new DisplacedProblem(*this, *_displaced_mesh, displacements);
  Moose::setup_perf_log.pop("Create DisplacedProblem","Setup");
}

void
MProblem::updateGeomSearch()
{
  _geometric_search_data.update();

  if(_displaced_problem)
    _displaced_problem->updateGeomSearch();
}

void
MProblem::output()
{
  _out.output();
  if (_displaced_problem != NULL && _output_displaced)
    _displaced_problem->output();

  // save the input file if we did not do so already
  if (!_input_file_saved)
  {
    _out.outputInput();
    _input_file_saved = true;
  }

}

#ifdef LIBMESH_ENABLE_AMR
void
MProblem::adaptMesh()
{
  _adaptivity.adaptMesh();
  meshChanged();
}
#endif //LIBMESH_ENABLE_AMR

void
MProblem::meshChanged()
{
  // mesh changed
  _eq.reinit();
  _mesh.meshChanged();
  _geometric_search_data.update();

  if(_displaced_problem != NULL)
    _displaced_problem->meshChanged();
}

void
MProblem::checkProblemIntegrity()
{
  // Check for unsatisfied actions
  const std::set<subdomain_id_type> & mesh_subdomains = _mesh.meshSubdomains();
  Moose::action_warehouse.checkUnsatisfiedActions();

  // Check kernel coverage of subdomains (blocks) in the mesh
  _nl.checkKernelCoverage(mesh_subdomains);

  // Check materials
  {
#ifdef LIBMESH_ENABLE_AMR
    if (_material_props.hasStatefulProperties() && _adaptivity.isOn())
      mooseError("Cannot use Material classes with stateful properties while utilizing adaptivity!");
#endif

    std::set<subdomain_id_type> local_mesh_subs(mesh_subdomains);
    /**
     * If a material is specified for any block in the simulation, then all blocks must
     * have a material specified.
     */
    bool check_material_coverage = false;
    for (std::set<int>::const_iterator i = _materials[0].blocks().begin(); i != _materials[0].blocks().end(); ++i)
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
      /// <unsigned int> is necessary to print subdomain_id_types in the statement below
      std::copy (local_mesh_subs.begin(), local_mesh_subs.end(), std::ostream_iterator<unsigned int>(extra_subdomain_ids, " "));

      mooseError("The following blocks from your input mesh do not contain on active material: " + extra_subdomain_ids.str() + "\nWhen ANY mesh block contains a Material object, all blocks must contain a Material object.\n");
    }
  }

  // Check that BCs used in your simulation exist in your mesh
  _nl.checkBCCoverage();
}

void
MProblem::serializeSolution()
{
  _nl.serializeSolution();
  _aux.serializeSolution();
}

void
MProblem::restartFromFile()
{
  _eq.read(_restart_file_name, libMeshEnums::READ, EquationSystems::READ_DATA);
  _nl.update();
}

std::vector<std::string>
MProblem::getVariableNames()
{
  std::vector<std::string> names;
  
  System & nl = _nl.sys();
  System & aux = _aux.sys();

  for(unsigned int i=0; i<nl.n_vars(); i++)
    names.push_back(nl.variable_name(i));

  for(unsigned int i=0; i<aux.n_vars(); i++)
    names.push_back(aux.variable_name(i));

  return names;
}
