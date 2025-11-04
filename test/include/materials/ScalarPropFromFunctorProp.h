//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Material.h"

class ScalarPropFromFunctorProp : public Material
{
public:
  static InputParameters validParams();

  ScalarPropFromFunctorProp(const InputParameters & params);

protected:
  void computeQpProperties() override;
  const Moose::Functor<ADReal> & _functor;
  ADMaterialProperty<Real> & _prop;
};
