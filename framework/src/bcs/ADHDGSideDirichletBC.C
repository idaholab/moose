//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHDGSideDirichletBC.h"

// MOOSE includes
#include "Function.h"
#include "MooseVariableFE.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/utility.h"

// C++ includes
#include <cmath>

registerMooseObject("MooseApp", ADHDGSideDirichletBC);

InputParameters
ADHDGSideDirichletBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<FunctionName>("exact_soln", "The exact solution.");
  return params;
}

ADHDGSideDirichletBC::ADHDGSideDirichletBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _func(getFunction("exact_soln"))
{
}

ADReal
ADHDGSideDirichletBC::computeQpResidual()
{
  return (_u[_qp] - _func.value(_t, _q_point[_qp])) * _test[_i][_qp];
}
