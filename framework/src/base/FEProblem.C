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
//#include "SystemBase.h"
#include "Factory.h"
#include "ProblemFactory.h"
#include "DisplacedProblem.h"
#include "OutputProblem.h"
#include "MaterialData.h"
#include "ComputeUserObjectsThread.h"
#include "ComputeNodalUserObjectsThread.h"
#include "ComputeMaterialsObjectThread.h"
#include "ComputeIndicatorThread.h"
#include "ComputeMarkerThread.h"
#include "ActionWarehouse.h"
#include "Conversion.h"
#include "Material.h"
#include "ConstantIC.h"
#include "FP.h"
#include "Parser.h"
#include "ElementH1Error.h"
#include "Function.h"
#include "Material.h"

#include "ElementPostprocessor.h"
#include "NodalPostprocessor.h"
#include "SidePostprocessor.h"
#include "GeneralPostprocessor.h"
#include "Indicator.h"
#include "Marker.h"

#include "ElementUserObject.h"
#include "NodalUserObject.h"
#include "SideUserObject.h"
#include "GeneralUserObject.h"

#include "InternalSideIndicator.h"

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
  InputParameters params = validParams<SubProblem>();
  params.addRequiredParam<MooseMesh *>("mesh", "The Mesh");
  return params;
}

FEProblem::FEProblem(const std::string & name, InputParameters parameters) :
    SubProblem(name, parameters),
    _mesh(*parameters.get<MooseMesh *>("mesh")),
    _eq(_mesh),

    _transient(false),
    _time(_eq.parameters.set<Real>("time")),
    _time_old(_eq.parameters.set<Real>("time_old")),
    _t_step(_eq.parameters.set<int>("t_step")),
    _dt(_eq.parameters.set<Real>("dt")),

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
    _output_es_info(true),
    _input_file_saved(false),
    _has_dampers(false),
    _has_constraints(false),
    _resurrector(NULL),
//    _solve_only_perf_log("Solve Only"),
    _output_setup_log_early(false),
    // debugging
    _dbg_top_residuals(0)
{
#ifdef LIBMESH_HAVE_PETSC
  // put in empty arrays for PETSc options
  this->parameters().set<std::vector<std::string> >("petsc_options") = std::vector<std::string>();
  this->parameters().set<std::vector<std::string> >("petsc_inames") = std::vector<std::string>();
  this->parameters().set<std::vector<std::string> >("petsc_values") = std::vector<std::string>();
#endif

  _n++;

  _time = 0.0;
  _time_old = 0.0;
  _t_step = 0;
  _dt = 0;
  _dt_old = _dt;

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
  _objects_by_name.resize(n_threads);

  _indicators.resize(n_threads);
  _markers.resize(n_threads);

  _active_elemental_moose_variables.resize(n_threads);

  _resurrector = new Resurrector(*this);

  _eq.parameters.set<FEProblem *>("_fe_problem") = this;
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

    for (std::map<std::string, Function *>::iterator it = _functions[i].begin(); it != _functions[i].end(); ++it)
      delete it->second;
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

  delete _resurrector;
  delete &_mesh;
}

Moose::CoordinateSystemType
FEProblem::getCoordSystem(SubdomainID sid)
{
  std::map<SubdomainID, Moose::CoordinateSystemType>::iterator it = _coord_sys.find(sid);
  if (it != _coord_sys.end())
    return (*it).second;
  else
  {
    std::stringstream err;
    err << "Requested subdomain "
        << sid
        << " does not exist.";
    mooseError(err.str());
  }
}

void
FEProblem::setCoordSystem(const std::vector<SubdomainName> & blocks, const std::vector<std::string> & coord_sys)
{
  const std::set<SubdomainID> & subdomains = _mesh.meshSubdomains();
  if (blocks.size() == 0)
  {
    // no blocks specified -> assume the whole domain
    Moose::CoordinateSystemType coord_type = Moose::COORD_XYZ;                          // all is going to be XYZ by default
    if (coord_sys.size() == 0)
      ; // relax, do nothing
    else if (coord_sys.size() == 1)
      coord_type = Moose::stringToEnum<Moose::CoordinateSystemType>(coord_sys[0]);      // one system specified, the whole domain is going to have that system
    else
      mooseError("Multiple coordinate systems specified, but no blocks given.");

    for (std::set<SubdomainID>::const_iterator it = subdomains.begin(); it != subdomains.end(); ++it)
      _coord_sys[*it] = coord_type;
  }
  else
  {
    if (blocks.size() != coord_sys.size())
      mooseError("Number of blocks and coordinate systems does not match.");

    for (unsigned int i = 0; i < blocks.size(); i++)
    {
      SubdomainID sid = _mesh.getSubdomainID(blocks[i]);
      Moose::CoordinateSystemType coord_type = Moose::stringToEnum<Moose::CoordinateSystemType>(coord_sys[i]);
      _coord_sys[sid] = coord_type;
    }

    for (std::set<SubdomainID>::const_iterator it = subdomains.begin(); it != subdomains.end(); ++it)
    {
      SubdomainID sid = *it;
      if (_coord_sys.find(sid) == _coord_sys.end())
        mooseError("Subdomain '" + Moose::stringify(sid) + "' does not have a coordinate system specified.");
    }
  }
}

