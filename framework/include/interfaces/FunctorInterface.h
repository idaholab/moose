//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
   * Retrieves a functor from the subproblem. This method also leverages the ability to create
   * default functors if the user passed an integer or real in the input file
   * @param name The name of the functor to retrieve. This should match the functor parameter name,
   * \emph not the actual name of the functor created in the input file
   * @param tid The thread ID used to retrieve the functor from this interface's subproblem
   * @return The functor
   */
  template <typename T>
  const Moose::Functor<T> & getFunctor(const std::string & name, THREAD_ID tid);

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
   * Retrieves a functor from the passed-in subproblem. This method also leverages the ability to
   * create default functors if the user passed an integer or real in the input file
   * @param name The name of the functor to retrieve. This should match the functor parameter name,
   * \emph not the actual name of the functor created in the input file
   * @param subproblem The subproblem to query for the functor
   * @param tid The thread ID used to retrieve the functor from the \p subproblem
   * @return The functor
   */
  template <typename T>
  const Moose::Functor<T> &
  getFunctor(const std::string & name, SubProblem & subproblem, THREAD_ID tid);

  /**
   * Checks the subproblem for the given functor. This will not query default functors
   * potentially stored in this object, e.g. this method will return false if the user passed an
   * int or real to the functor param in the input file
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

  /**
   * Throws error if the functor does not support the requested side integration
   *
   * @param[in] name  Name of functor or functor parameter
   * @param[in] qp_integration  True if performing qp integration, false if face info
   */
  template <typename T>
  void checkFunctorSupportsSideIntegration(const std::string & name, bool qp_integration);

private:
  /**
   * Retrieves a functor from the passed-in subproblem. This method also leverages the ability to
   * create default functors if the user passed an integer or real in the input file
   * @param name The actual name of the functor to retrieve instead of the parameter name
   * @param subproblem The subproblem to query for the functor
   * @param tid The thread ID used to retrieve the functor from the \p subproblem
   * @return The functor
   */
  template <typename T>
  const Moose::Functor<T> &
  getFunctorByName(const std::string & name, SubProblem & subproblem, THREAD_ID tid);

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
FunctorInterface::getFunctor(const std::string & name, SubProblem & subproblem, const THREAD_ID tid)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string functor_name = deduceFunctorName(name);
  return getFunctorByName<T>(functor_name, subproblem, tid);
}

template <typename T>
const Moose::Functor<T> &
FunctorInterface::getFunctor(const std::string & name, SubProblem & subproblem)
{
  return getFunctor<T>(name, subproblem, _fi_tid);
}

template <typename T>
const Moose::Functor<T> &
FunctorInterface::getFunctor(const std::string & name, const THREAD_ID tid)
{
  mooseAssert(_fi_subproblem, "This must be non-null");
  return getFunctor<T>(name, *_fi_subproblem, tid);
}

template <typename T>
const Moose::Functor<T> &
FunctorInterface::getFunctor(const std::string & name)
{
  mooseAssert(_fi_subproblem, "This must be non-null");
  return getFunctor<T>(name, *_fi_subproblem, _fi_tid);
}

template <typename T>
const Moose::Functor<T> &
FunctorInterface::getFunctorByName(const std::string & name,
                                   SubProblem & subproblem,
                                   const THREAD_ID tid)
{
  // Check if it's just a constant
  const auto * const default_functor = defaultFunctor<T>(name);
  if (default_functor)
    return *default_functor;

  return subproblem.getFunctor<T>(name, tid, _fi_name, isADObject());
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

template <typename T>
void
FunctorInterface::checkFunctorSupportsSideIntegration(const std::string & name, bool qp_integration)
{
  const std::string functor_name = deduceFunctorName(name);
  const auto & functor = getFunctor<T>(name);
  if (qp_integration)
  {
    if (!functor.supportsElemSideQpArg())
      mooseError("Quadrature point integration was requested, but the functor '",
                 functor_name,
                 "' does not support this.");
  }
  else
  {
    if (!functor.supportsFaceArg())
      mooseError("Face info integration was requested, but the functor '",
                 functor_name,
                 "' does not support this.");
  }
}
