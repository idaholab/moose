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

// MOOSE includes
#include "DisplacedProblem.h"
#include "Problem.h"
#include "SubProblem.h"
#include "UpdateDisplacedMeshThread.h"
#include "ResetDisplacedMeshThread.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"

// libMesh includes
#include "libmesh/numeric_vector.h"

template<>
InputParameters validParams<DisplacedProblem>()
{
  InputParameters params = validParams<SubProblem>();
  params.addPrivateParam<std::vector<std::string> >("displacements");
  return params;
}

DisplacedProblem::DisplacedProblem(const InputParameters & parameters) :
    SubProblem(parameters),
    _mproblem(parameters.have_parameter<FEProblemBase *>("_fe_problem_base") ? *getParam<FEProblemBase *>("_fe_problem_base") : *getParam<FEProblem *>("_fe_problem")),
    _mesh(*getParam<MooseMesh *>("mesh")),
    _eq(_mesh),
    _ref_mesh(_mproblem.mesh()),
    _displacements(getParam<std::vector<std::string> >("displacements")),
    _displaced_nl(*this, _mproblem.getNonlinearSystemBase(), _mproblem.getNonlinearSystemBase().name() + "_displaced", Moose::VAR_NONLINEAR),
    _displaced_aux(*this, _mproblem.getAuxiliarySystem(), _mproblem.getAuxiliarySystem().name() + "_displaced", Moose::VAR_AUXILIARY),
    _geometric_search_data(_mproblem, _mesh)
{
  // TODO: Move newAssemblyArray further up to SubProblem so that we can use it here
  unsigned int n_threads = libMesh::n_threads();

  _assembly.resize(n_threads);
  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i] = new Assembly(_displaced_nl, i);
}

DisplacedProblem::~DisplacedProblem()
{
  for (unsigned int i = 0; i < libMesh::n_threads(); ++i)
    delete _assembly[i];
}

bool
DisplacedProblem::isTransient() const
{
  return _mproblem.isTransient();
}

Moose::CoordinateSystemType
DisplacedProblem::getCoordSystem(SubdomainID sid)
{
  return _mproblem.getCoordSystem(sid);
}

std::set<dof_id_type> &
DisplacedProblem::ghostedElems()
{
  return _mproblem.ghostedElems();
}

void
DisplacedProblem::createQRules(QuadratureType type, Order order, Order volume_order, Order face_order)
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    _assembly[tid]->createQRules(type, order, volume_order, face_order);
}

void
DisplacedProblem::useFECache(bool fe_cache)
{
  unsigned int n_threads = libMesh::n_threads();

  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i]->useFECache(fe_cache); // fe caching is turned off for now for the displaced system.
}

void
DisplacedProblem::init()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    _assembly[tid]->init(_mproblem.couplingMatrix());

  _displaced_nl.dofMap().attach_extra_send_list_function(&extraSendList, &_displaced_nl);
  _displaced_aux.dofMap().attach_extra_send_list_function(&extraSendList, &_displaced_aux);

  _displaced_nl.init();
  _displaced_aux.init();

  Moose::perf_log.push("DisplacedProblem::init::eq.init()", "Setup");
  _eq.init();
  Moose::perf_log.pop("DisplacedProblem::init::eq.init()", "Setup");

  Moose::perf_log.push("DisplacedProblem::init::meshChanged()", "Setup");
  _mesh.meshChanged();
  Moose::perf_log.pop("DisplacedProblem::init::meshChanged()", "Setup");
}

void
DisplacedProblem::initAdaptivity()
{
}

void
DisplacedProblem::saveOldSolutions()
{
  _displaced_nl.saveOldSolutions();
  _displaced_aux.saveOldSolutions();
}

void
DisplacedProblem::restoreOldSolutions()
{
  _displaced_nl.restoreOldSolutions();
  _displaced_aux.restoreOldSolutions();
}

void
DisplacedProblem::syncSolutions()
{
  (*_displaced_nl.sys().solution) = *_mproblem.getNonlinearSystemBase().currentSolution();
  (*_displaced_aux.sys().solution) = *_mproblem.getAuxiliarySystem().currentSolution();
  _displaced_nl.update();
  _displaced_aux.update();
}

void
DisplacedProblem::syncSolutions(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln)
{
  (*_displaced_nl.sys().solution) = soln;
  (*_displaced_aux.sys().solution) = aux_soln;
  _displaced_nl.update();
  _displaced_aux.update();
}

