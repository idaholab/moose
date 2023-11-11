//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Standard includes
#include <string>

// MOOSE includes
#include "MooseTypes.h"
#include "Material.h"

// Forward Declarations
class FEProblemBase;
class InputParameters;
class PostprocessorName;
class MooseObject;

/**
 * Interface class for classes which interact with Postprocessors.
 * Provides the getPostprocessorValueXYZ() and related interfaces.
 */
class InterpolatedStatefulMaterialPropertyInterface
{
public:
  InterpolatedStatefulMaterialPropertyInterface(MooseObject * moose_object);

  template <typename U>
  const MaterialProperty<U> & getInterpolatedMaterialPropertyOld(const std::string & name);
  template <typename U>
  const MaterialProperty<U> & getInterpolatedMaterialPropertyOldByName(const std::string & name);

protected:
  const std::string _pomps_prefix;

  MooseObject * _moose_object;
  MaterialPropertyInterface * _material_property_interface;
  Material * _material;
};

InterpolatedStatefulMaterialPropertyInterface::InterpolatedStatefulMaterialPropertyInterface(
    MooseObject * moose_object)
  : _pomps_prefix("_pomps_"),
    _moose_object(moose_object),
    _material_property_interface(dynamic_cast<MaterialPropertyInterface *>(moose_object)),
    _material(dynamic_cast<Material *>(moose_object))
{
  if (!_material_property_interface && !_material)
    mooseError("InterpolatedStatefulMaterialPropertyInterface can only be used from a class that "
               "either derives from Material or MaterialPropertyInterface");
}

template <typename U>
const MaterialProperty<U> &
InterpolatedStatefulMaterialPropertyInterface::getInterpolatedMaterialPropertyOld(
    const std::string & name)
{
  return getInterpolatedMaterialPropertyOldByName<U>(
      _material->getParam<MaterialPropertyName>(name));
}

template <typename U>
const MaterialProperty<U> &
InterpolatedStatefulMaterialPropertyInterface::getInterpolatedMaterialPropertyOldByName(
    const std::string & name)
{
  if (_material)
    return _material->getMaterialPropertyByName<U>(_pomps_prefix + "mat_" + name);
  else
    return _material_property_interface->getMaterialPropertyOldByName<U>(_pomps_prefix + "mat_" +
                                                                         name);
}
