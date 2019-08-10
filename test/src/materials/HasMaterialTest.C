//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HasMaterialTest.h"

registerMooseObject("MooseTestApp", HasMaterialTest);

template <>
InputParameters
validParams<HasMaterialTest>()
{
  return validParams<Material>();
}

HasMaterialTest::HasMaterialTest(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _before_found(hasMaterialProperty<Real>("before") ? true : false),
    _after_found(hasMaterialProperty<Real>("after") ? true : false),
    _before(getDefaultMaterialProperty<Real>("before")),
    _after(getDefaultMaterialProperty<Real>("after"))
{
  std::cout << "property before found before getDefaultMaterialProperty? " << _before_found << " <- Should be 1 if material was found"
            << std::endl;
  std::cout << "property after found before getDefaultMaterialProperty? " << _after_found << " <- Should be 1 if material was found ### THIS IS THE PROBLEM HERE ###"
            << std::endl;
  std::cout << "property before found after getDefaultMaterialProperty? "
            << hasMaterialProperty<Real>("before") <<  " <- Should be 1 if since material is defined now by getDefaultMaterialProperty" << std::endl;
  std::cout << "property after found after getDefaultMaterialProperty? "
            << hasMaterialProperty<Real>("after") << " <- Should be 1 if since material is defined now by getDefaultMaterialProperty" << std::endl << std::endl;
}

void
HasMaterialTest::computeQpProperties()
{
  std::cout << "before value: " << _before[_qp] << " <- Should be 1 if getDefaultMaterialProperty finds the property correctly" << std::endl;
  std::cout << "after value: " << _after[_qp] << " <- Should be 1 if getDefaultMaterialProperty finds the property correctly" << std::endl << std::endl;
}
