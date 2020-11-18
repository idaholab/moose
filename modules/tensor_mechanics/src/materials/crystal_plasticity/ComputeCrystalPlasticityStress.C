//*This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeCrystalPlasticityStress.h"

#include "CrystalPlasticityUpdate.h"

registerMooseObject("TensorMechanicsApp", ComputeCrystalPlasticityStress);

InputParameters
ComputeCrystalPlasticityStress::validParams()
{
  InputParameters params = ComputeFiniteStrainElasticStress::validParams();
  params.addClassDescription("Compute stress using a crystal plasticity consitutive relation");
  params.addRequiredParam<MaterialName>(
      "crystal_plasticity_update_model",
      "The stress update crystal plasticity material objects to use to calculate stress.");

  return params;
}

ComputeCrystalPlasticityStress::ComputeCrystalPlasticityStress(const InputParameters & parameters)
  : ComputeFiniteStrainElasticStress(parameters),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_base_name + "elasticity_tensor"))
{
}

void
ComputeCrystalPlasticityStress::initialSetup()
{
  MaterialName model_name = getParam<MaterialName>("crystal_plasticity_update_model");
  _model =
      dynamic_cast<CrystalPlasticityUpdate *>(&getMaterialByName(model_name));
  if (!_model)
    mooseError("Model " + model_name + " is not compatible with ComputeCrystalPlasticityStress");
}

void
ComputeCrystalPlasticityStress::computeQpStress()
{
  RankTwoTensor stress_new;
  RankFourTensor jacobian_mult;

  _model->setQp(_qp);
  _model->updateStress(stress_new, jacobian_mult);

  _elastic_strain[_qp].zero();
  _stress[_qp] = stress_new;

  // Compute dstress_dstrain
  _Jacobian_mult[_qp] = jacobian_mult; // This is NOT the exact jacobian
}
