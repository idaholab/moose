/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowViscosityConst.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowViscosityConst>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();

  params.addRequiredParam<Real>("viscosity", "The viscosity, which is assumed constant for this material");
  params.addRequiredParam<unsigned int>("phase", "The phase number");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates the viscosity assuming it is constant");
  return params;
}

PorousFlowViscosityConst::PorousFlowViscosityConst(const InputParameters & parameters) :
    PorousFlowFluidPropertiesBase(parameters),

    _input_viscosity(getParam<Real>("viscosity")),
    _phase_num(getParam<unsigned int>("phase")),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),

    _viscosity(declareProperty<Real>("PorousFlow_viscosity" + Moose::stringify(_phase_num)))
{
  if (_phase_num >= _dictator.numPhases())
    mooseError("PorousFlowViscosityConst: The Dictator proclaims that the number of fluid phases is " << _dictator.numPhases() << " while you have foolishly entered phase = " << _phase_num << ".  Be aware that the Dictator does not tolerate mistakes.");
}

void
PorousFlowViscosityConst::computeQpProperties()
{
  _viscosity[_qp] = _input_viscosity;
}
