//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADAllenCahnBase.h"

class ADKKSMultiACBulkBase : public ADAllenCahnBase<Real>
{
public:
  ADKKSMultiACBulkBase(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void initialSetup();

protected:
  VariableName _etai_name;

  std::vector<MaterialPropertyName> _Fj_names;
  unsigned int _num_j;

  std::vector<const ADMaterialProperty<Real> *> _prop_Fj;

  std::vector<MaterialPropertyName> _hj_names;

  std::vector<const ADMaterialProperty<Real> *> _prop_dhjdetai;
};