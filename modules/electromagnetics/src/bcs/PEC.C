//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PEC.h"

registerMooseObject("ElectromagneticsApp", PEC);

InputParameters
PEC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription("NodalNormals system description of the Perfect Electrical Conductor "
                             "(PEC) Boundary Condition, where $\\hat{\\mathbf{n}} \\times \\vec{E} = 0$.");
  params.addCoupledVar("coupled_0", 0.0, "Coupled field variable, 0 component.");
  params.addCoupledVar("coupled_1", 0.0, "Coupled field variable, 1 component.");
  params.addCoupledVar("coupled_2", 0.0, "Coupled field variable, 2 component.");
  return params;
}

PEC::PEC(const InputParameters & parameters)
  : IntegratedBC(parameters),

    _coupled_val_0(coupledValue("coupled_0")),
    _coupled_val_1(coupledValue("coupled_1")),
    _coupled_val_2(coupledValue("coupled_2"))
{
}

Real
PEC::computeQpResidual()
{
  RealVectorValue field(_coupled_val_0[_qp], _coupled_val_1[_qp], _coupled_val_2[_qp]);
  RealVectorValue test(_test[_i][_qp], _test[_i][_qp], _test[_i][_qp]);

  RealVectorValue n_cross_f = _normals[_qp].cross(field);
  //RealVectorValue n_cross_t = _normals[_qp].cross(test);

  return -test * n_cross_f;
}
