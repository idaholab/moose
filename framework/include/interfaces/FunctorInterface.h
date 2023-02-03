//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADReal.h"
#include "MooseTypes.h"
#include "SubProblem.h"
#include "MooseFunctorForward.h"

#include <vector>
#include <memory>
#include <string>

class InputParameters;
class MooseObject;

/**
 * \class FunctorInterface
 * \brief An interface for accessing \p Moose::Functors
 */
class FunctorInterface
{
public:
  static InputParameters validParams();

  FunctorInterface(const MooseObject * moose_object);

  /**
   * Helper to look up a functor name through the input parameter keys
   * @param name The input parameter name that we are trying to deduce the functor name for
   * @param params The input parameters object that we will be checking for parameters named \p name
   * @return The functor name
   */
  static std::string deduceFunctorName(const std::string & name, const InputParameters & params);

protected:
  /**
   * Retrieves a functor from the subproblem. This method also leverages the ability to create
   * default functors if the user passed an integer or real in the input file
   * @param name The name of the functor to retrieve. This should match the functor parameter name,
   * \emph not the actual name of the functor created in the input file
   * @return The functor
   */
  template <typename T>
  const Moose::Functor<T> & getFunctor(const std::string & name);

  /**
   * Retrieves a functor from the passed-in subproblem. This method also leverages the ability to
   * create default functors if the user passed an integer or real in the input file
   * @param name The name of the functor to retrieve. This should match the functor parameter name,
   * \emph not the actual name of the functor created in the input file
   * @param subproblem The subproblem to query for the functor
   * @return The functor
   */
  template <typename T>
  const Moose::Functor<T> & getFunctor(const std::string & name, SubProblem & subproblem);

  /**
   * Checks the subproblem for the given functor. This will not query default functors potentially
   * stored in this object, e.g. this method will return false if the user passed an int or real to
   * the functor param in the input file
   * @param name The name of the functor to check. This should match the functor parameter name,
   * \emph not the actual name of the functor created in the input file
   * @return Whether the subproblem has the specified functor
   */
  bool isFunctor(const std::string & name) const;

  /**
   * Checks the passed-in subproblem for the given functor. This will not query default functors
   * potentially stored in this object, e.g. this method will return false if the user passed an int
   * or real to the functor param in the input file
   * @param name The name of the functor to check. This should match the functor parameter name,
   * \emph not the actual name of the functor created in the input file
   * @param subproblem The subproblem to query for the functor
   * @return Whether the subproblem has the specified functor
   */
  bool isFunctor(const std::string & name, const SubProblem & subproblem) const;

  /**
   * Small helper to look up a functor name through the input parameter keys
   */
  std::string deduceFunctorName(const std::string & name) const;

  /**
   * Helper method to create an elemental argument for a functor that includes whether to perform
   * skewness corrections
   */
  Moose::ElemArg makeElemArg(const Elem * elem, bool correct_skewnewss = false) const;

private:
  /**
   * Whether this interface is for an AD object
   */
  virtual bool isADObject() const = 0;

  /**
   * Helper function to parse default functor values. This is implemented
   * as a specialization for supported types and returns NULL in all other cases.
   */
  template <typename T>
  const Moose::Functor<T> * defaultFunctor(const std::string & name);

  /// Parameters of the object with this interface
  const InputParameters & _fi_params;

  /// The name of the object that this interface belongs to
  const std::string _fi_name;

  /// Pointer to subproblem if the subproblem pointer parameter was set
  SubProblem * const _fi_subproblem;

  /// Current threaded it
  const THREAD_ID _fi_tid;

  /// Storage vector for Moose::Functor<Real> default objects
  std::vector<std::unique_ptr<Moose::Functor<Real>>> _default_real_functors;

  /// Storage vector for Moose::Functor<ADReal> default objects
  std::vector<std::unique_ptr<Moose::Functor<ADReal>>> _default_ad_real_functors;
};

template <typename T>
const Moose::Functor<T> &
FunctorInterface::getFunctor(const std::string & name, SubProblem & subproblem)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string functor_name = deduceFunctorName(name);

  // Check if it's just a constant
  const auto * const default_functor = defaultFunctor<T>(functor_name);
  if (default_functor)
    return *default_functor;

  return subproblem.getFunctor<T>(functor_name, _fi_tid, _fi_name, isADObject());
}

template <typename T>
const Moose::Functor<T> &
FunctorInterface::getFunctor(const std::string & name)
{
  mooseAssert(_fi_subproblem, "This must be non-null");
  return getFunctor<T>(name, *_fi_subproblem);
}

template <>
const Moose::Functor<Real> * FunctorInterface::defaultFunctor<Real>(const std::string & name);

template <>
const Moose::Functor<ADReal> * FunctorInterface::defaultFunctor<ADReal>(const std::string & name);

// General version for types that do not accept default values
template <typename T>
const Moose::Functor<T> *
FunctorInterface::defaultFunctor(const std::string & /*name*/)
{
  return nullptr;
}
