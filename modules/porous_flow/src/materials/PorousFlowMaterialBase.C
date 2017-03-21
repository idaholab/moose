/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMaterialBase.h"
#include "Conversion.h"

template <>
InputParameters
validParams<PorousFlowMaterialBase>()
{
  InputParameters params = validParams<PorousFlowMaterial>();
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
    mooseError("PorousFlowMaterial: The Dictator proclaims that the number of fluid phases is ",
               _dictator.numPhases(),
               " while you have foolishly entered phase = ",
               _phase_num,
               " in ",
               _name,
               ".  Be aware that the Dictator does not tolerate mistakes.");
}
