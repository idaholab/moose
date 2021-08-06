//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes

#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"
#include "Problem.h"
#include "ResetDisplacedMeshThread.h"
#include "SubProblem.h"
#include "UpdateDisplacedMeshThread.h"
#include "Assembly.h"
#include "DisplacedProblem.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/fe_interface.h"

registerMooseObject("MooseApp", DisplacedProblem);

defineLegacyParams(DisplacedProblem);

InputParameters
DisplacedProblem::validParams()
{
  InputParameters params = SubProblem::validParams();
  params.addClassDescription(
      "A Problem object for providing access to the displaced finite element "
      "mesh and associated variables.");
  params.addPrivateParam<MooseMesh *>("mesh");
  params.addPrivateParam<std::vector<std::string>>("displacements");
  return params;
}

DisplacedProblem::DisplacedProblem(const InputParameters & parameters)
  : SubProblem(parameters),
    _mproblem(parameters.have_parameter<FEProblemBase *>("_fe_problem_base")
                  ? *getParam<FEProblemBase *>("_fe_problem_base")
                  : *getParam<FEProblem *>("_fe_problem")),
    _mesh(*getParam<MooseMesh *>("mesh")),
    _eq(_mesh),
    _ref_mesh(_mproblem.mesh()),
    _displacements(getParam<std::vector<std::string>>("displacements")),
    _displaced_nl(*this,
                  _mproblem.getNonlinearSystemBase(),
                  "displaced_" + _mproblem.getNonlinearSystemBase().name(),
                  Moose::VAR_NONLINEAR),
    _displaced_aux(*this,
                   _mproblem.getAuxiliarySystem(),
                   "displaced_" + _mproblem.getAuxiliarySystem().name(),
                   Moose::VAR_AUXILIARY),
    _geometric_search_data(*this, _mesh)

{
  // TODO: Move newAssemblyArray further up to SubProblem so that we can use it here
  unsigned int n_threads = libMesh::n_threads();

  _assembly.reserve(n_threads);
  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly.emplace_back(std::make_unique<Assembly>(_displaced_nl, i));

  _displaced_nl.addTimeIntegrator(_mproblem.getNonlinearSystemBase().getSharedTimeIntegrator());
  _displaced_aux.addTimeIntegrator(_mproblem.getAuxiliarySystem().getSharedTimeIntegrator());

  // // Generally speaking, the mesh is prepared for use, and consequently remote elements are deleted
  // // well before our Problem(s) are constructed. Historically, in MooseMesh we have a bunch of
  // // needs_prepare type flags that make it so we never call prepare_for_use (and consequently
  // // delete_remote_elements) again. So the below line, historically, has had no impact. HOWEVER:
  // // I've added some code in SetupMeshCompleteAction for deleting remote elements post
  // // EquationSystems::init. If I execute that code without default ghosting, then I get > 40 MOOSE
  // // test failures, so we clearly have some simulations that are not yet covered properly by
  // // relationship managers. Until that is resolved, I am going to retain default geometric ghosting
  // if (!_default_ghosting)
  //   _mesh.getMesh().remove_ghosting_functor(_mesh.getMesh().default_ghosting());

  automaticScaling(_mproblem.automaticScaling());

  _mesh.setCoordData(_ref_mesh);
}

bool
DisplacedProblem::isTransient() const
{
  return _mproblem.isTransient();
}

std::set<dof_id_type> &
DisplacedProblem::ghostedElems()
{
  return _mproblem.ghostedElems();
}

void
DisplacedProblem::createQRules(QuadratureType type,
                               Order order,
                               Order volume_order,
                               Order face_order,
                               SubdomainID block,
                               const bool allow_negative_qweights)
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    _assembly[tid]->createQRules(
        type, order, volume_order, face_order, block, allow_negative_qweights);
}

void
DisplacedProblem::bumpVolumeQRuleOrder(Order order, SubdomainID block)
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    _assembly[tid]->bumpVolumeQRuleOrder(order, block);
}

