//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosElementIntegralMaterialProperty.h"
#include "KokkosSideIntegralMaterialProperty.h"

template <typename Base>
class KokkosAverageMaterialProperty : public Base
{
public:
  static InputParameters validParams();

  KokkosAverageMaterialProperty(const InputParameters & parameters);
};

typedef KokkosAverageMaterialProperty<KokkosElementIntegralMaterialProperty>
    KokkosElementAverageMaterialProperty;
typedef KokkosAverageMaterialProperty<KokkosSideIntegralMaterialProperty>
    KokkosSideAverageMaterialProperty;
