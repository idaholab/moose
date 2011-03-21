#include "SubProblem.h"
#include "Factory.h"
#include "ImplicitSystem.h"
#include "Postprocessor.h"
#include "ThreadedElementLoop.h"
#include "MaterialData.h"

// libMesh includes

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


//
class ComputePostprocessorsThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputePostprocessorsThread(Problem & problem, System & sys, const NumericVector<Number>& in_soln, std::vector<PostprocessorWarehouse> & pps) :
    ThreadedElementLoop<ConstElemRange>(problem, sys),
     _soln(in_soln),
     _pps(pps)
  {}

  // Splitting Constructor
  ComputePostprocessorsThread(ComputePostprocessorsThread & x, Threads::split) :
    ThreadedElementLoop<ConstElemRange>(x._problem, x._system),
    _soln(x._soln),
    _pps(x._pps)
  {
  }

  virtual void pre()
  {
    //Initialize side and element post processors

    std::set<unsigned int>::iterator block_begin = _pps[_tid]._block_ids_with_postprocessors.begin();
    std::set<unsigned int>::iterator block_end = _pps[_tid]._block_ids_with_postprocessors.end();
    std::set<unsigned int>::iterator block_it = block_begin;

    for (block_it=block_begin;block_it!=block_end;++block_it)
    {
      unsigned int block_id = *block_it;

      PostprocessorIterator postprocessor_begin = _pps[_tid].elementPostprocessorsBegin(block_id);
      PostprocessorIterator postprocessor_end = _pps[_tid].elementPostprocessorsEnd(block_id);
      PostprocessorIterator postprocessor_it = postprocessor_begin;

      for (postprocessor_it=postprocessor_begin;postprocessor_it!=postprocessor_end;++postprocessor_it)
        (*postprocessor_it)->initialize();
    }

    std::set<unsigned int>::iterator boundary_begin = _pps[_tid]._boundary_ids_with_postprocessors.begin();
    std::set<unsigned int>::iterator boundary_end = _pps[_tid]._boundary_ids_with_postprocessors.end();
    std::set<unsigned int>::iterator boundary_it = boundary_begin;

    for (boundary_it=boundary_begin;boundary_it!=boundary_end;++boundary_it)
    {
      //note: for threaded applications where the elements get broken up it
      //may be more efficient to initialize these on demand inside the loop
      PostprocessorIterator side_postprocessor_begin = _pps[_tid].sidePostprocessorsBegin(*boundary_it);
      PostprocessorIterator side_postprocessor_end = _pps[_tid].sidePostprocessorsEnd(*boundary_it);
      PostprocessorIterator side_postprocessor_it = side_postprocessor_begin;

      for (side_postprocessor_it=side_postprocessor_begin;side_postprocessor_it!=side_postprocessor_end;++side_postprocessor_it)
        (*side_postprocessor_it)->initialize();
    }
  }

  virtual void preElement(const Elem * elem)
  {
//    _moose_system.reinitKernels(_tid, _soln, elem, NULL);
//    _moose_system._element_data[_tid]->reinitMaterials(_moose_system._materials[_tid].getMaterials(elem->subdomain_id()));
    _problem.prepare(elem, _tid);
  }

  virtual void onElement(const Elem *elem)
  {
    unsigned int subdomain = elem->subdomain_id();

    _problem.reinitElem(elem, _tid);
    _problem.reinitMaterials(subdomain, _tid);


    //Global Postprocessors
    PostprocessorIterator postprocessor_begin = _pps[_tid].elementPostprocessorsBegin(Moose::ANY_BLOCK_ID);
    PostprocessorIterator postprocessor_end = _pps[_tid].elementPostprocessorsEnd(Moose::ANY_BLOCK_ID);
    PostprocessorIterator postprocessor_it = postprocessor_begin;

    for (postprocessor_it=postprocessor_begin;postprocessor_it!=postprocessor_end;++postprocessor_it)
      (*postprocessor_it)->execute();

    postprocessor_begin = _pps[_tid].elementPostprocessorsBegin(subdomain);
    postprocessor_end = _pps[_tid].elementPostprocessorsEnd(subdomain);
    postprocessor_it = postprocessor_begin;

    for (postprocessor_it=postprocessor_begin;postprocessor_it!=postprocessor_end;++postprocessor_it)
      (*postprocessor_it)->execute();
  }

  virtual void onBoundary(const Elem *elem, unsigned int side, short int bnd_id)
  {
    PostprocessorIterator side_postprocessor_begin = _pps[_tid].sidePostprocessorsBegin(bnd_id);
    PostprocessorIterator side_postprocessor_end = _pps[_tid].sidePostprocessorsEnd(bnd_id);
    PostprocessorIterator side_postprocessor_it = side_postprocessor_begin;

    if (side_postprocessor_begin != side_postprocessor_end)
    {
      _problem.reinitElemFace(elem, side, bnd_id, _tid);
      _problem.reinitMaterialsFace(elem->subdomain_id(), side, _tid);

      for (; side_postprocessor_it!=side_postprocessor_end; ++side_postprocessor_it)
        (*side_postprocessor_it)->execute();
    }
  }

  void join(const ComputePostprocessorsThread & /*y*/)
  {
  }

