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
    _adaptivity(*this),
    _print_mesh_changed(false),
    _displaced_mesh(NULL),
    _displaced_problem(NULL),
    _geometric_search_data(*this, _mesh),
    _reinit_displaced_elem(false),
    _reinit_displaced_face(false),
    _output_displaced(false),
    _has_dampers(false)
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
  _pps.resize(n_threads);
  _pps_residual.resize(n_threads);
  _pps_jacobian.resize(n_threads);
  _pps_newtonit.resize(n_threads);
}

MProblem::~MProblem()
{
  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; i++)
  {
    delete _asm_info[i];
    delete _material_data[i];
    delete _bnd_material_data[i];
  }

//  delete _displaced_mesh;
  delete _displaced_problem;
}


void MProblem::initialSetup()
{
  Moose::setup_perf_log.push("copySolutionsBackwards()","Setup");
  copySolutionsBackwards();
  Moose::setup_perf_log.pop("copySolutionsBackwards()","Setup");

  Moose::setup_perf_log.push("adaptivity().initial()","Setup");
  adaptivity().initial();
  Moose::setup_perf_log.pop("adaptivity().initial()","Setup");

  Moose::setup_perf_log.push("Initial updateGeomSearch()","Setup");
  //Update the geometric searches (has to be called after the problem is all set up)
  updateGeomSearch();
  Moose::setup_perf_log.pop("Initial updateGeomSearch()","Setup");

  Moose::setup_perf_log.push("reinit() after updateGeomSearch()","Setup");
  // Call reinit to get the ghosted vectors correct now that some geometric search has been done
  _eq.reinit();
  Moose::setup_perf_log.pop("reinit() after updateGeomSearch()","Setup");
  unsigned int n_threads = libMesh::n_threads();

  for(unsigned int i=0; i<n_threads; i++)
  {
    _materials[i].initialSetup();
    _pps[i].initialSetup();
    _pps_residual[i].initialSetup();
    _pps_jacobian[i].initialSetup();
    _pps_newtonit[i].initialSetup();
  }

  _aux.initialSetup();

  Moose::setup_perf_log.push("Initial computePostprocessors()","Setup");
  computePostprocessors();
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

  Moose::setup_perf_log.print_log();
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
}

void
MProblem::prepare(const Elem * elem, THREAD_ID tid)
{
  _asm_info[tid]->reinit(elem);
  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->prepare(_displaced_mesh->elem(elem->id()), tid);

  _nl.prepare(tid);
  _aux.prepare(tid);
}

void
MProblem::addGhostedElem(unsigned int elem_id)
{
  _ghosted_elems.insert(elem_id);
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

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitElem(_displaced_mesh->elem(elem->id()), tid);

  _nl.reinitElem(elem, tid);
  _aux.reinitElem(elem, tid);
}

void
MProblem::reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid)
{
  _asm_info[tid]->reinit(elem, side);

  unsigned int n_points = _asm_info[tid]->qRule()->n_points();
  _zero[tid].resize(n_points, 0);
  _grad_zero[tid].resize(n_points, 0);
  _second_zero[tid].resize(n_points, 0);
  
  if (_displaced_problem != NULL && _reinit_displaced_face)
    _displaced_problem->reinitElemFace(_displaced_mesh->elem(elem->id()), side, bnd_id, tid);

  _nl.reinitElemFace(elem, side, bnd_id, tid);
  _aux.reinitElemFace(elem, side, bnd_id, tid);
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
  ExplicitSystem & system = es.get_system<ExplicitSystem>(system_name);
  system.project_solution(Moose::initial_value, Moose::initial_gradient, es.parameters);
}

void
MProblem::addMaterial(const std::string & mat_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  parameters.set<SubProblemInterface *>("_subproblem") = this;
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

  if (_material_data[0]->hasStatefulProperties())
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
MProblem::addStabilizer(const std::string & stabilizer_name, const std::string & name, InputParameters parameters)
{
  parameters.set<Problem *>("_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;
  parameters.set<SystemBase *>("_sys") = &_nl;
  _nl.addStabilizer(stabilizer_name, name, parameters);
}

void
MProblem::addPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters, Moose::PostprocessorType pps_type)
{
  std::vector<PostprocessorWarehouse> * pps;
  switch (pps_type)
  {
  case Moose::PPS_RESIDUAL: pps = &_pps_residual; break;
  case Moose::PPS_JACOBIAN: pps = &_pps_jacobian; break;
  case Moose::PPS_NEWTONIT: pps = &_pps_newtonit; break;
  default: pps = &_pps; break;
  }

  parameters.set<Problem *>("_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblemInterface *>("_subproblem") = _displaced_problem;
  }
  else
  {
    parameters.set<SubProblemInterface *>("_subproblem") = this;
  }

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
          (*pps)[tid].addPostprocessor(pp);
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
        (*pps)[tid].addPostprocessor(pp);

//      }
//      else
//        mooseError("Duplicate postprocessor name '" + name + "'");
    }
  }
}

