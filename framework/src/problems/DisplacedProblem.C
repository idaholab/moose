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
#include "libmesh/mesh_base.h"

using namespace libMesh;

registerMooseObject("MooseApp", DisplacedProblem);

InputParameters
DisplacedProblem::validParams()
{
  InputParameters params = SubProblem::validParams();
  params.addClassDescription(
      "A Problem object for providing access to the displaced finite element "
      "mesh and associated variables.");
  params.addPrivateParam<MooseMesh *>("mesh");
  params.addPrivateParam<std::vector<std::string>>("displacements", {});
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
    _geometric_search_data(*this, _mesh)

{
  // TODO: Move newAssemblyArray further up to SubProblem so that we can use it here
  unsigned int n_threads = libMesh::n_threads();

  _assembly.resize(n_threads);
  for (const auto nl_sys_num : make_range(_mproblem.numNonlinearSystems()))
  {
    _displaced_solver_systems.emplace_back(std::make_unique<DisplacedSystem>(
        *this,
        _mproblem,
        _mproblem.getNonlinearSystemBase(nl_sys_num),
        "displaced_" + _mproblem.getNonlinearSystemBase(nl_sys_num).name() + "_" +
            std::to_string(nl_sys_num),
        Moose::VAR_SOLVER));
    auto & displaced_nl = _displaced_solver_systems.back();

    for (unsigned int i = 0; i < n_threads; ++i)
      _assembly[i].emplace_back(std::make_unique<Assembly>(*displaced_nl, i));
  }

  _nl_solution.resize(_displaced_solver_systems.size(), nullptr);

  _displaced_aux =
      std::make_unique<DisplacedSystem>(*this,
                                        _mproblem,
                                        _mproblem.getAuxiliarySystem(),
                                        "displaced_" + _mproblem.getAuxiliarySystem().name(),
                                        Moose::VAR_AUXILIARY);

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

DisplacedProblem::~DisplacedProblem() = default;

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
    for (const auto sys_num : index_range(_assembly[tid]))
      _assembly[tid][sys_num]->createQRules(
          type, order, volume_order, face_order, block, allow_negative_qweights);
}

void
DisplacedProblem::bumpVolumeQRuleOrder(Order order, SubdomainID block)
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    for (const auto nl_sys_num : index_range(_assembly[tid]))
      _assembly[tid][nl_sys_num]->bumpVolumeQRuleOrder(order, block);
}

void
DisplacedProblem::bumpAllQRuleOrder(Order order, SubdomainID block)
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    for (const auto nl_sys_num : index_range(_assembly[tid]))
      _assembly[tid][nl_sys_num]->bumpAllQRuleOrder(order, block);
}

void
DisplacedProblem::init()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    for (const auto nl_sys_num : index_range(_displaced_solver_systems))
      _assembly[tid][nl_sys_num]->init(_mproblem.couplingMatrix(nl_sys_num));

    for (const auto nl_sys_num : index_range(_displaced_solver_systems))
    {
      std::vector<std::pair<unsigned int, unsigned short>> disp_numbers_and_directions;
      for (const auto direction : index_range(_displacements))
      {
        const auto & disp_string = _displacements[direction];
        const auto & disp_variable = getVariable(tid, disp_string);
        if (disp_variable.sys().number() == nl_sys_num)
          disp_numbers_and_directions.push_back(
              std::make_pair(disp_variable.number(), cast_int<unsigned short>(direction)));
      }
      _assembly[tid][nl_sys_num]->assignDisplacements(std::move(disp_numbers_and_directions));
    }
  }

  for (auto & nl : _displaced_solver_systems)
  {
    nl->dofMap().attach_extra_send_list_function(&extraSendList, nl.get());
    nl->preInit();
  }

  _displaced_aux->dofMap().attach_extra_send_list_function(&extraSendList, _displaced_aux.get());
  _displaced_aux->preInit();

  {
    TIME_SECTION("eq::init", 2, "Initializing Displaced Equation System");
    _eq.init();
  }

  for (auto & nl : _displaced_solver_systems)
    nl->postInit();
  _displaced_aux->postInit();

  _mesh.meshChanged();

  if (haveFV())
    _mesh.setupFiniteVolumeMeshData();
}

void
DisplacedProblem::initAdaptivity()
{
}