protected:
  const NumericVector<Number>& _soln;
  std::vector<PostprocessorWarehouse> & _pps;
};


// SubProblem /////

SubProblem::SubProblem(Mesh & mesh, Problem * parent) :
    _parent(parent == NULL ? this : parent),
    _mesh(mesh),
    _eq(parent == NULL ? *new EquationSystems(_mesh) : parent->es()),
    _transient(false),
    _time(_parent != this ? _parent->time() : _eq.parameters.set<Real>("time")),
    _t_step(_parent != this ? _parent->timeStep() : _eq.parameters.set<int>("t_step")),
    _dt(_parent != this ? _parent->dt() : _eq.parameters.set<Real>("dt")),
    _out(*this),
    _geometric_search_data(*this, _mesh),
    _postprocessor_screen_output(true),
    _postprocessor_csv_output(false),
    _postprocessor_ensight_output(false),
    _postprocessor_gnuplot_output(false),
    _gnuplot_format("ps"),
    _adaptivity(*this)
{
  if (_parent == this)
  {
    _time = 0.0;
    _t_step = 0;
    _dt = 0;
    _dt_old = _dt;
    _eq.parameters.set<Problem *>("_problem") = this;
  }

  unsigned int n_threads = libMesh::n_threads();
  _functions.resize(n_threads);
  _materials.resize(n_threads);

  _material_data.resize(n_threads);
  _bnd_material_data.resize(n_threads);
  for (unsigned int i = 0; i < n_threads; i++)
  {
    _material_data[i] = new MaterialData();
    _bnd_material_data[i] = new MaterialData();
  }

  _pps_data.resize(n_threads);
  _pps.resize(n_threads);
  _pps_residual.resize(n_threads);
  _pps_jacobian.resize(n_threads);
  _pps_newtonit.resize(n_threads);
}

SubProblem::~SubProblem()
{
  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; i++)
  {
    delete _material_data[i];
    delete _bnd_material_data[i];
  }

  if (_parent == this)
    delete &_eq;
}

void
SubProblem::init()
{
  _eq.init();
  _eq.print_info();
  _mesh.applyMeshModifications();
  _mesh.meshChanged();
}

void
SubProblem::update()
{
  for (std::vector<System *>::iterator it = _sys.begin(); it != _sys.end(); ++it)
    (*it)->update();
}

void
SubProblem::solve()
{
  for (std::vector<System *>::iterator it = _sys.begin(); it != _sys.end(); ++it)
    (*it)->solve();
}

bool
SubProblem::hasVariable(const std::string & var_name)
{
  for (unsigned int i = 0; i < _sys.size(); i++)
  {
    if (_sys[i]->hasVariable(var_name))
      return true;
  }
  return false;
}

Variable &
SubProblem::getVariable(THREAD_ID tid, const std::string & var_name)
{
  for (unsigned int i = 0; i < _sys.size(); i++)
    if (_sys[i]->hasVariable(var_name))
      return _sys[i]->getVariable(tid, var_name);
  mooseError("Unknown variable " + var_name);
}

void
SubProblem::copySolutionsBackwards()
{
}

// Initial Conditions /////
void
SubProblem::addInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters, std::string var_name)
{
  parameters.set<SubProblem *>("_problem") = this;
  parameters.set<std::string>("var_name") = var_name;
  _ics[var_name] = static_cast<InitialCondition *>(Factory::instance()->create(ic_name, name, parameters));
}

void
SubProblem::addInitialCondition(const std::string & var_name, Real value)
{
  _pars.set<Real>("initial_" + var_name) = value;
}

void
SubProblem::addFunction(std::string type, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_problem") = this;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    Function * func = static_cast<Function *>(Factory::instance()->create(type, name, parameters));
    _functions[tid][name] = func;
  }
}

