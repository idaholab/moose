//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeElasticityTensorConstantRotationCP.h"
#include "RotationTensor.h"

registerMooseObjectDeprecated("TensorMechanicsApp",
                              ComputeElasticityTensorConstantRotationCP,
                              "11/15/2023 12:00");

InputParameters
ComputeElasticityTensorConstantRotationCP::validParams()
{
  InputParameters params = ComputeElasticityTensor::validParams();
  params.addClassDescription("Deprecated Class: please use ComputeElasticityTensorCP instead. "
                             "Compute an elasticity tensor for crystal plasticity, formulated in "
                             "the reference frame, with constant Euler angles.");
  return params;
}

ComputeElasticityTensorConstantRotationCP::ComputeElasticityTensorConstantRotationCP(
    const InputParameters & parameters)
  : ComputeElasticityTensor(parameters),
    _crysrot(declareProperty<RankTwoTensor>(_base_name + "crysrot")),
    _Euler_angles_material_property(declareProperty<RealVectorValue>("Euler_angles")),
    _R(_Euler_angles),
    _crysrot_constant(_R.transpose())
{
  // the base class performs a passive rotation, but the crystal plasticity
  // materials use active rotation: recover unrotated _Cijkl here
  _Cijkl.rotate(_crysrot_constant);

  // and perform the constant active rotation from the unrotated orientation
  // to find the elasticity tensor rotation for the crystal plasticity model
  _Cijkl.rotate(_crysrot_constant);
}

void
ComputeElasticityTensorConstantRotationCP::initQpStatefulProperties()
{
  _crysrot[_qp] = _crysrot_constant;
  _Euler_angles_material_property[_qp] = _Euler_angles;
}
