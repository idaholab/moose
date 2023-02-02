//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorInterface.h"
#include "RawValueFunctor.h"

/**
 * \class NonADFunctorInterface
 * \brief An interface for accessing \p Moose::Functors for systems that do not care about automatic
 * differentiation, e.g. auxiliary kernels
 */
class NonADFunctorInterface : public FunctorInterface
{
public:
  static InputParameters validParams();

  NonADFunctorInterface(const MooseObject * moose_object);

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

private:
  /// Storage vector for raw functors
  std::vector<std::unique_ptr<Moose::FunctorEnvelopeBase>> _raw_value_functors;
};

template <typename T>
const Moose::Functor<T> &
NonADFunctorInterface::getFunctor(const std::string & name, SubProblem & subproblem)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string functor_name = deduceFunctorName(name);

  // Check if it's just a constant
  const auto * const default_functor = defaultFunctor<T>(functor_name);
  if (default_functor)
    return *default_functor;

  if (subproblem.hasFunctorWithType<T>(functor_name, _fi_tid))
    return subproblem.getFunctor<T>(functor_name, _fi_tid, _fi_name);
  // has someone requested a non-AD functor when the subproblem holds an AD functor? If so, then we
  // create the raw-value functor
  else if (std::is_same<T, typename MetaPhysicL::RawType<T>::value_type>::value &&
           subproblem.hasFunctorWithType<typename Moose::ADType<T>::type>(functor_name, _fi_tid))
  {
    const Moose::FunctorBase<typename Moose::ADType<T>::type> & ad_functor =
        subproblem.getFunctor<typename Moose::ADType<T>::type>(functor_name, _fi_tid, _fi_name);
    _raw_value_functors.emplace_back(std::make_unique<Moose::Functor<T>>(
        std::make_unique<Moose::RawValueFunctor<T>>(ad_functor)));
    return static_cast<Moose::Functor<T> &>(*_raw_value_functors.back());
  }
  else
    mooseError("SubProblem doesn't hold any functors that will work with requested type ",
               demangle(typeid(T).name()));
}

template <typename T>
const Moose::Functor<T> &
NonADFunctorInterface::getFunctor(const std::string & name)
{
  mooseAssert(_fi_subproblem, "This must be non-null");
  return getFunctor<T>(name, *_fi_subproblem);
}
