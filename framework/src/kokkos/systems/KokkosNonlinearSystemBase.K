//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosKernel.h"
#include "KokkosKernelBase.h"
#include "KokkosNodalKernelBase.h"
#include "KokkosNodalBCBase.h"
#include "KokkosDirichletBCBase.h"
#include "KokkosDirichletValueFunction.h"
#include "KokkosIntegratedBCBase.h"
#include "KokkosLevelTransfer.h"

#include "MaterialBase.h"
#include "NonlinearSystemBase.h"
#include "MooseVariableFieldBase.h"
#include "FEProblemBase.h"
#include "MoosePreconditioner.h"

#include "libmesh/petsc_matrix_shell_matrix.h"
#include "libmesh/dirichlet_boundaries.h"
#include "libmesh/fe_interface.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/periodic_boundaries.h"

namespace
{
PetscErrorCode
mooseKokkosMatShellMult(Mat shell, Vec x, Vec y)
{
  void * ctx;
  LibmeshPetscCallQ(MatShellGetContext(shell, &ctx));
  static_cast<NonlinearSystemBase *>(ctx)->computeKokkosJacobianVectorProduct(x, y);
  return LIBMESH_PETSC_SUCCESS;
}

PetscErrorCode
mooseKokkosMatShellGetDiagonal(Mat shell, Vec diag)
{
  void * ctx;
  LibmeshPetscCallQ(MatShellGetContext(shell, &ctx));
  static_cast<NonlinearSystemBase *>(ctx)->computeKokkosJacobianDiagonal(diag);
  return LIBMESH_PETSC_SUCCESS;
}
} // namespace

void
NonlinearSystemBase::addKokkosKernel(const std::string & kernel_name,
                                     const std::string & name,
                                     InputParameters & parameters)
{
  // Create the kernel object via the factory and add to warehouse
  std::shared_ptr<Moose::Kokkos::KernelBase> kernel =
      _factory.create<Moose::Kokkos::KernelBase>(kernel_name, name, parameters);
  _kokkos_kernels.addObject(kernel, 0);
  postAddResidualObject(*kernel);
}

void
NonlinearSystemBase::addKokkosNodalKernel(const std::string & kernel_name,
                                          const std::string & name,
                                          InputParameters & parameters)
{
  // Create the kernel object via the factory and add to warehouse
  std::shared_ptr<Moose::Kokkos::NodalKernelBase> kernel =
      _factory.create<Moose::Kokkos::NodalKernelBase>(kernel_name, name, parameters);
  _kokkos_nodal_kernels.addObject(kernel, 0);
  postAddResidualObject(*kernel);
}

void
NonlinearSystemBase::addKokkosBoundaryCondition(const std::string & bc_name,
                                                const std::string & name,
                                                InputParameters & parameters)
{
  // Create the object
  std::shared_ptr<Moose::Kokkos::BoundaryCondition> bc =
      _factory.create<Moose::Kokkos::BoundaryCondition>(bc_name, name, parameters, 0);
  postAddResidualObject(*bc);

  // Active BoundaryIDs for the object
  const std::set<BoundaryID> & boundary_ids = bc->boundaryIDs();
  auto bc_var = dynamic_cast<const MooseVariableFieldBase *>(&bc->variable());
  _vars[0].addBoundaryVar(boundary_ids, bc_var);

  // Cast to the various types of BCs
  std::shared_ptr<Moose::Kokkos::NodalBCBase> nbc =
      std::dynamic_pointer_cast<Moose::Kokkos::NodalBCBase>(bc);
  std::shared_ptr<Moose::Kokkos::IntegratedBCBase> ibc =
      std::dynamic_pointer_cast<Moose::Kokkos::IntegratedBCBase>(bc);

  // NodalBCBase
  if (nbc)
  {
    if (!nbc->variable().isNodal())
      mooseError("Trying to use nodal boundary condition '",
                 nbc->name(),
                 "' on a non-nodal variable '",
                 nbc->variable().name(),
                 "'.");

    _kokkos_nodal_bcs.addObject(nbc);
    _vars[0].addBoundaryVars(boundary_ids, nbc->getCoupledVars());

    // DirichletBCs that are preset
    if (nbc->preset())
      _kokkos_preset_nodal_bcs.addObject(nbc);
  }

  // IntegratedBCBase
  else if (ibc)
  {
    _kokkos_integrated_bcs.addObject(ibc);
    _vars[0].addBoundaryVars(boundary_ids, ibc->getCoupledVars());
  }

  else
    mooseError("Unknown Kokkos BoundaryCondition type for object named ", bc->name());
}

void
NonlinearSystemBase::setKokkosInitialSolution()
{
  // The preset Dirichlet BCs can have coupled variables from other systems, so we grab all systems
  auto & systems = _fe_problem.getKokkosFESystems();

  auto tag = _fe_problem.addVectorTag("parallel_solution", Moose::VECTOR_TAG_SOLUTION);

  associateVectorToTag(solution(), tag);

  std::set<TagID> needed_fe_var_vector_tags = {tag};

  for (auto boundary : _fe_problem.mesh().meshBoundaryIds())
    _kokkos_preset_nodal_bcs.updateBoundaryFEVariableCoupledVectorTagDependency(
        boundary, needed_fe_var_vector_tags);

  for (auto & system : systems.constructedEntries())
    system.sync(needed_fe_var_vector_tags, Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);

  systems.copyToDevice();

  for (auto & nbc : _kokkos_preset_nodal_bcs.getActiveObjects())
    std::static_pointer_cast<Moose::Kokkos::NodalBCBase>(nbc)->presetSolution(tag);

  // Every DOF a preset Dirichlet-type boundary condition pins -- vertex and bubble (e.g.
  // HIERARCHIC edge/face mode) alike -- overwritten last since the per-boundary-condition preset
  // above only reaches a boundary's mesh nodes
  systems[number()].presetConstrainedSolution(tag);

  Kokkos::fence();

  for (auto & system : systems.constructedEntries())
    system.sync(needed_fe_var_vector_tags, Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);

  disassociateVectorFromTag(solution(), tag);
}