void
DisplacedProblem::addTimeIntegrator()
{
  for (const auto nl_sys_num : make_range(_mproblem.numNonlinearSystems()))
    _displaced_solver_systems[nl_sys_num]->copyTimeIntegrators(
        _mproblem.getNonlinearSystemBase(nl_sys_num));
  _displaced_aux->copyTimeIntegrators(_mproblem.getAuxiliarySystem());
}

void
DisplacedProblem::saveOldSolutions()
{
  for (auto & displaced_nl : _displaced_solver_systems)
    displaced_nl->saveOldSolutions();
  _displaced_aux->saveOldSolutions();
}

void
DisplacedProblem::restoreOldSolutions()
{
  for (auto & displaced_nl : _displaced_solver_systems)
    displaced_nl->restoreOldSolutions();
  _displaced_aux->restoreOldSolutions();
}

void
DisplacedProblem::syncSolutions()
{
  TIME_SECTION("syncSolutions", 5, "Syncing Displaced Solutions");

  for (const auto nl_sys_num : index_range(_displaced_solver_systems))
  {
    auto & displaced_nl = _displaced_solver_systems[nl_sys_num];
    mooseAssert(nl_sys_num == displaced_nl->number(),
                "We should have designed things such that the nl system numbers make their system "
                "numbering in the EquationSystems object");
    (*displaced_nl->sys().solution) =
        *_mproblem.getNonlinearSystemBase(displaced_nl->number()).currentSolution();
    displaced_nl->update();
  }
  (*_displaced_aux->sys().solution) = *_mproblem.getAuxiliarySystem().currentSolution();
}

void
DisplacedProblem::syncSolutions(
    const std::map<unsigned int, const NumericVector<Number> *> & nl_solns,
    const NumericVector<Number> & aux_soln)
{
  TIME_SECTION("syncSolutions", 5, "Syncing Displaced Solutions");

  for (const auto [nl_sys_num, nl_soln] : nl_solns)
  {
    (*_displaced_solver_systems[nl_sys_num]->sys().solution) = *nl_soln;
    _displaced_solver_systems[nl_sys_num]->update();
  }
  (*_displaced_aux->sys().solution) = aux_soln;
}