void
DisplacedProblem::updateMesh()
{
  Moose::perf_log.push("updateDisplacedMesh()", "Execution");

  unsigned int n_threads = libMesh::n_threads();

  syncSolutions();

  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i]->invalidateCache();

  _nl_solution = _mproblem.getNonlinearSystemBase().currentSolution();
  _aux_solution = _mproblem.getAuxiliarySystem().currentSolution();

  UpdateDisplacedMeshThread udmt(_mproblem, *this);

  Threads::parallel_reduce(*_mesh.getActiveSemiLocalNodeRange(), udmt);

  // Update the geometric searches that depend on the displaced mesh
  _geometric_search_data.update();

  // Since the Mesh changed, update the PointLocator object used by DiracKernels.
  _dirac_kernel_info.updatePointLocator(_mesh);

  Moose::perf_log.pop("updateDisplacedMesh()", "Execution");
}

void
DisplacedProblem::updateMesh(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln)
{
  Moose::perf_log.push("updateDisplacedMesh()", "Execution");

  unsigned int n_threads = libMesh::n_threads();

  syncSolutions(soln, aux_soln);

  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i]->invalidateCache();

  _nl_solution = &soln;
  _aux_solution = &aux_soln;

  UpdateDisplacedMeshThread udmt(_mproblem, *this);

  Threads::parallel_reduce(*_mesh.getActiveSemiLocalNodeRange(), udmt);

  // Update the geometric searches that depend on the displaced mesh
  _geometric_search_data.update();

  // Since the Mesh changed, update the PointLocator object used by DiracKernels.
  _dirac_kernel_info.updatePointLocator(_mesh);

  Moose::perf_log.pop("updateDisplacedMesh()", "Execution");
}

bool
DisplacedProblem::hasVariable(const std::string & var_name)
{
  if (_displaced_nl.hasVariable(var_name))
    return true;
  else if (_displaced_aux.hasVariable(var_name))
    return true;
  else
    return false;
}

MooseVariable &
DisplacedProblem::getVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_displaced_nl.hasVariable(var_name))
    return _displaced_nl.getVariable(tid, var_name);
  else if (!_displaced_aux.hasVariable(var_name))
    mooseError("No variable with name '" + var_name + "'");

  return _displaced_aux.getVariable(tid, var_name);
}

bool
DisplacedProblem::hasScalarVariable(const std::string & var_name)
{
  if (_displaced_nl.hasScalarVariable(var_name))
    return true;
  else if (_displaced_aux.hasScalarVariable(var_name))
    return true;
  else
    return false;
}

MooseVariableScalar &
DisplacedProblem::getScalarVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_displaced_nl.hasScalarVariable(var_name))
    return _displaced_nl.getScalarVariable(tid, var_name);
  else if (_displaced_aux.hasScalarVariable(var_name))
    return _displaced_aux.getScalarVariable(tid, var_name);
  else
    mooseError("No variable with name '" + var_name + "'");
}

void
DisplacedProblem::addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< SubdomainID > * const active_subdomains)
{
  _displaced_nl.addVariable(var_name, type, scale_factor, active_subdomains);
}

void
DisplacedProblem::addAuxVariable(const std::string & var_name, const FEType & type, const std::set< SubdomainID > * const active_subdomains)
{
  _displaced_aux.addVariable(var_name, type, 1.0, active_subdomains);
}

void
DisplacedProblem::addScalarVariable(const std::string & var_name, Order order, Real scale_factor, const std::set< SubdomainID > * const active_subdomains)
{
  _displaced_nl.addScalarVariable(var_name, order, scale_factor, active_subdomains);
}

void
DisplacedProblem::addAuxScalarVariable(const std::string & var_name, Order order, Real scale_factor, const std::set< SubdomainID > * const active_subdomains)
{
  _displaced_aux.addScalarVariable(var_name, order, scale_factor, active_subdomains);
}

void
DisplacedProblem::prepare(const Elem * elem, THREAD_ID tid)
{
  _assembly[tid]->reinit(elem);

  _displaced_nl.prepare(tid);
  _displaced_aux.prepare(tid);
  _assembly[tid]->prepare();
}

void
DisplacedProblem::prepareNonlocal(THREAD_ID tid)
{
  _assembly[tid]->prepareNonlocal();
}

void
DisplacedProblem::prepareFace(const Elem * /*elem*/, THREAD_ID tid)
{
  _displaced_nl.prepareFace(tid, true);
  _displaced_aux.prepareFace(tid, false);
}