void
DisplacedProblem::bumpAllQRuleOrder(Order order, SubdomainID block)
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    _assembly[tid]->bumpAllQRuleOrder(order, block);
}

void
DisplacedProblem::init()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    _assembly[tid]->init(_mproblem.couplingMatrix());
    std::vector<unsigned> disp_numbers;
    for (const auto & disp_string : _displacements)
    {
      const auto & disp_variable = getVariable(tid, disp_string);
      if (disp_variable.kind() == Moose::VarKindType::VAR_NONLINEAR)
        disp_numbers.push_back(disp_variable.number());
    }
    _assembly[tid]->assignDisplacements(std::move(disp_numbers));
  }

  _displaced_nl.dofMap().attach_extra_send_list_function(&extraSendList, &_displaced_nl);
  _displaced_aux.dofMap().attach_extra_send_list_function(&extraSendList, &_displaced_aux);

  _displaced_nl.init();

  _displaced_aux.init();

  {
    TIME_SECTION("eq::init", 2, "Initializing Displaced Equation System");
    _eq.init();
  }

  _mesh.meshChanged();
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
  TIME_SECTION("syncSolutions", 5, "Syncing Displaced Solutions");

  (*_displaced_nl.sys().solution) = *_mproblem.getNonlinearSystemBase().currentSolution();
  (*_displaced_aux.sys().solution) = *_mproblem.getAuxiliarySystem().currentSolution();
  _displaced_nl.update();
  _displaced_aux.update();
}

void
DisplacedProblem::syncSolutions(const NumericVector<Number> & soln,
                                const NumericVector<Number> & aux_soln)
{
  TIME_SECTION("syncSolutions", 5, "Syncing Displaced Solutions");

  (*_displaced_nl.sys().solution) = soln;
  (*_displaced_aux.sys().solution) = aux_soln;
  _displaced_nl.update();
  _displaced_aux.update();
}

void
DisplacedProblem::updateMesh(bool mesh_changing)
{
  TIME_SECTION("updateMesh", 3, "Updating Displaced Mesh");

  if (mesh_changing)
  {
    // We are probably performing adaptivity. We do not want to use the undisplaced
    // mesh solution because it may be out-of-sync, whereas our displaced mesh solution should be in
    // the correct state after getting restricted/prolonged in EquationSystems::reinit (must have
    // been called before this method)
    _nl_solution = _displaced_nl.sys().solution.get();
    _aux_solution = _displaced_aux.sys().solution.get();
  }
  else
  {
    syncSolutions();

    _nl_solution = _mproblem.getNonlinearSystemBase().currentSolution();
    _aux_solution = _mproblem.getAuxiliarySystem().currentSolution();
  }

  // If the displaced mesh has been serialized to one processor (as
  // may have occurred if it was used for Exodus output), then we need
  // the reference mesh to be also.  For that matter, did anyone
  // somehow serialize the whole mesh?  Hopefully not but let's avoid
  // causing errors if so.
  if (_mesh.getMesh().is_serial() && !this->refMesh().getMesh().is_serial())
    this->refMesh().getMesh().allgather();

  if (_mesh.getMesh().is_serial_on_zero() && !this->refMesh().getMesh().is_serial_on_zero())
    this->refMesh().getMesh().gather_to_zero();

  UpdateDisplacedMeshThread udmt(_mproblem, *this);

  // We displace all nodes, not just semilocal nodes, because
  // parallel-inconsistent mesh geometry makes libMesh cry.
  NodeRange node_range(_mesh.getMesh().nodes_begin(),
                       _mesh.getMesh().nodes_end(),
                       /*grainsize=*/1);

  Threads::parallel_reduce(node_range, udmt);

  // Update the geometric searches that depend on the displaced mesh. This call can end up running
  // NearestNodeThread::operator() which has a throw inside of it. We need to catch it and make sure
  // it's propagated to all processes before updating the point locator because the latter requires
  // communication
  try
  {
    // We may need to re-run geometric operations like SecondaryNeighborhoodTread if, for instance,
    // we have performed mesh adaptivity
    if (mesh_changing)
      _geometric_search_data.reinit();
    else
      _geometric_search_data.update();
  }
  catch (MooseException & e)
  {
    _mproblem.setException(e.what());
  }

  // The below call will throw an exception on all processes if any of our processes had an
  // exception above. This exception will be caught higher up the call stack and the error message
  // will be printed there
  _mproblem.checkExceptionAndStopSolve(/*print_message=*/false);

  // Since the Mesh changed, update the PointLocator object used by DiracKernels.
  _dirac_kernel_info.updatePointLocator(_mesh);
}

