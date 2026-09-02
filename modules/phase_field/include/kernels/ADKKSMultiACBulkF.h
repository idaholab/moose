//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKKSMultiACBulkBase.h"

class ADKKSMultiACBulkF : public ADKKSMultiACBulkBase
{
public:
  static InputParameters validParams();

  ADKKSMultiACBulkF(const InputParameters & parameters);

protected:
  virtual ADReal computeDFDOP();

  ADReal _wi;

  const MaterialPropertyName _gi_name;

  const ADMaterialProperty<Real> & _prop_dgi;
};