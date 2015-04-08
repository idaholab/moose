/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "InteractionIntegralBenchmarkBC.h"
#include "Function.h"

template<>
InputParameters validParams<InteractionIntegralBenchmarkBC>()
{
  MooseEnum disp_component("x=0 y=1 z=2");
  InputParameters params = validParams<PresetNodalBC>();
  params.addRequiredParam<MooseEnum>("component", disp_component, "The component of the displacement to apply BC on.");
  params.addRequiredParam<UserObjectName>("crack_front_definition","The CrackFrontDefinition user object name");
  params.addParam<unsigned int>("crack_front_node_index",0,"The index of the node on the crack front.");
  params.addParam<Real>("poissons_ratio", "Poisson's ratio for the material.");
  params.addParam<Real>("youngs_modulus", "Young's modulus of the material.");
  params.addParam<Real>("KI",1.0,"Mode I stress intensity factor to apply.");
  params.addParam<Real>("KII",1.0,"Mode II stress intensity factor to apply.");
  params.addParam<Real>("KIII",1.0,"Mode III stress intensity factor to apply.");
  return params;
}

InteractionIntegralBenchmarkBC::InteractionIntegralBenchmarkBC(const std::string & name, InputParameters parameters) :
    PresetNodalBC(name, parameters),
    _component(getParam<MooseEnum>("component")),
    _crack_front_definition(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _crack_front_node_index(getParam<unsigned int>("crack_front_node_index")),
    _poissons_ratio(getParam<Real>("poissons_ratio")),
    _youngs_modulus(getParam<Real>("youngs_modulus")),
    _ki(getParam<Real>("KI")),
    _kii(getParam<Real>("KII")),
    _kiii(getParam<Real>("KIII"))
{
  _kappa = 3 - 4*_poissons_ratio;
  _mu = _youngs_modulus / (2*(1 + _poissons_ratio));
}

Real
InteractionIntegralBenchmarkBC::computeQpValue()
{
  Point p(*_current_node);
  _crack_front_definition->calculateRThetaToCrackFront(p,_crack_front_node_index,_r,_theta);

  if (_r == 0)
    _theta = 0;

  Real st2 = std::sin(_theta/2);
  Real ct2 = std::cos(_theta/2);

  Real disp(0.0);

  if (_component == 0)
    disp = 1/(2*_mu) * std::sqrt(_r/(2*libMesh::pi)) * (_ki * ct2 * (_kappa - 1 + 2*st2*st2) + _kii * st2 * (_kappa + 1 + 2*ct2*ct2));
  else if (_component == 1)
    disp = 1/(2*_mu) * std::sqrt(_r/(2*libMesh::pi)) * (_ki * st2 * (_kappa + 1 - 2*ct2*ct2) - _kii * ct2 * (_kappa - 1 - 2*st2*st2));
  else if (_component == 2)
    disp = 1/_mu * std::sqrt(2*_r/libMesh::pi) * _kiii * st2;

  return disp;
}