void
DisplacedProblem::updateMesh(const NumericVector<Number> & soln,
                             const NumericVector<Number> & aux_soln)
{
  TIME_SECTION("updateMesh", 3, "Updating Displaced Mesh");

  syncSolutions(soln, aux_soln);

  _nl_solution = &soln;
  _aux_solution = &aux_soln;

  UpdateDisplacedMeshThread udmt(_mproblem, *this);

  // We displace all nodes, not just semilocal nodes, because
  // parallel-inconsistent mesh geometry makes libMesh cry.
  NodeRange node_range(_mesh.getMesh().nodes_begin(),
                       _mesh.getMesh().nodes_end(),
                       /*grainsize=*/1);

  Threads::parallel_reduce(node_range, udmt);

  // Update the geometric searches that depend on the displaced mesh. This call can end up running
  // NearestNodeThread::operator() which has a throw inside of it. We need to catch it and make sure
  // it's propagated to all processes before updating the point locator because the latter requires
  // communication
  try
  {
    _geometric_search_data.update();
  }
  catch (MooseException & e)
  {
    _mproblem.setException(e.what());
  }

  // The below call will throw an exception on all processes if any of our processes had an
  // exception above. This exception will be caught higher up the call stack and the error message
  // will be printed there
  _mproblem.checkExceptionAndStopSolve(/*print_message=*/false);

  // Since the Mesh changed, update the PointLocator object used by DiracKernels.
  _dirac_kernel_info.updatePointLocator(_mesh);
}

TagID
DisplacedProblem::addVectorTag(const TagName & tag_name,
                               const Moose::VectorTagType type /* = Moose::VECTOR_TAG_RESIDUAL */)
{
  return _mproblem.addVectorTag(tag_name, type);
}

const VectorTag &
DisplacedProblem::getVectorTag(const TagID tag_id) const
{
  return _mproblem.getVectorTag(tag_id);
}

TagID
DisplacedProblem::getVectorTagID(const TagName & tag_name) const
{
  return _mproblem.getVectorTagID(tag_name);
}

TagName
DisplacedProblem::vectorTagName(const TagID tag_id) const
{
  return _mproblem.vectorTagName(tag_id);
}

bool
DisplacedProblem::vectorTagExists(const TagID tag_id) const
{
  return _mproblem.vectorTagExists(tag_id);
}

unsigned int
DisplacedProblem::numVectorTags(const Moose::VectorTagType type /* = Moose::VECTOR_TAG_ANY */) const
{
  return _mproblem.numVectorTags(type);
}

const std::vector<VectorTag> &
DisplacedProblem::getVectorTags(const Moose::VectorTagType type /* = Moose::VECTOR_TAG_ANY */) const
{
  return _mproblem.getVectorTags(type);
}

Moose::VectorTagType
DisplacedProblem::vectorTagType(const TagID tag_id) const
{
  return _mproblem.vectorTagType(tag_id);
}

TagID
DisplacedProblem::addMatrixTag(TagName tag_name)
{
  return _mproblem.addMatrixTag(tag_name);
}

TagID
DisplacedProblem::getMatrixTagID(const TagName & tag_name)
{
  return _mproblem.getMatrixTagID(tag_name);
}

TagName
DisplacedProblem::matrixTagName(TagID tag)
{
  return _mproblem.matrixTagName(tag);
}

bool
DisplacedProblem::matrixTagExists(const TagName & tag_name)
{
  return _mproblem.matrixTagExists(tag_name);
}

