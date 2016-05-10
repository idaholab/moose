/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowRelativePermeabilityUnity.h"
#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowRelativePermeabilityUnity>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<unsigned int>("phase", "The phase number for which to calculate the relative permeability for");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of PorousFlow variable names");
  params.addClassDescription("Base class for PorousFlow relative permeability materials.  This class sets the relative permeability = 1");
  return params;
}

PorousFlowRelativePermeabilityUnity::PorousFlowRelativePermeabilityUnity(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _phase_num(getParam<unsigned int>("phase")),
    _dictator_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
    _saturation_variable_name(_dictator_UO.saturationVariableNameDummy()),
    _saturation_nodal(getMaterialProperty<std::vector<Real> >("PorousFlow_saturation_nodal")),
    _relative_permeability(declareProperty<Real>("PorousFlow_relative_permeability" + Moose::stringify(_phase_num))),
    _drelative_permeability_ds(declarePropertyDerivative<Real>("PorousFlow_relative_permeability" + Moose::stringify(_phase_num), _saturation_variable_name))
{
  if (_phase_num >= _dictator_UO.numPhases())
    mooseError("PorousFlowRelativePermeability: The Dictator proclaims that the number of fluid phases is " << _dictator_UO.numPhases() << " while you have foolishly entered phase = " << _phase_num << ".  Be aware that the Dictator does not tolerate mistakes.");
}

void
PorousFlowRelativePermeabilityUnity::computeQpProperties()
{
  _relative_permeability[_qp] = 1.0;
  _drelative_permeability_ds[_qp] = 0.0;
}