void FEProblem::initialSetup()
{
  if (isRestarting())
    _resurrector->restartFromFile();
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

  if (!isRestarting())
    projectSolution();

  if (_output_es_info)
    _eq.print_info();

  unsigned int n_threads = libMesh::n_threads();

  Moose::setup_perf_log.push("copySolutionsBackwards()","Setup");
  copySolutionsBackwards();
  Moose::setup_perf_log.pop("copySolutionsBackwards()","Setup");

  for(unsigned int i=0; i<n_threads; i++)
    _materials[i].initialSetup();

  _aux.initialSetup();
  _aux.compute(EXEC_INITIAL);

  if (_material_props.hasStatefulProperties())
  {
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    ComputeMaterialsObjectThread cmt(*this, _nl, _material_data, _bnd_material_data, _material_props, _bnd_material_props, _materials, _assembly);
    Threads::parallel_reduce(elem_range, cmt);
  }

  if (isRestarting())
  {
    // now if restarting and we have stateful material properties, go overwrite the values with the ones
    // from the restart file.  We need to do it this way, since we have no idea about sizes of user-defined material
    // properties (i.e. things like std:vector<std::vector<SymmTensor> >)
    if (_material_props.hasStatefulProperties())
    {
      // load the stateful material props from a file
      _resurrector->restartStatefulMaterialProps();
    }

    // TODO: Reenable restarting for UserObjects!
//    if (_user_objects[0].size() > 0)
//      _resurrector->restartUserData();
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
    _user_objects(EXEC_RESIDUAL)[i].updateDependObjects(_aux.getDependObjects(EXEC_RESIDUAL));
    _user_objects(EXEC_JACOBIAN)[i].updateDependObjects(_aux.getDependObjects(EXEC_JACOBIAN));
    _user_objects(EXEC_TIMESTEP)[i].updateDependObjects(_aux.getDependObjects(EXEC_TIMESTEP));
    _user_objects(EXEC_TIMESTEP_BEGIN)[i].updateDependObjects(_aux.getDependObjects(EXEC_TIMESTEP_BEGIN));
    _user_objects(EXEC_INITIAL)[i].updateDependObjects(_aux.getDependObjects(EXEC_INITIAL));
    _user_objects(EXEC_CUSTOM)[i].updateDependObjects(_aux.getDependObjects(EXEC_CUSTOM));

    _user_objects(EXEC_RESIDUAL)[i].initialSetup();
    _user_objects(EXEC_JACOBIAN)[i].initialSetup();
    _user_objects(EXEC_TIMESTEP)[i].initialSetup();
    _user_objects(EXEC_TIMESTEP_BEGIN)[i].initialSetup();
    _user_objects(EXEC_INITIAL)[i].initialSetup();
    _user_objects(EXEC_CUSTOM)[i].initialSetup();

    for(std::map<std::string, Function *>::iterator vit = _functions[i].begin();
        vit != _functions[i].end();
        ++vit)
      vit->second->initialSetup();
  }

  _aux.compute(EXEC_TIMESTEP_BEGIN);

  Moose::setup_perf_log.push("Initial computeUserObjects()","Setup");
  computeUserObjects();
  computeUserObjects(EXEC_INITIAL);
  computeUserObjects(EXEC_TIMESTEP_BEGIN);
  computeUserObjects(EXEC_RESIDUAL);
  Moose::setup_perf_log.pop("Initial computeUserObjects()","Setup");


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

  // Here we will initialize the stateful properties once more since they may have been updated
  // during initialSetup by calls to computeProperties.
  if (_material_props.hasStatefulProperties())
  {
    if (isRestarting())
      _resurrector->restartStatefulMaterialProps();
    else
    {
      ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
      ComputeMaterialsObjectThread cmt(*this, _nl, _material_data, _bnd_material_data, _material_props, _bnd_material_props, _materials, _assembly);
      Threads::parallel_reduce(elem_range, cmt);
    }
  }


  for(unsigned int i=0; i<n_threads; i++)
  {
    _indicators[i].initialSetup();
    _markers[i].initialSetup();
  }
}

