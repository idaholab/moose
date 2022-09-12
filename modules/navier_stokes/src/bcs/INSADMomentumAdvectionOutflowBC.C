//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumAdvectionOutflowBC.h"
#include "MooseMesh.h"
#include "INSADObjectTracker.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSADMomentumAdvectionOutflowBC);

InputParameters
INSADMomentumAdvectionOutflowBC::validParams()
{
  InputParameters params = ADVectorIntegratedBC::validParams();
  params.addParam<MaterialPropertyName>(NS::density, NS::density, "The density");
  params.addCoupledVar(NS::porosity, 1, "The porosity");
  return params;
}

INSADMomentumAdvectionOutflowBC::INSADMomentumAdvectionOutflowBC(const InputParameters & parameters)
  : ADVectorIntegratedBC(parameters),
    _rho(getADMaterialProperty<Real>(NS::density)),
    _eps(coupledValue(NS::porosity))
{
}

ADReal
INSADMomentumAdvectionOutflowBC::computeQpResidual()
{
  return _test[_i][_qp] * (_rho[_qp] / _eps[_qp]) *
         (libMesh::outer_product(_u[_qp], _u[_qp]) * _normals[_qp]);
}