Real &
MProblem::getPostprocessorValue(const std::string & name, THREAD_ID tid)
{
  // Todo: This is hardcoded to zero because we only compute postprocessors serially now.
  return _pps_data[0].getPostprocessorValue(name);
}

void
MProblem::computePostprocessorsInternal(std::vector<PostprocessorWarehouse> & pps)
{
  if (pps[0]._block_ids_with_postprocessors.size() > 0 || pps[0]._boundary_ids_with_postprocessors.size() > 0)
  {
    serializeSolution();

    if (_displaced_problem != NULL)
      _displaced_problem->updateMesh(*_nl.currentSolution(), *_aux.currentSolution());

    _aux.compute();

    ComputePostprocessorsThread cppt(*this, getNonlinearSystem(), *getNonlinearSystem().currentSolution(), pps);
    cppt(*_mesh.getActiveLocalElementRange());

    // Store element postprocessors values
    std::set<unsigned int>::iterator block_ids_begin = pps[0]._block_ids_with_postprocessors.begin();
    std::set<unsigned int>::iterator block_ids_end = pps[0]._block_ids_with_postprocessors.end();
    std::set<unsigned int>::iterator block_ids_it = block_ids_begin;

    for (; block_ids_it != block_ids_end; ++block_ids_it)
    {
      unsigned int block_id = *block_ids_it;

      PostprocessorIterator element_postprocessor_begin = pps[0].elementPostprocessorsBegin(block_id);
      PostprocessorIterator element_postprocessor_end = pps[0].elementPostprocessorsEnd(block_id);
      PostprocessorIterator element_postprocessor_it = element_postprocessor_begin;

      // Store element postprocessors values
      for (element_postprocessor_it=element_postprocessor_begin;
          element_postprocessor_it!=element_postprocessor_end;
          ++element_postprocessor_it)
      {
        std::string name = (*element_postprocessor_it)->name();
        Real value = (*element_postprocessor_it)->getValue();

        _pps_data[0].storeValue(name, value);
      }
    }

    // Store side postprocessors values
    std::set<unsigned int>::iterator boundary_ids_begin = pps[0]._boundary_ids_with_postprocessors.begin();
    std::set<unsigned int>::iterator boundary_ids_end = pps[0]._boundary_ids_with_postprocessors.end();
    std::set<unsigned int>::iterator boundary_ids_it = boundary_ids_begin;

    for (; boundary_ids_it != boundary_ids_end; ++boundary_ids_it)
    {
      unsigned int boundary_id = *boundary_ids_it;

      PostprocessorIterator side_postprocessor_begin = pps[0].sidePostprocessorsBegin(boundary_id);
      PostprocessorIterator side_postprocessor_end = pps[0].sidePostprocessorsEnd(boundary_id);
      PostprocessorIterator side_postprocessor_it = side_postprocessor_begin;

      for (side_postprocessor_it=side_postprocessor_begin;
          side_postprocessor_it!=side_postprocessor_end;
          ++side_postprocessor_it)
      {
        std::string name = (*side_postprocessor_it)->name();
        Real value = (*side_postprocessor_it)->getValue();

        _pps_data[0].storeValue(name, value);
      }
    }
  }

  // Compute and store generic postprocessors values
  PostprocessorIterator generic_postprocessor_begin = pps[0].genericPostprocessorsBegin();
  PostprocessorIterator generic_postprocessor_end = pps[0].genericPostprocessorsEnd();
  PostprocessorIterator generic_postprocessor_it = generic_postprocessor_begin;

  for (generic_postprocessor_it =generic_postprocessor_begin;
      generic_postprocessor_it!=generic_postprocessor_end;
      ++generic_postprocessor_it)
  {
    std::string name = (*generic_postprocessor_it)->name();
    (*generic_postprocessor_it)->initialize();
    (*generic_postprocessor_it)->execute();
    Real value = (*generic_postprocessor_it)->getValue();

    _pps_data[0].storeValue(name, value);
  }
}