void
DisplacedProblem::prepare(const Elem * elem, unsigned int ivar, unsigned int jvar, const std::vector<dof_id_type> & dof_indices, THREAD_ID tid)
{
  _assembly[tid]->reinit(elem);

  _displaced_nl.prepare(tid);
  _displaced_aux.prepare(tid);
  _assembly[tid]->prepareBlock(ivar, jvar, dof_indices);
}

void
DisplacedProblem::prepareBlockNonlocal(unsigned int ivar, unsigned int jvar, const std::vector<dof_id_type> & idof_indices, const std::vector<dof_id_type> & jdof_indices, THREAD_ID tid)
{
  _assembly[tid]->prepareBlockNonlocal(ivar, jvar, idof_indices, jdof_indices);
}

void
DisplacedProblem::prepareAssembly(THREAD_ID tid)
{
  _assembly[tid]->prepare();
}

void
DisplacedProblem::prepareAssemblyNeighbor(THREAD_ID tid)
{
  _assembly[tid]->prepareNeighbor();
}

bool
DisplacedProblem::reinitDirac(const Elem * elem, THREAD_ID tid)
{
  std::vector<Point> & points = _dirac_kernel_info.getPoints()[elem].first;

  unsigned int n_points = points.size();

  if (n_points)
  {
    _assembly[tid]->reinitAtPhysical(elem, points);

    _displaced_nl.prepare(tid);
    _displaced_aux.prepare(tid);

    reinitElem(elem, tid);
  }

  _assembly[tid]->prepare();

  return n_points > 0;
}


void
DisplacedProblem::reinitElem(const Elem * elem, THREAD_ID tid)
{
  _displaced_nl.reinitElem(elem, tid);
  _displaced_aux.reinitElem(elem, tid);
}

void
DisplacedProblem::reinitElemPhys(const Elem * elem, std::vector<Point> phys_points_in_elem, THREAD_ID tid)
{
  std::vector<Point> points(phys_points_in_elem.size());
  std::copy(phys_points_in_elem.begin(), phys_points_in_elem.end(), points.begin());

  _assembly[tid]->reinitAtPhysical(elem, points);

  _displaced_nl.prepare(tid);
  _displaced_aux.prepare(tid);
  _assembly[tid]->prepare();

  reinitElem(elem, tid);
}


void
DisplacedProblem::reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid)
{
  _assembly[tid]->reinit(elem, side);
  _displaced_nl.reinitElemFace(elem, side, bnd_id, tid);
  _displaced_aux.reinitElemFace(elem, side, bnd_id, tid);
}

void
DisplacedProblem::reinitNode(const Node * node, THREAD_ID tid)
{
  _assembly[tid]->reinit(node);
  _displaced_nl.reinitNode(node, tid);
  _displaced_aux.reinitNode(node, tid);
}

void
DisplacedProblem::reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid)
{
  _assembly[tid]->reinit(node);
  _displaced_nl.reinitNodeFace(node, bnd_id, tid);
  _displaced_aux.reinitNodeFace(node, bnd_id, tid);
}

void
DisplacedProblem::reinitNodes(const std::vector<dof_id_type> & nodes, THREAD_ID tid)
{
  _displaced_nl.reinitNodes(nodes, tid);
  _displaced_aux.reinitNodes(nodes, tid);
}

void
DisplacedProblem::reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, THREAD_ID tid)
{
  _displaced_nl.reinitNodesNeighbor(nodes, tid);
  _displaced_aux.reinitNodesNeighbor(nodes, tid);
}

void
DisplacedProblem::reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid)
{
  const Elem * neighbor = elem->neighbor(side);
  unsigned int neighbor_side = neighbor->which_neighbor_am_i(elem);

  _assembly[tid]->reinitElemAndNeighbor(elem, side, neighbor, neighbor_side);

  _displaced_nl.prepareNeighbor(tid);
  _displaced_aux.prepareNeighbor(tid);

  _assembly[tid]->prepareNeighbor();

  BoundaryID bnd_id = 0;              // some dummy number (it is not really used for anything, right now)
  _displaced_nl.reinitElemFace(elem, side, bnd_id, tid);
  _displaced_aux.reinitElemFace(elem, side, bnd_id, tid);

  _displaced_nl.reinitNeighborFace(neighbor, neighbor_side, bnd_id, tid);
  _displaced_aux.reinitNeighborFace(neighbor, neighbor_side, bnd_id, tid);
}

