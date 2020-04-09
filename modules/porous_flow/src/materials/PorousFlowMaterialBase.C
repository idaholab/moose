//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowMaterialBase.h"
#include "Conversion.h"

InputParameters
PorousFlowMaterialBase::validParams()
{
  InputParameters params = PorousFlowMaterial::validParams();
  params.addRequiredParam<unsigned int>("phase", "The phase number");
  params.addClassDescription("Base class for PorousFlow materials");
  return params;
}

PorousFlowMaterialBase::PorousFlowMaterialBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<PorousFlowMaterial>(parameters),
    _phase_num(getParam<unsigned int>("phase")),
    _phase(Moose::stringify(_phase_num))
{
  if (_phase_num >= _dictator.numPhases())
    paramError("phase",
               "The Dictator proclaims that the number of fluid phases is ",
               _dictator.numPhases(),
               " while you have foolishly entered ",
               _phase_num,
               ". Remember that indexing starts at 0. Be aware that the Dictator does not tolerate "
               "mistakes.");
}