void FEProblem::timestepSetup()
{
  unsigned int n_threads = libMesh::n_threads();

  for(unsigned int i=0; i<n_threads; i++)
  {
    _materials[i].timestepSetup();

    for(std::map<std::string, Function *>::iterator vit = _functions[i].begin();
        vit != _functions[i].end();
        ++vit)
      vit->second->timestepSetup();
  }

  _aux.timestepSetup();
  _nl.timestepSetup();

  for(unsigned int i=0; i<n_threads; i++)
  {
    _indicators[i].timestepSetup();
    _markers[i].timestepSetup();
  }

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
FEProblem::prepareFace(const Elem * elem, THREAD_ID tid)
{
  _nl.prepareFace(tid);
  _aux.prepareFace(tid);

  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->prepareFace(_displaced_mesh->elem(elem->id()), tid);
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
  _second_zero[tid].resize(n_points, RealTensor(0.));

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
  _second_zero[tid].resize(n_points, RealTensor(0.));

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
  _second_zero[tid].resize(n_points, RealTensor(0.));

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
  _second_zero[tid].resize(n_points, RealTensor(0.));

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
  _second_zero[tid].resize(n_points, RealTensor(0.));

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
  _second_zero[tid].resize(n_points, RealTensor(0.));

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

  _assembly[tid]->reinitElemAndNeighbor(elem, side, neighbor);

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
  _assembly[tid]->reinitNeighborAtPhysical(neighbor, physical_points);

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

const std::vector<MooseObject *> &
FEProblem::getObjectsByName(const std::string & name, THREAD_ID tid)
{
  std::map<std::string, std::vector<MooseObject *> >::iterator it = _objects_by_name[tid].find(name);
  if (it != _objects_by_name[tid].end())
    return (*it).second;
  else
    mooseError("Unable to find objects with a given name.");
}

void
FEProblem::addFunction(std::string type, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    Function * func = static_cast<Function *>(Factory::instance()->create(type, name, parameters));
    _functions[tid][name] = func;
    _objects_by_name[tid][name].push_back(func);
  }
}

Function &
FEProblem::getFunction(const std::string & name, THREAD_ID tid)
{
  Function * function = _functions[tid][name];
  if (!function)
  {
    mooseError("Unable to find function " + name);
  }
  return *function;
}

void
FEProblem::addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< SubdomainID > * const active_subdomains/* = NULL*/)
{
  if (_aux.hasVariable(var_name))
    mooseError("Cannot have an auxiliary variable and a nonlinear variable with the same name!");

  if (_nl.hasVariable(var_name))
  {
    const Variable & var = _nl.sys().variable(_nl.sys().variable_number(var_name));
    if(var.type() != type)
      mooseError("Variable with name '" << var_name << "' already exists but is of a differing type!");

    return;
  }

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
  parameters.set<FEProblem *>("_fe_problem") = this;
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
  parameters.set<FEProblem *>("_fe_problem") = this;
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
  parameters.set<FEProblem *>("_fe_problem") = this;
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

  parameters.set<FEProblem *>("_fe_problem") = this;
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
  if (_nl.hasVariable(var_name))
    mooseError("Cannot have an auxiliary variable and a nonlinear variable with the same name!");

  if (_aux.hasVariable(var_name))
  {
    const Variable & var = _aux.sys().variable(_aux.sys().variable_number(var_name));
    if(var.type() != type)
      mooseError("AuxVariable with name '" << var_name << "' already exists but is of a differing type!");

    return;
  }


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
  parameters.set<FEProblem *>("_fe_problem") = this;
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
  parameters.set<FEProblem *>("_fe_problem") = this;
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
  parameters.set<FEProblem *>("_fe_problem") = this;
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
  parameters.set<FEProblem *>("_fe_problem") = this;
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
  parameters.set<FEProblem *>("_fe_problem") = this;
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
  const std::string & var_name = parameters.get<VariableName>("variable");

  parameters.set<FEProblem *>("_fe_problem") = this;
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
  parameters.set<FEProblem *>("_fe_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;
  parameters.set<SubProblem *>("_subproblem_displaced") = _displaced_problem;

  std::vector<SubdomainName> blocks = parameters.get<std::vector<SubdomainName> >("block");
  std::vector<SubdomainID> block_ids(blocks.size());

  std::vector<BoundaryName> boundaries = parameters.get<std::vector<BoundaryName> >("boundary");
  std::vector<BoundaryID> boundary_ids(boundaries.size());

  for (unsigned int i=0; i<blocks.size(); ++i)
    block_ids[i] = _mesh.getSubdomainID(blocks[i]);
  for (unsigned int i=0; i < boundaries.size(); ++i)
    boundary_ids[i] = _mesh.getBoundaryID(boundaries[i]);

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    if (blocks.size() > 0)
    {
      // volume material
      parameters.set<bool>("_bnd") = false;
      parameters.set<MaterialData *>("_material_data") = _material_data[tid];
      Material *volume_material = static_cast<Material *>(Factory::instance()->create(mat_name, name, parameters));
      mooseAssert(volume_material != NULL, "Not a Material object");
      _materials[tid].addMaterial(block_ids, volume_material);
      _objects_by_name[tid][name].push_back(volume_material);

      // face material
      parameters.set<bool>("_bnd") = true;
      parameters.set<MaterialData *>("_material_data") = _bnd_material_data[tid];
      Material *face_material = static_cast<Material *>(Factory::instance()->create(mat_name, name, parameters));
      mooseAssert(face_material != NULL, "Not a Material object");
      _materials[tid].addFaceMaterial(block_ids, face_material);
      _objects_by_name[tid][name].push_back(face_material);

      // neighbor material
      parameters.set<bool>("_bnd") = true;
      parameters.set<MaterialData *>("_material_data") = _neighbor_material_data[tid];
      Material *neighbor_material = static_cast<Material *>(Factory::instance()->create(mat_name, name, parameters));
      mooseAssert(neighbor_material != NULL, "Not a Material object");
      _materials[tid].addNeighborMaterial(block_ids, neighbor_material);
      _objects_by_name[tid][name].push_back(neighbor_material);
    }
    else if (boundaries.size() > 0)
    {
      parameters.set<bool>("_bnd") = true;
      parameters.set<MaterialData *>("_material_data") = _bnd_material_data[tid];
      Material *bnd_material = static_cast<Material *>(Factory::instance()->create(mat_name, name, parameters));
      mooseAssert(bnd_material != NULL, "Not a Material object");
      _materials[tid].addBoundaryMaterial(boundary_ids, bnd_material);
      _objects_by_name[tid][name].push_back(bnd_material);
    }
    else
      mooseError("Material '" + name + "' did not specify either block or boundary parameter");

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
  return _materials[tid].getFaceMaterials(block_id);
}

const std::vector<Material*> &
FEProblem::getBndMaterials(BoundaryID boundary_id, THREAD_ID tid)
{
  mooseAssert( tid < _materials.size(), "Requesting a material warehouse that does not exist");
  return _materials[tid].getBoundaryMaterials(boundary_id);
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
FEProblem::prepareMaterials(SubdomainID blk_id, THREAD_ID tid)
{
  if (_materials[tid].hasMaterials(blk_id))
  {
    std::set<MooseVariable *> needed_moose_vars;

    std::vector<Material *> & materials = _materials[tid].getMaterials(blk_id);

    for(std::vector<Material *>::iterator it = materials.begin();
        it != materials.end();
        ++it)
    {
      const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
      needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }

    const std::set<MooseVariable *> & current_active_elemental_moose_variables = getActiveElementalMooseVariables(tid);

    needed_moose_vars.insert(current_active_elemental_moose_variables.begin(), current_active_elemental_moose_variables.end());

    setActiveElementalMooseVariables(needed_moose_vars, tid);
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
  if (_materials[tid].hasFaceMaterials(blk_id))
  {
    const Elem * & elem = _assembly[tid]->elem();
    _bnd_material_data[tid]->reinit(_materials[tid].getFaceMaterials(blk_id), _assembly[tid]->qRuleFace()->n_points(), *elem, side);
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
FEProblem::reinitMaterialsBoundary(BoundaryID bnd_id, THREAD_ID tid)
{
  if (_materials[tid].hasBoundaryMaterials(bnd_id))
  {
    // this works b/c we called reinitElemFace before, so assembly has the right values
    const Elem * & elem = _assembly[tid]->elem();
    unsigned int side = _assembly[tid]->side();
    _bnd_material_data[tid]->reinit(_materials[tid].getBoundaryMaterials(bnd_id), _assembly[tid]->qRuleFace()->n_points(), *elem, side);
  }
}

/**
 * Small helper function used by addPostprocessor to try to get a Postprocessor pointer from a MooseObject
 */
Postprocessor *
getPostprocessorPointer(MooseObject * mo)
{
  {
    ElementPostprocessor * intermediate = dynamic_cast<ElementPostprocessor *>(mo);
    if(intermediate)
      return static_cast<Postprocessor *>(intermediate);
  }

  {
    NodalPostprocessor * intermediate = dynamic_cast<NodalPostprocessor *>(mo);
    if(intermediate)
      return static_cast<Postprocessor *>(intermediate);
  }

  {
    SidePostprocessor * intermediate = dynamic_cast<SidePostprocessor *>(mo);
    if(intermediate)
      return static_cast<Postprocessor *>(intermediate);
  }

  {
    GeneralPostprocessor * intermediate = dynamic_cast<GeneralPostprocessor *>(mo);
    if(intermediate)
      return static_cast<Postprocessor *>(intermediate);
  }

  return NULL;
}

void
FEProblem::addPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
  else
    parameters.set<SubProblem *>("_subproblem") = this;

  ExecFlagType type = Moose::stringToEnum<ExecFlagType>(parameters.get<MooseEnum>("execute_on"));

  // Check for name collision
  if (_user_objects(type)[0].getUserObjectByName(name))
    mooseError(std::string("A UserObject with the name \"") + name + "\" already exists.  You may not add a Postprocessor by the same name.");

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    // distinguish between side and the rest of PPs to provide the right material object
    if(parameters.have_parameter<std::vector<BoundaryName> >("boundary") && !parameters.have_parameter<bool>("block_restricted_nodal"))
    {
      if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
        _reinit_displaced_face = true;

      parameters.set<MaterialData *>("_material_data") = _bnd_material_data[tid];

      MooseObject * mo = Factory::instance()->create(pp_name, name, parameters);
      if(!mo)
        mooseError("Unable to determine type for Postprocessor: " + mo->name());

      Postprocessor * pp = getPostprocessorPointer(mo);
      _pps(type)[tid].addPostprocessor(pp);
      _objects_by_name[tid][name].push_back(mo);

      // Add it to the user object warehouse as well...
      {
        UserObject * user_object = dynamic_cast<UserObject *>(mo);
        if(!user_object)
          mooseError("Unknown user object type: " + pp_name);

        _user_objects(type)[tid].addUserObject(user_object);
      }
    }
    else
    {
      if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
        _reinit_displaced_elem = true;

      parameters.set<MaterialData *>("_material_data") = _material_data[tid];

      MooseObject * mo = Factory::instance()->create(pp_name, name, parameters);
      if(!mo)
        mooseError("Unable to determine type for Postprocessor: " + mo->name());

      Postprocessor * pp = getPostprocessorPointer(mo);
      _pps(type)[tid].addPostprocessor(pp);
      _objects_by_name[tid][name].push_back(mo);

      // Add it to the user object warehouse as well...
      {
        UserObject * user_object = dynamic_cast<UserObject *>(mo);
        if(!user_object)
          mooseError("Unknown user object type: " + pp_name);

        _user_objects(type)[tid].addUserObject(user_object);
      }
    }
  }
}

void
FEProblem::clearPostprocessorTables()
{
  // Clear the tables for cases when this routine is called multiple times by an executioner
  _pps_output_table_file.clear();
  _pps_output_table_screen.clear();
}

void
FEProblem::addUserObject(std::string user_object_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
  }

  ExecFlagType type = Moose::stringToEnum<ExecFlagType>(parameters.get<MooseEnum>("execute_on"));

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    // distinguish between side and the rest of USER_OBJECTs to provide the right material object
    if(parameters.have_parameter<std::vector<BoundaryName> >("boundary") && !parameters.have_parameter<bool>("block_restricted_nodal"))
    {
      if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
        _reinit_displaced_face = true;

      parameters.set<MaterialData *>("_material_data") = _bnd_material_data[tid];

      MooseObject * mo = Factory::instance()->create(user_object_name, name, parameters);

      UserObject * user_object = dynamic_cast<UserObject *>(mo);
      if(!user_object)
        mooseError("Unknown user object type: " + user_object_name);

      _user_objects(type)[tid].addUserObject(user_object);
      _objects_by_name[tid][name].push_back(mo);
    }
    else
    {
      if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
        _reinit_displaced_elem = true;

      parameters.set<MaterialData *>("_material_data") = _material_data[tid];
      MooseObject * mo = Factory::instance()->create(user_object_name, name, parameters);

      UserObject * user_object = dynamic_cast<UserObject *>(mo);
      if(!user_object)
        mooseError("Unknown user object type: " + user_object_name);

      _user_objects(type)[tid].addUserObject(user_object);
      _objects_by_name[tid][name].push_back(mo);
    }
  }
}

bool
FEProblem::hasUserObject(const std::string & name)
{
  ExecFlagType types[] = { EXEC_TIMESTEP, EXEC_TIMESTEP_BEGIN, EXEC_INITIAL, EXEC_JACOBIAN, EXEC_RESIDUAL, EXEC_CUSTOM };
  for (unsigned int i = 0; i < LENGTHOF(types); i++)
    if (_user_objects(types[i])[0].hasUserObject(name))
      return true;
  return false;
}

Real &
FEProblem::getPostprocessorValue(const std::string & name, THREAD_ID tid)
{
  return _pps_data[tid].getPostprocessorValue(name);
}

Real &
FEProblem::getPostprocessorValueOld(const std::string & name, THREAD_ID tid)
{
  return _pps_data[tid].getPostprocessorValueOld(name);
}

void
FEProblem::computeIndicatorsAndMarkers()
{
  // Zero them out first
  if(_indicators[0].all().size() || _markers[0].all().size())
  {
    std::vector<std::string> fields;

    // Add Indicator Fields
    {
      const std::vector<Indicator *> & all_indicators = _indicators[0].all();

      for(std::vector<Indicator *>::const_iterator i=all_indicators.begin();
          i != all_indicators.end();
          ++i)
        fields.push_back((*i)->name());
    }

    // Add Marker Fields
    {
      const std::vector<Marker *> & all_markers = _markers[0].all();

      for(std::vector<Marker *>::const_iterator i=all_markers.begin();
          i != all_markers.end();
          ++i)
        fields.push_back((*i)->name());
    }

    _aux.zeroVariables(fields);
  }

  // compute Indicators
  if(_indicators[0].all().size())
  {
    ComputeIndicatorThread cit(*this, getAuxiliarySystem(), _indicators);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cit);
    _aux.solution().close();
    _aux.update();
  }

  // finalize Indicators
  if(_indicators[0].all().size())
  {
    ComputeIndicatorThread cit(*this, getAuxiliarySystem(), _indicators, true);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cit);
    _aux.solution().close();
    _aux.update();
  }

  // compute Markers
  if(_markers[0].all().size())
  {
    _adaptivity.updateErrorVectors();

    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
      _markers[tid].markerSetup();

    ComputeMarkerThread cmt(*this, getAuxiliarySystem(), _markers);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cmt);

    _aux.solution().close();
    _aux.update();
  }
}