void
DisplacedProblem::reinitNeighborPhys(const Elem * neighbor, unsigned int neighbor_side, const std::vector<Point> & physical_points, THREAD_ID tid)
{
  // Reinit shape functions
  _assembly[tid]->reinitNeighborAtPhysical(neighbor, neighbor_side, physical_points);

  // Set the neighbor dof indices
  _displaced_nl.prepareNeighbor(tid);
  _displaced_aux.prepareNeighbor(tid);

  prepareAssemblyNeighbor(tid);

  // Compute values at the points
  _displaced_nl.reinitNeighborFace(neighbor, neighbor_side, 0, tid);
  _displaced_aux.reinitNeighborFace(neighbor, neighbor_side, 0, tid);
}

void
DisplacedProblem::reinitNeighborPhys(const Elem * neighbor, const std::vector<Point> & physical_points, THREAD_ID tid)
{
  // Reinit shape functions
  _assembly[tid]->reinitNeighborAtPhysical(neighbor, physical_points);

  // Set the neighbor dof indices
  _displaced_nl.prepareNeighbor(tid);
  _displaced_aux.prepareNeighbor(tid);

  prepareAssemblyNeighbor(tid);

  // Compute values at the points
  _displaced_nl.reinitNeighbor(neighbor, tid);
  _displaced_aux.reinitNeighbor(neighbor, tid);
}

void
DisplacedProblem::reinitNodeNeighbor(const Node * node, THREAD_ID tid)
{
  _assembly[tid]->reinitNodeNeighbor(node);
  _displaced_nl.reinitNodeNeighbor(node, tid);
  _displaced_aux.reinitNodeNeighbor(node, tid);
}

void
DisplacedProblem::reinitScalars(THREAD_ID tid)
{
  _displaced_nl.reinitScalars(tid);
  _displaced_aux.reinitScalars(tid);
}

void
DisplacedProblem::reinitOffDiagScalars(THREAD_ID tid)
{
  _assembly[tid]->prepareOffDiagScalar();
}

void
DisplacedProblem::getDiracElements(std::set<const Elem *> & elems)
{
  elems = _dirac_kernel_info.getElements();
}

void
DisplacedProblem::clearDiracInfo()
{
  _dirac_kernel_info.clearPoints();
}

void
DisplacedProblem::addResidual(THREAD_ID tid)
{
  _assembly[tid]->addResidual(_mproblem.residualVector(Moose::KT_TIME), Moose::KT_TIME);
  _assembly[tid]->addResidual(_mproblem.residualVector(Moose::KT_NONTIME), Moose::KT_NONTIME);
}

void
DisplacedProblem::addResidualNeighbor(THREAD_ID tid)
{
  _assembly[tid]->addResidualNeighbor(_mproblem.residualVector(Moose::KT_TIME), Moose::KT_TIME);
  _assembly[tid]->addResidualNeighbor(_mproblem.residualVector(Moose::KT_NONTIME), Moose::KT_NONTIME);
}

void
DisplacedProblem::cacheResidual(THREAD_ID tid)
{
  _assembly[tid]->cacheResidual();
}

void
DisplacedProblem::cacheResidualNeighbor(THREAD_ID tid)
{
  _assembly[tid]->cacheResidualNeighbor();
}

void
DisplacedProblem::addCachedResidual(THREAD_ID tid)
{
  _assembly[tid]->addCachedResidual(_mproblem.residualVector(Moose::KT_TIME), Moose::KT_TIME);
  _assembly[tid]->addCachedResidual(_mproblem.residualVector(Moose::KT_NONTIME), Moose::KT_NONTIME);
}

void
DisplacedProblem::addCachedResidualDirectly(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->addCachedResidual(residual, Moose::KT_TIME);
  _assembly[tid]->addCachedResidual(residual, Moose::KT_NONTIME);
}

void
DisplacedProblem::setResidual(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->setResidual(residual);
}

void
DisplacedProblem::setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->setResidualNeighbor(residual);
}

void
DisplacedProblem::addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _assembly[tid]->addJacobian(jacobian);
}

void
DisplacedProblem::addJacobianNonlocal(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _assembly[tid]->addJacobianNonlocal(jacobian);
}


void
DisplacedProblem::addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _assembly[tid]->addJacobianNeighbor(jacobian);
}

void
DisplacedProblem::cacheJacobian(THREAD_ID tid)
{
  _assembly[tid]->cacheJacobian();
}

