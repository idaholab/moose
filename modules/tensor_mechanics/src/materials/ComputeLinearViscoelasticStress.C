/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeLinearViscoelasticStress.h"

template <>
InputParameters
validParams<ComputeLinearViscoelasticStress>()
{
  InputParameters params = validParams<ComputeLinearElasticStress>();
  params.addClassDescription("Divides total strain into elastic + creep + eigenstrains");
  params.addParam<std::string>(
      "viscoelastic_model",
      "name of the viscoelastic model provinding the creep strain");
  return params;
}

ComputeLinearViscoelasticStress::ComputeLinearViscoelasticStress(const InputParameters & parameters)
  : ComputeLinearElasticStress(parameters),
    _creep_strain(declareProperty<RankTwoTensor>(isParamValid("base_name") ? _base_name + "_creep_strain" : "creep_strain")),
    _creep_strain_old(getMaterialPropertyOld<RankTwoTensor>(isParamValid("base_name") ? _base_name + "_creep_strain" : "creep_strain")),
    _viscoelastic_model_name(getParam<std::string>("viscoelastic_model")),
    _viscoelastic_model(nullptr)
{
}

void
ComputeLinearViscoelasticStress::initQpStatefulProperties()
{
  _creep_strain[_qp].zero();
}

void
ComputeLinearViscoelasticStress::initialSetup()
{
  std::shared_ptr<Material> test = _mi_feproblem.getMaterial( _viscoelastic_model_name, _material_data_type, _mi_tid, true );

  if (!test)
    mooseError(_viscoelastic_model_name + " does not exist");

  _viscoelastic_model = std::dynamic_pointer_cast<LinearViscoelasticityBase>(test);

  if (!_viscoelastic_model)
      mooseError(_viscoelastic_model_name + " is not a LinearViscoelasticityBase object");
}

void
ComputeLinearViscoelasticStress::computeQpStress()
{
  _creep_strain[_qp] = _viscoelastic_model->computeQpCreepStrain(_qp, _mechanical_strain[_qp]);

  _elastic_strain[_qp] = _mechanical_strain[_qp] - _creep_strain[_qp];

  _stress[_qp] = _elasticity_tensor[_qp] * _elastic_strain[_qp];

  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}