void
DisplacedProblem::updateMesh(bool mesh_changing)
{
  TIME_SECTION("updateMesh", 3, "Updating Displaced Mesh");

  // If the mesh is changing, we are probably performing adaptivity. In that case, we do *not* want
  // to use the undisplaced mesh solution because it may be out-of-sync, whereas our displaced mesh
  // solution should be in the correct state after getting restricted/prolonged in
  // EquationSystems::reinit (must have been called before this method)
  if (!mesh_changing)
    syncSolutions();

  for (const auto nl_sys_num : index_range(_displaced_solver_systems))
    _nl_solution[nl_sys_num] = _displaced_solver_systems[nl_sys_num]->sys().solution.get();
  _aux_solution = _displaced_aux->sys().solution.get();

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
  // Displacement of the mesh has invalidated the point locator data (e.g. bounding boxes)
  _mesh.getMesh().clear_point_locator();

  // The mesh has changed. Face information normals, areas, etc. must be re-calculated
  if (haveFV())
    _mesh.setupFiniteVolumeMeshData();

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
DisplacedProblem::updateMesh(const std::map<unsigned int, const NumericVector<Number> *> & nl_solns,
                             const NumericVector<Number> & aux_soln)
{
  TIME_SECTION("updateMesh", 3, "Updating Displaced Mesh");

  syncSolutions(nl_solns, aux_soln);

  for (const auto nl_sys_num : index_range(_displaced_solver_systems))
    _nl_solution[nl_sys_num] = _displaced_solver_systems[nl_sys_num]->sys().solution.get();
  _aux_solution = _displaced_aux->sys().solution.get();

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

bool
DisplacedProblem::vectorTagExists(const TagName & tag_name) const
{
  return _mproblem.vectorTagExists(tag_name);
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
DisplacedProblem::getMatrixTagID(const TagName & tag_name) const
{
  return _mproblem.getMatrixTagID(tag_name);
}

TagName
DisplacedProblem::matrixTagName(TagID tag)
{
  return _mproblem.matrixTagName(tag);
}

bool
DisplacedProblem::matrixTagExists(const TagName & tag_name) const
{
  return _mproblem.matrixTagExists(tag_name);
}

bool
DisplacedProblem::matrixTagExists(TagID tag_id) const
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
  for (auto & nl : _displaced_solver_systems)
    if (nl->hasVariable(var_name))
      return true;
  if (_displaced_aux->hasVariable(var_name))
    return true;

  return false;
}

const MooseVariableFieldBase &
DisplacedProblem::getVariable(const THREAD_ID tid,
                              const std::string & var_name,
                              Moose::VarKindType expected_var_type,
                              Moose::VarFieldType expected_var_field_type) const
{
  return getVariableHelper(tid,
                           var_name,
                           expected_var_type,
                           expected_var_field_type,
                           _displaced_solver_systems,
                           *_displaced_aux);
}

MooseVariable &
DisplacedProblem::getStandardVariable(const THREAD_ID tid, const std::string & var_name)
{
  for (auto & nl : _displaced_solver_systems)
    if (nl->hasVariable(var_name))
      return nl->getFieldVariable<Real>(tid, var_name);
  if (_displaced_aux->hasVariable(var_name))
    return _displaced_aux->getFieldVariable<Real>(tid, var_name);

  mooseError("No variable with name '" + var_name + "'");
}

MooseVariableFieldBase &
DisplacedProblem::getActualFieldVariable(const THREAD_ID tid, const std::string & var_name)
{
  for (auto & nl : _displaced_solver_systems)
    if (nl->hasVariable(var_name))
      return nl->getActualFieldVariable<Real>(tid, var_name);
  if (_displaced_aux->hasVariable(var_name))
    return _displaced_aux->getActualFieldVariable<Real>(tid, var_name);

  mooseError("No variable with name '" + var_name + "'");
}

VectorMooseVariable &
DisplacedProblem::getVectorVariable(const THREAD_ID tid, const std::string & var_name)
{
  for (auto & nl : _displaced_solver_systems)
    if (nl->hasVariable(var_name))
      return nl->getFieldVariable<RealVectorValue>(tid, var_name);
  if (_displaced_aux->hasVariable(var_name))
    return _displaced_aux->getFieldVariable<RealVectorValue>(tid, var_name);

  mooseError("No variable with name '" + var_name + "'");
}

ArrayMooseVariable &
DisplacedProblem::getArrayVariable(const THREAD_ID tid, const std::string & var_name)
{
  for (auto & nl : _displaced_solver_systems)
    if (nl->hasVariable(var_name))
      return nl->getFieldVariable<RealEigenVector>(tid, var_name);
  if (_displaced_aux->hasVariable(var_name))
    return _displaced_aux->getFieldVariable<RealEigenVector>(tid, var_name);

  mooseError("No variable with name '" + var_name + "'");
}

bool
DisplacedProblem::hasScalarVariable(const std::string & var_name) const
{
  for (auto & nl : _displaced_solver_systems)
    if (nl->hasScalarVariable(var_name))
      return true;
  if (_displaced_aux->hasScalarVariable(var_name))
    return true;

  return false;
}

MooseVariableScalar &
DisplacedProblem::getScalarVariable(const THREAD_ID tid, const std::string & var_name)
{
  for (auto & nl : _displaced_solver_systems)
    if (nl->hasScalarVariable(var_name))
      return nl->getScalarVariable(tid, var_name);
  if (_displaced_aux->hasScalarVariable(var_name))
    return _displaced_aux->getScalarVariable(tid, var_name);

  mooseError("No variable with name '" + var_name + "'");
}

System &
DisplacedProblem::getSystem(const std::string & var_name)
{
  for (const auto sys_num : make_range(_eq.n_systems()))
  {
    auto & sys = _eq.get_system(sys_num);
    if (sys.has_variable(var_name))
      return sys;
  }

  mooseError("Unable to find a system containing the variable " + var_name);
}

void
DisplacedProblem::addVariable(const std::string & var_type,
                              const std::string & name,
                              InputParameters & parameters,
                              const unsigned int nl_system_number)
{
  _displaced_solver_systems[nl_system_number]->addVariable(var_type, name, parameters);
}

void
DisplacedProblem::addAuxVariable(const std::string & var_type,
                                 const std::string & name,
                                 InputParameters & parameters)
{
  _displaced_aux->addVariable(var_type, name, parameters);
}

unsigned int
DisplacedProblem::currentNlSysNum() const
{
  return _mproblem.currentNlSysNum();
}

unsigned int
DisplacedProblem::currentLinearSysNum() const
{
  return _mproblem.currentLinearSysNum();
}

void
DisplacedProblem::prepare(const Elem * elem, const THREAD_ID tid)
{
  for (const auto nl_sys_num : index_range(_displaced_solver_systems))
  {
    _assembly[tid][nl_sys_num]->reinit(elem);
    _displaced_solver_systems[nl_sys_num]->prepare(tid);
    // This method is called outside of residual/Jacobian callbacks during initial condition
    // evaluation
    if (!_mproblem.hasJacobian() || !_mproblem.constJacobian())
      _assembly[tid][nl_sys_num]->prepareJacobianBlock();
    _assembly[tid][nl_sys_num]->prepareResidual();
  }

  _displaced_aux->prepare(tid);
}

void
DisplacedProblem::prepareNonlocal(const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->prepareNonlocal();
}

void
DisplacedProblem::prepareFace(const Elem * /*elem*/, const THREAD_ID tid)
{
  for (auto & nl : _displaced_solver_systems)
    nl->prepareFace(tid, true);
  _displaced_aux->prepareFace(tid, false);
}

void
DisplacedProblem::prepare(const Elem * elem,
                          unsigned int ivar,
                          unsigned int jvar,
                          const std::vector<dof_id_type> & dof_indices,
                          const THREAD_ID tid)
{
  for (const auto nl_sys_num : index_range(_displaced_solver_systems))
  {
    _assembly[tid][nl_sys_num]->reinit(elem);
    _displaced_solver_systems[nl_sys_num]->prepare(tid);
  }
  _displaced_aux->prepare(tid);
  _assembly[tid][currentNlSysNum()]->prepareBlock(ivar, jvar, dof_indices);
}

void
DisplacedProblem::setCurrentSubdomainID(const Elem * elem, const THREAD_ID tid)
{
  SubdomainID did = elem->subdomain_id();
  for (auto & assembly : _assembly[tid])
    assembly->setCurrentSubdomainID(did);
}

void
DisplacedProblem::setNeighborSubdomainID(const Elem * elem, unsigned int side, const THREAD_ID tid)
{
  SubdomainID did = elem->neighbor_ptr(side)->subdomain_id();
  for (auto & assembly : _assembly[tid])
    assembly->setCurrentNeighborSubdomainID(did);
}

void
DisplacedProblem::prepareBlockNonlocal(unsigned int ivar,
                                       unsigned int jvar,
                                       const std::vector<dof_id_type> & idof_indices,
                                       const std::vector<dof_id_type> & jdof_indices,
                                       const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->prepareBlockNonlocal(ivar, jvar, idof_indices, jdof_indices);
}

void
DisplacedProblem::prepareAssembly(const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->prepare();
}

void
DisplacedProblem::prepareAssemblyNeighbor(const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->prepareNeighbor();
}

bool
DisplacedProblem::reinitDirac(const Elem * elem, const THREAD_ID tid)
{
  std::vector<Point> & points = _dirac_kernel_info.getPoints()[elem].first;

  unsigned int n_points = points.size();

  if (n_points)
  {
    for (const auto nl_sys_num : index_range(_displaced_solver_systems))
    {
      _assembly[tid][nl_sys_num]->reinitAtPhysical(elem, points);
      _displaced_solver_systems[nl_sys_num]->prepare(tid);
    }
    _displaced_aux->prepare(tid);

    reinitElem(elem, tid);
  }

  _assembly[tid][currentNlSysNum()]->prepare();

  return n_points > 0;
}

void
DisplacedProblem::reinitElem(const Elem * elem, const THREAD_ID tid)
{
  for (auto & nl : _displaced_solver_systems)
    nl->reinitElem(elem, tid);
  _displaced_aux->reinitElem(elem, tid);
}

void
DisplacedProblem::reinitElemPhys(const Elem * elem,
                                 const std::vector<Point> & phys_points_in_elem,
                                 const THREAD_ID tid)
{
  mooseAssert(_mesh.queryElemPtr(elem->id()) == elem,
              "Are you calling this method with a undisplaced mesh element?");

  for (const auto nl_sys_num : index_range(_displaced_solver_systems))
  {
    _assembly[tid][nl_sys_num]->reinitAtPhysical(elem, phys_points_in_elem);
    _displaced_solver_systems[nl_sys_num]->prepare(tid);
    _assembly[tid][nl_sys_num]->prepare();
  }
  _displaced_aux->prepare(tid);

  reinitElem(elem, tid);
}

void
DisplacedProblem::reinitElemFace(const Elem * elem, unsigned int side, const THREAD_ID tid)
{
  for (const auto nl_sys_num : index_range(_displaced_solver_systems))
  {
    _assembly[tid][nl_sys_num]->reinit(elem, side);
    _displaced_solver_systems[nl_sys_num]->reinitElemFace(elem, side, tid);
  }
  _displaced_aux->reinitElemFace(elem, side, tid);
}

void
DisplacedProblem::reinitNode(const Node * node, const THREAD_ID tid)
{
  for (const auto nl_sys_num : index_range(_displaced_solver_systems))
  {
    _assembly[tid][nl_sys_num]->reinit(node);
    _displaced_solver_systems[nl_sys_num]->reinitNode(node, tid);
  }
  _displaced_aux->reinitNode(node, tid);
}

void
DisplacedProblem::reinitNodeFace(const Node * node, BoundaryID bnd_id, const THREAD_ID tid)
{
  for (const auto nl_sys_num : index_range(_displaced_solver_systems))
  {
    _assembly[tid][nl_sys_num]->reinit(node);
    _displaced_solver_systems[nl_sys_num]->reinitNodeFace(node, bnd_id, tid);
  }
  _displaced_aux->reinitNodeFace(node, bnd_id, tid);
}

void
DisplacedProblem::reinitNodes(const std::vector<dof_id_type> & nodes, const THREAD_ID tid)
{
  for (auto & nl : _displaced_solver_systems)
    nl->reinitNodes(nodes, tid);
  _displaced_aux->reinitNodes(nodes, tid);
}

void
DisplacedProblem::reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, const THREAD_ID tid)
{
  for (auto & nl : _displaced_solver_systems)
    nl->reinitNodesNeighbor(nodes, tid);
  _displaced_aux->reinitNodesNeighbor(nodes, tid);
}

void
DisplacedProblem::reinitNeighbor(const Elem * elem, unsigned int side, const THREAD_ID tid)
{
  reinitNeighbor(elem, side, tid, nullptr);
}

void
DisplacedProblem::reinitNeighbor(const Elem * elem,
                                 unsigned int side,
                                 const THREAD_ID tid,
                                 const std::vector<Point> * neighbor_reference_points)
{
  setNeighborSubdomainID(elem, side, tid);

  const Elem * neighbor = elem->neighbor_ptr(side);
  unsigned int neighbor_side = neighbor->which_neighbor_am_i(elem);

  for (const auto nl_sys_num : index_range(_displaced_solver_systems))
  {
    _assembly[tid][nl_sys_num]->reinitElemAndNeighbor(
        elem, side, neighbor, neighbor_side, neighbor_reference_points);
    _displaced_solver_systems[nl_sys_num]->prepareNeighbor(tid);
    // Called during stateful material property evaluation outside of solve
    _assembly[tid][nl_sys_num]->prepareNeighbor();
  }
  _displaced_aux->prepareNeighbor(tid);

  for (auto & nl : _displaced_solver_systems)
  {
    nl->reinitElemFace(elem, side, tid);
    nl->reinitNeighborFace(neighbor, neighbor_side, tid);
  }
  _displaced_aux->reinitElemFace(elem, side, tid);
  _displaced_aux->reinitNeighborFace(neighbor, neighbor_side, tid);
}

void
DisplacedProblem::reinitNeighborPhys(const Elem * neighbor,
                                     unsigned int neighbor_side,
                                     const std::vector<Point> & physical_points,
                                     const THREAD_ID tid)
{
  mooseAssert(_mesh.queryElemPtr(neighbor->id()) == neighbor,
              "Are you calling this method with a undisplaced mesh element?");

  for (const auto nl_sys_num : index_range(_displaced_solver_systems))
  {
    // Reinit shape functions
    _assembly[tid][nl_sys_num]->reinitNeighborAtPhysical(neighbor, neighbor_side, physical_points);

    // Set the neighbor dof indices
    _displaced_solver_systems[nl_sys_num]->prepareNeighbor(tid);
  }
  _displaced_aux->prepareNeighbor(tid);

  prepareAssemblyNeighbor(tid);

  // Compute values at the points
  for (auto & nl : _displaced_solver_systems)
    nl->reinitNeighborFace(neighbor, neighbor_side, tid);
  _displaced_aux->reinitNeighborFace(neighbor, neighbor_side, tid);
}

void
DisplacedProblem::reinitNeighborPhys(const Elem * neighbor,
                                     const std::vector<Point> & physical_points,
                                     const THREAD_ID tid)
{
  mooseAssert(_mesh.queryElemPtr(neighbor->id()) == neighbor,
              "Are you calling this method with a undisplaced mesh element?");

  for (const auto nl_sys_num : index_range(_displaced_solver_systems))
  {
    // Reinit shape functions
    _assembly[tid][nl_sys_num]->reinitNeighborAtPhysical(neighbor, physical_points);

    // Set the neighbor dof indices
    _displaced_solver_systems[nl_sys_num]->prepareNeighbor(tid);
  }
  _displaced_aux->prepareNeighbor(tid);

  prepareAssemblyNeighbor(tid);

  // Compute values at the points
  for (auto & nl : _displaced_solver_systems)
    nl->reinitNeighbor(neighbor, tid);
  _displaced_aux->reinitNeighbor(neighbor, tid);
}

void
DisplacedProblem::reinitElemNeighborAndLowerD(const Elem * elem,
                                              unsigned int side,
                                              const THREAD_ID tid)
{
  reinitNeighbor(elem, side, tid);

  const Elem * lower_d_elem = _mesh.getLowerDElem(elem, side);
  if (lower_d_elem && _mesh.interiorLowerDBlocks().count(lower_d_elem->subdomain_id()) > 0)
    reinitLowerDElem(lower_d_elem, tid);
  else
  {
    // with mesh refinement, lower-dimensional element might be defined on neighbor side
    auto & neighbor = _assembly[tid][currentNlSysNum()]->neighbor();
    auto & neighbor_side = _assembly[tid][currentNlSysNum()]->neighborSide();
    const Elem * lower_d_elem_neighbor = _mesh.getLowerDElem(neighbor, neighbor_side);
    if (lower_d_elem_neighbor &&
        _mesh.interiorLowerDBlocks().count(lower_d_elem_neighbor->subdomain_id()) > 0)
    {
      auto qps = _assembly[tid][currentNlSysNum()]->qPointsFaceNeighbor().stdVector();
      std::vector<Point> reference_points;
      FEInterface::inverse_map(
          lower_d_elem_neighbor->dim(), FEType(), lower_d_elem_neighbor, qps, reference_points);
      reinitLowerDElem(lower_d_elem_neighbor, tid, &qps);
    }
  }
}

void
DisplacedProblem::reinitScalars(const THREAD_ID tid,
                                bool reinit_for_derivative_reordering /*=false*/)
{
  for (auto & nl : _displaced_solver_systems)
    nl->reinitScalars(tid, reinit_for_derivative_reordering);
  _displaced_aux->reinitScalars(tid, reinit_for_derivative_reordering);
}

void
DisplacedProblem::reinitOffDiagScalars(const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->prepareOffDiagScalar();
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
DisplacedProblem::addResidual(const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->addResidual(Assembly::GlobalDataKey{},
                                                 currentResidualVectorTags());
}

void
DisplacedProblem::addResidualNeighbor(const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->addResidualNeighbor(Assembly::GlobalDataKey{},
                                                         currentResidualVectorTags());
}

void
DisplacedProblem::addResidualLower(const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->addResidualLower(Assembly::GlobalDataKey{},
                                                      currentResidualVectorTags());
}

void
DisplacedProblem::addCachedResidualDirectly(NumericVector<Number> & residual, const THREAD_ID tid)
{
  if (_displaced_solver_systems[currentNlSysNum()]->hasVector(
          _displaced_solver_systems[currentNlSysNum()]->timeVectorTag()))
    _assembly[tid][currentNlSysNum()]->addCachedResidualDirectly(
        residual,
        Assembly::GlobalDataKey{},
        getVectorTag(_displaced_solver_systems[currentNlSysNum()]->timeVectorTag()));

  if (_displaced_solver_systems[currentNlSysNum()]->hasVector(
          _displaced_solver_systems[currentNlSysNum()]->nonTimeVectorTag()))
    _assembly[tid][currentNlSysNum()]->addCachedResidualDirectly(
        residual,
        Assembly::GlobalDataKey{},
        getVectorTag(_displaced_solver_systems[currentNlSysNum()]->nonTimeVectorTag()));

  // We do this because by adding the cached residual directly, we cannot ensure that all of the
  // cached residuals are emptied after only the two add calls above
  _assembly[tid][currentNlSysNum()]->clearCachedResiduals(Assembly::GlobalDataKey{});
}

void
DisplacedProblem::setResidual(NumericVector<Number> & residual, const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->setResidual(
      residual,
      Assembly::GlobalDataKey{},
      getVectorTag(_displaced_solver_systems[currentNlSysNum()]->residualVectorTag()));
}

void
DisplacedProblem::setResidualNeighbor(NumericVector<Number> & residual, const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->setResidualNeighbor(
      residual,
      Assembly::GlobalDataKey{},
      getVectorTag(_displaced_solver_systems[currentNlSysNum()]->residualVectorTag()));
}

void
DisplacedProblem::addJacobian(const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->addJacobian(Assembly::GlobalDataKey{});
}

void
DisplacedProblem::addJacobianNonlocal(const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->addJacobianNonlocal(Assembly::GlobalDataKey{});
}

void
DisplacedProblem::addJacobianNeighbor(const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->addJacobianNeighbor(Assembly::GlobalDataKey{});
}

void
DisplacedProblem::addJacobianNeighborLowerD(const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->addJacobianNeighborLowerD(Assembly::GlobalDataKey{});
}

void
DisplacedProblem::addJacobianLowerD(const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->addJacobianLowerD(Assembly::GlobalDataKey{});
}

void
DisplacedProblem::cacheJacobianNonlocal(const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->cacheJacobianNonlocal(Assembly::GlobalDataKey{});
}

void
DisplacedProblem::addJacobianBlockTags(SparseMatrix<Number> & jacobian,
                                       unsigned int ivar,
                                       unsigned int jvar,
                                       const DofMap & dof_map,
                                       std::vector<dof_id_type> & dof_indices,
                                       const std::set<TagID> & tags,
                                       const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->addJacobianBlockTags(
      jacobian, ivar, jvar, dof_map, dof_indices, Assembly::GlobalDataKey{}, tags);
}

void
DisplacedProblem::addJacobianBlockNonlocal(SparseMatrix<Number> & jacobian,
                                           unsigned int ivar,
                                           unsigned int jvar,
                                           const DofMap & dof_map,
                                           const std::vector<dof_id_type> & idof_indices,
                                           const std::vector<dof_id_type> & jdof_indices,
                                           const std::set<TagID> & tags,
                                           const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->addJacobianBlockNonlocalTags(
      jacobian, ivar, jvar, dof_map, idof_indices, jdof_indices, Assembly::GlobalDataKey{}, tags);
}

void
DisplacedProblem::addJacobianNeighbor(SparseMatrix<Number> & jacobian,
                                      unsigned int ivar,
                                      unsigned int jvar,
                                      const DofMap & dof_map,
                                      std::vector<dof_id_type> & dof_indices,
                                      std::vector<dof_id_type> & neighbor_dof_indices,
                                      const std::set<TagID> & tags,
                                      const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->addJacobianNeighborTags(jacobian,
                                                             ivar,
                                                             jvar,
                                                             dof_map,
                                                             dof_indices,
                                                             neighbor_dof_indices,
                                                             Assembly::GlobalDataKey{},
                                                             tags);
}

void
DisplacedProblem::prepareShapes(unsigned int var, const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->copyShapes(var);
}

void
DisplacedProblem::prepareFaceShapes(unsigned int var, const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->copyFaceShapes(var);
}

void
DisplacedProblem::prepareNeighborShapes(unsigned int var, const THREAD_ID tid)
{
  _assembly[tid][currentNlSysNum()]->copyNeighborShapes(var);
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
  // Since the mesh has changed, we need to make sure that we update any of our
  // MOOSE-system specific data.
  for (auto & nl : _displaced_solver_systems)
    nl->reinit();
  _displaced_aux->reinit();

  // We've performed some mesh adaptivity. We need to
  // clear any quadrature nodes such that when we build the boundary node lists in
  // MooseMesh::meshChanged we don't have any extraneous extra boundary nodes lying around
  _mesh.clearQuadratureNodes();

  _mesh.meshChanged();

  // Before performing mesh adaptivity we un-displaced the mesh. We need to re-displace the mesh and
  // then reinitialize GeometricSearchData such that we have all the correct geometric information
  // for the changed mesh
  updateMesh(/*mesh_changing=*/true);
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

bool
DisplacedProblem::solverSystemConverged(const unsigned int sys_num)
{
  return _mproblem.converged(sys_num);
}

bool
DisplacedProblem::computingPreSMOResidual(const unsigned int nl_sys_num) const
{
  return _mproblem.computingPreSMOResidual(nl_sys_num);
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
DisplacedProblem::couplingMatrix(const unsigned int nl_sys_num) const
{
  return _mproblem.couplingMatrix(nl_sys_num);
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

  for (auto & nl : _displaced_solver_systems)
    nl->initialSetup();
  _displaced_aux->initialSetup();
}

void
DisplacedProblem::timestepSetup()
{
  SubProblem::timestepSetup();

  for (auto & nl : _displaced_solver_systems)
    nl->timestepSetup();
  _displaced_aux->timestepSetup();
}

void
DisplacedProblem::customSetup(const ExecFlagType & exec_type)
{
  SubProblem::customSetup(exec_type);

  for (auto & nl : _displaced_solver_systems)
    nl->customSetup(exec_type);
  _displaced_aux->customSetup(exec_type);
}

void
DisplacedProblem::residualSetup()
{
  SubProblem::residualSetup();

  for (auto & nl : _displaced_solver_systems)
    nl->residualSetup();
  _displaced_aux->residualSetup();
}

void
DisplacedProblem::jacobianSetup()
{
  SubProblem::jacobianSetup();

  for (auto & nl : _displaced_solver_systems)
    nl->jacobianSetup();
  _displaced_aux->jacobianSetup();
}

void
DisplacedProblem::haveADObjects(const bool have_ad_objects)
{
  _have_ad_objects = have_ad_objects;
  _mproblem.SubProblem::haveADObjects(have_ad_objects);
}

std::pair<bool, unsigned int>
DisplacedProblem::determineSolverSystem(const std::string & var_name,
                                        const bool error_if_not_found) const
{
  return _mproblem.determineSolverSystem(var_name, error_if_not_found);
}

Assembly &
DisplacedProblem::assembly(const THREAD_ID tid, const unsigned int sys_num)
{
  mooseAssert(tid < _assembly.size(), "Assembly objects not initialized");
  mooseAssert(sys_num < _assembly[tid].size(),
              "System number larger than the assembly container size");
  return *_assembly[tid][sys_num];
}

const Assembly &
DisplacedProblem::assembly(const THREAD_ID tid, const unsigned int sys_num) const
{
  mooseAssert(tid < _assembly.size(), "Assembly objects not initialized");
  mooseAssert(sys_num < _assembly[tid].size(),
              "System number larger than the assembly container size");
  return *_assembly[tid][sys_num];
}

std::size_t
DisplacedProblem::numNonlinearSystems() const
{
  return _mproblem.numNonlinearSystems();
}

std::size_t
DisplacedProblem::numLinearSystems() const
{
  return _mproblem.numLinearSystems();
}

std::size_t
DisplacedProblem::numSolverSystems() const
{
  return _mproblem.numSolverSystems();
}

const std::vector<VectorTag> &
DisplacedProblem::currentResidualVectorTags() const
{
  return _mproblem.currentResidualVectorTags();
}

bool
DisplacedProblem::safeAccessTaggedMatrices() const
{
  return _mproblem.safeAccessTaggedMatrices();
}

bool
DisplacedProblem::safeAccessTaggedVectors() const
{
  return _mproblem.safeAccessTaggedVectors();
}

void
DisplacedProblem::needFV()
{
  _mproblem.needFV();
}

bool
DisplacedProblem::haveFV() const
{
  return _mproblem.haveFV();
}

bool
DisplacedProblem::hasNonlocalCoupling() const
{
  return _mproblem.hasNonlocalCoupling();
}

unsigned int
DisplacedProblem::nlSysNum(const NonlinearSystemName & nl_sys_name) const
{
  return _mproblem.nlSysNum(nl_sys_name);
}

unsigned int
DisplacedProblem::linearSysNum(const LinearSystemName & sys_name) const
{
  return _mproblem.linearSysNum(sys_name);
}

unsigned int
DisplacedProblem::solverSysNum(const SolverSystemName & sys_name) const
{
  return _mproblem.solverSysNum(sys_name);
}
