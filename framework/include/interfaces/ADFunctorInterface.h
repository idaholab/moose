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
#include "ADifyFunctor.h"

/**
 * \class ADFunctorInterface
 * \brief An interface for accessing \p Moose::Functors for systems that care about automatic
 * differentiation, e.g. AD kernels
 */
class ADFunctorInterface : public FunctorInterface
{
public:
  static InputParameters validParams();

  ADFunctorInterface(const MooseObject * moose_object);

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
  std::vector<std::unique_ptr<Moose::FunctorEnvelopeBase>> _adified_functors;
};

template <typename T>
const Moose::Functor<T> &
ADFunctorInterface::getFunctor(const std::string & name, SubProblem & subproblem)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string functor_name = deduceFunctorName(name);

  // Check if it's just a constant
  const auto * const default_functor = defaultFunctor<T>(functor_name);
  if (default_functor)
    return *default_functor;

  if (subproblem.hasFunctorWithType<T>(functor_name, _fi_tid))
    return subproblem.getFunctor<T>(functor_name, _fi_tid, _fi_name);
  // has someone requested an AD functor when the subproblem holds a non-AD functor? If so, then we
  // create the AD-ified functor
  else if (std::is_same<T, typename Moose::ADType<T>::type>::value &&
           subproblem.hasFunctorWithType<typename MetaPhysicL::RawType<T>::value_type>(functor_name,
                                                                                       _fi_tid))
  {
    const Moose::FunctorBase<typename MetaPhysicL::RawType<T>::value_type> & non_ad_functor =
        subproblem.getFunctor<typename MetaPhysicL::RawType<T>::value_type>(
            functor_name, _fi_tid, _fi_name);
    _adified_functors.emplace_back(std::make_unique<Moose::Functor<T>>(
        std::make_unique<Moose::ADifyFunctor<T>>(non_ad_functor)));
    return static_cast<Moose::Functor<T> &>(*_adified_functors.back());
  }
  else
    mooseError("SubProblem doesn't hold any functors that will work with requested type ",
               demangle(typeid(T).name()));
}

template <typename T>
const Moose::Functor<T> &
ADFunctorInterface::getFunctor(const std::string & name)
{
  mooseAssert(_fi_subproblem, "This must be non-null");
  return getFunctor<T>(name, *_fi_subproblem);
}
