//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDGCoupledTest.h"

// MOOSE includes
#include "MooseVariableFE.h"

#include "libmesh/utility.h"

registerMooseObject("MooseTestApp", ADDGCoupledTest);

InputParameters
ADDGCoupledTest::validParams()
{
  InputParameters params = ADDGKernel::validParams();
  params.addRequiredCoupledVar("v", "The coupling variable");
  return params;
}

ADDGCoupledTest::ADDGCoupledTest(const InputParameters & parameters)
  : ADDGKernel(parameters),
    _v_var(dynamic_cast<MooseVariable &>(*getVar("v", 0))),
    _v(_v_var.adSln()),
    _v_neighbor(_v_var.adSlnNeighbor())
{
}

ADReal
ADDGCoupledTest::computeQpResidual(Moose::DGResidualType type)
{
  auto fake_flux = 5 * _u[_qp] - 4 * _u_neighbor[_qp] + 3 * _v[_qp] - 2 * _v_neighbor[_qp];

  if (type == Moose::DGResidualType::Element)
    return _test[_i][_qp] * fake_flux;
  else
    return _test_neighbor[_i][_qp] * -fake_flux;
}