void FEProblem::computeUserObjectsInternal(std::vector<UserObjectWarehouse> & pps, UserObjectWarehouse::GROUP group)
{
  if (pps[0].blocks().size() > 0 || pps[0].boundaryIds().size() > 0 || pps[0].nodesetIds().size() > 0 || pps[0].blockNodalIds().size() > 0)
  {

    /* Note: The fact that we compute the aux system when we compute the user_objects
     * is a very bad behavior that some of our applications have come to rely on.  This
     * needs to be fixed.  For now we cannot easily change this behavior without
     * affecting a number of applications.  However when I added the nodal user_objects
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
    bool have_elemental_uo = false;
    bool have_side_uo = false;
    bool have_nodal_uo = false;
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      for (std::set<SubdomainID>::const_iterator block_it = pps[tid].blocks().begin();
          block_it != pps[tid].blocks().end();
          ++block_it)
      {
        SubdomainID block_id = *block_it;

        for (std::vector<ElementUserObject *>::const_iterator user_object_it = pps[tid].elementUserObjects(block_id, group).begin();
             user_object_it != pps[tid].elementUserObjects(block_id, group).end();
            ++user_object_it)
        {
          (*user_object_it)->initialize();
          have_elemental_uo = true;
        }
      }

      for (std::set<BoundaryID>::const_iterator boundary_it = pps[tid].boundaryIds().begin();
          boundary_it != pps[tid].boundaryIds().end();
          ++boundary_it)
      {
        //note: for threaded applications where the elements get broken up it
        //may be more efficient to initialize these on demand inside the loop
        for (std::vector<SideUserObject *>::const_iterator side_user_object_it = pps[tid].sideUserObjects(*boundary_it, group).begin();
             side_user_object_it != pps[tid].sideUserObjects(*boundary_it, group).end();
             ++side_user_object_it)
        {
          (*side_user_object_it)->initialize();
          have_side_uo = true;
        }
      }

      for (std::set<BoundaryID>::const_iterator boundary_it = pps[tid].nodesetIds().begin();
          boundary_it != pps[tid].nodesetIds().end();
          ++boundary_it)
      {
        for (std::vector<NodalUserObject *>::const_iterator nodal_user_object_it = pps[tid].nodalUserObjects(*boundary_it, group).begin();
             nodal_user_object_it != pps[tid].nodalUserObjects(*boundary_it, group).end();
             ++nodal_user_object_it)
        {
          (*nodal_user_object_it)->initialize();
          have_nodal_uo = true;
        }
      }

      // Block restricted nodal user objects
      for (std::set<SubdomainID>::const_iterator block_it = pps[tid].blockNodalIds().begin();
          block_it != pps[tid].blockNodalIds().end();
          ++block_it)
      {
        for (std::vector<NodalUserObject *>::const_iterator nodal_user_object_it = pps[tid].blockNodalUserObjects(*block_it, group).begin();
             nodal_user_object_it != pps[tid].blockNodalUserObjects(*block_it, group).end();
             ++nodal_user_object_it)
        {
          (*nodal_user_object_it)->initialize();
          have_nodal_uo = true;
        }
      }
    }

    // Store element user_objects values
    std::set<UserObject *> already_gathered;

    // compute
    if (have_elemental_uo || have_side_uo)
    {
      ComputeUserObjectsThread cppt(*this, getNonlinearSystem(), *getNonlinearSystem().currentSolution(), pps, group);
      Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cppt);

      for (std::set<SubdomainID>::const_iterator block_ids_it = pps[0].blocks().begin();
           block_ids_it != pps[0].blocks().end();
           ++block_ids_it)
      {
        SubdomainID block_id = *block_ids_it;

        const std::vector<ElementUserObject *> & element_user_objects = pps[0].elementUserObjects(block_id, group);
        // Store element user_objects values
        for (unsigned int i = 0; i < element_user_objects.size(); ++i)
        {
          ElementUserObject *ps = element_user_objects[i];
          std::string name = ps->name();

          // join across the threads (gather the value in thread #0)
          if (already_gathered.find(ps) == already_gathered.end())
          {
            for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
              ps->threadJoin(*pps[tid].elementUserObjects(block_id, group)[i]);

            ps->finalize();

            Postprocessor * pp = getPostprocessorPointer(ps);

            if(pp)
            {
              Real value = pp->getValue();
              // store the value in each thread

              for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
                _pps_data[tid].storeValue(name, value);
            }

            already_gathered.insert(ps);
          }
        }
      }

      // Store side user_objects values
      already_gathered.clear();
      for (std::set<BoundaryID>::const_iterator boundary_ids_it = pps[0].boundaryIds().begin();
           boundary_ids_it != pps[0].boundaryIds().end();
           ++boundary_ids_it)
      {
        BoundaryID boundary_id = *boundary_ids_it;

        const std::vector<SideUserObject *> & side_user_objects = pps[0].sideUserObjects(boundary_id, group);
        for (unsigned int i = 0; i < side_user_objects.size(); ++i)
        {
          SideUserObject *ps = side_user_objects[i];
          std::string name = ps->name();

          // join across the threads (gather the value in thread #0)
          if (already_gathered.find(ps) == already_gathered.end())
          {
            for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
              ps->threadJoin(*pps[tid].sideUserObjects(boundary_id, group)[i]);

            ps->finalize();

            Postprocessor * pp = getPostprocessorPointer(ps);

            if(pp)
            {
              Real value = pp->getValue();

              // store the value in each thread
              for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
                _pps_data[tid].storeValue(name, value);
            }

            already_gathered.insert(ps);
          }
        }
      }
    }

    // Don't waste time looping over nodes if there aren't any nodal user_objects to calculate
    if (have_nodal_uo)
    {
      ComputeNodalUserObjectsThread cnppt(*this, pps, group);
      Threads::parallel_reduce(*_mesh.getLocalNodeRange(), cnppt);

      // Store nodal user_objects values
      already_gathered.clear();
      for (std::set<BoundaryID>::const_iterator boundary_ids_it = pps[0].nodesetIds().begin();
           boundary_ids_it != pps[0].nodesetIds().end();
           ++boundary_ids_it)
      {
        BoundaryID boundary_id = *boundary_ids_it;

        const std::vector<NodalUserObject *> & nodal_user_objects = pps[0].nodalUserObjects(boundary_id, group);
        for (unsigned int i = 0; i < nodal_user_objects.size(); ++i)
        {
          NodalUserObject *ps = nodal_user_objects[i];
          std::string name = ps->name();

          // join across the threads (gather the value in thread #0)
          if (already_gathered.find(ps) == already_gathered.end())
          {
            for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
              ps->threadJoin(*pps[tid].nodalUserObjects(boundary_id, group)[i]);

            ps->finalize();

            Postprocessor * pp = getPostprocessorPointer(ps);

            if(pp)
            {
              Real value = pp->getValue();

              // store the value in each thread
              for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
                _pps_data[tid].storeValue(name, value);
            }

            already_gathered.insert(ps);
          }
        }
      }

      // Block restricted nodal user_objects
      for (std::set<SubdomainID>::const_iterator block_ids_it = pps[0].blockNodalIds().begin();
           block_ids_it != pps[0].blockNodalIds().end();
           ++block_ids_it)
      {
        SubdomainID block_id = *block_ids_it;

        const std::vector<NodalUserObject *> & nodal_user_objects = pps[0].blockNodalUserObjects(block_id, group);
        for (unsigned int i = 0; i < nodal_user_objects.size(); ++i)
        {
          NodalUserObject *ps = nodal_user_objects[i];
          std::string name = ps->name();

          // join across the threads (gather the value in thread #0)
          if (already_gathered.find(ps) == already_gathered.end())
          {
            for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
              ps->threadJoin(*pps[tid].blockNodalUserObjects(block_id, group)[i]);

            ps->finalize();

            Postprocessor * pp = getPostprocessorPointer(ps);

            if(pp)
            {
              Real value = pp->getValue();

              // store the value in each thread
              for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
                _pps_data[tid].storeValue(name, value);
            }

            already_gathered.insert(ps);
          }
        }
      }
    }
  }

  // Compute and store generic user_objects values
  for (std::vector<GeneralUserObject *>::const_iterator generic_user_object_it = pps[0].genericUserObjects(group).begin();
      generic_user_object_it != pps[0].genericUserObjects(group).end();
      ++generic_user_object_it)
  {
    std::string name = (*generic_user_object_it)->name();
    (*generic_user_object_it)->initialize();
    (*generic_user_object_it)->execute();

    (*generic_user_object_it)->finalize();

    Postprocessor * pp = getPostprocessorPointer(*generic_user_object_it);

    if(pp)
    {
      Real value = pp->getValue();

      // store the value in each thread
      for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
        _pps_data[tid].storeValue(name, value);
    }
  }
}

void
FEProblem::computeUserObjects(ExecFlagType type/* = EXEC_TIMESTEP*/, UserObjectWarehouse::GROUP group)
{
  Moose::perf_log.push("compute_user_objects()","Solve");

  switch (type)
  {
  case EXEC_RESIDUAL:
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
      _user_objects(type)[tid].residualSetup();
    break;

  case EXEC_JACOBIAN:
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
      _user_objects(type)[tid].jacobianSetup();
    break;

  case EXEC_TIMESTEP:
  case EXEC_TIMESTEP_BEGIN:
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
      _user_objects(type)[tid].timestepSetup();
    break;

  case EXEC_INITIAL:
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
      _user_objects(type)[tid].initialSetup();
    break;
  case EXEC_CUSTOM:
    break;
  }
  computeUserObjectsInternal(_user_objects(type), group);

  Moose::perf_log.pop("compute_user_objects()","Solve");
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
      std::string name = pps->PPName();
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
      case Moose::PPS_OUTPUT_AUTO:
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
  ExecFlagType types[] = { EXEC_TIMESTEP, EXEC_TIMESTEP_BEGIN, EXEC_INITIAL, EXEC_JACOBIAN, EXEC_RESIDUAL, EXEC_CUSTOM };
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
  parameters.set<FEProblem *>("_fe_problem") = this;
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

void
FEProblem::addIndicator(std::string indicator_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
  }

//    std::vector<SubdomainName> blocks = parameters.get<std::vector<SubdomainName> >("block");
//    std::vector<SubdomainID> block_ids(blocks.size());

//  this->addAuxVariable(parameters.get<std::string>("field_name"), FEType(CONSTANT, MONOMIAL) );

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    parameters.set<MaterialData *>("_material_data") = _material_data[tid];

    //if(dynamic_cast<InternalSideIndicator*>(indicator))
//    {
      parameters.set<MaterialData *>("_material_data") = _bnd_material_data[tid];
      parameters.set<MaterialData *>("_neighbor_material_data") = _neighbor_material_data[tid];
//    }

    Indicator *indicator = static_cast<Indicator *>(Factory::instance()->create(indicator_name, name, parameters) );
    mooseAssert(indicator != NULL, "Not a Indicator object");



    /*    std::set<SubdomainID> blk_ids;
          if (!parameters.isParamValid("block"))
          blk_ids = _var_map[indicator->variable().number()];
          else
          {
          std::vector<SubdomainName> blocks = parameters.get<std::vector<SubdomainName> >("block");
          for (unsigned int i=0; i<blocks.size(); ++i)
          {
          SubdomainID blk_id = _mesh.getSubdomainID(blocks[i]);

          if (_var_map[indicator->variable().number()].count(blk_id) > 0 || _var_map[indicator->variable().number()].size() == 0)
          blk_ids.insert(blk_id);
          else
          mooseError("indicator (" + indicator->name() + "): block outside of the domain of the variable");
          }
          }*/
    std::vector<SubdomainID> block_ids;
    _indicators[tid].addIndicator(indicator, block_ids);

    _objects_by_name[tid][name].push_back(indicator);
  }
}

