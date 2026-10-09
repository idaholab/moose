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

void
ADSlotSetup::assign(const MooseObject & object, const Variable & ivar, const unsigned int max_slots)
{
  _table = ADSlotTable();

  auto & ivar_copy = const_cast<Variable &>(ivar);
  const auto & residual_var = *ivar_copy.mooseVar();
  const auto & problem = residual_var.sys().feProblem();
  const auto residual_sys = residual_var.sys().number();

  for (auto & request : _requests)
  {
    auto & slot = *request.slot;
    auto & var = request.var;

    slot.first = libMesh::invalid_uint;
    slot.seed = 1;
    slot.seed_source = nullptr;
    slot.finalized = true;

    // Derivatives exist with respect to the current state of nonlinear variables of the residual
    // system that the coupling matrix couples to the residual variable
    if (!var.coupled() || var.old() || var.sys() != residual_sys)
      continue;

    const auto * moose_var = var.mooseVar();
    if (moose_var->kind() != Moose::VarKindType::VAR_SOLVER)
      continue;

    if (!problem.areCoupled(ivar.var(), var.var(), residual_sys))
      continue;

    slot.first = _table.add(var.sys(), var.var(), request.gradient);

    if (var.dot())
    {
      slot.seed_source = &moose_var->sys().duDotDu(var.var());
      slot.refresh();
    }
  }

  if (_table.numSlots() > max_slots)
    object.mooseError("The pointwise AD residual of this object reads ",
                      _table.numSlots(),
                      " derivative slots, more than its declared max_ad_slots of ",
                      max_slots,
                      ". Increase max_ad_slots in the class to the number of value and gradient "
                      "slots its residual depends on.");

  _table.finalize();
}

} // namespace Moose::Kokkos
