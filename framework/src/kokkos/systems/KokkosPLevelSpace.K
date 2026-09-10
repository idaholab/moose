//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosPLevelSpace.h"

#include "KokkosConstraintOperator.h"
#include "KokkosDirichletValueFunction.h"
#include "KokkosElementEmbeddingTransfer.h"
#include "KokkosFESystem.h"
#include "KokkosNodalBCBase.h"
#include "KokkosQpJacobianOperator.h"

#include "ConsoleStream.h"
#include "NonlinearSystemBase.h"
#include "FEProblemBase.h"

#include "libmesh/dirichlet_boundaries.h"
#include "libmesh/dof_map.h"
#include "libmesh/equation_systems.h"
#include "libmesh/fe_interface.h"
#include "libmesh/petsc_vector.h"

namespace Moose::Kokkos
{

namespace
{
PetscErrorCode
levelMatMult(Mat shell, Vec x, Vec y)
{
  void * ctx;
  LibmeshPetscCallQ(MatShellGetContext(shell, &ctx));
  static_cast<PLevelSpace *>(ctx)->applyToVec(x, y);
  return LIBMESH_PETSC_SUCCESS;
}

PetscErrorCode
levelMatGetDiagonal(Mat shell, Vec diagonal)
{
  void * ctx;
  LibmeshPetscCallQ(MatShellGetContext(shell, &ctx));
  static_cast<PLevelSpace *>(ctx)->diagonalToVec(diagonal);
  return LIBMESH_PETSC_SUCCESS;
}

PetscErrorCode
levelPCSetUp(PC pc)
{
  void * ctx;
  LibmeshPetscCallQ(PCShellGetContext(pc, &ctx));
  static_cast<PLevelSpace *>(ctx)->setupBlockSmoother();
  return LIBMESH_PETSC_SUCCESS;
}

PetscErrorCode
levelPCApply(PC pc, Vec r, Vec x)
{
  void * ctx;
  LibmeshPetscCallQ(PCShellGetContext(pc, &ctx));
  static_cast<PLevelSpace *>(ctx)->applyBlockSmoother(r, x);
  return LIBMESH_PETSC_SUCCESS;
}

PetscErrorCode
levelMatInterpolate(Mat shell, Vec x, Vec y)
{
  void * ctx;
  LibmeshPetscCallQ(MatShellGetContext(shell, &ctx));
  static_cast<PLevelSpace *>(ctx)->prolongToVec(x, y);
  return LIBMESH_PETSC_SUCCESS;
}

PetscErrorCode
levelMatRestrict(Mat shell, Vec x, Vec y)
{
  void * ctx;
  LibmeshPetscCallQ(MatShellGetContext(shell, &ctx));
  static_cast<PLevelSpace *>(ctx)->restrictToVec(x, y);
  return LIBMESH_PETSC_SUCCESS;
}
} // namespace

std::string
PLevelSpace::systemName(const NonlinearSystemBase & fine, const unsigned int order)
{
  return fine.name() + "_p" + std::to_string(order);
}

PLevelSpace::PLevelSpace(NonlinearSystemBase & fine, const unsigned int order, const bool assemble)
  : _fine(fine),
    _order(order),
    _assemble(assemble),
    _sys(fine.feProblem().es().add_system<libMesh::System>(systemName(fine, order)))
{
  const auto & fine_sys = _fine.system();

  for (const auto var : make_range(fine_sys.n_vars()))
  {
    const auto & fine_var = fine_sys.variable(var);
    auto fe_type = fine_var.type();

    if (libMesh::FEInterface::field_type(fe_type) != libMesh::TYPE_SCALAR)
      mooseError("Variable '",
                 fine_var.name(),
                 "' is vector-valued. A p-multigrid level contracts the quadrature-point Jacobian "
                 "cache, which holds scalar value/flux blocks, so only scalar-valued variables can "
                 "be coarsened.");

    if (fe_type.order.get_order() <= static_cast<int>(order))
      mooseError("Variable '",
                 fine_var.name(),
                 "' is of order ",
                 fe_type.order.get_order(),
                 ", so a level of order ",
                 order,
                 " would not coarsen it. Every level order must be below the order of every "
                 "variable of system '",
                 _fine.name(),
                 "'.");

    fe_type.order = libMesh::Order(order);

    // An empty active subdomain set means the variable lives everywhere, which libMesh spells as a
    // null set pointer
    const auto & subdomains = fine_var.active_subdomains();

    _sys.add_variable(fine_var.name(), fe_type, subdomains.empty() ? nullptr : &subdomains);
  }

  _fine.feProblem().kokkosAssembly().addShapeSystem(_sys);

  if (_assemble)
  {
    _matrix_object = &_sys.add_matrix("level operator");

    // The device assembly is indexed by the sparsity pattern libMesh computes for the level's DOF
    // map, which libMesh keeps past initialization only when it is asked to
    _sys.get_dof_map().full_sparsity_pattern_needed();
  }
}

PLevelSpace::~PLevelSpace() = default;

void
PLevelSpace::init()
{
  _dof_space = std::make_unique<DofSpace>(_fine.feProblem().mesh(), _sys);

  const auto & assembly = _fine.feProblem().kokkosAssembly();

  _fe_types.create(_sys.n_vars());

  for (const auto var : make_range(_sys.n_vars()))
    _fe_types[var] = assembly.getFETypeID(_sys.variable(var).type());

  _fe_types.copyToDevice();

  _vectors.create(NUM_SLOTS);

  // The loops gather the operator input by local, ghost-inclusive DOF index, so the input arrives
  // through a ghosted vector of the level's own
  _x = &_sys.add_vector("operator input", false, libMesh::GHOSTED);

  // A Dirichlet boundary condition of the fine system additionally has to be registered on this
  // level's own DofMap, so that libMesh's constraint machinery reports the same HIERARCHIC
  // edge/face modes on it that it reports on the fine system's DofMap. This has to happen here
  // rather than in the constructor because the fine system's nodal boundary condition warehouse is
  // not yet populated at preconditioner-construction time, and it has to happen before
  // setupConstrainedDofs() reads the level's DofMap constraints below.
  bool has_dirichlet_bc = false;

  for (const auto & bc : _fine.getKokkosNodalBCWarehouse().getActiveObjects())
  {
    const auto & nbc = static_cast<const NodalBCBase &>(*bc);

    if (nbc.isDirichletBC())
    {
      _sys.get_dof_map().add_dirichlet_boundary(libMesh::DirichletBoundary(
          nbc.boundaryIDs(), {nbc.variable().number()}, DirichletValueFunction(nbc)));
      has_dirichlet_bc = true;
    }
  }

  if (has_dirichlet_bc)
    _sys.reinit_constraints();

  setupConstrainedDofs();

  if (_assemble)
  {
    _dof_space->setupSparsity();

    if (!dofSpace().getSparsity().row_ptr.size())
      mooseError("The p-multigrid level of order ",
                 _order,
                 " assembles its operator, but libMesh computed no sparsity pattern for the "
                 "level's DOF map, so there is no pattern to assemble over.");

    _matrix.create(*_matrix_object, dofSpace());
  }
  else
  {
    // A level that applies its operator hands PETSc a shell over the level's own DOF layout, whose
    // product and diagonal are the contraction the level performs
    createShell(_sys, _sys, *_operator);

    LibmeshPetscCallA(_sys.comm().get(),
                      MatShellSetOperation(_operator, MATOP_MULT, (void (*)(void))levelMatMult));
    LibmeshPetscCallA(
        _sys.comm().get(),
        MatShellSetOperation(_operator, MATOP_GET_DIAGONAL, (void (*)(void))levelMatGetDiagonal));
  }
}

void
PLevelSpace::createShell(const libMesh::System & rows, const libMesh::System & columns, Mat & mat)
{
  LibmeshPetscCallA(_sys.comm().get(),
                    MatCreateShell(_sys.comm().get(),
                                   rows.get_dof_map().n_local_dofs(),
                                   columns.get_dof_map().n_local_dofs(),
                                   rows.get_dof_map().n_dofs(),
                                   columns.get_dof_map().n_dofs(),
                                   this,
                                   &mat));
}

void
PLevelSpace::setupConstrainedDofs()
{
  const auto & dof_map = _sys.get_dof_map();

  auto * const solution =
      libMesh::cast_ptr<libMesh::PetscVector<Number> *>(_sys.current_local_solution.get());

  // Every DOF a registered Dirichlet-type boundary condition of the fine system pins on this
  // level's own DofMap -- vertex and bubble (e.g. HIERARCHIC edge/face mode) alike -- comes from
  // libMesh's own DOF constraints, registered on this level's DofMap in init() above. A level
  // carries no nonlinear solve of its own, so only the mask of constrained rows matters here;
  // which row is preset is a solve-time distinction this level's operator does not make.
  ConstraintOperator constraint_operator;

  constraint_operator.setup(
      dof_map, *solution, dofSpace().getNumLocalDofs() + dofSpace().getNumGhostDofs(), {});

  _constrained_dof = constraint_operator.constrainedMask();

  // The operator's fixed rows carry the identity, which this vector places there
  _constrained_rows = &_sys.add_vector("constrained rows", false);

  _constrained_rows->zero();

  for (const auto dof : make_range(dofSpace().getNumLocalDofs()))
    if (_constrained_dof[dof])
    {
      _constrained_rows->set(dof_map.first_dof() + dof, 1.0);
      ++_num_constrained_dofs;
    }

  _constrained_rows->close();

  _sys.comm().sum(_num_constrained_dofs);
}

void
PLevelSpace::initTransfer(PLevelSpace * const finer)
{
  auto & fine_system = _fine.feProblem().getKokkosFESystems()[_fine.number()];

  // Lagrange p-coarsening nests the spaces as functions while sharing no degrees of freedom between
  // them, which is what a per-element embedding transfers
  _prolongation = std::make_unique<ElementEmbeddingTransfer>(
      _fine.feProblem().kokkosAssembly(),
      transferSide(),
      finer ? finer->transferSide()
            : TransferSide{_fine.system(),
                           fine_system,
                           fine_system.feTypes(),
                           fine_system.getNodalBCMatrixTagDofs(_fine.systemMatrixTag())});

  // The transfer reaches PETSc as a shell from this level's space to the next finer one, which is
  // the orientation PETSc's multigrid interpolation carries: its product prolongs and its
  // transposed product restricts
  createShell(finer ? finer->system() : _fine.system(), _sys, *_interpolation);

  LibmeshPetscCallA(
      _sys.comm().get(),
      MatShellSetOperation(_interpolation, MATOP_MULT, (void (*)(void))levelMatInterpolate));
  LibmeshPetscCallA(
      _sys.comm().get(),
      MatShellSetOperation(_interpolation, MATOP_MULT_TRANSPOSE, (void (*)(void))levelMatRestrict));
}

void
PLevelSpace::verifyTransfer(const ConsoleStream & console)
{
  mooseAssert(_prolongation, "The p-multigrid level transfer has not been built yet.");

  console << "  p = " << _order << ": ";

  _prolongation->verify(console);
}

void
PLevelSpace::prolong(const libMesh::NumericVector<Number> & x, libMesh::NumericVector<Number> & y)
{
  mooseAssert(_prolongation, "The p-multigrid level transfer has not been built yet.");

  _prolongation->prolong(x, y);
}

void PLevelSpace::restrict(const libMesh::NumericVector<Number> & x,
                           libMesh::NumericVector<Number> & y)
{
  mooseAssert(_prolongation, "The p-multigrid level transfer has not been built yet.");

  _prolongation->restrict(x, y);
}

void
PLevelSpace::zeroConstrained(libMesh::NumericVector<Number> & x)
{
  if (!_num_constrained_dofs)
    return;

  // The work vector takes the fixed part of x, which subtracting leaves the free part behind
  *_x = x;

  _x->pointwise_mult(*_x, *_constrained_rows);

  x.add(-1.0, *_x);
}

TransferSide
PLevelSpace::transferSide() const
{
  return TransferSide{_sys, dofSpace(), _fe_types, _constrained_dof};
}

QpJacobianLevel
PLevelSpace::level() const
{
  // The hierarchy operates on the homogenized linearized problem, so the level holds fixed the rows
  // a nodal boundary condition of the fine system pins, and the vectors it carries are corrections
  // that vanish on those rows. Eliminating the matching columns therefore changes nothing the
  // hierarchy computes, and keeps the level's action equal to the matrix it assembles from the same
  // quadrature-point cache.
  return QpJacobianLevel(dofSpace(), _fe_types, _vectors, _constrained_dof, true);
}

void
PLevelSpace::dispatch(const QpJacobianCache & cache,
                      libMesh::NumericVector<Number> & y,
                      const libMesh::NumericVector<Number> * const x)
{
  if (x)
  {
    *_x = *x;

    auto * const petsc_x = libMesh::cast_ptr<libMesh::PetscVector<Number> *>(_x);

    LibmeshPetscCallA(_sys.comm().get(),
                      VecGhostUpdateBegin(petsc_x->vec(), INSERT_VALUES, SCATTER_FORWARD));
    LibmeshPetscCallA(_sys.comm().get(),
                      VecGhostUpdateEnd(petsc_x->vec(), INSERT_VALUES, SCATTER_FORWARD));
  }

  y.zero();

  // The output is scattered into, so it is wrapped for assembly: its ghost-row contributions are
  // buffered on device and added into the owning processes by close()
  _vectors[Y].create(y, dofSpace(), true);
  _vectors[Y].copyToDevice();

  if (x)
  {
    _vectors[X].create(*_x, dofSpace(), false);
    _vectors[X].copyToDevice();
  }

  _vectors.copyToDevice();

  QpJacobianOperator op(_fine.feProblem().kokkosAssembly(), cache, level());

  if (x)
    op.apply(X, Y);
  else
    op.diagonal(Y);

  _vectors[Y].close();

  if (x)
    _vectors[X].restore();

  y.close();

  // The element loops contribute nothing to a row the level holds fixed, so the identity goes on
  // those rows here, which leaves the level's operator nonsingular and its diagonal usable by a
  // smoother. The input work vector has served the loops by this point and holds the operator
  // input, which is what the fixed rows of an application reproduce.
  if (_num_constrained_dofs)
  {
    if (x)
    {
      _x->pointwise_mult(*_x, *_constrained_rows);

      y.add(1.0, *_x);
    }
    else
      y.add(1.0, *_constrained_rows);
  }
}

void
PLevelSpace::apply(const QpJacobianCache & cache,
                   const libMesh::NumericVector<Number> & x,
                   libMesh::NumericVector<Number> & y)
{
  dispatch(cache, y, &x);
}

void
PLevelSpace::diagonal(const QpJacobianCache & cache, libMesh::NumericVector<Number> & diagonal)
{
  dispatch(cache, diagonal, nullptr);
}

void
PLevelSpace::assembleMatrix(const QpJacobianCache & cache)
{
  mooseAssert(_assemble, "This p-multigrid level does not assemble its operator.");

  // The device assembly adds into the matrix, so both sides of it start from zero
  _matrix_object->zero();

  _matrix = 0.0;

  QpJacobianOperator op(_fine.feProblem().kokkosAssembly(), cache, level());

  op.assemble(_matrix);

  _matrix.close();

  // The element loops contribute nothing to a row the level holds fixed, so the identity goes on
  // those rows here, which is the same operator the matrix-free application realizes
  if (_num_constrained_dofs)
  {
    const auto first_dof = _sys.get_dof_map().first_dof();

    for (const auto dof : make_range(dofSpace().getNumLocalDofs()))
      if (_constrained_dof[dof])
        _matrix_object->set(first_dof + dof, first_dof + dof, 1.0);

    _matrix_object->close();
  }
}

libMesh::SparseMatrix<Number> &
PLevelSpace::matrix()
{
  mooseAssert(_matrix_object, "This p-multigrid level does not assemble its operator.");

  return *_matrix_object;
}

void
PLevelSpace::updateOperator(const QpJacobianCache & cache)
{
  if (_assemble)
  {
    assembleMatrix(cache);
    return;
  }

  // A shell operator holds no state of its own: it reads the cache when it is applied. What a
  // smoother built on top of it does hold, the diagonal above all, is rebuilt only when PETSc sees
  // the operator change, which is what advancing the object state reports.
  LibmeshPetscCallA(_sys.comm().get(), PetscObjectStateIncrease((PetscObject)(*_operator)));
}

Mat
PLevelSpace::operatorMat() const
{
  if (_assemble)
    return libMesh::cast_ptr<libMesh::PetscMatrix<Number> *>(_matrix_object)->mat();

  mooseAssert(_operator, "The p-multigrid level operator has not been built yet.");

  return _operator;
}

Mat
PLevelSpace::interpolationMat() const
{
  mooseAssert(_interpolation, "The p-multigrid level transfer has not been built yet.");

  return _interpolation;
}

const QpJacobianCache &
PLevelSpace::cache() const
{
  return _fine.feProblem().getKokkosFESystems()[_fine.number()].qpJacobianCache();
}

void
PLevelSpace::applyToVec(Vec x, Vec y)
{
  libMesh::PetscVector<Number> x_wrap(x, _sys.comm());
  libMesh::PetscVector<Number> y_wrap(y, _sys.comm());

  apply(cache(), x_wrap, y_wrap);
}

void
PLevelSpace::diagonalToVec(Vec diagonal)
{
  libMesh::PetscVector<Number> diagonal_wrap(diagonal, _sys.comm());

  this->diagonal(cache(), diagonal_wrap);
}

void
PLevelSpace::initEntityBlocks()
{
  _blocks.init(dofSpace(), _sys.get_mesh(), _constrained_dof);

  _num_entity_blocks = _blocks.numBlocks();
  _max_entity_block_size = _blocks.maxBlockSize();

  _sys.comm().sum(_num_entity_blocks);
  _sys.comm().max(_max_entity_block_size);
}

void
PLevelSpace::verifyEntityBlocks(const ConsoleStream & console)
{
  mooseAssert(_assemble, "Only a level that assembles its operator can check its entity blocks.");

  // The check reads this level's own assembled operator, which is the same contraction taken entry
  // by entry and is itself checked against the matrix-free action by verify_level_matrices, so the
  // blocks are compared against something already established rather than against a second guess.
  // They are built here whether or not this level is one that smooths with them.
  if (!_blocks.isBuilt())
    initEntityBlocks();

  _blocks.zero();

  QpJacobianOperator op(_fine.feProblem().kokkosAssembly(), cache(), level());

  op.assembleBlocks(_blocks);

  // Read before factoring, which overwrites the entries with their factor
  _blocks.copyEntriesToHost();

  const auto first_dof = _sys.get_dof_map().first_dof();

  Real error = 0;
  Real scale = 0;

  for (const auto block : make_range(_blocks.numBlocks()))
    for (const auto i : make_range(_blocks.blockSize(block)))
    {
      const auto row = first_dof + _blocks.blockDof(block, i);

      for (const auto j : make_range(_blocks.blockSize(block)))
      {
        const auto col = first_dof + _blocks.blockDof(block, j);
        const auto assembled = (*_matrix_object)(row, col);

        error = std::max(error, std::abs(_blocks.entry(block, i, j) - assembled));
        scale = std::max(scale, std::abs(assembled));
      }
    }

  _sys.comm().max(error);
  _sys.comm().max(scale);

  // The two paths contract the same cache against the same basis tables in a different loop order,
  // so they agree to the roundoff of that contraction, with margin for the element and
  // quadrature-point sums
  const auto tolerance = 1e-10 * std::max(scale, 1.0);

  console << "  p = " << _order << ": entity blocks agree with the assembled operator to " << error
          << " (tolerance " << tolerance << ")\n";
}

void
PLevelSpace::setupBlockSmootherPC(const PC pc)
{
  mooseAssert(_blocks.isBuilt(), "This p-multigrid level has no entity-block decomposition.");

  LibmeshPetscCallA(_sys.comm().get(), PCSetType(pc, PCSHELL));
  LibmeshPetscCallA(_sys.comm().get(), PCShellSetContext(pc, this));
  LibmeshPetscCallA(_sys.comm().get(), PCShellSetName(pc, "entity-block Schwarz"));
  LibmeshPetscCallA(_sys.comm().get(), PCShellSetSetUp(pc, levelPCSetUp));
  LibmeshPetscCallA(_sys.comm().get(), PCShellSetApply(pc, levelPCApply));
}

void
PLevelSpace::setupBlockSmoother()
{
  // The blocks hold entries rather than reading the cache when they are applied, so they are rebuilt
  // here, which PETSc calls whenever the operator this preconditioner is built on has changed
  _blocks.zero();

  QpJacobianOperator op(_fine.feProblem().kokkosAssembly(), cache(), level());

  op.assembleBlocks(_blocks);

  _blocks.factor();
}

void
PLevelSpace::applyBlockSmoother(Vec r, Vec x)
{
  libMesh::PetscVector<Number> residual(r, _sys.comm());
  libMesh::PetscVector<Number> correction(x, _sys.comm());

  // PETSc hands the preconditioner its input locked against write access, and wrapping a vector for
  // device access takes a writable array of it, so the residual is read through the level's own work
  // vector, as an operator application reads its input
  *_x = residual;

  _vectors[X].create(*_x, dofSpace(), false);
  _vectors[X].copyToDevice();
  _vectors[Y].create(correction, dofSpace(), false);
  _vectors[Y].copyToDevice();

  _vectors.copyToDevice();

  QpJacobianOperator op(_fine.feProblem().kokkosAssembly(), cache(), level());

  op.applyBlocks(_blocks, X, Y);

  _vectors[Y].copyToHost();
  _vectors[Y].restore();
  _vectors[X].restore();
}

void
PLevelSpace::prolongToVec(Vec x, Vec y)
{
  libMesh::PetscVector<Number> x_wrap(x, _sys.comm());
  libMesh::PetscVector<Number> y_wrap(y, _sys.comm());

  prolong(x_wrap, y_wrap);
}

void
PLevelSpace::restrictToVec(Vec x, Vec y)
{
  libMesh::PetscVector<Number> x_wrap(x, _sys.comm());
  libMesh::PetscVector<Number> y_wrap(y, _sys.comm());

  restrict(x_wrap, y_wrap);
}

const DofSpace &
PLevelSpace::dofSpace() const
{
  mooseAssert(_dof_space, "The p-multigrid level DOF layout has not been built yet.");

  return *_dof_space;
}

} // namespace Moose::Kokkos
