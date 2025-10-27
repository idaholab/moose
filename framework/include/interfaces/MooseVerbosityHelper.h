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
class FEProblemBase;
class MooseBase;

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

/**
 * An interface that allows:
 * - the creation of info messages, warnings and errors
 * - the marking of invalid solutions during a solve
 */
class MooseVerbosityHelper
{
public:
  MooseVerbosityHelper(const MooseBase * const moose_base, const InputParameters & params);

#ifdef MOOSE_KOKKOS_ENABLED
  /**
   * Special constructor used for Kokkos functor copy during parallel dispatch
   */
  MooseVerbosityHelper(const MooseVerbosityHelper & object, const Moose::Kokkos::FunctorCopy & key);
#endif
  /**
   * @returns The block-level hit node for this object, if any
   */
  const hit::Node * getHitNode() const;

  /**
   * Emits an error prefixed with the file and line number of the given param (from the input
   * file) along with the full parameter path+name followed by the given args as the message.
   * If this object's parameters were not created directly by the Parser, then this function falls
   * back to the normal behavior of mooseError - only printing a message using the given args.
   */
  template <typename... Args>
  [[noreturn]] void paramError(const std::string & param, Args... args) const;

  /**
   * Emits an informational message prefixed with the file and line number of the given param
   * (from the input file) along with the full parameter path+name followed by the given args as
   * the message.  If this object's parameters were not created directly by the Parser, then this
   * function falls back to the normal behavior of mooseInfo - only printing a message using
   * the given args.
   */
  template <typename... Args>
  void paramInfo(const std::string & param, Args... args) const;

  /**
   * Emits a warning prefixed with object name and type.
   */
  template <typename... Args>
  void untrackedMooseWarning(Args &&... args) const;

  /**
   * Emits a warning without the prefixing included in mooseWarning().
   */
  template <typename... Args>
  void untrackedMooseWarningNonPrefixed(Args &&... args) const;

  /**
   * Emits a warning prefixed with the file and line number of the given param (from the input
   * file) along with the full parameter path+name followed by the given args as the message.
   * If this object's parameters were not created directly by the Parser, then this function falls
   * back to the normal behavior of mooseWarning - only printing a message using the given args.
   */
  template <typename... Args>
  void untrackedParamWarning(const std::string & param, Args... args) const;

  template <typename... Args>
  void untrackedMooseDeprecated(Args &&... args) const;

  template <typename... Args>
  void mooseWarning(Args &&... args) const;

  template <typename... Args>
  void mooseWarningNonPrefixed(Args &&... args) const;

  template <typename... Args>
  void mooseDeprecated(Args &&... args) const;

  template <typename... Args>
  void mooseInfo(Args &&... args) const;

  template <typename... Args>
  void paramWarning(const std::string & param, Args... args) const;

  /**
   * @returns A prefix to be used in messages that contain the input
   * file location associated with this object (if any) and the
   * name and type of the object.
   */
  std::string messagePrefix(const bool hit_prefix = true) const;

  /**
   * Deprecated message prefix; the error type is no longer used
   */
  std::string errorPrefix(const std::string &) const { return messagePrefix(); }

  /**
   * Emits an error prefixed with object name and type and optionally a file path
   * to the top-level block parameter if available.
   */
  template <typename... Args>
  [[noreturn]] void mooseError(Args &&... args) const
  {
    callMooseError(argumentsToString(std::forward<Args>(args)...), /* with_prefix = */ true);
  }

  template <typename... Args>
  [[noreturn]] void mooseDocumentedError(const std::string & repo_name,
                                         const unsigned int issue_num,
                                         Args &&... args) const
  {
    callMooseError(moose::internal::formatMooseDocumentedError(
                       repo_name, issue_num, argumentsToString(std::forward<Args>(args)...)),
                   /* with_prefix = */ true);
  }

  /**
   * Emits an error without the prefixing included in mooseError().
   */
  template <typename... Args>
  [[noreturn]] void mooseErrorNonPrefixed(Args &&... args) const
  {
    callMooseError(argumentsToString(std::forward<Args>(args)...), /* with_prefix = */ false);
  }

  /**
   * External method for calling moose error with added object context.
   * @param msg The message
   * @param with_prefix If true, add the prefix from messagePrefix(), which is the object
   * information (type, name, etc)
   * @param node Optional hit node to add file path context as a prefix
   */
  [[noreturn]] void
  callMooseError(std::string msg, const bool with_prefix, const hit::Node * node = nullptr) const;

  /**
   * External method for calling moose error with added object context.
   *
   * Needed so that objects without the MooseBase context (InputParameters)
   * can call errors with context
   *
   * @param app The app pointer (if available); adds multiapp context and clears the console
   * @param params The parameters, needed to obtain object information
   * @param msg The message
   * @param with_prefix If true, add the prefix from messagePrefix(), which is the object
   * information (type, name, etc)
   * @param node Optional hit node to add file path context as a prefix
   */
  [[noreturn]] static void callMooseError(MooseApp * const app,
                                          const InputParameters & params,
                                          std::string msg,
                                          const bool with_prefix,
                                          const hit::Node * node);

protected:
  template <bool warning>
  void flagInvalidSolutionInternal(const InvalidSolutionID invalid_solution_id) const;

  // Register invalid solution with a message
  InvalidSolutionID registerInvalidSolutionInternal(const std::string & message,
                                                    const bool warning) const;

private:
  /**
   * Internal method for getting the message prefix for an object (object type, name, etc).
   *
   * Needs to be static so that we can call it externally from InputParameters for
   * errors that do not have context of the MooseBase
   */
  static std::string messagePrefix(const InputParameters & params, const bool hit_prefix);

  /**
   * Internal method for getting a hit node (if available) given a set of parameters
   *
   * Needs to be static so that we can call it externally from InputParameters for
   * errors that do not have context of the MooseBase
   */
  static const hit::Node * getHitNode(const InputParameters & params);

  /// The MooseBase that owns this interface
  const MooseBase & _moose_base;

  /// A pointer to FEProblem base
  const FEProblemBase * _fe_problem_base;
};
