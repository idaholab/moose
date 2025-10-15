//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Moose.h"
#include "SolutionInvalidity.h"

// Forward declarations
class MooseObject;
class FEProblemBase;

#define flagInvalidSolution(message)                                                               \
  do                                                                                               \
  {                                                                                                \
    static const auto __invalid_id = this->registerInvalidSolutionInternal(message, false);        \
    this->flagInvalidSolutionInternal<false>(__invalid_id);                                        \
  } while (0)

#define flagSolutionWarning(message)                                                               \
  do                                                                                               \
  {                                                                                                \
    static const auto __invalid_id = this->registerInvalidSolutionInternal(message, true);         \
    this->flagInvalidSolutionInternal<true>(__invalid_id);                                         \
  } while (0)

// This macro is useful when we have different messages but appearing in the same place in the code
// for example when nesting a solution warning creation under the mooseWarning
#define flagSolutionWarningMultipleRegistration(message)                                           \
  do                                                                                               \
  {                                                                                                \
    const auto __invalid_id = this->registerInvalidSolutionInternal(message, true);                \
    this->flagInvalidSolutionInternal<true>(__invalid_id);                                         \
  } while (0)

// Every class using this interface must specify either
// 'usingCombinedWarningSolutionWarnings' or 'usingMooseBaseWarnings'
#define usingCombinedWarningSolutionWarnings                                                       \
  using SolutionInvalidInterface::mooseWarning;                                                    \
  using SolutionInvalidInterface::mooseWarningNonPrefixed;                                         \
  using SolutionInvalidInterface::mooseDeprecated;                                                 \
  using SolutionInvalidInterface::paramWarning
#define usingMooseBaseWarnings                                                                     \
  using MooseBase::mooseWarning;                                                                   \
  using MooseBase::mooseWarningNonPrefixed;                                                        \
  using MooseBase::mooseDeprecated;                                                                \
  using MooseBase::paramWarning

/**
 * An interface that allows the marking of invalid solutions during a solve
 */
class SolutionInvalidInterface
{
public:
  SolutionInvalidInterface(const MooseObject * const moose_object);

#ifdef MOOSE_KOKKOS_ENABLED
  /**
   * Special constructor used for Kokkos functor copy during parallel dispatch
   */
  SolutionInvalidInterface(const SolutionInvalidInterface & object,
                           const Moose::Kokkos::FunctorCopy & key);
#endif

protected:
  template <bool warning>
  void flagInvalidSolutionInternal(const InvalidSolutionID invalid_solution_id) const;

  // Register invalid solution with a message
  InvalidSolutionID registerInvalidSolutionInternal(const std::string & message,
                                                    const bool warning) const;

  template <typename... Args>
  void mooseWarning(Args &&... args) const
  {
    _si_moose_object.MooseBase::mooseWarning(std::forward<Args>(args)...);
    flagSolutionWarningMultipleRegistration(_si_moose_object.name() + ": warning");
  }

  template <typename... Args>
  void mooseWarningNonPrefixed(Args &&... args) const
  {
    _si_moose_object.MooseBase::mooseWarningNonPrefixed(std::forward<Args>(args)...);
    flagSolutionWarningMultipleRegistration(_si_moose_object.name() + ": warning");
  }

  template <typename... Args>
  void mooseDeprecated(Args &&... args) const
  {
    _si_moose_object.MooseBase::mooseDeprecated(std::forward<Args>(args)...);
    flagSolutionWarningMultipleRegistration(_si_moose_object.name() + ": deprecation");
  }

  template <typename... Args>
  void paramWarning(const std::string & param, Args... args) const
  {
    _si_moose_object.MooseBase::paramWarning(param, std::forward<Args>(args)...);
    flagSolutionWarningMultipleRegistration(_si_moose_object.name() + ": warning for parameter '" +
                                            param + "'");
  }

private:
  /// The MooseObject that owns this interface
  const MooseObject & _si_moose_object;

  /// A reference to FEProblem base
  const FEProblemBase & _si_problem;
};