void
FEProblem::addMarker(std::string marker_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
  }

//    std::vector<SubdomainName> blocks = parameters.get<std::vector<SubdomainName> >("block");
//    std::vector<SubdomainID> block_ids(blocks.size());

//  this->addAuxVariable(parameters.get<std::string>("field_name"), FEType(CONSTANT, MONOMIAL) );

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    Marker *marker = static_cast<Marker *>(Factory::instance()->create(marker_name, name, parameters) );
    mooseAssert(marker != NULL, "Not a Marker object");


    /*    std::set<SubdomainID> blk_ids;
          if (!parameters.isParamValid("block"))
          blk_ids = _var_map[indicator->variable().number()];
          else
          {
          std::vector<SubdomainName> blocks = parameters.get<std::vector<SubdomainName> >("block");
          for (unsigned int i=0; i<blocks.size(); ++i)
          {
          SubdomainID blk_id = _mesh.getSubdomainID(blocks[i]);

          if (_var_map[indicator->variable().number()].count(blk_id) > 0 || _var_map[indicator->variable().number()].size() == 0)
          blk_ids.insert(blk_id);
          else
          mooseError("indicator (" + indicator->name() + "): block outside of the domain of the variable");
          }
          }*/
    std::vector<SubdomainID> block_ids;
    _markers[tid].addMarker(marker, block_ids);

    _objects_by_name[tid][name].push_back(marker);
  }
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

  if (n_vars + n_scalar_vars == 0)
    mooseError("No variables specified in the FEProblem '" << name() << "'.");

  Moose::setup_perf_log.push("eq.init()","Setup");
  _eq.init();
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

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    _pps_data[tid].copyValuesBack();
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
FEProblem::computeResidual(NonlinearImplicitSystem &/*sys*/, const NumericVector<Number> & soln, NumericVector<Number> & residual)
{
  computeResidualType(soln, residual, Moose::KT_ALL);
}

