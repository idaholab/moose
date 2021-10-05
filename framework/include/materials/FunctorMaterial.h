//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

template <typename>
class FunctorMaterialProperty;
template <typename>
class FunctorMaterialPropertyImpl;

/**
 * FunctorMaterials compute functor material properties
 */
class FunctorMaterial : public Material
{
public:
  static InputParameters validParams();

  FunctorMaterial(const InputParameters & parameters);
  void computeProperties() override final {}

protected:
  void computeQpProperties() override final {}

  /**
   * Declare a functor material property that this object will responsible for providing
   */
  template <typename T,
            template <typename> class P = FunctorMaterialPropertyImpl,
            class... ConstructionArgs>
  FunctorMaterialProperty<T> & declareFunctorProperty(const std::string & name,
                                                      ConstructionArgs &&... construction_args);
};

template <typename T, template <typename> class P, class... ConstructionArgs>
FunctorMaterialProperty<T> &
FunctorMaterial::declareFunctorProperty(const std::string & name,
                                        ConstructionArgs &&... construction_args)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = name;
  if (_pars.have_parameter<MaterialPropertyName>(name))
    prop_name = _pars.get<MaterialPropertyName>(name);

  return _subproblem.declareFunctorProperty<T, P>(
      prop_name, _tid, false, std::forward<ConstructionArgs>(construction_args)...);
}
