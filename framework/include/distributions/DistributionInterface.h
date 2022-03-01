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
class Distribution;
class FEProblemBase;

/**
 * Interface for objects that need to use distributions
 *
 * Inherit from this class at a very low level to make the getDistribution method available.
 */
class DistributionInterface
{
public:
  static InputParameters validParams();

  DistributionInterface(const MooseObject * moose_object);

  ///@{
  /**
   * Get a distribution with a given name
   * @param name The name of the parameter key of the distribution to retrieve
   * @return The distribution with name associated with the parameter 'name'
   */
  const Distribution & getDistribution(const std::string & name) const;

  template <typename T>
  const T & getDistribution(const std::string & name) const;
  ///@}

  ///@{
  /**
   * Get a distribution with a given name
   * @param name The name of the distribution to retrieve
   * @return The distribution with name 'name'
   */
  const Distribution & getDistributionByName(const DistributionName & name) const;

  template <typename T>
  const T & getDistributionByName(const std::string & name) const;
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
DistributionInterface::getDistribution(const std::string & name) const
{
  try
  {
    const T & dist = dynamic_cast<const T &>(getDistribution(name));
    return dist;
  }
  catch (std::bad_cast & exception)
  {
    DistributionName dist_name = _dni_params.get<DistributionName>(name);
    mooseError("The '",
               _dni_moose_object_ptr->name(),
               "' object failed to retrieve '",
               dist_name,
               "' distribution with the desired type.");
  }
}

template <typename T>
const T &
DistributionInterface::getDistributionByName(const std::string & name) const
{
  try
  {
    const T & dist = dynamic_cast<const T &>(getDistribution(name));
    return dist;
  }
  catch (std::bad_cast & exception)
  {
    mooseError("The '",
               _dni_moose_object_ptr->name(),
               "' object failed to retrieve '",
               name,
               "' distribution with the desired type.");
  }
}
