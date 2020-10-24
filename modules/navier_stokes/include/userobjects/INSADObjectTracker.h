//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "MooseTypes.h"
#include "MooseEnum.h"
#include "MooseError.h"
#include "InputParameters.h"

#include "libmesh/vector_value.h"

#include <unordered_map>
#include <set>

template <typename>
class ADMaterialProperty;
template <typename>
class MaterialProperty;

/**
 * Global for adding ambient convection parameters
 */
void addAmbientConvectionParams(InputParameters & params);

/**
 * Object for tracking what kernels have been added to an INSAD simulation. This is used to
 * determine what properties to calculate in the INSADMaterial, which is important particularly for
 * ensuring that we have consistenly included all the strong terms for stabilization methods like
 * PSPG and SUPG
 */
class INSADObjectTracker : public GeneralUserObject
{
public:
  static InputParameters validParams();

  INSADObjectTracker(const InputParameters & parameters);

  /**
   * Set the internal parameter \p name to \p value. This will check whether \p name has already
   * been set and if so, it will whether the old and new values are consistent. If they are not,
   * then we will error
   */
  template <typename T>
  void set(const std::string & name, const T & value, SubdomainID sub_id);

  /**
   * Get the internal parameter \p name. This will check whether \p name has already
   * been set by the user. If it has not, then we will error
   */
  template <typename T>
  const T & get(const std::string & name, SubdomainID sub_id) const;

  virtual void initialize() final {}
  virtual void execute() final {}
  virtual void finalize() final {}

  bool isTrackerParamValid(const std::string & name, SubdomainID sub_id) const;

  /**
   * Add additional block coverage to this
   */
  void addBlockIDs(const std::set<SubdomainID> & additional_block_ids);

private:
  template <typename T>
  static bool notEqual(const T & val1, const T & val2);

  std::unordered_map<SubdomainID, InputParameters> _block_id_to_params;

  static InputParameters validTrackerParams();

  template <typename T>
  static void set(const std::string & name, const T & value, InputParameters & params);

  bool singleMaterialCoverage() const;

  const InputParameters & getParams(SubdomainID sub_id) const;
};

template <typename T>
bool
INSADObjectTracker::notEqual(const T & val1, const T & val2)
{
  return val1 != val2;
}

template <>
bool INSADObjectTracker::notEqual(const MooseEnum & val1, const MooseEnum & val2);

template <typename T>
void
INSADObjectTracker::set(const std::string & name, const T & value, InputParameters & params)
{
  if (params.isParamSetByUser(name))
  {
    const T & current_value = params.get<T>(name);
    if (INSADObjectTracker::notEqual(current_value, value))
      ::mooseError("Two INSADObjects set different values for the parameter ", name);
  }
  else if (!params.have_parameter<T>(name))
    ::mooseError("Attempting to set parameter ", name, " that is not a valid param");
  else
    params.set<T>(name) = value;
}

template <typename T>
void
INSADObjectTracker::set(const std::string & name, const T & value, const SubdomainID sub_id)
{
  if (sub_id == Moose::ANY_BLOCK_ID)
  {
    for (auto & pr : _block_id_to_params)
      INSADObjectTracker::set(name, value, pr.second);

    return;
  }

  INSADObjectTracker::set(name, value, const_cast<InputParameters &>(getParams(sub_id)));
}

template <typename T>
const T &
INSADObjectTracker::get(const std::string & name, const SubdomainID sub_id) const
{
  const InputParameters & params = getParams(sub_id);

  if (!params.isParamValid(name))
    mooseError("The parameter ", name, " is being retrieved before being set");

  return params.get<T>(name);
}