void
MProblem::computePostprocessors(int pps_type)
{
  Moose::perf_log.push("compute_postprocessors()","Solve");

  /* TODO: Remove the 0 when we actually thread these guys! */
  if ((pps_type & Moose::PPS_RESIDUAL) == Moose::PPS_RESIDUAL)
  {
    _pps_residual[0].residualSetup();
    computePostprocessorsInternal(_pps_residual);
  }
  if ((pps_type & Moose::PPS_JACOBIAN) == Moose::PPS_JACOBIAN)
  {
    _pps_residual[0].jacobianSetup();
    computePostprocessorsInternal(_pps_jacobian);
  }
  if ((pps_type & Moose::PPS_TIMESTEP) == Moose::PPS_TIMESTEP)
  {
    _pps_residual[0].timestepSetup();
    computePostprocessorsInternal(_pps);
  }

  Moose::perf_log.pop("compute_postprocessors()","Solve");
}

void
MProblem::outputPostprocessors()
{
  // Store values into table
  PostprocessorIterator postprocessor_begin = _pps[0].allPostprocessorsBegin();
  PostprocessorIterator postprocessor_end = _pps[0].allPostprocessorsEnd();
  PostprocessorIterator postprocessor_it = postprocessor_begin;

  for (postprocessor_it = postprocessor_begin; postprocessor_it != postprocessor_end; ++postprocessor_it)
  {
    std::string name = (*postprocessor_it)->name();
    Real value = _pps_data[0].getPostprocessorValue(name);

    _pps_output_table.addData(name, value, _time);
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
  Moose::setup_perf_log.push("getMinQuadratureOrder()","Setup");
  _quadrature_order = _nl.getMinQuadratureOrder();
  Moose::setup_perf_log.pop("getMinQuadratureOrder()","Setup");
  
  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    _asm_info[tid]->createQRules(_quadrature_order);

  Moose::setup_perf_log.push("NonlinearSystem::update()","Setup");
  _nl.update();
  Moose::setup_perf_log.pop("NonlinearSystem::update()","Setup");
  
  _nl.init();

  if (_displaced_problem)
  {
    _displaced_problem->init();
    _displaced_problem->updateMesh(*_nl.currentSolution(), *_aux.currentSolution());
  }
  _aux.init();

}

void
MProblem::solve()
{
  Moose::setSolverDefaults(*this);
  Moose::perf_log.push("solve()","Solve");
  _nl.solve();
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
  _aux.compute_ts();

  _nl.printVarNorms();
}

void
MProblem::computeResidual(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number>& soln, NumericVector<Number>& residual)
{
  _nl.set_solution(soln);
  computePostprocessors(Moose::PPS_RESIDUAL);
  
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
}

void
MProblem::computeJacobian(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian)
{
  _nl.set_solution(soln);
  computePostprocessors(Moose::PPS_JACOBIAN);

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
MProblem::initDisplacedProblem(const std::vector<std::string> & displacements)
{
  Moose::setup_perf_log.push("Create displaced_mesh","Setup");
  _displaced_mesh = new MooseMesh(_mesh);
  Moose::setup_perf_log.pop("Create displaced_mesh","Setup");

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
}

void
MProblem::adaptMesh()
{
  _adaptivity.adaptMesh();
  meshChanged();
}

void
MProblem::meshChanged()
{
  // mesh changed
  _eq.reinit();
  _mesh.meshChanged();
  _geometric_search_data.update();

  if(_displaced_problem != NULL)
    _displaced_problem->meshChanged();

  if (_print_mesh_changed)
  {
    std::cout << "\nMesh Changed:\n";
    _mesh.printInfo();
  }
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
    bool adaptivity = _adaptivity.isOn();
    if (_material_data[0]->hasStatefulProperties() && adaptivity)
      mooseError("Cannot use Material classes with stateful properties while utilizing adaptivity!");

    std::set<subdomain_id_type> local_mesh_subs(mesh_subdomains);
    /**
     * If a material is specified for any block in the simulation, then all blocks must
     * have a material specified.
     */
    bool check_material_coverage = false;
    for (MaterialIterator i = _materials[0].activeMaterialsBegin(); i != _materials[0].activeMaterialsEnd(); ++i)
    {
      if (mesh_subdomains.find(i->first) == mesh_subdomains.end())
      {
        std::stringstream oss;
        oss << "Material block \"" << i->first << "\" specified in the input file does not exist";
        mooseError (oss.str());
      }

      local_mesh_subs.erase(i->first);
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
  {
    const std::set<short> & mesh_bcs = _mesh._mesh.boundary_info->get_boundary_ids();
    
    _nl.checkBCCoverage(mesh_bcs);
  }
}

void
MProblem::serializeSolution()
{
  _nl.serializeSolution();
  _aux.serializeSolution();
}