Function &
SubProblem::getFunction(const std::string & name, THREAD_ID tid)
{
  return *_functions[tid][name];
}

void
SubProblem::addMaterial(const std::string & mat_name, const std::string & name, InputParameters parameters)
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
    }
  }
}

Number
SubProblem::initialValue (const Point& p,
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
SubProblem::initialGradient (const Point& p,
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
SubProblem::initialCondition(EquationSystems& es, const std::string& system_name)
{
  ExplicitSystem & system = es.get_system<ExplicitSystem>(system_name);
  system.project_solution(Moose::initial_value, Moose::initial_gradient, es.parameters);
}

void
SubProblem::updateMaterials()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    _materials[tid].updateMaterialDataState();
}

void
SubProblem::reinitMaterials(unsigned int blk_id, THREAD_ID tid)
{
  if (_materials[tid].hasMaterials(blk_id))
  {
    std::vector<Material *> & mats = _materials[tid].getMaterials(blk_id);
    for (std::vector<Material *>::iterator it = mats.begin(); it != mats.end(); ++it)
      (*it)->reinit();
  }
}

void
SubProblem::reinitMaterialsFace(unsigned int blk_id, unsigned int side, THREAD_ID tid)
{
  if (_materials[tid].hasBoundaryMaterials(blk_id))
  {
    std::vector<Material *> & mats = _materials[tid].getBoundaryMaterials(blk_id);
    for (std::vector<Material *>::iterator it = mats.begin(); it != mats.end(); ++it)
      (*it)->reinit(side);
  }
}

void
SubProblem::addPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters, Moose::PostprocessorType pps_type)
{
  std::vector<PostprocessorWarehouse> * pps;
  switch (pps_type)
  {
  case Moose::PPS_RESIDUAL: pps = &_pps_residual; break;
  case Moose::PPS_JACOBIAN: pps = &_pps_jacobian; break;
  case Moose::PPS_NEWTONIT: pps = &_pps_newtonit; break;
  default: pps = &_pps; break;
  }

  parameters.set<SubProblem *>("_problem") = this;

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    if(parameters.have_parameter<std::vector<unsigned int> >("boundary"))
    {
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
SubProblem::getPostprocessorValue(const std::string & name, THREAD_ID tid)
{
  return _pps_data[tid].getPostprocessorValue(name);
}

void
SubProblem::computePostprocessorsInternal(std::vector<PostprocessorWarehouse> & pps)
{
  if (pps[0]._block_ids_with_postprocessors.size() > 0 || pps[0]._boundary_ids_with_postprocessors.size() > 0)
  {
    // FIXME: this should go somewhere else
//    if (_serialize_solution)
//      serializeSolution(soln);
//
//    if (_has_displaced_mesh)
//      updateDisplacedMesh(soln);
//
//    updateAuxVars(soln);

    ComputePostprocessorsThread cppt(*this, getNonlinearSystem(), getNonlinearSystem().solution(), pps);
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
}

void
SubProblem::computePostprocessors(int pps_type)
{
  Moose::perf_log.push("compute_postprocessors()","Solve");

  // This resets stuff so that id 0 is the first id
  ParallelUniqueId::reinitialize();

  if ((pps_type & Moose::PPS_RESIDUAL) == Moose::PPS_RESIDUAL)
    computePostprocessorsInternal(_pps_residual);
  if ((pps_type & Moose::PPS_JACOBIAN) == Moose::PPS_JACOBIAN)
    computePostprocessorsInternal(_pps_jacobian);
  if ((pps_type & Moose::PPS_TIMESTEP) == Moose::PPS_TIMESTEP)
    computePostprocessorsInternal(_pps);

  Moose::perf_log.pop("compute_postprocessors()","Solve");
}

void
SubProblem::outputPostprocessors()
{
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
SubProblem::addDiracKernel(std::string dirac_kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_problem") = this;

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    DiracKernel * dk = static_cast<DiracKernel *>(Factory::instance()->create(dirac_kernel_name, name, parameters));
    _dirac_kernels[tid].addDiracKernel(dk);
  }
}


void
SubProblem::dump()
{
}

void
SubProblem::output()
{
  _out.output();
}

void
SubProblem::adaptMesh()
{
  _adaptivity.adaptMesh();

  // mesh changed
  _eq.reinit();
  _mesh.meshChanged();
  _geometric_search_data.update();
  _out.meshChanged();
}


} // namespace
