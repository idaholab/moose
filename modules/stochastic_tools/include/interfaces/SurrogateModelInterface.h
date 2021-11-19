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
class SurrogateModel;
class SurrogateTrainerBase;

/**
 * Interface for objects that need to use samplers.
 *
 * This practically adds two methods for getting SurrogateModel objects:
 *
 *  1. Call `getSurrogateModel` or `getSurrogateModelByName` without a template parameter and you
 * will get a `SurrogateModel` base object (see SurrogateModelInterface.C for the template
 * specialization).
 *  2. Call `getSurrogateModel<MySurrogateModel>` or `getSurrogateModelByName<MySurrogateModel>` to
 * perform a cast to the desired type, as done for UserObjects.
 */
class SurrogateModelInterface
{
public:
  static InputParameters validParams();

  /**
   * @param params The parameters used by the object being instantiated. This
   *        class needs them so it can get the sampler named in the input file,
   *        but the object calling getSurrogateModel only needs to use the name on the
   *        left hand side of the statement "sampler = sampler_name"
   */
  SurrogateModelInterface(const MooseObject * moose_object);

  ///@{
  /**
   * Get a SurrogateModel/Trainer with a given name
   * @param name The name of the parameter key of the sampler to retrieve
   * @return The sampler with name associated with the parameter 'name'
   */
  template <typename T = SurrogateModel>
  T & getSurrogateModel(const std::string & name) const;
  template <typename T = SurrogateTrainerBase>
  T & getSurrogateTrainer(const std::string & name) const;
  ///@}

  ///@{
  /**
   * Get a sampler with a given name
   * @param name The name of the sampler to retrieve
   * @return The sampler with name 'name'
   */
  template <typename T = SurrogateModel>
  T & getSurrogateModelByName(const UserObjectName & name) const;
  template <typename T = SurrogateTrainerBase>
  T & getSurrogateTrainerByName(const UserObjectName & name) const;

  ///@}
private:
  /// Parameters of the object with this interface
  const InputParameters & _smi_params;

  /// Reference to FEProblemBase instance
  FEProblemBase & _smi_feproblem;

  /// Thread ID
  const THREAD_ID _smi_tid;
};

template <typename T>
T &
SurrogateModelInterface::getSurrogateModel(const std::string & name) const
{
  return getSurrogateModelByName<T>(_smi_params.get<UserObjectName>(name));
}

template <typename T>
T &
SurrogateModelInterface::getSurrogateModelByName(const UserObjectName & name) const
{
  std::vector<T *> models;
  _smi_feproblem.theWarehouse()
      .query()
      .condition<AttribName>(name)
      .condition<AttribSystem>("SurrogateModel")
      .queryInto(models);
  if (models.empty())
    mooseError("Unable to find a SurrogateModel object of type " + std::string(typeid(T).name()) +
               " with the name '" + name + "'");
  return *(models[0]);
}

template <typename T>
T &
SurrogateModelInterface::getSurrogateTrainer(const std::string & name) const
{
  return getSurrogateTrainerByName<T>(_smi_params.get<UserObjectName>(name));
}

template <typename T>
T &
SurrogateModelInterface::getSurrogateTrainerByName(const UserObjectName & name) const
{
  SurrogateTrainerBase * base_ptr =
      &_smi_feproblem.getUserObject<SurrogateTrainerBase>(name, _smi_tid);
  T * obj_ptr = dynamic_cast<T *>(base_ptr);
  if (!obj_ptr)
    mooseError("Failed to find a SurrogateTrainer object of type " + std::string(typeid(T).name()) +
                   " with the name '",
               name,
               "' for the desired type.");
  return *obj_ptr;
}
