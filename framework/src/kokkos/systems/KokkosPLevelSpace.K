//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosPLevelSpace.h"

#include "KokkosElementEmbeddingTransfer.h"
#include "KokkosFESystem.h"
#include "KokkosQpJacobianOperator.h"

#include "ConsoleStream.h"
#include "NonlinearSystemBase.h"
#include "FEProblemBase.h"

#include "libmesh/equation_systems.h"
#include "libmesh/fe_interface.h"
#include "libmesh/petsc_vector.h"

namespace Moose::Kokkos
{

std::string
PLevelSpace::systemName(const NonlinearSystemBase & fine, const unsigned int order)
{
  return fine.name() + "_p" + std::to_string(order);
}

PLevelSpace::PLevelSpace(NonlinearSystemBase & fine, const unsigned int order)
  : _fine(fine),
    _order(order),
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
            : TransferSide{_fine.system(), fine_system, fine_system.feTypes()});
}

void
PLevelSpace::verifyTransfer(const ConsoleStream & console)
{
  mooseAssert(_prolongation, "The p-multigrid level transfer has not been built yet.");

  console << "  p = " << _order << ": ";

  _prolongation->verify(console);
}

TransferSide
PLevelSpace::transferSide() const
{
  return TransferSide{_sys, dofSpace(), _fe_types};
}

QpJacobianLevel
PLevelSpace::level() const
{
  // A level holds no rows fixed: the hierarchy operates on the homogenized linearized problem, and
  // the zero-Dirichlet constraints of each level's own DofMap are a separate piece of the design
  return QpJacobianLevel(dofSpace(), _fe_types, _vectors, Array<bool>());
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

const DofSpace &
PLevelSpace::dofSpace() const
{
  mooseAssert(_dof_space, "The p-multigrid level DOF layout has not been built yet.");

  return *_dof_space;
}

} // namespace Moose::Kokkos
