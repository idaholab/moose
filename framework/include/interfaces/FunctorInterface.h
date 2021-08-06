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
   * helper to look up a functor name through the input parameter keys
   * @param name The input parameter name that we are trying to deduce the functor name for
   * @param params The input parameters object that we will be checking for parameters named \p name
   * @return The functor name
   */
  static std::string deduceFunctorName(const std::string & name, const InputParameters & params);

protected:
  /**
   * retrieves a functor from the subproblem. This method also leverages the ability to create
   * default functors if the user passed an integer or real in the input file
   */
  template <typename T>
  const Moose::Functor<T> & getFunctor(const std::string & name);

  /**
   * checks the subproblem for the given functor. This will not query default functors potentially
   * stored in this object, e.g. this method will return false if the user passed an int or real to
   * the functor param in the input file
   */
  bool isFunctor(const std::string & name) const;

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
   * Helper function to parse default functor values. This is implemented
   * as a specialization for supported types and returns NULL in all other cases.
   */
  template <typename T>
  const Moose::Functor<T> * defaultFunctor(const std::string & name);

  /// Parameters of the object with this interface
  const InputParameters & _fi_params;

  /// The name of the object that this interface belongs to
  const std::string _fi_name;

  /// Reference to the subproblem
  SubProblem & _fi_subproblem;

  /// Current threaded it
  const THREAD_ID _fi_tid;

  /// Storage vector for Moose::Functor<Real> default objects
  std::vector<std::unique_ptr<Moose::Functor<Real>>> _default_real_functors;

  /// Storage vector for Moose::Functor<ADReal> default objects
  std::vector<std::unique_ptr<Moose::Functor<ADReal>>> _default_ad_real_functors;
};

template <typename T>
const Moose::Functor<T> &
FunctorInterface::getFunctor(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string functor_name = deduceFunctorName(name);

  // Check if it's just a constant
  const auto * const default_functor = defaultFunctor<T>(functor_name);
  if (default_functor)
    return *default_functor;

  return _fi_subproblem.getFunctor<T>(functor_name, _fi_tid, _fi_name);
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