void
NonlinearSystemBase::setupKokkosMatrixFreeSystemMatrix()
{
  // Register a shell under the name libMesh uses for an implicit system's own matrix, so that
  // ImplicitSystem::add_matrices() finds it already present and keeps it. That matrix is what
  // PetscNonlinearSolver::init() hands SNESSetJacobian for both Amat and Pmat, so the operator
  // action serves Amat and the partially assembled diagonal serves everything a preconditioner
  // reads from Pmat, with no compressed-row matrix allocated for this system at all.
  //
  // Running before EquationSystems::init() makes late_matrix_init() a no-op here; the ordinary
  // System::init_matrices() then attaches the DofMap and calls
  // PetscMatrixShellMatrix::init(ParallelType), which sizes the shell from that DofMap.
  _matrix_free_operator = &system().add_matrix(
      "System Matrix", std::make_unique<libMesh::PetscMatrixShellMatrix<Number>>(comm()));
}

void
NonlinearSystemBase::setupKokkosDirichletConstraints()
{
  auto & dof_map = _sys.get_dof_map();

  // Null until the system has at least one periodic boundary
  const auto * periodic_boundaries = dof_map.get_periodic_boundaries();

  bool registered = false;

  for (auto & nbc : _kokkos_nodal_bcs.getObjects(0))
  {
    auto * bc = dynamic_cast<Moose::Kokkos::NodalBCBase *>(nbc.get());

    if (!bc || !bc->isDirichletBC())
      continue;

    // Enforcing this boundary condition beyond a mesh node requires disabling libMesh's exact
    // constraint enforcement for the whole system below, which would also stop enforcing any
    // periodic constraint already registered on it
    if (periodic_boundaries && !periodic_boundaries->empty())
      mooseError("Kokkos matrix-free boundary condition '",
                 bc->name(),
                 "' of type '",
                 bc->type(),
                 "' is a Dirichlet-type boundary condition, but system '",
                 name(),
                 "' also has a periodic boundary condition. This combination is not yet "
                 "supported for a Kokkos matrix-free system.");

    // A C_ONE family's projection needs a gradient functor no MOOSE Dirichlet boundary condition
    // supplies (libMesh's ConstrainDirichlet::apply_dirichlet_impl only calls a plain-valued
    // libMesh::FunctionBase for it)
    if (libMesh::FEInterface::get_continuity(bc->variable().feType()) == libMesh::C_ONE)
      mooseError("Kokkos matrix-free boundary condition '",
                 bc->name(),
                 "' of type '",
                 bc->type(),
                 "' is a Dirichlet-type boundary condition on variable '",
                 bc->variable().name(),
                 "', whose finite element family has C1 continuity. This is not supported.");

    dof_map.add_dirichlet_boundary(libMesh::DirichletBoundary(
        bc->boundaryIDs(), {bc->variable().number()}, Moose::Kokkos::DirichletValueFunction(*bc)));

    registered = true;
  }

  // With no Dirichlet boundary registered above, exact constraint enforcement stays at libMesh's
  // default so any periodic constraint on this system continues to be enforced as before
  if (registered)
    libMesh::cast_ref<libMesh::NonlinearImplicitSystem &>(_sys)
        .nonlinear_solver->set_exact_constraint_enforcement(false);
}

void
NonlinearSystemBase::refreshKokkosDirichletConstraints()
{
  reinitConstraints();

  _fe_problem.getKokkosFESystems()[number()].setupConstraintOperator();
}