bool
DisplacedProblem::matrixTagExists(TagID tag_id)
{
  return _mproblem.matrixTagExists(tag_id);
}

unsigned int
DisplacedProblem::numMatrixTags() const
{
  return _mproblem.numMatrixTags();
}

bool
DisplacedProblem::hasVariable(const std::string & var_name) const
{
  if (_displaced_nl.hasVariable(var_name))
    return true;
  else if (_displaced_aux.hasVariable(var_name))
    return true;
  else
    return false;
}

const MooseVariableFieldBase &
DisplacedProblem::getVariable(THREAD_ID tid,
                              const std::string & var_name,
                              Moose::VarKindType expected_var_type,
                              Moose::VarFieldType expected_var_field_type) const
{
  return getVariableHelper(
      tid, var_name, expected_var_type, expected_var_field_type, _displaced_nl, _displaced_aux);
}

MooseVariable &
DisplacedProblem::getStandardVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_displaced_nl.hasVariable(var_name))
    return _displaced_nl.getFieldVariable<Real>(tid, var_name);
  else if (!_displaced_aux.hasVariable(var_name))
    mooseError("No variable with name '" + var_name + "'");

  return _displaced_aux.getFieldVariable<Real>(tid, var_name);
}

MooseVariableFieldBase &
DisplacedProblem::getActualFieldVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_displaced_nl.hasVariable(var_name))
    return _displaced_nl.getActualFieldVariable<Real>(tid, var_name);
  else if (!_displaced_aux.hasVariable(var_name))
    mooseError("No variable with name '" + var_name + "'");

  return _displaced_aux.getActualFieldVariable<Real>(tid, var_name);
}

VectorMooseVariable &
DisplacedProblem::getVectorVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_displaced_nl.hasVariable(var_name))
    return _displaced_nl.getFieldVariable<RealVectorValue>(tid, var_name);
  else if (!_displaced_aux.hasVariable(var_name))
    mooseError("No variable with name '" + var_name + "'");

  return _displaced_aux.getFieldVariable<RealVectorValue>(tid, var_name);
}

ArrayMooseVariable &
DisplacedProblem::getArrayVariable(THREAD_ID tid, const std::string & var_name)
{
  if (!_displaced_nl.hasVariable(var_name))
    mooseError("No variable with name '" + var_name + "'");

  return _displaced_nl.getFieldVariable<RealEigenVector>(tid, var_name);
}

bool
DisplacedProblem::hasScalarVariable(const std::string & var_name) const
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

System &
DisplacedProblem::getSystem(const std::string & var_name)
{
  if (_displaced_nl.hasVariable(var_name))
    return _displaced_nl.system();
  else if (_displaced_aux.hasVariable(var_name))
    return _displaced_aux.system();
  else
    mooseError("Unable to find a system containing the variable " + var_name);
}

void
DisplacedProblem::addVariable(const std::string & var_type,
                              const std::string & name,
                              InputParameters & parameters)
{
  _displaced_nl.addVariable(var_type, name, parameters);
}

void
DisplacedProblem::addAuxVariable(const std::string & var_type,
                                 const std::string & name,
                                 InputParameters & parameters)
{
  _displaced_aux.addVariable(var_type, name, parameters);
}

