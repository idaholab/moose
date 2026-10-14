//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// KokkosKernels must see its headers before the MOOSE Kokkos macros, as in KokkosArray.K, because
// unity builds compile this file ahead of it
#include "KokkosBlas.hpp"

#include "KokkosADResidualObject.h"

#include "FEProblemBase.h"
#include "MooseVariableBase.h"
#include "SystemBase.h"

namespace Moose::Kokkos
{

void
ADSlotSetup::add(ADSlotAssignment & slot, const Variable & var, const bool gradient)
{
  _requests.push_back({&slot, var, gradient});
}

bool
ADSlotSetup::seeded(const Variable * ivar, const Variable & var, const unsigned int comp) const
{
  // Derivatives exist with respect to the current state of nonlinear variables
  if (!var.coupled() || var.old())
    return false;

  const auto * moose_var = const_cast<Variable &>(var).mooseVar(comp);
  if (moose_var->kind() != Moose::VarKindType::VAR_SOLVER)
    return false;

  if (!ivar)
    return true;

  // A residual object differentiates with respect to the variables of the residual system that the
  // coupling matrix couples to the residual variable
  const auto & residual_var = *const_cast<Variable *>(ivar)->mooseVar();
  const auto & problem = residual_var.sys().feProblem();
  const auto residual_sys = residual_var.sys().number();

  if (var.sys(comp) != residual_sys)
    return false;

  return problem.areCoupled(ivar->var(), var.var(comp), residual_sys);
}

void
ADSlotSetup::assign(const MooseObject & object, const Variable & ivar, const unsigned int max_slots)
{
  assign(object, ivar, max_slots, Variable());
}

void
ADSlotSetup::assign(const MooseObject & object,
                    const Variable & ivar,
                    const unsigned int max_slots,
                    const Variable & displacements)
{
  assignSlots(object, &ivar, max_slots, displacements);
}

void
ADSlotSetup::assign(const MooseObject & object,
                    const unsigned int max_slots,
                    const Variable & displacements)
{
  assignSlots(object, nullptr, max_slots, displacements);
}

void
ADSlotSetup::assignSlots(const MooseObject & object,
                         const Variable * ivar,
                         const unsigned int max_slots,
                         const Variable & displacements)
{
  _table = ADSlotTable();

  for (auto & request : _requests)
  {
    auto & slot = *request.slot;
    auto & var = request.var;

    slot.first = libMesh::invalid_uint;
    slot.seed = 1;
    slot.seed_source = nullptr;
    slot.finalized = true;

    if (!seeded(ivar, var))
      continue;

    const auto * moose_var = var.mooseVar();
    slot.first = _table.add(var.sys(), var.var(), request.gradient);

    if (var.dot())
    {
      slot.seed_source = &moose_var->sys().duDotDu(var.var());
      slot.refresh();
    }
  }

  if (_table.numSlots() > max_slots)
    object.mooseError("The pointwise AD computation of this object reads ",
                      _table.numSlots(),
                      " derivative slots, more than its declared max_ad_slots of ",
                      max_slots,
                      ". Increase max_ad_slots in the class to the number of value and gradient "
                      "slots its computation depends on.");

  if (displacements.coupled())
  {
    // The displacement slots follow the max_ad_slots of the accessors, so that their position is
    // the same for every object of a class
    _table.reserve(max_slots);

    std::vector<bool> seeded_components(displacements.components());
    for (const auto c : make_range(displacements.components()))
      seeded_components[c] = seeded(ivar, displacements, c);

    _table.addDisplacements(displacements, seeded_components);
  }

  _table.finalize();
}

} // namespace Moose::Kokkos