void
NonlinearSystemBase::setupKokkosMatrixFreeJacobian()
{
  if (_kokkos_mf_enabled)
    return;

  using Moose::Kokkos::DispatcherRegistry;

  // Suffixed with name() because these tags are added at the FEProblem level and would otherwise
  // collide across multiple matrix-free systems
  _kokkos_mf_x_tag = _fe_problem.addVectorTag(name() + "_kokkos_mf_x", Moose::VECTOR_TAG_SOLUTION);
  _kokkos_mf_y_tag = _fe_problem.addVectorTag(name() + "_kokkos_mf_y", Moose::VECTOR_TAG_RESIDUAL);
  _kokkos_mf_diag_tag =
      _fe_problem.addVectorTag(name() + "_kokkos_mf_diag", Moose::VECTOR_TAG_RESIDUAL);

  _kokkos_mf_x = &addVector(_kokkos_mf_x_tag, false, libMesh::GHOSTED);

  auto & fe_system = _fe_problem.getKokkosFESystems()[number()];

  fe_system.enableQpJacobianCache();

  for (auto & kernel : _kokkos_kernels.getObjects(0))
  {
    auto & object = *std::static_pointer_cast<Moose::Kokkos::KernelBase>(kernel);

    // A kernel that supplies its quadrature-point linearization is served by the cache, whose
    // consumers contract it against the basis tables themselves
    const bool cached = object.usesQpJacobianCache();

    // A kernel outside the cache must implement the operator-action loop itself. When it does
    // neither, its matvec hook is an abort-by-default stub; catch that here with a clean error
    // instead of a device abort.
    if (DispatcherRegistry::hasUserMethod<Moose::Kokkos::ResidualObject::JacobianLoop>(
            object.type()) &&
        !DispatcherRegistry::hasUserMethod<
            Moose::Kokkos::ResidualObject::JacobianVectorProductLoop>(object.type()) &&
        !cached)
      mooseError("The Kokkos matrix-free Jacobian-vector product does not support kernel '",
                 object.name(),
                 "' of type '",
                 object.type(),
                 "'. Define computeQpJacobianTensor() so its linearization enters the "
                 "quadrature-point Jacobian cache.");

    // The cache is keyed on (subdomain, variable) and every operator-action loop contracts against
    // the kernel's own variable, so a coupling to another variable would be dropped silently.
    if (DispatcherRegistry::hasUserMethod<Moose::Kokkos::ResidualObject::OffDiagJacobianLoop>(
            object.type()))
      mooseError("The Kokkos matrix-free Jacobian-vector product does not support kernel '",
                 object.name(),
                 "' of type '",
                 object.type(),
                 "', which has an off-diagonal Jacobian contribution.");

    if (cached)
    {
      const auto blocks = object.qpJacobianBlocks();

      if (blocks == Moose::Kokkos::QP_JACOBIAN_NONE)
        mooseError("Kokkos kernel '",
                   object.name(),
                   "' of type '",
                   object.type(),
                   "' defines computeQpJacobianTensor() but declares no quadrature-point "
                   "Jacobian blocks. Override qpJacobianBlocks() to return the blocks it "
                   "populates.");

      for (const auto subdomain : _mesh.meshSubdomains())
        if (object.hasBlocks(subdomain))
          fe_system.addQpJacobianBlocks(subdomain, object.variable().number(), blocks);
    }

    object.setMatrixFreeTags(_kokkos_mf_x_tag, _kokkos_mf_y_tag, _kokkos_mf_diag_tag);
  }

  // Integrated boundary conditions contribute to the Jacobian through the same element assembly the
  // kernels use, and nothing in the matrix-free path evaluates them, so any Jacobian contribution
  // they carry would be missing from the operator action.
  for (auto & ibc : _kokkos_integrated_bcs.getObjects(0))
  {
    auto & object = *std::static_pointer_cast<Moose::Kokkos::ResidualObject>(ibc);

    if (DispatcherRegistry::hasUserMethod<Moose::Kokkos::ResidualObject::JacobianLoop>(
            object.type()) ||
        DispatcherRegistry::hasUserMethod<Moose::Kokkos::ResidualObject::OffDiagJacobianLoop>(
            object.type()))
      mooseError("The Kokkos matrix-free Jacobian-vector product does not support the integrated "
                 "boundary condition '",
                 object.name(),
                 "' of type '",
                 object.type(),
                 "', which has a Jacobian contribution.");
  }

  for (auto & nbc : _kokkos_nodal_bcs.getObjects(0))
  {
    auto & object = *std::static_pointer_cast<Moose::Kokkos::ResidualObject>(nbc);

    if (DispatcherRegistry::hasUserMethod<Moose::Kokkos::ResidualObject::OffDiagJacobianLoop>(
            object.type()))
      mooseError("The Kokkos matrix-free Jacobian-vector product does not support the nodal "
                 "boundary condition '",
                 object.name(),
                 "', which has an off-diagonal Jacobian contribution.");

    object.setMatrixFreeTags(_kokkos_mf_x_tag, _kokkos_mf_y_tag, _kokkos_mf_diag_tag);
  }

  // Automatic scaling reads its factors from the diagonal of an assembled Jacobian, which a
  // matrix-free system never forms.
  if (automaticScaling())
    mooseError("Automatic scaling is not supported for a Kokkos matrix-free system, which forms no "
               "assembled Jacobian for the scaling factors to be read from.");

  // Every Jacobian contribution has to come from a Kokkos object. With no compressed-row matrix in
  // the system, the assembly sweep serving the non-Kokkos warehouses has nowhere to write, so a
  // Jacobian contribution from one of them would be missing from the operator action.
  auto check_non_kokkos = [](const auto & warehouse, const std::string & description)
  {
    if (warehouse.hasObjects())
      mooseError("The Kokkos matrix-free Jacobian does not support ",
                 description,
                 ", whose Jacobian contribution is assembled into a matrix a matrix-free system "
                 "does not have.");
  };

  check_non_kokkos(_kernels, "non-Kokkos kernels");
  check_non_kokkos(_nodal_kernels, "non-Kokkos nodal kernels");
  check_non_kokkos(_scalar_kernels, "scalar kernels");
  check_non_kokkos(_dg_kernels, "DG kernels");
  check_non_kokkos(_interface_kernels, "interface kernels");
  check_non_kokkos(_hybridized_kernels, "hybridized kernels");
  check_non_kokkos(_dirac_kernels, "Dirac kernels");
  check_non_kokkos(_integrated_bcs, "non-Kokkos integrated boundary conditions");
  check_non_kokkos(_nodal_bcs, "non-Kokkos nodal boundary conditions");
  check_non_kokkos(_constraints, "constraints");

  // With the whole residual coming from the objects audited above, a Dirichlet-constrained DOF
  // reaches a free row only through the solution the quadrature-point cache is built from, so
  // enforcing the constraints on that solution removes the dependence entirely and the operator can
  // eliminate the matching column, making it symmetric.
  //
  // A transient is the exception: the time integrator forms the solution time derivative on the
  // host out of the raw solution, so a free row's mass contribution keeps reading a constrained
  // DOF's own iterate value no matter what the cache is built from. Eliminating the column there
  // would leave the operator disagreeing with the residual it linearizes, so a transient keeps the
  // constrained columns and the asymmetric operator that comes with them.
  if (!_fe_problem.isTransient())
    fe_system.eliminateConstrainedColumns();

  // The system matrix installed by setupKokkosMatrixFreeSystemMatrix() is already both Amat and
  // Pmat, so all that remains is registering the two PETSc operations the solve calls: MatMult for
  // the operator action, and MatGetDiagonal for the partially assembled diagonal a Jacobi or
  // Chebyshev preconditioner smooths with.
  auto & shell = libMesh::cast_ref<libMesh::PetscMatrixShellMatrix<Number> &>(
      libMesh::cast_ref<libMesh::ImplicitSystem &>(system()).get_system_matrix());

  LibmeshPetscCallA(comm().get(), MatShellSetContext(shell.mat(), this));
  LibmeshPetscCallA(
      comm().get(),
      MatShellSetOperation(shell.mat(), MATOP_MULT, (void (*)(void))mooseKokkosMatShellMult));
  LibmeshPetscCallA(comm().get(),
                    MatShellSetOperation(shell.mat(),
                                         MATOP_GET_DIAGONAL,
                                         (void (*)(void))mooseKokkosMatShellGetDiagonal));

  _kokkos_mf_enabled = true;
}

