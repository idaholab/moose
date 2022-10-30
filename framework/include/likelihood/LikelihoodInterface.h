//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "ParallelUniqueId.h"
#include "MooseObject.h"

// Forward declarations
class Likelihood;
class FEProblemBase;

/**
 * Interface for objects that need to use likelihoods
 *
 * Inherit from this class at a very low level to make the getLikelihood method available.
 */
class LikelihoodInterface
{
public:
  static InputParameters validParams();

  LikelihoodInterface(const MooseObject * moose_object);

  ///@{
  /**
   * Get a likelihood with a given name
   * @param name The name of the parameter key of the likelihood to retrieve
   * @return The likelihood with name associated with the parameter 'name'
   */
  const Likelihood & getLikelihood(const std::string & name) const;

  template <typename T>
  const T & getLikelihood(const std::string & name) const;
  ///@}

  ///@{
  /**
   * Get a likelihood with a given name
   * @param name The name of the likelihood to retrieve
   * @return The likelihood with name 'name'
   */
  const Likelihood & getLikelihoodByName(const LikelihoodName & name) const;

  template <typename T>
  const T & getLikelihoodByName(const std::string & name) const;
  ///@}

private:
  /// Parameters of the object with this interface
  const InputParameters & _dni_params;

  /// Reference to FEProblemBase instance
  FEProblemBase & _dni_feproblem;

  /// Pointer to the MooseObject
  const MooseObject * const _dni_moose_object_ptr;
};

template <typename T>
const T &
LikelihoodInterface::getLikelihood(const std::string & name) const
{
  try
  {
    const T & dist = dynamic_cast<const T &>(getLikelihood(name));
    return dist;
  }
  catch (std::bad_cast & exception)
  {
    LikelihoodName dist_name = _dni_params.get<LikelihoodName>(name);
    mooseError("The '",
               _dni_moose_object_ptr->name(),
               "' object failed to retrieve '",
               dist_name,
               "' likelihood with the desired type.");
  }
}

template <typename T>
const T &
LikelihoodInterface::getLikelihoodByName(const std::string & name) const
{
  try
  {
    const T & dist = dynamic_cast<const T &>(getLikelihood(name));
    return dist;
  }
  catch (std::bad_cast & exception)
  {
    mooseError("The '",
               _dni_moose_object_ptr->name(),
               "' object failed to retrieve '",
               name,
               "' likelihood with the desired type.");
  }
}
