//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADDummyDisplaceBoundaryIntegratedBC.h"

registerMooseObject("NavierStokesApp", INSADDummyDisplaceBoundaryIntegratedBC);

InputParameters
INSADDummyDisplaceBoundaryIntegratedBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addClassDescription(
      "This object adds Jacobian entries for the boundary displacement dependence on the velocity");
  params.addRequiredParam<MooseFunctorName>("velocity", "The velocity at which to displace");
  params.addRequiredParam<unsigned short>(
      "component", "What component of velocity/displacement this object is acting on.");
  return params;
}

INSADDummyDisplaceBoundaryIntegratedBC::INSADDummyDisplaceBoundaryIntegratedBC(
    const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _velocity(getFunctor<ADRealVectorValue>("velocity")),
    _component(getParam<unsigned short>("component"))
{
}

ADReal
INSADDummyDisplaceBoundaryIntegratedBC::computeQpResidual()
{
  const Moose::ElemSideQpArg elem_side_qp = {
      _current_elem, _current_side, _qp, _qrule, _q_point[_qp]};
  return 0 * _velocity(elem_side_qp, determineState())(_component);
}
