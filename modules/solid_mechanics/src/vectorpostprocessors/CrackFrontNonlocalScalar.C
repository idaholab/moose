//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackFrontNonlocalScalar.h"
#include "Moose.h"
#include "RankTwoTensor.h"
#include "libmesh/quadrature.h"
#include "RankTwoScalarTools.h"

registerMooseObject("SolidMechanicsApp", CrackFrontNonlocalScalar);

InputParameters
CrackFrontNonlocalScalar::validParams()
{
  InputParameters params = CrackFrontNonlocalMaterialBase::validParams();
  params.addClassDescription("Computes the average material normal to the crack face.");
  return params;
}

CrackFrontNonlocalScalar::CrackFrontNonlocalScalar(const InputParameters & parameters)
  : CrackFrontNonlocalMaterialBase(parameters),
    _scalar(getMaterialProperty<Real>(_base_name + getParam<std::string>("material_name")))
{
}

Real
CrackFrontNonlocalScalar::getQPCrackFrontScalar(const unsigned int qp, Point /*direction*/) const
{
  return _scalar[qp];
}
