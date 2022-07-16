//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MaterialProperty.h"

/**
 * Helper class for deferred getting of material properties after the construction
 * phase for materials. This enables "optional material properties" in materials.
 * It works by returning a reference to a pointer to a material property (rather
 * than a reference to the property value). The pointer will be set to point to
 * either an existing material property or to nullptr if the requested property
 * does not exist.
 */
template <class M>
class DeferredMaterialPropertyProxyBase
{
public:
  DeferredMaterialPropertyProxyBase(const std::string & name, MaterialPropState state)
    : _name(name), _state(state)
  {
  }
  virtual ~DeferredMaterialPropertyProxyBase() {}
  virtual void resolve(M & material) = 0;

protected:
  const std::string _name;
  const MaterialPropState _state;
};

/**
 * property type specific derived proxy object
 */

template <class M, typename T, bool is_ad>
class DeferredMaterialPropertyProxy : public DeferredMaterialPropertyProxyBase<M>
{
public:
  DeferredMaterialPropertyProxy(const std::string & name, MaterialPropState state)
    : DeferredMaterialPropertyProxyBase<M>(name, state)
  {
  }
  const GenericOptionalMaterialProperty<T, is_ad> & value() const { return _value; }
  void resolve(M & mpi) override;

protected:
  GenericOptionalMaterialProperty<T, is_ad> _value;
};

/**
 * Inteface class for MaterialProperty consumers to support deferred resolution of optional material
 * properties.
 */
class DeferredMaterialPropertyResolutionInterface
{
public:
  DeferredMaterialPropertyResolutionInterface(const MooseObject * moose_object)
  {
    moose_object->getMooseApp().registerInterfaceObject(*this);
  }

  virtual void resolveDeferredProperties() = 0;
};

template <class M, typename T, bool is_ad>
void
DeferredMaterialPropertyProxy<M, T, is_ad>::resolve(M & mpi)
{
  if (mpi.template hasGenericMaterialProperty<T, is_ad>(this->_name))
    switch (this->_state)
    {
      case MaterialPropState::CURRENT:
        _value.set(&mpi.template getGenericMaterialProperty<T, is_ad>(this->_name));
        break;

      case MaterialPropState::OLD:
        if constexpr (is_ad)
          mooseError("Old material properties are not available as AD");
        else
          _value.set(&mpi.template getMaterialPropertyOld<T>(this->_name));
        break;

      case MaterialPropState::OLDER:
        if constexpr (is_ad)
          mooseError("Older material properties are not available as AD");
        else
          _value.set(&mpi.template getMaterialPropertyOlder<T>(this->_name));
        break;
    }
}