void
DisplacedProblem::cacheJacobianNonlocal(THREAD_ID tid)
{
  _assembly[tid]->cacheJacobianNonlocal();
}

void
DisplacedProblem::cacheJacobianNeighbor(THREAD_ID tid)
{
  _assembly[tid]->cacheJacobianNeighbor();
}

void
DisplacedProblem::addCachedJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _assembly[tid]->addCachedJacobian(jacobian);
}

void
DisplacedProblem::addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<dof_id_type> & dof_indices, THREAD_ID tid)
{
  _assembly[tid]->addJacobianBlock(jacobian, ivar, jvar, dof_map, dof_indices);
}

void
DisplacedProblem::addJacobianBlockNonlocal(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, const std::vector<dof_id_type> & idof_indices, const std::vector<dof_id_type> & jdof_indices, THREAD_ID tid)
{
  _assembly[tid]->addJacobianBlockNonlocal(jacobian, ivar, jvar, dof_map, idof_indices, jdof_indices);
}

void
DisplacedProblem::addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<dof_id_type> & dof_indices, std::vector<dof_id_type> & neighbor_dof_indices, THREAD_ID tid)
{
  _assembly[tid]->addJacobianNeighbor(jacobian, ivar, jvar, dof_map, dof_indices, neighbor_dof_indices);
}

void
DisplacedProblem::prepareShapes(unsigned int var, THREAD_ID tid)
{
  _assembly[tid]->copyShapes(var);
}

void
DisplacedProblem::prepareFaceShapes(unsigned int var, THREAD_ID tid)
{
  _assembly[tid]->copyFaceShapes(var);
}

void
DisplacedProblem::prepareNeighborShapes(unsigned int var, THREAD_ID tid)
{
  _assembly[tid]->copyNeighborShapes(var);
}

void
DisplacedProblem::updateGeomSearch(GeometricSearchData::GeometricSearchType type)
{
  _geometric_search_data.update(type);
}

void
DisplacedProblem::meshChanged()
{
  // mesh changed
  _eq.reinit();
  _mesh.meshChanged();

  // Since the Mesh changed, update the PointLocator object used by DiracKernels.
  _dirac_kernel_info.updatePointLocator(_mesh);

  unsigned int n_threads = libMesh::n_threads();

  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i]->invalidateCache();
  _geometric_search_data.update();
}

void
DisplacedProblem::addGhostedElem(dof_id_type elem_id)
{
  _mproblem.addGhostedElem(elem_id);
}

void
DisplacedProblem::addGhostedBoundary(BoundaryID boundary_id)
{
  _mproblem.addGhostedBoundary(boundary_id);
}

void
DisplacedProblem::ghostGhostedBoundaries()
{
  _mproblem.ghostGhostedBoundaries();
}

MooseMesh &
DisplacedProblem::refMesh()
{
  return _ref_mesh;
}

void
DisplacedProblem::solve()
{
}

bool
DisplacedProblem::converged()
{
  return _mproblem.converged();
}

bool
DisplacedProblem::computingInitialResidual()
{
  return _mproblem.computingInitialResidual();
}

void
DisplacedProblem::onTimestepBegin()
{
}

void
DisplacedProblem::onTimestepEnd()
{
}

void
DisplacedProblem::undisplaceMesh()
{
  // If undisplaceMesh() is called during initial adaptivity, it is
  // not valid to call _mesh.getActiveSemiLocalNodeRange() since it is
  // not set up yet.  So we are creating the Range by hand.
  //
  // We must undisplace *all* our nodes to the _ref_mesh
  // configuration, not just the local ones, since the partitioners
  // require this.  We are using the GRAIN_SIZE=1 from MooseMesh.C,
  // not sure how this value was decided upon.
  //
  // (DRG: The grainsize parameter is ultimately passed to TBB to help
  // it choose how to split up the range.  A grainsize of 1 says "split
  // it as much as you want".  Years ago I experimentally found that it
  // didn't matter much and that using 1 was fine.)
  //
  // Note: we don't have to invalidate/update as much stuff as
  // DisplacedProblem::updateMesh() does, since this will be handled
  // by a later call to updateMesh().
  NodeRange node_range(_mesh.getMesh().nodes_begin(),
                       _mesh.getMesh().nodes_end(),
                       /*grainsize=*/1);

  ResetDisplacedMeshThread rdmt(_mproblem, *this);

  // Undisplace the mesh using threads.
  Threads::parallel_reduce (node_range, rdmt);
}