void
FEProblem::computeTransientImplicitResidual(Real time, const NumericVector<Number>& u, const NumericVector<Number>& udot, NumericVector<Number>& residual)
{
  _nl.setSolutionUDot(udot);
  NonlinearImplicitSystem &sys = _nl.sys();
  _time = time;
  computeResidual(sys,u,residual);
}

void
FEProblem::computeTransientImplicitJacobian(Real time, const NumericVector<Number>& u, const NumericVector<Number>& udot, Real shift, SparseMatrix<Number> &jacobian)
{
  if (0)
  { // The current interface guarantees that the residual is called before Jacobian, thus udot has already been set
    _nl.setSolutionUDot(udot);
  }
  _nl.setSolutionDuDotDu(shift);
  NonlinearImplicitSystem &sys = _nl.sys();
  _time = time;
  computeJacobian(sys,u,jacobian);
}

void
FEProblem::computeResidualType( const NumericVector<Number>& soln, NumericVector<Number>& residual, Moose::KernelType type)
{
  _nl.set_solution(soln);

  _nl.zeroVariablesForResidual();
  _aux.zeroVariablesForResidual();

  computeUserObjects(EXEC_RESIDUAL);

  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh(soln, *_aux.currentSolution());

  unsigned int n_threads = libMesh::n_threads();

  for(unsigned int i=0; i<n_threads; i++)
  {
    _materials[i].residualSetup();

    for(std::map<std::string, Function *>::iterator vit = _functions[i].begin();
        vit != _functions[i].end();
        ++vit)
      vit->second->initialSetup();
  }
  _aux.residualSetup();

  _aux.compute();
  _nl.computeResidual(residual, type);

  // Need to close and update the aux system in case residuals were saved to it.
  _aux.solution().close();
  _aux.update();

  if (_dbg_top_residuals)
    _nl.printTopResiduals(residual, _dbg_top_residuals);
}

