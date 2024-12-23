//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackFrontNonlocalScalarMaterial.h"

registerMooseObject("SolidMechanicsApp", CrackFrontNonlocalScalarMaterial);

InputParameters
CrackFrontNonlocalScalarMaterial::validParams()
{
  InputParameters params = CrackFrontNonlocalMaterialBase::validParams();
  params.addClassDescription("Computes the average material at points provided by the "
                             "crack_front_definition vectorpostprocessor.");
  return params;
}

CrackFrontNonlocalScalarMaterial::CrackFrontNonlocalScalarMaterial(const InputParameters & parameters)
  : CrackFrontNonlocalMaterialBase(parameters),
    _scalar(getMaterialProperty<Real>(_base_name + getParam<std::string>("material_name")))
{
}

Real
CrackFrontNonlocalScalarMaterial::getQPCrackFrontScalar(const unsigned int qp,
                                                const Point /*crack_face_normal*/) const
{
  return _scalar[qp];
}
