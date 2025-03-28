//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatPropBC.h"

registerMooseObject("MooseTestApp", MatPropBC);

InputParameters
MatPropBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<MooseFunctorName>(
      "mat_prop", "The material property that will provide this residual.");
  return params;
}

MatPropBC::MatPropBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _coef(getFunctor<ADReal>("mat_prop"))
{
}

ADReal
MatPropBC::computeQpResidual()
{
  return _test[_i][_qp] *
         _coef(Moose::ElemSideQpArg{_current_elem, _current_side, _qp, _qrule, _q_point[_qp]},
               Moose::currentState());
}