void
DisplacedProblem::prepare(const Elem * elem, THREAD_ID tid)
{
  _assembly[tid]->reinit(elem);

  _displaced_nl.prepare(tid);
  _displaced_aux.prepare(tid);
  if (!_mproblem.hasJacobian() || !_mproblem.constJacobian())
    _assembly[tid]->prepareJacobianBlock();
  _assembly[tid]->prepareResidual();
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
DisplacedProblem::prepare(const Elem * elem,
                          unsigned int ivar,
                          unsigned int jvar,
                          const std::vector<dof_id_type> & dof_indices,
                          THREAD_ID tid)
{
  _assembly[tid]->reinit(elem);

  _displaced_nl.prepare(tid);
  _displaced_aux.prepare(tid);
  _assembly[tid]->prepareBlock(ivar, jvar, dof_indices);
}

void
DisplacedProblem::setCurrentSubdomainID(const Elem * elem, THREAD_ID tid)
{
  SubdomainID did = elem->subdomain_id();
  _assembly[tid]->setCurrentSubdomainID(did);
}

void
DisplacedProblem::setNeighborSubdomainID(const Elem * elem, unsigned int side, THREAD_ID tid)
{
  SubdomainID did = elem->neighbor_ptr(side)->subdomain_id();
  _assembly[tid]->setCurrentNeighborSubdomainID(did);
}

void
DisplacedProblem::prepareBlockNonlocal(unsigned int ivar,
                                       unsigned int jvar,
                                       const std::vector<dof_id_type> & idof_indices,
                                       const std::vector<dof_id_type> & jdof_indices,
                                       THREAD_ID tid)
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
DisplacedProblem::reinitElemPhys(const Elem * elem,
                                 const std::vector<Point> & phys_points_in_elem,
                                 THREAD_ID tid)
{
  mooseAssert(_mesh.queryElemPtr(elem->id()) == elem,
              "Are you calling this method with a undisplaced mesh element?");

  _assembly[tid]->reinitAtPhysical(elem, phys_points_in_elem);

  _displaced_nl.prepare(tid);
  _displaced_aux.prepare(tid);
  _assembly[tid]->prepare();

  reinitElem(elem, tid);
}

void
DisplacedProblem::reinitElemFace(const Elem * elem,
                                 unsigned int side,
                                 BoundaryID bnd_id,
                                 THREAD_ID tid)
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
  reinitNeighbor(elem, side, tid, nullptr);
}

void
DisplacedProblem::reinitNeighbor(const Elem * elem,
                                 unsigned int side,
                                 THREAD_ID tid,
                                 const std::vector<Point> * neighbor_reference_points)
{
  setNeighborSubdomainID(elem, side, tid);

  const Elem * neighbor = elem->neighbor_ptr(side);
  unsigned int neighbor_side = neighbor->which_neighbor_am_i(elem);

  _assembly[tid]->reinitElemAndNeighbor(
      elem, side, neighbor, neighbor_side, neighbor_reference_points);

  _displaced_nl.prepareNeighbor(tid);
  _displaced_aux.prepareNeighbor(tid);

  _assembly[tid]->prepareNeighbor();

  BoundaryID bnd_id = 0; // some dummy number (it is not really used for anything, right now)
  _displaced_nl.reinitElemFace(elem, side, bnd_id, tid);
  _displaced_aux.reinitElemFace(elem, side, bnd_id, tid);

  _displaced_nl.reinitNeighborFace(neighbor, neighbor_side, bnd_id, tid);
  _displaced_aux.reinitNeighborFace(neighbor, neighbor_side, bnd_id, tid);
}