void
NonlinearSystemBase::computeKokkosJacobianDiagonal(Vec diag)
{
  TIME_SECTION("computeKokkosJacobianDiagonal", 1);

  LibmeshPetscCallA(comm().get(), VecZeroEntries(diag));

  if (!_kokkos_kernels.hasActiveObjects() && !_kokkos_nodal_bcs.hasActiveObjects())
  {
    LibmeshPetscCallA(comm().get(), VecAssemblyBegin(diag));
    LibmeshPetscCallA(comm().get(), VecAssemblyEnd(diag));
    return;
  }

  libMesh::PetscVector<Number> diag_wrap(diag, comm());
  associateVectorToTag(diag_wrap, _kokkos_mf_diag_tag);

  auto & systems = _fe_problem.getKokkosFESystems();

  systems[number()].setActiveResidualTags({_kokkos_mf_diag_tag});

  // No reinit(): kernels read _u/_grad_u from the qp-solution cache populated the last time
  // computeKokkosJacobian() ran, at the same Newton point this diagonal must reflect.
  for (auto & system : systems.constructedEntries())
    system.sync(Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);

  systems.copyToDevice();

  if (!systems[number()].qpJacobianCacheValid())
    mooseError("The Kokkos quadrature-point Jacobian cache does not hold a linearization. The "
               "Jacobian diagonal must be requested only after a Jacobian evaluation at the "
               "solution state it is to reflect.");

  systems[number()].computeQpJacobianDiagonal(_kokkos_mf_diag_tag, systemMatrixTag());

  for (auto & kernel : _kokkos_kernels.getActiveObjects())
  {
    auto & object = static_cast<Moose::Kokkos::KernelBase &>(*kernel);

    if (!object.usesQpJacobianCache())
      object.computeJacobianDiagonal();
  }

  for (auto & nbc : _kokkos_nodal_bcs.getActiveObjects())
    std::static_pointer_cast<Moose::Kokkos::NodalBCBase>(nbc)->computeJacobianDiagonal();

  // Every DOF a registered Dirichlet-type boundary condition pins -- vertex and bubble (e.g.
  // HIERARCHIC edge/face mode) alike -- overwritten last since the per-boundary-condition diagonal
  // above only reaches a boundary's mesh nodes
  systems[number()].finalizeConstrainedDiagonal(_kokkos_mf_diag_tag, 1.0);

  for (auto & system : systems.constructedEntries())
    system.sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);

  systems[number()].clearActiveResidualTags();

  disassociateVectorFromTag(diag_wrap, _kokkos_mf_diag_tag);

  LibmeshPetscCallA(comm().get(), VecAssemblyBegin(diag));
  LibmeshPetscCallA(comm().get(), VecAssemblyEnd(diag));
}

void
NonlinearSystemBase::computeKokkosResidual(const std::set<TagID> & tags)
{
  TIME_SECTION("computeKokkosResidual", 1);

  // Get warehouses

  const auto & kernels = _kokkos_kernels.getVectorTagsObjectWarehouse(tags, 0);
  const auto & nodal_kernels = _kokkos_nodal_kernels.getVectorTagsObjectWarehouse(tags, 0);
  const auto & integrated_bcs = _kokkos_integrated_bcs.getVectorTagsObjectWarehouse(tags, 0);

  if (!kernels.size() && !nodal_kernels.size() && !integrated_bcs.size())
    return;

  // Resolve dependencies

  auto & systems = _fe_problem.getKokkosFESystems();

  systems[number()].setActiveResidualTags(tags);

  std::set<MooseVariableFieldBase *> needed_moose_vars;
  std::set<TagID> needed_fe_var_vector_tags;
  std::unordered_set<unsigned int> needed_mat_props;

  for (auto tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    needed_fe_var_vector_tags.insert(tag._id);

  kernels.updateVariableDependency(needed_moose_vars);
  kernels.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  kernels.updateMatPropDependency(needed_mat_props);

  nodal_kernels.updateVariableDependency(needed_moose_vars);
  nodal_kernels.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);

  integrated_bcs.updateVariableDependency(needed_moose_vars);
  integrated_bcs.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  integrated_bcs.updateMatPropDependency(needed_mat_props);

  if (needed_mat_props.size())
  {
    _fe_problem.getKokkosMaterialsWarehouse().updateVariableDependency(needed_moose_vars);
    _fe_problem.getKokkosMaterialsWarehouse().updateFEVariableCoupledVectorTagDependency(
        needed_fe_var_vector_tags);
  }

  // Copy solution vectors and residuals and cache variable values at element quadature
  // points

  for (auto & system : systems.constructedEntries())
  {
    system.setActiveVariables(needed_moose_vars);
    system.setActiveSolutionTags(needed_fe_var_vector_tags);

    {
      TIME_SECTION("KokkosCopy", 1);
      system.sync(Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);
    }
    {
      TIME_SECTION("KokkosReinit", 1);
      system.reinit();
    }
  }

  systems.copyToDevice();

  {
    TIME_SECTION("KokkosMaterial", 1);

    // Compute material properties

    if (needed_mat_props.size())
    {
      _fe_problem.prepareKokkosMaterials(needed_mat_props);
      _fe_problem.reinitKokkosMaterials();
    }
  }

  {
    TIME_SECTION("KokkosKernel", 1);

    // Compute kernels

    for (auto & kernel : kernels.getActiveObjects())
      kernel->computeResidual();

    for (auto & nodal_kernel : nodal_kernels.getActiveObjects())
      nodal_kernel->computeResidual();

    for (auto & ibc : integrated_bcs.getActiveObjects())
      ibc->computeResidual();
  }

  // Close and restore vectors

  {
    TIME_SECTION("KokkosClose", 1);

    for (auto & system : systems.constructedEntries())
      system.sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);
  }

  // Clear

  systems[number()].clearActiveResidualTags();

  for (auto & system : systems.constructedEntries())
  {
    system.clearActiveVariables();
    system.clearActiveSolutionTags();
  }
}

