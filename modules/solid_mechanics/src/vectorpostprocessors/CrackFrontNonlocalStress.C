//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackFrontNonlocalStress.h"
#include "Moose.h"
#include "RankTwoTensor.h"
#include "libmesh/quadrature.h"
#include "RankTwoScalarTools.h"

registerMooseObject("SolidMechanicsApp", CrackFrontNonlocalStress);

InputParameters
CrackFrontNonlocalStress::validParams()
{
  InputParameters params = CrackFrontNonlocalMaterialBase::validParams();
  params.addClassDescription("Computes the average stress normal to the crack face.");
  return params;
}

CrackFrontNonlocalStress::CrackFrontNonlocalStress(const InputParameters & parameters)
  : CrackFrontNonlocalMaterialBase(parameters),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress"))
{
}

Real
CrackFrontNonlocalStress::getCrackFrontScalar(const unsigned int qp, Point direction) const
{
  return RankTwoScalarTools::directionValueTensor(_stress[qp], direction);
}
