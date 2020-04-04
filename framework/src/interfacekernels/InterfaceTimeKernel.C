//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceTimeKernel.h"

InputParameters
InterfaceTimeKernel::validParams()
{
  InputParameters params = InterfaceKernel::validParams();

  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";

  return params;
}

InterfaceTimeKernel::InterfaceTimeKernel(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _u_dot(_is_transient ? _var.uDot() : _zero),
    _du_dot_du(_is_transient ? _var.duDotDu() : _zero),
    _neighbor_value_dot(_is_transient ? coupledNeighborValueDot("neighbor_var") : _zero),
    _dneighbor_value_dot_du(_is_transient ? coupledNeighborValueDotDu("neighbor_var") : _zero)

{
  if (!_is_transient)
    mooseError("In order to use an interface time kernel the executioner must be transient");
}

Real
InterfaceTimeKernel::computeQpOffDiagJacobian(Moose::DGJacobianType /*type*/, unsigned int /*jvar*/)
{
  return 0.;
}
