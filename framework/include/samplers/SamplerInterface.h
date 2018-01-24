//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SAMPLERINTERFACE_H
#define SAMPLERINTERFACE_H

#include "ParallelUniqueId.h"
#include "InputParameters.h"
#include "FEProblemBase.h"

// Forward declarations
class Sampler;
class SamplerInterface;

template <>
InputParameters validParams<SamplerInterface>();

/**
 * Interface for objects that need to use samplers
 *
 * Inherit from this class at a very low level to make the getSampler method
 * available.
 */
class SamplerInterface
{
public:
  /**
   * @param params The parameters used by the object being instantiated. This
   *        class needs them so it can get the sampler named in the input file,
   *        but the object calling getSampler only needs to use the name on the
   *        left hand side of the statement "sampler = sampler_name"
   */
  SamplerInterface(const MooseObject * moose_object);

  /**
   * Get a sampler with a given name
   * @param name The name of the parameter key of the sampler to retrieve
   * @return The sampler with name associated with the parameter 'name'
   */
  Sampler & getSampler(const std::string & name);

  /**
   * Get a sampler with a given name
   * @param name The name of the sampler to retrieve
   * @return The sampler with name 'name'
   */
  Sampler & getSamplerByName(const SamplerName & name);

private:
  /// Parameters of the object with this interface
  const InputParameters & _smi_params;

  /// Reference to FEProblemBase instance
  FEProblemBase & _smi_feproblem;

  /// Thread ID
  THREAD_ID _smi_tid;
};

#endif /* SAMPLERINTERFACE_H */
