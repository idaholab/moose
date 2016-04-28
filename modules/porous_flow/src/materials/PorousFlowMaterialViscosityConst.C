/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMaterialViscosityConst.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowMaterialViscosityConst>()
{
  InputParameters params = validParams<PorousFlowMaterialFluidPropertiesBase>();

  params.addRequiredParam<Real>("viscosity", "The viscosity, which is assumed constant for this material");
  params.addRequiredParam<unsigned int>("phase", "The phase number");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates the viscosity assuming it is constant");
  return params;
}

PorousFlowMaterialViscosityConst::PorousFlowMaterialViscosityConst(const InputParameters & parameters) :
    PorousFlowMaterialFluidPropertiesBase(parameters),

    _input_viscosity(getParam<Real>("viscosity")),
    _phase_num(getParam<unsigned int>("phase")),
    _dictator_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),

    _viscosity(declareProperty<Real>("PorousFlow_viscosity" + Moose::stringify(_phase_num)))
{
  if (_phase_num >= _dictator_UO.numPhases())
    mooseError("PorousFlowMaterialViscosityConst: The Dictator proclaims that the number of fluid phases is " << _dictator_UO.numPhases() << " while you have foolishly entered phase = " << _phase_num << ".  Be aware that the Dictator does not tolerate mistakes.");
}

void
PorousFlowMaterialViscosityConst::computeQpProperties()
{
  _viscosity[_qp] = _input_viscosity;
}