void
NonlinearSystemBase::computeKokkosNodalBCsResidual(const std::set<TagID> & tags)
{
  TIME_SECTION("computeKokkosNodalBCsResidual", 1);

  // Get warehouses

  const auto & nodal_bcs = _kokkos_nodal_bcs.getVectorTagsObjectWarehouse(tags, 0);

  if (!nodal_bcs.hasActiveObjects())
    return;

  // Resolve dependencies

  auto & systems = _fe_problem.getKokkosFESystems();

  systems[number()].setActiveResidualTags(tags);

  std::set<TagID> needed_fe_var_vector_tags;

  for (auto tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    needed_fe_var_vector_tags.insert(tag._id);

  nodal_bcs.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);

  // Copy solution vectors and residuals

  for (auto & system : systems.constructedEntries())
  {
    system.setActiveSolutionTags(needed_fe_var_vector_tags);
    {
      TIME_SECTION("KokkosCopy", 1);
      system.sync(Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);
    }
  }

  systems.copyToDevice();

  {
    TIME_SECTION("KokkosKernel", 1);

    // Compute kernels

    for (auto & nbc : nodal_bcs.getActiveObjects())
      nbc->computeResidual();

    // Every DOF a registered Dirichlet-type boundary condition pins -- vertex and bubble (e.g.
    // HIERARCHIC edge/face mode) alike -- overwritten last since the per-boundary-condition
    // residual above only reaches a boundary's mesh nodes.
    //
    // The row's whole equation goes into each tag being computed, which is safe because this pass
    // runs after a time integrator's postResidual() has already summed the tagged vectors into the
    // residual the solver is handed, and because the write replaces the row rather than adding to
    // it. Both halves matter: were this pass to run before that summation, a transient computing a
    // time and a non-time tag together would count the row's equation once per tag.
    const auto solution_tag = _fe_problem.getVectorTagID(Moose::SOLUTION_TAG);

    for (auto tag : tags)
      systems[number()].finalizeConstrainedResidual(tag, solution_tag);
  }

  // Close and restore vectors

  {
    TIME_SECTION("KokkosClose", 1);

    for (auto & system : systems.constructedEntries())
      system.sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);
  }

  // Clear

  systems[number()].clearActiveResidualTags();

  for (auto & system : systems.constructedEntries())
    system.clearActiveSolutionTags();
}

