//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowMaterialVectorBase.h"

InputParameters
PorousFlowMaterialVectorBase::validParams()
{
  InputParameters params = PorousFlowMaterial::validParams();
  params.addClassDescription("Base class for PorousFlow materials that combine phase-dependent "
                             "properties into vectors expected by the kernels");
  return params;
}

PorousFlowMaterialVectorBase::PorousFlowMaterialVectorBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<PorousFlowMaterial>(parameters),
    _num_phases(_dictator.numPhases()),
    _num_components(_dictator.numComponents()),
    _num_var(_dictator.numVariables())
{
}
