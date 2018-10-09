//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DISTRIBUTIONINTERFACE_H
#define DISTRIBUTIONINTERFACE_H

#include "InputParameters.h"
#include "ParallelUniqueId.h"
#include "FEProblemBase.h"

// Forward declarations
class Distribution;
class DistributionInterface;

template <>
InputParameters validParams<DistributionInterface>();

/**
 * Interface for objects that need to use distributions
 *
 * Inherit from this class at a very low level to make the getDistribution method
 * available.
 */
class DistributionInterface
{
public:
  /**
   * @param params The parameters used by the object being instantiated. This
   *        class needs them so it can get the distribution named in the input file,
   *        but the object calling getDistribution only needs to use the name on the
   *        left hand side of the statement "distribution = dist_name"
   */
  DistributionInterface(const MooseObject * moose_object);

  /**
   * Get a distribution with a given name
   * @param name The name of the parameter key of the distribution to retrieve
   * @return The distribution with name associated with the parameter 'name'
   */
  Distribution & getDistribution(const std::string & name);

  /**
   * Get a distribution with a given name
   * @param name The name of the distribution to retrieve
   * @return The distribution with name 'name'
   */
  Distribution & getDistributionByName(const DistributionName & name);

private:
  /// Parameters of the object with this interface
  const InputParameters & _dni_params;

  /// Reference to FEProblemBase instance
  FEProblemBase & _dni_feproblem;
};

#endif /* DISTRIBUTIONINTERFACE_H */