void
NonlinearSystemBase::computeKokkosJacobian(const std::set<TagID> & tags)
{
  TIME_SECTION("computeKokkosJacobian", 1);

  // Get warehouses

  const auto & kernels = _kokkos_kernels.getMatrixTagsObjectWarehouse(tags, 0);
  const auto & nodal_kernels = _kokkos_nodal_kernels.getMatrixTagsObjectWarehouse(tags, 0);
  const auto & integrated_bcs = _kokkos_integrated_bcs.getMatrixTagsObjectWarehouse(tags, 0);
  const auto & nodal_bcs = _kokkos_nodal_bcs.getMatrixTagsObjectWarehouse(tags, 0);

  if (!kernels.hasActiveObjects() && !nodal_kernels.hasActiveObjects() &&
      !integrated_bcs.hasActiveObjects() && !nodal_bcs.hasActiveObjects())
    return;

  // Resolve dependencies

  auto & systems = _fe_problem.getKokkosFESystems();

  systems[number()].setActiveMatrixTags(tags);

  std::set<MooseVariableFieldBase *> needed_moose_vars;
  std::set<TagID> needed_fe_var_vector_tags;
  std::unordered_set<unsigned int> needed_mat_props;

  for (auto tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    needed_fe_var_vector_tags.insert(tag._id);

  kernels.updateVariableDependency(needed_moose_vars);
  kernels.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  kernels.updateMatPropDependency(needed_mat_props);

  nodal_kernels.updateVariableDependency(needed_moose_vars);
  nodal_kernels.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);

  integrated_bcs.updateVariableDependency(needed_moose_vars);
  integrated_bcs.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  integrated_bcs.updateMatPropDependency(needed_mat_props);

  nodal_bcs.updateVariableDependency(needed_moose_vars);
  nodal_bcs.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);

  if (needed_mat_props.size())
  {
    _fe_problem.getKokkosMaterialsWarehouse().updateVariableDependency(needed_moose_vars);
    _fe_problem.getKokkosMaterialsWarehouse().updateFEVariableCoupledVectorTagDependency(
        needed_fe_var_vector_tags);
  }

  // Copy solution vectors, initialize matrices, and cache variable values at element quadature
  // points

  for (auto & system : systems.constructedEntries())
  {
    system.setActiveVariables(needed_moose_vars);
    system.setActiveSolutionTags(needed_fe_var_vector_tags);

    {
      TIME_SECTION("KokkosCopy", 1);
      system.sync(Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);
    }
    {
      TIME_SECTION("KokkosReinit", 1);
      system.reinit();
    }
  }

  systems.copyToDevice();

  {
    TIME_SECTION("KokkosMaterial", 1);

    // Compute material properties

    if (needed_mat_props.size())
    {
      _fe_problem.prepareKokkosMaterials(needed_mat_props);
      _fe_problem.reinitKokkosMaterials();
    }
  }

  {
    TIME_SECTION("KokkosKernel", 1);

    // Compute kernels. A matrix-free system has no compressed-row matrix for these to accumulate
    // into: the quadrature-point cache filled below carries everything the operator action and its
    // preconditioner need, including the rows the nodal boundary conditions constrain.
    if (!_kokkos_mf_enabled)
    {
      for (auto & kernel : kernels.getActiveObjects())
        kernel->computeJacobian();

      for (auto & nodal_kernel : nodal_kernels.getActiveObjects())
        nodal_kernel->computeJacobian();

      for (auto & ibc : integrated_bcs.getActiveObjects())
        ibc->computeJacobian();

      for (auto & nbc : nodal_bcs.getActiveObjects())
        nbc->computeJacobian();
    }

    // Refill the quadrature-point Jacobian cache, which the matrix-free operator action and
    // diagonal are contracted from, at the same solution state this sweep was evaluated at
    if (_kokkos_mf_enabled)
    {
      {
        TIME_SECTION("KokkosQpJacobianCache", 2);

        systems[number()].clearQpJacobianCache();

        for (auto & kernel : kernels.getActiveObjects())
        {
          auto & object = static_cast<Moose::Kokkos::KernelBase &>(*kernel);

          if (object.usesQpJacobianCache())
            object.computeQpJacobianCache();
        }

        systems[number()].validateQpJacobianCache();
      }

      // Reported outside the timed section so the one-time diagnostic is not attributed to the fill
      reportKokkosMatrixFreeCoverage();

      // A preconditioner whose operators are contracted from this linearization builds or checks
      // them here, while the tags the sweep gathered it under are still held
      if (_preconditioner)
        _preconditioner->postLinearization();
    }
  }

  // Close and restore vectors and matrices

  {
    TIME_SECTION("KokkosClose", 1);

    for (auto & system : systems.constructedEntries())
      system.sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);
  }

  // Clear

  systems[number()].clearActiveMatrixTags();

  for (auto & system : systems.constructedEntries())
  {
    system.clearActiveVariables();
    system.clearActiveSolutionTags();
  }

  // This system's matrix-free operator manages the tags it needs itself, so it may only be applied
  // once the Jacobian sweep has released the tags it held: the state here is the state an operator
  // application sees when the solve asks for one
  if (_kokkos_mf_enabled && _preconditioner)
    _preconditioner->postJacobianAssembly();
}

void
NonlinearSystemBase::computeKokkosResidualAndJacobian(const std::set<TagID> & vector_tags,
                                                      const std::set<TagID> & matrix_tags)
{
  TIME_SECTION("computeKokkosResidualAndJacobian", 1);

  if (!_kokkos_kernels.hasActiveObjects() && !_kokkos_nodal_kernels.hasActiveObjects() &&
      !_kokkos_integrated_bcs.hasActiveObjects() && !_kokkos_nodal_bcs.hasActiveObjects())
    return;

  // Resolve dependencies

  auto & systems = _fe_problem.getKokkosFESystems();

  systems[number()].setActiveResidualTags(vector_tags);
  systems[number()].setActiveMatrixTags(matrix_tags);

  std::set<MooseVariableFieldBase *> needed_moose_vars;
  std::set<TagID> needed_fe_var_vector_tags;
  std::unordered_set<unsigned int> needed_mat_props;

  for (auto tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    needed_fe_var_vector_tags.insert(tag._id);

  _kokkos_kernels.updateVariableDependency(needed_moose_vars);
  _kokkos_kernels.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  _kokkos_kernels.updateMatPropDependency(needed_mat_props);

  _kokkos_nodal_kernels.updateVariableDependency(needed_moose_vars);
  _kokkos_nodal_kernels.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);

  _kokkos_integrated_bcs.updateVariableDependency(needed_moose_vars);
  _kokkos_integrated_bcs.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);
  _kokkos_integrated_bcs.updateMatPropDependency(needed_mat_props);

  _kokkos_nodal_bcs.updateVariableDependency(needed_moose_vars);
  _kokkos_nodal_bcs.updateFEVariableCoupledVectorTagDependency(needed_fe_var_vector_tags);

  if (needed_mat_props.size())
  {
    _fe_problem.getKokkosMaterialsWarehouse().updateVariableDependency(needed_moose_vars);
    _fe_problem.getKokkosMaterialsWarehouse().updateFEVariableCoupledVectorTagDependency(
        needed_fe_var_vector_tags);
  }

  // Copy solution vectors and residuals, initialize matrices, and cache variable values at element
  // quadature points

  for (auto & system : systems.constructedEntries())
  {
    system.setActiveVariables(needed_moose_vars);
    system.setActiveSolutionTags(needed_fe_var_vector_tags);

    {
      TIME_SECTION("KokkosCopy", 1);
      system.sync(Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);
    }
    {
      TIME_SECTION("KokkosReinit", 1);
      system.reinit();
    }
  }

  systems.copyToDevice();

  {
    TIME_SECTION("KokkosMaterial", 1);

    // Compute material properties

    if (needed_mat_props.size())
    {
      _fe_problem.prepareKokkosMaterials(needed_mat_props);
      _fe_problem.reinitKokkosMaterials();
    }
  }

  {
    TIME_SECTION("KokkosKernel", 1);

    // Compute kernels. A matrix-free system has no compressed-row matrix for the Jacobian half of
    // these contributions to accumulate into, so only the residual half is swept there; the
    // quadrature-point cache filled below carries everything the operator action and its
    // preconditioner need, including the rows the nodal boundary conditions constrain.
    if (_kokkos_mf_enabled)
    {
      for (auto & kernel : _kokkos_kernels.getActiveObjects())
        kernel->computeResidual();

      for (auto & nodal_kernel : _kokkos_nodal_kernels.getActiveObjects())
        nodal_kernel->computeResidual();

      for (auto & ibc : _kokkos_integrated_bcs.getActiveObjects())
        ibc->computeResidual();
    }
    else
    {
      for (auto & kernel : _kokkos_kernels.getActiveObjects())
        kernel->computeResidualAndJacobian();

      for (auto & nodal_kernel : _kokkos_nodal_kernels.getActiveObjects())
        nodal_kernel->computeResidualAndJacobian();

      for (auto & ibc : _kokkos_integrated_bcs.getActiveObjects())
        ibc->computeResidualAndJacobian();

      /// Nodal BC residuals are computed separately
      for (auto & nbc : _kokkos_nodal_bcs.getActiveObjects())
        nbc->computeJacobian();
    }

    // Refill the quadrature-point Jacobian cache, which the matrix-free operator action and
    // diagonal are contracted from, at the same solution state this sweep was evaluated at
    if (_kokkos_mf_enabled)
    {
      {
        TIME_SECTION("KokkosQpJacobianCache", 2);

        systems[number()].clearQpJacobianCache();

        for (auto & kernel : _kokkos_kernels.getActiveObjects())
        {
          auto & object = static_cast<Moose::Kokkos::KernelBase &>(*kernel);

          if (object.usesQpJacobianCache())
            object.computeQpJacobianCache();
        }

        systems[number()].validateQpJacobianCache();
      }

      // Reported outside the timed section so the one-time diagnostic is not attributed to the fill
      reportKokkosMatrixFreeCoverage();

      // A preconditioner whose operators are contracted from this linearization builds or checks
      // them here, while the tags the sweep gathered it under are still held
      if (_preconditioner)
        _preconditioner->postLinearization();
    }
  }

  // Close and restore vectors and matrices

  {
    TIME_SECTION("KokkosClose", 1);

    for (auto & system : systems.constructedEntries())
      system.sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);
  }

  // Clear

  systems[number()].clearActiveResidualTags();
  systems[number()].clearActiveMatrixTags();

  for (auto & system : systems.constructedEntries())
  {
    system.clearActiveVariables();
    system.clearActiveSolutionTags();
  }

  // This system's matrix-free operator manages the tags it needs itself, so it may only be applied
  // once the Jacobian sweep has released the tags it held: the state here is the state an operator
  // application sees when the solve asks for one
  if (_kokkos_mf_enabled && _preconditioner)
    _preconditioner->postJacobianAssembly();
}

