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

class ADKKSMultiACBulkC : public ADKKSMultiACBulkBase
{
public:
  static InputParameters validParams();

  ADKKSMultiACBulkC(const InputParameters & parameters);

protected:
  virtual ADReal computeDFDOP();

  MaterialPropertyName _c1_name;
  const std::vector<const ADVariableValue *> _cjs;
  const std::vector<unsigned int> _cjs_var;

  const ADMaterialProperty<Real> & _prop_dF1dc1;
};
