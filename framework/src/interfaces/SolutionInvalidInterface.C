//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SolutionInvalidInterface.h"
#include "MooseBase.h"
#include "MooseApp.h"
#include "FEProblemBase.h"
#include "SolutionInvalidityRegistry.h"

SolutionInvalidInterface::SolutionInvalidInterface(const MooseBase * const moose_base,
                                                   const InputParameters & parameters)
  : _si_moose_base(*moose_base),
    _si_problem(
        parameters.isParamValid("_fe_problem_base") ?
                                                    // MooseObjects get the problem that way
            parameters.get<FEProblemBase *>("_fe_problem_base")
                                                    :
                                                    // Actions get the problem that way
            parameters.getCheckedPointerParam<ActionWarehouse *>("awh")->problemBase().get())
{
}

#ifdef MOOSE_KOKKOS_ENABLED
SolutionInvalidInterface::SolutionInvalidInterface(const SolutionInvalidInterface & object,
                                                   const Moose::Kokkos::FunctorCopy &)
  : _si_moose_base(object._si_moose_base), _si_problem(object._si_problem)
{
}
#endif

/// Set solution invalid mark for the given solution ID
template <bool warning>
void
SolutionInvalidInterface::flagInvalidSolutionInternal(
    const InvalidSolutionID invalid_solution_id) const
{
  mooseAssert(
      warning == moose::internal::getSolutionInvalidityRegistry().item(invalid_solution_id).warning,
      "Inconsistent warning flag");
  auto & solution_invalidity = _si_moose_base.getMooseApp().solutionInvalidity();
  if constexpr (!warning)
    if (!_si_problem || _si_problem->immediatelyPrintInvalidSolution())
      solution_invalidity.printDebug(invalid_solution_id);
  return solution_invalidity.flagInvalidSolutionInternal(invalid_solution_id);
}

InvalidSolutionID
SolutionInvalidInterface::registerInvalidSolutionInternal(const std::string & message,
                                                          const bool warning) const
{
  return moose::internal::getSolutionInvalidityRegistry().registerInvalidity(
      _si_moose_base.type(), message, warning);
}

template void SolutionInvalidInterface::flagInvalidSolutionInternal<true>(
    const InvalidSolutionID invalid_solution_id) const;
template void SolutionInvalidInterface::flagInvalidSolutionInternal<false>(
    const InvalidSolutionID invalid_solution_id) const;