void
NonlinearSystemBase::reportKokkosMatrixFreeCoverage()
{
  if (_kokkos_mf_reported)
    return;

  _kokkos_mf_reported = true;

  using Moose::Kokkos::DispatcherRegistry;

  _console << "\nKokkos matrix-free Jacobian coverage for system '" << name() << "':\n";

  for (auto & kernel : _kokkos_kernels.getObjects(0))
  {
    auto & object = *std::static_pointer_cast<Moose::Kokkos::KernelBase>(kernel);

    std::string coverage;

    if (object.usesQpJacobianCache())
      coverage = "quadrature-point Jacobian cache";
    else if (DispatcherRegistry::hasUserMethod<
                 Moose::Kokkos::ResidualObject::JacobianVectorProductLoop>(object.type()))
      coverage = "operator action of its own";
    else
      coverage = "no Jacobian contribution";

    _console << "  " << object.name() << " (" << object.type() << "): " << coverage << '\n';
  }

  auto bytes = _fe_problem.getKokkosFESystems()[number()].qpJacobianCacheBytes();
  _communicator.sum(bytes);

  _console << "  quadrature-point Jacobian cache storage: " << Real(bytes) / (1024 * 1024)
           << " MB\n"
           << std::endl;
}

void
NonlinearSystemBase::initKokkosEntityBlockSmoother()
{
  _fe_problem.getKokkosFESystems()[number()].initEntityBlocks(systemMatrixTag());
}

dof_id_type
NonlinearSystemBase::numKokkosEntityBlocks() const
{
  // A block belongs to the process owning its entity's DOFs, so the count over all processes is the
  // number of blocks the smoother holds
  auto blocks = _fe_problem.getKokkosFESystems()[number()].numEntityBlocks();

  comm().sum(blocks);

  return blocks;
}

unsigned int
NonlinearSystemBase::maxKokkosEntityBlockSize() const
{
  auto size = _fe_problem.getKokkosFESystems()[number()].maxEntityBlockSize();

  comm().max(size);

  return size;
}

void
NonlinearSystemBase::setupKokkosEntityBlockSmoother()
{
  TIME_SECTION("setupKokkosEntityBlockSmoother", 1);

  auto & systems = _fe_problem.getKokkosFESystems();

  if (!systems[number()].qpJacobianCacheValid())
    mooseError("The Kokkos quadrature-point Jacobian cache does not hold a linearization. The "
               "entity-block smoother must be set up only after a Jacobian evaluation at the "
               "solution state its blocks are to reflect.");

  systems.copyToDevice();

  systems[number()].updateEntityBlocks(systemMatrixTag());
}

