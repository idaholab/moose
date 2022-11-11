//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MDFluidKernelStabilization.h"
#include "libmesh/quadrature.h"

InputParameters
MDFluidKernelStabilization::validParams()
{
  InputParameters params = MDFluidKernelBase::validParams();
  return params;
}

MDFluidKernelStabilization::MDFluidKernelStabilization(const InputParameters & parameters)
  : MDFluidKernelBase(parameters),
    _u_dot(_bTransient ? _var.uDot() : _zero),
    _du_dot_du(_bTransient ? _var.duDotDu() : _zero),
    _tauc(getMaterialProperty<Real>("tauc")),
    _taum(getMaterialProperty<Real>("taum")),
    _taue(getMaterialProperty<Real>("taue"))
{
}

void
MDFluidKernelStabilization::precalculateResidual()
{
  _vel_elem = RealVectorValue(0, 0, 0);
  // calculating element average velocity
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    _vel_elem(0) += _u_vel[_qp];
    _vel_elem(1) += _v_vel[_qp];
    _vel_elem(2) += _w_vel[_qp];
  }
  _vel_elem(0) = _vel_elem(0) / _qp;
  _vel_elem(1) = _vel_elem(1) / _qp;
  _vel_elem(2) = _vel_elem(2) / _qp;
}
