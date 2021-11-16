//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

class Material;

#include <string>

/**
 * Helper class for deferred getting of material properties after the construction
 * phase for materials. This enables "optional material properties" in materials.
 * It works by returning a reference to a pointer to a material property (rather
 * than a reference to the property value). The pointer will be set to point to
 * either an existing material property or to nullptr if the requested property
 * does not exist.
 */
class OptionalMaterialPropertyProxyBase
{
public:
  OptionalMaterialPropertyProxyBase(const std::string & name, Material * material)
    : _name(name), _material(material)
  {
  }
  virtual ~OptionalMaterialPropertyProxyBase() {}
  virtual void resolve() = 0;

protected:
  const std::string _name;
  Material * _material;
};

template <typename T>
class OptionalMaterialPropertyProxy : public OptionalMaterialPropertyProxyBase
{
public:
  OptionalMaterialPropertyProxy(const std::string & name, Material * material)
    : OptionalMaterialPropertyProxyBase(name, material), _value(nullptr)
  {
  }
  const MaterialProperty<T> * const & value() { return _value; }
  void resolve() override;

protected:
  const MaterialProperty<T> * _value;
};

template <typename T>
class OptionalADMaterialPropertyProxy : public OptionalMaterialPropertyProxyBase
{
public:
  OptionalADMaterialPropertyProxy(const std::string & name, Material * material)
    : OptionalMaterialPropertyProxyBase(name, material), _value(nullptr)
  {
  }
  const ADMaterialProperty<T> * const & value() { return _value; }
  void resolve();

protected:
  const ADMaterialProperty<T> * _value;
};
