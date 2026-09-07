//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosPLevelSpace.h"

#include "NonlinearSystemBase.h"
#include "FEProblemBase.h"

#include "libmesh/equation_systems.h"
#include "libmesh/fe_interface.h"

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

void
PLevelSpace::init()
{
  _dof_space = std::make_unique<DofSpace>(_fine.feProblem().mesh(), _sys);
}

const DofSpace &
PLevelSpace::dofSpace() const
{
  mooseAssert(_dof_space, "The p-multigrid level DOF layout has not been built yet.");

  return *_dof_space;
}

} // namespace Moose::Kokkos
