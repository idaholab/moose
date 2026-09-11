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
#include "PMultigrid.h"

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
mooseKokkosPreconditionerShellMult(Mat shell, Vec x, Vec y)
{
  void * ctx;
  LibmeshPetscCallQ(MatShellGetContext(shell, &ctx));
  LibmeshPetscCallQ(PCApply(static_cast<PC>(ctx), x, y));
  return LIBMESH_PETSC_SUCCESS;
}

PetscErrorCode
mooseKokkosInverseShellMult(Mat shell, Vec x, Vec y)
{
  void * ctx;
  LibmeshPetscCallQ(MatShellGetContext(shell, &ctx));
  LibmeshPetscCallQ(KSPSolve(static_cast<KSP>(ctx), x, y));
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
    // residual above only reaches a boundary's mesh nodes
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

      // The cache the levels contract holds this linearization, so this is where a level's operator
      // can be checked against itself, and where every level's operator is updated for it
      verifyKokkosLevelOperators();

      updateKokkosLevelOperators();

      verifyKokkosLevelMatrices();

      verifyKokkosEntityBlocks();
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

  // Both checks apply this system's matrix-free operator, which manages the tags it needs itself,
  // so they run once the Jacobian sweep has released the tags it held: the state here is the state
  // an operator application sees when the solve asks for one
  if (_kokkos_mf_enabled)
  {
    verifyKokkosLevelGalerkin();

    verifyKokkosMatrixFreeSymmetry();

    verifyKokkosPreconditionerSymmetry();

    verifyKokkosOperatorConditioning();
  }
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

      // The cache the levels contract holds this linearization, so this is where a level's operator
      // can be checked against itself, and where every level's operator is updated for it
      verifyKokkosLevelOperators();

      updateKokkosLevelOperators();

      verifyKokkosLevelMatrices();

      verifyKokkosEntityBlocks();
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

  // Both checks apply this system's matrix-free operator, which manages the tags it needs itself,
  // so they run once the Jacobian sweep has released the tags it held: the state here is the state
  // an operator application sees when the solve asks for one
  if (_kokkos_mf_enabled)
  {
    verifyKokkosLevelGalerkin();

    verifyKokkosMatrixFreeSymmetry();

    verifyKokkosPreconditionerSymmetry();

    verifyKokkosOperatorConditioning();
  }
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
NonlinearSystemBase::verifyKokkosLevelOperators()
{
  auto * const pmg = dynamic_cast<PMultigrid *>(_preconditioner.get());

  if (!pmg || !pmg->verifyLevelOperators())
    return;

  TIME_SECTION("verifyKokkosLevelOperators", 1);

  const auto & cache = _fe_problem.getKokkosFESystems()[number()].qpJacobianCache();

  for (const auto & level : pmg->levels())
  {
    auto & level_sys = level->system();
    const auto & dof_map = level_sys.get_dof_map();

    auto diagonal = level_sys.solution->zero_clone();
    auto unit = level_sys.solution->zero_clone();
    auto column = level_sys.solution->zero_clone();

    level->diagonal(cache, *diagonal);

    Real error = 0;
    Real scale = 0;

    for (const auto dof : make_range(level_sys.n_dofs()))
    {
      const auto owned = dof >= dof_map.first_dof() && dof < dof_map.end_dof();

      unit->zero();

      if (owned)
        unit->set(dof, 1.0);

      unit->close();

      level->apply(cache, *unit, *column);

      // Applying the operator to the unit vector of a degree of freedom produces that column of the
      // operator, whose entry for the same degree of freedom is the diagonal entry
      if (owned)
      {
        error = std::max(error, std::abs((*column)(dof) - (*diagonal)(dof)));
        scale = std::max(scale, std::abs((*diagonal)(dof)));
      }
    }

    _communicator.max(error);
    _communicator.max(scale);

    // The two paths contract the same cache against the same basis tables in a different loop
    // order, so they agree to the roundoff of that contraction; the margin over the unit roundoff
    // of double precision covers the element and quadrature-point sums
    const auto tolerance = 1e-10 * std::max(scale, 1.0);

    _console << "p-multigrid level p = " << level->order()
             << ": operator agrees with its diagonal to " << error << " (tolerance " << tolerance
             << ")" << std::endl;

    if (error > tolerance)
      mooseError("The operator of the p-multigrid level of order ",
                 level->order(),
                 " disagrees with the diagonal the level computes by ",
                 error,
                 ", which exceeds a tolerance of ",
                 tolerance,
                 ".");
  }
}

void
NonlinearSystemBase::updateKokkosLevelOperators()
{
  auto * const pmg = dynamic_cast<PMultigrid *>(_preconditioner.get());

  if (!pmg)
    return;

  TIME_SECTION("updateKokkosLevelOperators", 1);

  const auto & cache = _fe_problem.getKokkosFESystems()[number()].qpJacobianCache();

  for (const auto & level : pmg->levels())
    level->updateOperator(cache);
}

void
NonlinearSystemBase::verifyKokkosEntityBlocks()
{
  auto * const pmg = dynamic_cast<PMultigrid *>(_preconditioner.get());

  if (!pmg || !pmg->verifyEntityBlocks())
    return;

  TIME_SECTION("verifyKokkosEntityBlocks", 1);

  _console << "p-multigrid entity blocks for system '" << name() << "':\n";

  for (const auto & level : pmg->levels())
    if (level->assembles())
      level->verifyEntityBlocks(_console);

  _console << std::endl;
}

void
NonlinearSystemBase::verifyKokkosLevelMatrices()
{
  auto * const pmg = dynamic_cast<PMultigrid *>(_preconditioner.get());

  if (!pmg || !pmg->verifyLevelMatrices())
    return;

  TIME_SECTION("verifyKokkosLevelMatrices", 1);

  const auto & cache = _fe_problem.getKokkosFESystems()[number()].qpJacobianCache();

  for (const auto & level : pmg->levels())
  {
    if (!level->assembles())
      continue;

    auto & level_sys = level->system();

    auto x = level_sys.solution->zero_clone();
    auto matrix_free = level_sys.solution->zero_clone();
    auto assembled = level_sys.solution->zero_clone();

    Moose::Kokkos::LevelTransfer::fillVerificationPattern(*x, level_sys);

    // Both operators eliminate a fixed row and column, but the assembled one carries a literal
    // identity on the fixed row where the matrix-free one contributes nothing, so the two agree
    // only on a vector that is already zero at every fixed DOF -- which is what the hierarchy ever
    // applies either operator to during a real cycle
    level->zeroConstrained(*x);

    level->apply(cache, *x, *matrix_free);

    level->matrix().vector_mult(*assembled, *x);

    assembled->add(-1.0, *matrix_free);

    const auto error = assembled->linfty_norm();
    const auto scale = matrix_free->linfty_norm();

    // The two paths contract the same cache against the same basis tables in a different loop
    // order, so they agree to the roundoff of that contraction; the margin over the unit roundoff
    // of double precision covers the element and quadrature-point sums
    const auto tolerance = 1e-10 * std::max(scale, 1.0);

    _console << "p-multigrid level p = " << level->order()
             << ": assembled operator agrees with the matrix-free operator to " << error
             << " (tolerance " << tolerance << ")" << std::endl;

    // The Kokkos matrix-free constraint enforcement forms the level's operator as C^T A C plus an
    // identity on every constrained row (no asymmetric row-replacement term), so this measures, it
    // does not assume, that the assembled matrix a coarse solver factors is actually symmetric
    PetscBool is_symmetric;
    LibmeshPetscCallA(
        comm().get(),
        MatIsSymmetric(libMesh::cast_ref<libMesh::PetscMatrix<Number> &>(level->matrix()).mat(),
                       1e-10,
                       &is_symmetric));

    _console << "p-multigrid level p = " << level->order() << ": assembled operator is "
             << (is_symmetric ? "symmetric" : "NOT symmetric") << " (measured, tolerance 1e-10)"
             << std::endl;

    if (!is_symmetric)
      mooseError("The assembled operator of the p-multigrid level of order ",
                 level->order(),
                 " is not symmetric to within a tolerance of 1e-10.");

    if (error > tolerance)
      mooseError("The assembled operator of the p-multigrid level of order ",
                 level->order(),
                 " disagrees with the operator the level applies without a matrix by ",
                 error,
                 ", which exceeds a tolerance of ",
                 tolerance,
                 ".");
  }
}

void
NonlinearSystemBase::verifyKokkosLevelGalerkin()
{
  auto * const pmg = dynamic_cast<PMultigrid *>(_preconditioner.get());

  if (!pmg || !pmg->verifyLevelGalerkin())
    return;

  TIME_SECTION("verifyKokkosLevelGalerkin", 1);

  const auto & cache = _fe_problem.getKokkosFESystems()[number()].qpJacobianCache();
  const auto & levels = pmg->levels();

  for (const auto i : index_range(levels))
  {
    auto & level = *levels[i];

    // The finer side of this level's transfer: the next coarse level where there is one, and this
    // system at the finest pair
    auto * const finer = i + 1 < levels.size() ? levels[i + 1].get() : nullptr;
    auto & finer_sys = finer ? finer->system() : system();

    auto x = level.system().solution->zero_clone();
    auto direct = level.system().solution->zero_clone();
    auto galerkin = level.system().solution->zero_clone();
    auto prolonged = finer_sys.solution->zero_clone();
    auto applied = finer_sys.solution->zero_clone();

    Moose::Kokkos::LevelTransfer::fillVerificationPattern(*x, level.system());

    // The hierarchy carries only corrections that vanish on the DOFs a level holds fixed, and the
    // transfers leave those DOFs out of both directions, so the comparison is made on a vector of
    // that space.
    level.zeroConstrained(*x);

    level.apply(cache, *x, *direct);

    level.prolong(*x, *prolonged);

    if (finer)
      finer->apply(cache, *prolonged, *applied);
    else
      computeKokkosJacobianVectorProduct(
          libMesh::cast_ptr<libMesh::PetscVector<Number> *>(prolonged.get())->vec(),
          libMesh::cast_ptr<libMesh::PetscVector<Number> *>(applied.get())->vec());

    level.restrictTo(*applied, *galerkin);

    galerkin->add(-1.0, *direct);

    const auto error = galerkin->linfty_norm();
    const auto scale = direct->linfty_norm();

    // Both sides contract the same quadrature-point cache against bases that span the same space,
    // in a different order and over a different number of degrees of freedom, so they agree to the
    // roundoff of those contractions; the margin over the unit roundoff of double precision covers
    // the element and quadrature-point sums
    const auto tolerance = 1e-10 * std::max(scale, 1.0);

    _console << "p-multigrid level p = " << level.order()
             << ": operator agrees with the operator of the next finer level carried through the "
                "transfer to "
             << error << " (tolerance " << tolerance << ")" << std::endl;

    if (error > tolerance)
      mooseError("The operator of the p-multigrid level of order ",
                 level.order(),
                 " differs from P^T A P, the operator of the next finer level carried through the "
                 "level transfer, by ",
                 error,
                 ", which exceeds a tolerance of ",
                 tolerance,
                 ". Every level contracts the quadrature-point Jacobian cache, so a Jacobian "
                 "contribution made outside that cache reaches the fine operator and no coarse "
                 "level, and surfaces here.");
  }
}

void
NonlinearSystemBase::verifyKokkosOperatorConditioning()
{
  auto * const pmg = dynamic_cast<PMultigrid *>(_preconditioner.get());

  if (!pmg || !pmg->verifyOperatorConditioning())
    return;

  TIME_SECTION("verifyKokkosOperatorConditioning", 1);

  auto & operator_shell = libMesh::cast_ref<libMesh::PetscMatrix<Number> &>(*_matrix_free_operator);

  // One application per column
  Mat assembled;
  LibmeshPetscCallA(comm().get(), MatComputeOperator(operator_shell.mat(), MATAIJ, &assembled));

  // The operator is symmetric, so it factors by Cholesky. A factorization that fails is itself the
  // answer, so it is reported rather than treated as an error.
  KSP inverse;
  LibmeshPetscCallA(comm().get(), KSPCreate(comm().get(), &inverse));
  LibmeshPetscCallA(comm().get(), KSPSetOperators(inverse, assembled, assembled));
  LibmeshPetscCallA(comm().get(), KSPSetType(inverse, KSPPREONLY));
  PC inverse_pc;
  LibmeshPetscCallA(comm().get(), KSPGetPC(inverse, &inverse_pc));
  LibmeshPetscCallA(comm().get(), PCSetType(inverse_pc, PCCHOLESKY));
  LibmeshPetscCallA(comm().get(), KSPSetErrorIfNotConverged(inverse, PETSC_FALSE));

  PetscErrorCode factored = KSPSetUp(inverse);

  PetscReal operator_norm, inverse_norm = -1;

  LibmeshPetscCallA(comm().get(), MatNorm(assembled, NORM_INFINITY, &operator_norm));

  // The spread of the diagonal locates where the conditioning comes from: the norm above is a row
  // sum, dominated by whichever mode carries the largest entries, while the smallest diagonal
  // bounds the smallest eigenvalue and so bounds the norm of the inverse from below
  Vec diagonal;
  LibmeshPetscCallA(comm().get(), MatCreateVecs(assembled, nullptr, &diagonal));
  LibmeshPetscCallA(comm().get(), MatGetDiagonal(assembled, diagonal));

  PetscReal min_diagonal, max_diagonal;
  PetscInt where;
  LibmeshPetscCallA(comm().get(), VecMin(diagonal, &where, &min_diagonal));
  LibmeshPetscCallA(comm().get(), VecMax(diagonal, &where, &max_diagonal));
  LibmeshPetscCallA(comm().get(), VecDestroy(&diagonal));

  if (!factored)
  {
    PetscInt local_rows, local_cols, rows, cols;
    LibmeshPetscCallA(comm().get(), MatGetLocalSize(assembled, &local_rows, &local_cols));
    LibmeshPetscCallA(comm().get(), MatGetSize(assembled, &rows, &cols));

    // The inverse is formed column by column exactly as the cycle is, so its norm is taken over the
    // same quantity and the two are comparable
    Mat inverse_shell;
    LibmeshPetscCallA(
        comm().get(),
        MatCreateShell(comm().get(), local_rows, local_cols, rows, cols, inverse, &inverse_shell));
    LibmeshPetscCallA(comm().get(),
                      MatShellSetOperation(
                          inverse_shell, MATOP_MULT, (void (*)(void))mooseKokkosInverseShellMult));

    Mat formed_inverse;
    LibmeshPetscCallA(comm().get(), MatComputeOperator(inverse_shell, MATAIJ, &formed_inverse));
    LibmeshPetscCallA(comm().get(), MatNorm(formed_inverse, NORM_INFINITY, &inverse_norm));

    LibmeshPetscCallA(comm().get(), MatDestroy(&formed_inverse));
    LibmeshPetscCallA(comm().get(), MatDestroy(&inverse_shell));
  }

  LibmeshPetscCallA(comm().get(), KSPDestroy(&inverse));
  LibmeshPetscCallA(comm().get(), MatDestroy(&assembled));

  _console << "Kokkos matrix-free operator of system '" << name() << "': norm "
           << static_cast<Real>(operator_norm) << ", diagonal from "
           << static_cast<Real>(min_diagonal) << " to " << static_cast<Real>(max_diagonal);

  if (inverse_norm < 0)
    _console << ", inverse could not be factored by Cholesky, so the operator is numerically "
                "singular at this order"
             << std::endl;
  else
    _console << ", inverse norm " << static_cast<Real>(inverse_norm) << ", product "
             << static_cast<Real>(operator_norm) * static_cast<Real>(inverse_norm) << std::endl;
}

void
NonlinearSystemBase::verifyKokkosPreconditionerSymmetry()
{
  auto * const pmg = dynamic_cast<PMultigrid *>(_preconditioner.get());

  if (!pmg || !pmg->verifyPreconditionerSymmetry())
    return;

  TIME_SECTION("verifyKokkosPreconditionerSymmetry", 1);

  KSP ksp;
  LibmeshPetscCallA(comm().get(), SNESGetKSP(getSNES(), &ksp));
  PC pc;
  LibmeshPetscCallA(comm().get(), KSPGetPC(ksp, &pc));

  // A preconditioner has to be built before it can be applied. The levels' operators are up to date
  // by this point, so what is measured is the cycle the solve about to run will apply.
  LibmeshPetscCallA(comm().get(), PCSetUp(pc));

  // The cycle is a linear operator over the same layout the system's own operator is over
  auto & operator_shell = libMesh::cast_ref<libMesh::PetscMatrix<Number> &>(*_matrix_free_operator);

  PetscInt local_rows, local_cols, rows, cols;
  LibmeshPetscCallA(comm().get(), MatGetLocalSize(operator_shell.mat(), &local_rows, &local_cols));
  LibmeshPetscCallA(comm().get(), MatGetSize(operator_shell.mat(), &rows, &cols));

  Mat cycle;
  LibmeshPetscCallA(comm().get(),
                    MatCreateShell(comm().get(), local_rows, local_cols, rows, cols, pc, &cycle));
  LibmeshPetscCallA(
      comm().get(),
      MatShellSetOperation(cycle, MATOP_MULT, (void (*)(void))mooseKokkosPreconditionerShellMult));

  // One application of the whole cycle per column, which is why this is gated behind a flag
  Mat assembled;
  LibmeshPetscCallA(comm().get(), MatComputeOperator(cycle, MATAIJ, &assembled));

  Mat asymmetry;
  LibmeshPetscCallA(comm().get(), MatTranspose(assembled, MAT_INITIAL_MATRIX, &asymmetry));
  LibmeshPetscCallA(comm().get(), MatAXPY(asymmetry, -1.0, assembled, DIFFERENT_NONZERO_PATTERN));

  PetscReal asymmetry_norm, cycle_norm;
  LibmeshPetscCallA(comm().get(), MatNorm(asymmetry, NORM_INFINITY, &asymmetry_norm));
  LibmeshPetscCallA(comm().get(), MatNorm(assembled, NORM_INFINITY, &cycle_norm));

  LibmeshPetscCallA(comm().get(), MatDestroy(&asymmetry));
  LibmeshPetscCallA(comm().get(), MatDestroy(&assembled));
  LibmeshPetscCallA(comm().get(), MatDestroy(&cycle));

  const auto error = static_cast<Real>(asymmetry_norm);
  const auto scale = static_cast<Real>(cycle_norm);

  // A v-cycle is symmetric when each of its parts is: the smoothers are symmetric, the coarse solve
  // is a symmetric factorization, and restriction is the transpose of prolongation. What remains is
  // the roundoff of the sweeps, transfers and triangular solves the cycle performs, which scales
  // with the norm of the cycle itself, so the margin is relative to that norm and matches the one
  // the operator's own symmetry is held to.
  const auto tolerance = 1e-10 * std::max(scale, 1.0);

  _console << "Kokkos p-multigrid cycle of system '" << name()
           << "': differs from its transpose by " << error << " (tolerance " << tolerance
           << ", cycle norm " << scale << ")" << std::endl;

  if (error > tolerance)
    mooseError("The p-multigrid cycle of system '",
               name(),
               "' differs from its transpose by ",
               error,
               ", which exceeds a tolerance of ",
               tolerance,
               ". A Krylov method that requires a symmetric preconditioner, such as CG, is not "
               "valid over it. An asymmetric smoother, a coarse level solved by an iteration "
               "rather than a factorization, or a restriction that is not the transpose of the "
               "prolongation are the causes to check.");
}

void
NonlinearSystemBase::verifyKokkosMatrixFreeSymmetry()
{
  auto * const pmg = dynamic_cast<PMultigrid *>(_preconditioner.get());

  if (!pmg || !pmg->verifyOperatorSymmetry())
    return;

  TIME_SECTION("verifyKokkosMatrixFreeSymmetry", 1);

  const auto & fe_system = _fe_problem.getKokkosFESystems()[number()];

  // Refuse to measure what the operator is not built to satisfy, rather than reporting the
  // asymmetry a transient keeps by design as a tolerance failure
  if (!fe_system.eliminatesConstrainedColumns() && fe_system.numConstrainedDofs())
    mooseError(
        "Verification of the symmetry of the Kokkos matrix-free operator was requested for system "
        "'",
        name(),
        "', which is transient and has Dirichlet-constrained degrees of freedom. A transient's "
        "time integrator forms the solution time derivative from the raw solution, so a free row's "
        "residual reads a constrained degree of freedom's own iterate value, and the operator has "
        "to keep the matching column to remain the linearization of that residual. The operator is "
        "therefore asymmetric by design here, and a Krylov method requiring symmetry, such as CG, "
        "is not available for a transient Kokkos matrix-free system.");

  // The shell registered as this system's own matrix, which PetscNonlinearSolver hands SNES as both
  // Amat and Pmat, and whose MatMult is computeKokkosJacobianVectorProduct()
  auto & shell = libMesh::cast_ref<libMesh::PetscMatrix<Number> &>(*_matrix_free_operator);

  // One application per column, which is why this is gated behind a verification flag
  Mat assembled;
  LibmeshPetscCallA(comm().get(), MatComputeOperator(shell.mat(), MATAIJ, &assembled));

  Mat asymmetry;
  LibmeshPetscCallA(comm().get(), MatTranspose(assembled, MAT_INITIAL_MATRIX, &asymmetry));
  LibmeshPetscCallA(comm().get(), MatAXPY(asymmetry, -1.0, assembled, DIFFERENT_NONZERO_PATTERN));

  PetscReal asymmetry_norm, operator_norm;
  LibmeshPetscCallA(comm().get(), MatNorm(asymmetry, NORM_INFINITY, &asymmetry_norm));
  LibmeshPetscCallA(comm().get(), MatNorm(assembled, NORM_INFINITY, &operator_norm));

  LibmeshPetscCallA(comm().get(), MatDestroy(&asymmetry));
  LibmeshPetscCallA(comm().get(), MatDestroy(&assembled));

  const auto error = static_cast<Real>(asymmetry_norm);
  const auto scale = static_cast<Real>(operator_norm);

  // The two sides of the transpose are the same quadrature-point contraction with the test and
  // trial basis functions exchanged, so they agree to the roundoff of that contraction; the margin
  // over the unit roundoff of double precision covers the element and quadrature-point sums
  const auto tolerance = 1e-10 * std::max(scale, 1.0);

  _console << "Kokkos matrix-free operator of system '" << name()
           << "': differs from its transpose by " << error << " (tolerance " << tolerance << ")"
           << std::endl;

  if (error > tolerance)
    mooseError(
        "The Kokkos matrix-free Jacobian action of system '",
        name(),
        "' differs from its transpose by ",
        error,
        ", which exceeds a tolerance of ",
        tolerance,
        ". A Krylov method that requires a symmetric operator, such as CG, is not valid on "
        "it. A Dirichlet-constrained degree of freedom left in a free row's residual or in "
        "the operator's trial-space gather is the usual cause, since the constrained row's "
        "own action is the identity and so has nothing to match in the transposed position.");
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
  LibmeshPetscCallA(comm().get(), VecZeroEntries(y));

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
