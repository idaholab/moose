//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseBase.h"
#include "MooseApp.h"
#include "FEProblemBase.h"
#include "SolutionInvalidityRegistry.h"
#include "AppFactory.h"

MooseVerbosityHelper::MooseVerbosityHelper(const MooseBase * const moose_base,
                                           const InputParameters & parameters)
  : _moose_base(*moose_base),
    _fe_problem_base(
        parameters.isParamValid("_fe_problem_base") ?
                                                    // MooseObjects get the problem that way
            parameters.get<FEProblemBase *>("_fe_problem_base")
                                                    :
                                                    // Actions get the problem that way
            parameters.isParamValid("awh")
                ? parameters.getCheckedPointerParam<ActionWarehouse *>("awh")->problemBase().get()
                // The MOOSEApp is a MooseBase but is neither an action nor an object
                : nullptr)
{
}

#ifdef MOOSE_KOKKOS_ENABLED
MooseVerbosityHelper::MooseVerbosityHelper(const MooseVerbosityHelper & object,
                                           const Moose::Kokkos::FunctorCopy &)
  : _moose_base(object._moose_base), _fe_problem_base(object._fe_problem_base)
{
}
#endif

/// Set solution invalid mark for the given solution ID
template <bool warning>
void
MooseVerbosityHelper::flagInvalidSolutionInternal(const InvalidSolutionID invalid_solution_id) const
{
  mooseAssert(
      warning == moose::internal::getSolutionInvalidityRegistry().item(invalid_solution_id).warning,
      "Inconsistent warning flag");
  auto & solution_invalidity = _moose_base.getMooseApp().solutionInvalidity();
  if constexpr (!warning)
    if (!_fe_problem_base || _fe_problem_base->immediatelyPrintInvalidSolution())
      solution_invalidity.printDebug(invalid_solution_id);
  return solution_invalidity.flagInvalidSolutionInternal(invalid_solution_id);
}

InvalidSolutionID
MooseVerbosityHelper::registerInvalidSolutionInternal(const std::string & message,
                                                      const bool warning) const
{
  return moose::internal::getSolutionInvalidityRegistry().registerInvalidity(
      _moose_base.type(), message, warning);
}

const hit::Node *
MooseVerbosityHelper::getHitNode() const
{
  return getHitNode(_moose_base.parameters());
}

[[noreturn]] void
MooseVerbosityHelper::callMooseError(std::string msg,
                                     const bool with_prefix,
                                     const hit::Node * node /* = nullptr */) const
{
  callMooseError(&_moose_base.getMooseApp(), _moose_base.parameters(), msg, with_prefix, node);
}

[[noreturn]] void
MooseVerbosityHelper::callMooseError(MooseApp * const app,
                                     const InputParameters & params,
                                     std::string msg,
                                     const bool with_prefix,
                                     const hit::Node * node)
{
  if (!node)
    node = getHitNode(params);

  std::string multiapp_prefix = "";
  if (app)
  {
    if (!app->isUltimateMaster())
      multiapp_prefix = app->name();
    app->getOutputWarehouse().mooseConsole();
  }

  if (with_prefix)
    // False here because the hit context will get processed by the node
    msg = messagePrefix(params, false) + msg;

  moose::internal::mooseErrorRaw(msg, multiapp_prefix, node);
}

std::string
MooseVerbosityHelper::messagePrefix(const InputParameters & params, const bool hit_prefix)
{
  std::string prefix = "";

  if (hit_prefix)
    if (const auto node = getHitNode(params))
      prefix += Moose::hitMessagePrefix(*node);

  // Don't have context without type and name
  if (!params.isMooseBaseObject())
    return prefix;

  const auto & name = params.getObjectName();
  const std::string base = params.hasBase() ? params.getBase() : "object";
  const bool is_main_app = base == "Application" && name == AppFactory::main_app_name;
  prefix += "The following occurred in the ";
  if (is_main_app)
    prefix += "main " + base;
  else
    prefix += base;
  if (base != params.getObjectName() && name.size() && !is_main_app)
    prefix += " '" + name + "'";
  prefix += " of type " + params.getObjectType() + ".";
  return prefix + "\n\n";
}

const hit::Node *
MooseVerbosityHelper::getHitNode(const InputParameters & params)
{
  if (const auto hit_node = params.getHitNode())
    if (!hit_node->isRoot())
      return hit_node;
  return nullptr;
}

template void MooseVerbosityHelper::flagInvalidSolutionInternal<true>(
    const InvalidSolutionID invalid_solution_id) const;
template void MooseVerbosityHelper::flagInvalidSolutionInternal<false>(
    const InvalidSolutionID invalid_solution_id) const;