void
DisplacedProblem::reinitNeighborPhys(const Elem * neighbor,
                                     unsigned int neighbor_side,
                                     const std::vector<Point> & physical_points,
                                     THREAD_ID tid)
{
  mooseAssert(_mesh.queryElemPtr(neighbor->id()) == neighbor,
              "Are you calling this method with a undisplaced mesh element?");

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
DisplacedProblem::reinitNeighborPhys(const Elem * neighbor,
                                     const std::vector<Point> & physical_points,
                                     THREAD_ID tid)
{
  mooseAssert(_mesh.queryElemPtr(neighbor->id()) == neighbor,
              "Are you calling this method with a undisplaced mesh element?");

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
DisplacedProblem::reinitElemNeighborAndLowerD(const Elem * elem, unsigned int side, THREAD_ID tid)
{
  reinitNeighbor(elem, side, tid);

  const Elem * lower_d_elem = _mesh.getLowerDElem(elem, side);
  if (lower_d_elem && lower_d_elem->subdomain_id() == Moose::INTERNAL_SIDE_LOWERD_ID)
    reinitLowerDElem(lower_d_elem, tid);
  else
  {
    // with mesh refinement, lower-dimensional element might be defined on neighbor side
    auto & neighbor = _assembly[tid]->neighbor();
    auto & neighbor_side = _assembly[tid]->neighborSide();
    const Elem * lower_d_elem_neighbor = _mesh.getLowerDElem(neighbor, neighbor_side);
    if (lower_d_elem_neighbor &&
        lower_d_elem_neighbor->subdomain_id() == Moose::INTERNAL_SIDE_LOWERD_ID)
    {
      auto qps = _assembly[tid]->qPointsFaceNeighbor().stdVector();
      std::vector<Point> reference_points;
      FEInterface::inverse_map(
          lower_d_elem_neighbor->dim(), FEType(), lower_d_elem_neighbor, qps, reference_points);
      reinitLowerDElem(lower_d_elem_neighbor, tid, &qps);
    }
  }
}

void
DisplacedProblem::reinitScalars(THREAD_ID tid, bool reinit_for_derivative_reordering /*=false*/)
{
  _displaced_nl.reinitScalars(tid, reinit_for_derivative_reordering);
  _displaced_aux.reinitScalars(tid, reinit_for_derivative_reordering);
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
  _assembly[tid]->addResidual(getVectorTags(Moose::VECTOR_TAG_RESIDUAL));
}

void
DisplacedProblem::addResidualNeighbor(THREAD_ID tid)
{
  _assembly[tid]->addResidualNeighbor(getVectorTags(Moose::VECTOR_TAG_RESIDUAL));
}

void
DisplacedProblem::addResidualLower(THREAD_ID tid)
{
  _assembly[tid]->addResidualLower(getVectorTags(Moose::VECTOR_TAG_RESIDUAL));
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
  _assembly[tid]->addCachedResiduals();
}

void
DisplacedProblem::addCachedResidualDirectly(NumericVector<Number> & residual, THREAD_ID tid)
{
  if (_displaced_nl.hasVector(_displaced_nl.timeVectorTag()))
    _assembly[tid]->addCachedResidualDirectly(residual,
                                              getVectorTag(_displaced_nl.timeVectorTag()));

  if (_displaced_nl.hasVector(_displaced_nl.nonTimeVectorTag()))
    _assembly[tid]->addCachedResidualDirectly(residual,
                                              getVectorTag(_displaced_nl.nonTimeVectorTag()));

  // We do this because by adding the cached residual directly, we cannot ensure that all of the
  // cached residuals are emptied after only the two add calls above
  _assembly[tid]->clearCachedResiduals();
}

void
DisplacedProblem::setResidual(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->setResidual(residual, getVectorTag(_displaced_nl.residualVectorTag()));
}

void
DisplacedProblem::setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->setResidualNeighbor(residual, getVectorTag(_displaced_nl.residualVectorTag()));
}

void
DisplacedProblem::addJacobian(THREAD_ID tid)
{
  _assembly[tid]->addJacobian();
}

void
DisplacedProblem::addJacobianNonlocal(THREAD_ID tid)
{
  _assembly[tid]->addJacobianNonlocal();
}

void
DisplacedProblem::addJacobianNeighbor(THREAD_ID tid)
{
  _assembly[tid]->addJacobianNeighbor();
}

void
DisplacedProblem::addJacobianNeighborLowerD(THREAD_ID tid)
{
  _assembly[tid]->addJacobianNeighborLowerD();
}

void
DisplacedProblem::addJacobianLowerD(THREAD_ID tid)
{
  _assembly[tid]->addJacobianLowerD();
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
DisplacedProblem::addCachedJacobian(THREAD_ID tid)
{
  _assembly[tid]->addCachedJacobian();
}

void
DisplacedProblem::addCachedJacobianContributions(THREAD_ID tid)
{
  mooseDeprecated("please use addCachedJacobian");

  addCachedJacobian(tid);
}

void
DisplacedProblem::addJacobianBlock(SparseMatrix<Number> & jacobian,
                                   unsigned int ivar,
                                   unsigned int jvar,
                                   const DofMap & dof_map,
                                   std::vector<dof_id_type> & dof_indices,
                                   THREAD_ID tid)
{
  _assembly[tid]->addJacobianBlock(jacobian, ivar, jvar, dof_map, dof_indices);
}

void
DisplacedProblem::addJacobianBlockTags(SparseMatrix<Number> & jacobian,
                                       unsigned int ivar,
                                       unsigned int jvar,
                                       const DofMap & dof_map,
                                       std::vector<dof_id_type> & dof_indices,
                                       const std::set<TagID> & tags,
                                       THREAD_ID tid)
{
  _assembly[tid]->addJacobianBlockTags(jacobian, ivar, jvar, dof_map, dof_indices, tags);
}

void
DisplacedProblem::addJacobianBlockNonlocal(SparseMatrix<Number> & jacobian,
                                           unsigned int ivar,
                                           unsigned int jvar,
                                           const DofMap & dof_map,
                                           const std::vector<dof_id_type> & idof_indices,
                                           const std::vector<dof_id_type> & jdof_indices,
                                           THREAD_ID tid)
{
  _assembly[tid]->addJacobianBlockNonlocal(
      jacobian, ivar, jvar, dof_map, idof_indices, jdof_indices);
}

void
DisplacedProblem::addJacobianNeighbor(SparseMatrix<Number> & jacobian,
                                      unsigned int ivar,
                                      unsigned int jvar,
                                      const DofMap & dof_map,
                                      std::vector<dof_id_type> & dof_indices,
                                      std::vector<dof_id_type> & neighbor_dof_indices,
                                      THREAD_ID tid)
{
  _assembly[tid]->addJacobianNeighbor(
      jacobian, ivar, jvar, dof_map, dof_indices, neighbor_dof_indices);
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
  TIME_SECTION("updateGeometricSearch", 3, "Updating Displaced GeometricSearch");

  _geometric_search_data.update(type);
}

void
DisplacedProblem::meshChanged()
{
  // The mesh changed. The displaced equations system object only holds ExplicitSystems, so calling
  // EquationSystems::reinit only prolongs/restricts the solution vectors, which is something that
  // needs to happen for every step of mesh adaptivity.
  _eq.reinit();

  // We've performed some mesh adaptivity. We need to
  // clear any quadrature nodes such that when we build the boundary node lists in
  // MooseMesh::meshChanged we don't have any extraneous extra boundary nodes lying around
  _mesh.clearQuadratureNodes();

  _mesh.meshChanged();

  // Before performing mesh adaptivity we un-displaced the mesh. We need to re-displace the mesh and
  // then reinitialize GeometricSearchData such that we have all the correct geometric information
  // for the changed mesh
  updateMesh(/*mesh_changing=*/true);

  // Since the mesh has changed, we need to make sure that we update any of our
  // MOOSE-system specific data. libmesh system data has already been updated
  _displaced_nl.update(/*update_libmesh_system=*/false);
  _displaced_aux.update(/*update_libmesh_system=*/false);
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
DisplacedProblem::computingInitialResidual() const
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
  Threads::parallel_reduce(node_range, rdmt);
}

LineSearch *
DisplacedProblem::getLineSearch()
{
  return _mproblem.getLineSearch();
}

const CouplingMatrix *
DisplacedProblem::couplingMatrix() const
{
  return _mproblem.couplingMatrix();
}

bool
DisplacedProblem::computingScalingJacobian() const
{
  return _mproblem.computingScalingJacobian();
}

bool
DisplacedProblem::computingScalingResidual() const
{
  return _mproblem.computingScalingResidual();
}

void
DisplacedProblem::initialSetup()
{
  SubProblem::initialSetup();

  _displaced_nl.initialSetup();
  _displaced_aux.initialSetup();
}

void
DisplacedProblem::timestepSetup()
{
  SubProblem::timestepSetup();

  _displaced_nl.timestepSetup();
  _displaced_aux.timestepSetup();
}

void
DisplacedProblem::haveADObjects(const bool have_ad_objects)
{
  _have_ad_objects = have_ad_objects;
  _mproblem.SubProblem::haveADObjects(have_ad_objects);
}
