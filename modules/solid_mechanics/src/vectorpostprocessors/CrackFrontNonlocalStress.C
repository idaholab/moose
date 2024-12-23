//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackFrontNonlocalStress.h"
#include "RankTwoScalarTools.h"

registerMooseObject("SolidMechanicsApp", CrackFrontNonlocalStress);

InputParameters
CrackFrontNonlocalStress::validParams()
{
  InputParameters params = CrackFrontNonlocalMaterialBase::validParams();
  params.addClassDescription("Computes the average stress normal to the crack face.");
  params.addRequiredParam<MaterialPropertyName>(
      "stress_name", "Get name of stress tensor to compute at crack front");
  return params;
}

CrackFrontNonlocalStress::CrackFrontNonlocalStress(const InputParameters & parameters)
  : CrackFrontNonlocalMaterialBase(parameters, parameters.get<MaterialPropertyName>("stress_name")),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + _property_name))
{
}

Real
CrackFrontNonlocalStress::getQPCrackFrontScalar(const unsigned int qp,
                                                const Point crack_face_normal) const
{
  return RankTwoScalarTools::directionValueTensor(_stress[qp], crack_face_normal);
}
