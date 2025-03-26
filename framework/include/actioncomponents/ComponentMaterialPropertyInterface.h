//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActionComponent.h"
#include "InputParameters.h"
#include "MooseTypes.h"

/**
 * Helper class to help Components define the material properties the Physics may need
 * from the parameters specified by the user
 * Note: Trying out virtual inheritance. It makes things
 *       a little easier to define as we can use the attributes
 *       of the underlying ActionComponent
 */
class ComponentMaterialPropertyInterface : public virtual ActionComponent
{
public:
  static InputParameters validParams();

  ComponentMaterialPropertyInterface(const InputParameters & params);

  /// Whether the component has a property with that name
  bool hasProperty(const std::string & property_name) const;
  /// Return the name of the functor for that property
  const MooseFunctorName & getPropertyValue(const std::string & property_name,
                                            const std::string & requestor_name) const;

protected:
  virtual void addMaterials() override;

  /// Names of the material properties
  const std::vector<std::string> _property_names;
  /// Functor values of the material properties
  const std::vector<MooseFunctorName> _property_functors;
  /// Whether to use automatic differentiation when defining properties
  const bool _use_ad_for_props;
};
