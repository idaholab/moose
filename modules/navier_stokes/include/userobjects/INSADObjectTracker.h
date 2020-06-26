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

template <typename>
class ADMaterialProperty;
template <typename>
class MaterialProperty;

/**
 * Object for tracking what kernels have been added to an INSAD simulation. This is used to
 * determine what properties to calculate in the INSADMaterial, which is important particularly for
 * ensuring that we have consistenly included all the strong terms for stabilization methods like
 * PSPG and SUPG
 */
class INSADObjectTracker : public GeneralUserObject, public InputParameters
{
public:
  static InputParameters validParams();

  INSADObjectTracker(const InputParameters & parameters);

  // GeneralUserObject implicitly deletes operator=
  INSADObjectTracker & operator=(const INSADObjectTracker &) = delete;
  using InputParameters::operator=;

  /**
   * Set the internal parameter \p name to \p value. This will check whether \p name has already
   * been set and if so, it will whether the old and new values are consistent. If they are not,
   * then we will error
   */
  template <typename T>
  void set(const std::string & name, const T & value);

  /**
   * Get the internal parameter \p name. This will check whether \p name has already
   * been set by the user. If it has not, then we will error
   */
  template <typename T>
  const T & get(const std::string & name) const;

  virtual void initialize() final {}
  virtual void execute() final {}
  virtual void finalize() final {}
};

template <typename T>
void
INSADObjectTracker::set(const std::string & name, const T & value)
{
  if (isParamSetByUser(name))
  {
    const T & current_value = get<T>(name);
    if (current_value != value)
      mooseError("Two INSADObjects set different values for the parameter", name);
  }
  else if (!have_parameter<T>(name))
    mooseError("Attempting to set parameter ", name, " that is not a valid param");
  else
    InputParameters::template set<T>(name) = value;
}

template <typename T>
const T &
INSADObjectTracker::get(const std::string & name) const
{
  if (!InputParameters::isParamValid(name))
    mooseError("The parameter ", name, " is being retrieved before being set");

  return InputParameters::template get<T>(name);
}