void
FEProblem::computeJacobian(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, SparseMatrix<Number> & jacobian)
{
  _nl.set_solution(soln);

  _nl.zeroVariablesForJacobian();
  _aux.zeroVariablesForJacobian();

  computeUserObjects(EXEC_JACOBIAN);

  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh(soln, *_aux.currentSolution());

  unsigned int n_threads = libMesh::n_threads();

  for(unsigned int i=0; i<n_threads; i++)
  {
    _materials[i].jacobianSetup();

    for(std::map<std::string, Function *>::iterator vit = _functions[i].begin();
        vit != _functions[i].end();
        ++vit)
      vit->second->jacobianSetup();
  }

  _aux.jacobianSetup();

  // TODO: This can be made more efficient if we group the kernels together in a single group to be
  //       executed.  If the user has both Residual and Jacobian aux kernels, we are looping over both
  //       groups separately.
  _aux.compute();
  _aux.compute(EXEC_JACOBIAN);

  _nl.computeJacobian(jacobian);

  // This call is here to make sure the residual vector is up to date with any decisions that have been made in
  // the Jacobian evaluation.  That is important in JFNK because that residual is used for finite differencing
  computeResidual(sys, soln, *sys.rhs);
  sys.rhs->close();
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
  if(!_nl.hasVector("lower_bound") || !_nl.hasVector("upper_bound")) return;
  try
  {
    NumericVector<Number> & _lower = _nl.getVector("lower_bound");
    NumericVector<Number> & _upper = _nl.getVector("upper_bound");
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
  catch (MooseException & e)
  {
    // tell solver to stop
#ifdef LIBMESH_HAVE_PETSC
#if PETSC_VERSION_LESS_THAN(3,0,0)
#else
    PetscNonlinearSolver<Real> & solver = static_cast<PetscNonlinearSolver<Real> &>(*_nl.sys().nonlinear_solver);
    SNESSetFunctionDomainError(solver.snes());
#endif
#endif
  }
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

bool
FEProblem::shouldUpdateSolution()
{
  return false;
}

bool
FEProblem::updateSolution(NumericVector<Number>& /*vec_solution*/, NumericVector<Number>& /*ghosted_solution*/)
{
  return false;
}

void
FEProblem::predictorCleanup(NumericVector<Number>& /*ghosted_solution*/)
{}

void
FEProblem::initDisplacedProblem(MooseMesh * displaced_mesh, InputParameters params)
{
  if (displaced_mesh == NULL)
    mooseError("Trying to set displaced mesh to NULL");
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

  _resurrector->write();
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

    // Check that material properties exist when requested by other properties on a given block
    _materials[0].checkMaterialDependSanity();
  }

  // Check that BCs used in your simulation exist in your mesh
  _nl.checkBCCoverage();

  // Check UserObjects and Postprocessors
  checkUserObjects();

  // Verify that we don't have any Element type/Coordinate Type conflicts
  checkCoordinateSystems();
}

void
FEProblem::checkUserObjects()
{
  // Check user_objects block coverage
  std::set<SubdomainID> mesh_subdomains = _mesh.meshSubdomains();
  std::set<SubdomainID> user_objects_blocks;

  // gather names of all user_objects that were defined in the input file
  // and the blocks that they are defined on
  std::set<std::string> names;
  ExecFlagType types[] = { EXEC_INITIAL, EXEC_RESIDUAL, EXEC_JACOBIAN, EXEC_TIMESTEP, EXEC_TIMESTEP_BEGIN, EXEC_CUSTOM };
  for (unsigned int i = 0; i < LENGTHOF(types); i++)
  {
    for (std::vector<UserObject *>::const_iterator it = _user_objects(types[i])[0].all().begin(); it != _user_objects(types[i])[0].all().end(); ++it)
      names.insert((*it)->name());

    user_objects_blocks.insert(_user_objects(types[i])[0].blocks().begin(), _user_objects(types[i])[0].blocks().end());
  }

  // See if all referenced blocks are covered
  mesh_subdomains.insert(Moose::ANY_BLOCK_ID);
  std::set<SubdomainID> difference;
  std::set_difference(user_objects_blocks.begin(), user_objects_blocks.end(), mesh_subdomains.begin(), mesh_subdomains.end(),
                      std::inserter(difference, difference.end()));

  if (!difference.empty())
  {
    std::ostringstream oss;
    oss << "One or more UserObjects is referencing a nonexistent block:\n";
    for (std::set<SubdomainID>::iterator i = difference.begin(); i != difference.end();  ++i)
      oss << *i << "\n";
    mooseError(oss.str());
  }

  // check that all requested UserObjects were defined in the input file
  for (std::map<std::string, PostprocessorValue>::const_iterator it = _pps_data[0].values().begin(); it != _pps_data[0].values().end(); ++it)
  {
    if (names.find(it->first) == names.end())
      mooseError("Postprocessor '" + it->first + "' requested but not specified in the input file.");
  }

  // check to see if we have inconsistent output requests
  for (unsigned int i = 0; i < LENGTHOF(types); i++)
  {
    for (std::vector<Postprocessor *>::const_iterator it = _pps(types[i])[0].all().begin(); it != _pps(types[i])[0].all().end(); ++it)
    {
      Moose::PPSOutputType out_type = (*it)->getOutput();

      if ((out_type == Moose::PPS_OUTPUT_FILE || out_type == Moose::PPS_OUTPUT_BOTH) && _out.PpsFileOutputEnabled() == false)
        mooseWarning("Postprocessor file output has been requested, but there are no file formats enabled that support this feature.");
      else if ((out_type == Moose::PPS_OUTPUT_SCREEN || out_type == Moose::PPS_OUTPUT_BOTH) && _postprocessor_screen_output == false)
        mooseWarning("Postprocessor screen output has been requested, but it has been turned off.");
    }
  }
}

void
FEProblem::checkCoordinateSystems()
{
  MeshBase::const_element_iterator it = _mesh._mesh.elements_begin();
  MeshBase::const_element_iterator it_end = _mesh._mesh.elements_end();

  for ( ; it != it_end; ++it)
  {
    SubdomainID sid = (*it)->subdomain_id();
    if (_coord_sys[sid] == Moose::COORD_RZ && (*it)->dim() == 3)
      mooseError("An RZ coordinate system was requested for subdomain " + Moose::stringify(sid) + " which contains 3D elements.");
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
  _resurrector->setRestartFile(file_name);
}

void
FEProblem::setNumRestartFiles(unsigned int num_files)
{
  _resurrector->setNumRestartFiles(num_files);
}

unsigned int
FEProblem::getNumRestartFiles()
{
  return _resurrector->getNumRestartFiles();
}

bool
FEProblem::isRestarting()
{
  return _resurrector->isOn();
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

MooseNonlinearConvergenceReason
FEProblem::checkNonlinearConvergence(std::string &msg,
                                     const int it,
                                     const Real xnorm,
                                     const Real snorm,
                                     const Real fnorm,
                                     Real &ttol,
                                     const Real rtol,
                                     const Real stol,
                                     const Real abstol,
                                     const int nfuncs,
                                     const int max_funcs,
                                     const Real ref_resid,
                                     const Real div_threshold)
{
  NonlinearSystem & system = getNonlinearSystem();
  MooseNonlinearConvergenceReason reason = MOOSE_ITERATING;
  std::stringstream oss;

  if (!it)
  {
    // set parameter for default relative tolerance convergence test
    ttol = ref_resid*rtol;
  }
  if (fnorm != fnorm)
  {
    oss << "Failed to converge, function norm is NaN\n";
    reason = MOOSE_DIVERGED_FNORM_NAN;
  }
  else if (fnorm < abstol)
  {
    oss << "Converged due to function norm " << fnorm << " < " << abstol << std::endl;
    reason = MOOSE_CONVERGED_FNORM_ABS;
  }
  else if (nfuncs >= max_funcs)
  {
    oss << "Exceeded maximum number of function evaluations: " << nfuncs << " > " << max_funcs << std::endl;
    reason = MOOSE_DIVERGED_FUNCTION_COUNT;
  }
  else if(it &&
          fnorm > system._last_nl_rnorm &&
          fnorm >= div_threshold)
  {
    oss << "Nonlinear solve was blowing up!" << std::endl;
    reason = MOOSE_DIVERGED_LINE_SEARCH;
  }

  if (it && !reason)
  {
    if (fnorm <= ref_resid*rtol)
    {
      oss << "Converged due to function norm " << fnorm << " < " << " (relative tolerance)" << std::endl;
      reason = MOOSE_CONVERGED_FNORM_RELATIVE;
    }
    else if (snorm < stol*xnorm)
    {
      oss << "Converged due to small update length: " << snorm << " < " << stol << " * " << xnorm << std::endl;
      reason = MOOSE_CONVERGED_SNORM_RELATIVE;
    }
  }

  system._last_nl_rnorm = fnorm;
  system._current_nl_its = it;

  msg = oss.str();

  return(reason);
}

#ifdef LIBMESH_HAVE_PETSC
void
FEProblem::storePetscOptions(const std::vector<std::string> & petsc_options,
                             const std::vector<std::string> & petsc_options_inames,
                             const std::vector<std::string> & petsc_options_values)
{
  std::vector<std::string> & po = parameters().set<std::vector<std::string> >("petsc_options");         // set because we need a writable reference
  for (unsigned int i = 0; i < petsc_options.size(); i++)
    if (find(po.begin(), po.end(), petsc_options[i]) == po.end())
      po.push_back(petsc_options[i]);

  std::vector<std::string> & pn = parameters().set<std::vector<std::string> >("petsc_inames");         // set because we need a writable reference
  std::vector<std::string> & pv = parameters().set<std::vector<std::string> >("petsc_values");         // set because we need a writable reference
  for (unsigned int i = 0; i < petsc_options_inames.size(); i++)
  {
    if (find(pn.begin(), pn.end(), petsc_options_inames[i]) == pn.end())
    {
      pn.push_back(petsc_options_inames[i]);
      pv.push_back(petsc_options_values[i]);
    }
    else
    {
      for (unsigned int j = 0; j < pn.size(); j++)
        if (pn[j] == petsc_options_inames[i])
          pv[j] = petsc_options_values[i];
    }
  }
}
#endif
