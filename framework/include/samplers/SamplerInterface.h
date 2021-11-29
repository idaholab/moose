//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelUniqueId.h"
#include "InputParameters.h"
#include "FEProblemBase.h"

// Forward declarations
class Sampler;
/**
 * Interface for objects that need to use samplers.
 *
 * This practically adds two methods for getting Sampler objects:
 *
 *  1. Call `getSampler` or `getSamplerByName` without a template parameter and you will get
 *     a `Sampler` base object (see SamplerInterface.C for the template specialization).
 *  2. Call `getSampler<MySampler>` or `getSamplerByName<MySampler>` to perform a cast to the
 *     desired type, as done for UserObjects.
 */
class SamplerInterface
{
public:
  static InputParameters validParams();

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
  template <typename T = Sampler>
  T & getSampler(const std::string & name);

  /**
   * Get a sampler with a given name
   * @param name The name of the sampler to retrieve
   * @return The sampler with name 'name'
   */
  template <typename T = Sampler>
  T & getSamplerByName(const SamplerName & name);

private:
  /// Parameters of the object with this interface
  const InputParameters & _si_params;

  /// Reference to FEProblemBase instance
  FEProblemBase & _si_feproblem;

  /// Thread ID
  THREAD_ID _si_tid;
};

template <typename T>
T &
SamplerInterface::getSampler(const std::string & name)
{
  return getSamplerByName<T>(_si_params.get<SamplerName>(name));
}

template <typename T>
T &
SamplerInterface::getSamplerByName(const SamplerName & name)
{
  Sampler * base_ptr = &_si_feproblem.getSampler(name, _si_tid);
  T * obj_ptr = dynamic_cast<T *>(base_ptr);
  if (!obj_ptr)
    mooseError("Failed to find a Sampler object with the name '", name, "' for the desired type.");
  return *obj_ptr;
}