void
NonlinearSystemBase::applyKokkosEntityBlockSmoother(Vec r, Vec x)
{
  TIME_SECTION("applyKokkosEntityBlockSmoother", 1);

  // The residual arrives locked against writes, and wrapping a vector for device access takes a
  // writable array of it, so it is read through the persistent ghosted vector an operator
  // application reads its input through
  {
    libMesh::PetscVector<Number> r_wrap(r, comm());
    *_kokkos_mf_x = r_wrap;
  }

  auto * const petsc_r = libMesh::cast_ptr<libMesh::PetscVector<Number> *>(_kokkos_mf_x);
  LibmeshPetscCallA(comm().get(),
                    VecGhostUpdateBegin(petsc_r->vec(), INSERT_VALUES, SCATTER_FORWARD));
  LibmeshPetscCallA(comm().get(),
                    VecGhostUpdateEnd(petsc_r->vec(), INSERT_VALUES, SCATTER_FORWARD));

  // Every row is written by the smoother, a row belonging to no block taking the residual, so the
  // correction needs no initial value
  LibmeshPetscCallA(comm().get(), VecZeroEntries(x));

  libMesh::PetscVector<Number> x_wrap(x, comm());
  associateVectorToTag(x_wrap, _kokkos_mf_y_tag);

  auto & systems = _fe_problem.getKokkosFESystems();

  systems[number()].setActiveSolutionTags({_kokkos_mf_x_tag});
  systems[number()].setActiveResidualTags({_kokkos_mf_y_tag});

  for (auto & system : systems.constructedEntries())
    system.sync(Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);

  systems.copyToDevice();

  systems[number()].applyEntityBlocks(_kokkos_mf_x_tag, _kokkos_mf_y_tag, systemMatrixTag());

  for (auto & system : systems.constructedEntries())
    system.sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);

  systems[number()].clearActiveResidualTags();

  for (auto & system : systems.constructedEntries())
    system.clearActiveSolutionTags();

  disassociateVectorFromTag(x_wrap, _kokkos_mf_y_tag);

  LibmeshPetscCallA(comm().get(), VecAssemblyBegin(x));
  LibmeshPetscCallA(comm().get(), VecAssemblyEnd(x));
}

void
NonlinearSystemBase::computeKokkosJacobianVectorProduct(Vec x, Vec y)
{
  TIME_SECTION("computeKokkosJacobianVectorProduct", 1);

  // y is a caller-owned work vector holding whatever it last held, and the accumulation below adds
  // into it, so it is zeroed ahead of the early return as well: an operator with nothing to
  // contribute still owes its caller the zero action rather than that stale content
  LibmeshPetscCallA(comm().get(), VecZeroEntries(y));

  if (!_kokkos_kernels.hasActiveObjects() && !_kokkos_nodal_bcs.hasActiveObjects())
    return;

  // Populate the persistent ghosted direction vector from the raw PETSc x, then refresh its
  // ghost values -- kernels read x by LOCAL (ghost-inclusive) dof index, same as the solution
  {
    libMesh::PetscVector<Number> x_wrap(x, comm());
    *_kokkos_mf_x = x_wrap;
  }

  auto * const petsc_x = libMesh::cast_ptr<libMesh::PetscVector<Number> *>(_kokkos_mf_x);
  LibmeshPetscCallA(comm().get(),
                    VecGhostUpdateBegin(petsc_x->vec(), INSERT_VALUES, SCATTER_FORWARD));
  LibmeshPetscCallA(comm().get(),
                    VecGhostUpdateEnd(petsc_x->vec(), INSERT_VALUES, SCATTER_FORWARD));

  // Wrap the raw output vector for the duration of this call only; the caller owns it and must
  // see the accumulated result once we return
  libMesh::PetscVector<Number> y_wrap(y, comm());
  associateVectorToTag(y_wrap, _kokkos_mf_y_tag);

  auto & systems = _fe_problem.getKokkosFESystems();

  systems[number()].setActiveSolutionTags({_kokkos_mf_x_tag});
  systems[number()].setActiveResidualTags({_kokkos_mf_y_tag});

  // No reinit(): kernels read _u/_grad_u from the qp-solution cache populated the last time
  // computeKokkosJacobian() ran, at the same Newton point this matvec must reflect.
  for (auto & system : systems.constructedEntries())
    system.sync(Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);

  systems.copyToDevice();

  if (!systems[number()].qpJacobianCacheValid())
    mooseError("The Kokkos quadrature-point Jacobian cache does not hold a linearization. The "
               "matrix-free Jacobian-vector product must be requested only after a Jacobian "
               "evaluation at the solution state it is to reflect.");

  systems[number()].applyQpJacobian(_kokkos_mf_x_tag, _kokkos_mf_y_tag, systemMatrixTag());

  for (auto & kernel : _kokkos_kernels.getActiveObjects())
  {
    auto & object = static_cast<Moose::Kokkos::KernelBase &>(*kernel);

    if (!object.usesQpJacobianCache())
      object.computeJacobianVectorProduct();
  }

  for (auto & nbc : _kokkos_nodal_bcs.getActiveObjects())
    std::static_pointer_cast<Moose::Kokkos::NodalBCBase>(nbc)->computeJacobianVectorProduct();

  // Every DOF a registered Dirichlet-type boundary condition pins -- vertex and bubble (e.g.
  // HIERARCHIC edge/face mode) alike -- overwritten last since the per-boundary-condition action
  // above only reaches a boundary's mesh nodes
  systems[number()].finalizeConstrainedJacobianVectorProduct(_kokkos_mf_y_tag, _kokkos_mf_x_tag);

  for (auto & system : systems.constructedEntries())
    system.sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);

  systems[number()].clearActiveResidualTags();

  for (auto & system : systems.constructedEntries())
    system.clearActiveSolutionTags();

  disassociateVectorFromTag(y_wrap, _kokkos_mf_y_tag);

  LibmeshPetscCallA(comm().get(), VecAssemblyBegin(y));
  LibmeshPetscCallA(comm().get(), VecAssemblyEnd(y));
}

libMesh::SparseMatrix<Number> &
NonlinearSystemBase::kokkosMatrixFreeOperator() const
{
  mooseAssert(_matrix_free_operator, "No matrix-free operator; not a matrix-free system");
  return *_matrix_free_operator;
}
