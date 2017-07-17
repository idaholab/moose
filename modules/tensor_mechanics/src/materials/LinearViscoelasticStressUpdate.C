/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "LinearViscoelasticStressUpdate.h"

template <>
InputParameters
validParams<LinearViscoelasticStressUpdate>()
{
  InputParameters params = validParams<StressUpdateBase>();
  params.addRequiredParam<std::string>("viscoelastic_model","name of the LinearViscoelasticityBase object to manage");
  params.addParam<std::string>("base_name","optional string prepended to the creep strain name");
  return params;
}

LinearViscoelasticStressUpdate::LinearViscoelasticStressUpdate(const InputParameters & parameters) : 
    StressUpdateBase(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") : std::string()),
    _creep_strain(declareProperty<RankTwoTensor>(isParamValid("base_name") ? _base_name + "_creep_strain" : "creep_strain")),
    _creep_strain_old(getMaterialPropertyOld<RankTwoTensor>(isParamValid("base_name") ? _base_name + "_creep_strain" : "creep_strain")),
    _viscoelastic_model_name(getParam<std::string>("viscoelastic_model")),
    _viscoelastic_model(nullptr)
{
  std::shared_ptr<Material> test = _mi_feproblem.getMaterial( _viscoelastic_model_name, _material_data_type, _mi_tid, true );

  if (!test)
    mooseError(_viscoelastic_model_name + " does not exist");

  _viscoelastic_model = std::dynamic_pointer_cast<LinearViscoelasticityBase>(test);

  if (!_viscoelastic_model)
      mooseError(_viscoelastic_model_name + " is not a LinearViscoelasticityBase object");
}

void
LinearViscoelasticStressUpdate::initQpStatefulProperties()
{
  _creep_strain[_qp].zero();
}

void
LinearViscoelasticStressUpdate::propagateQpStatefulProperties()
{
  _creep_strain[_qp] = _creep_strain_old[_qp];
}

void 
LinearViscoelasticStressUpdate::updateState(RankTwoTensor & strain_increment,
                                            RankTwoTensor & inelastic_strain_increment,
                                            const RankTwoTensor & /*rotation_increment*/,
                                            RankTwoTensor & stress_new,
                                            const RankTwoTensor & /*stress_old*/,
                                            const RankFourTensor & elasticity_tensor,
                                            const RankTwoTensor & elastic_strain_old,
                                            bool /*compute_full_tangent_operator*/,
                                            RankFourTensor & tangent_operator)
{
  RankTwoTensor current_mechanical_strain = elastic_strain_old + _creep_strain_old[_qp] + strain_increment;
  _creep_strain[_qp] = _viscoelastic_model->computeQpCreepStrain(_qp, current_mechanical_strain);
  RankTwoTensor creep_strain_increment = _creep_strain[_qp] - _creep_strain_old[_qp];

  strain_increment -= creep_strain_increment;
  inelastic_strain_increment += creep_strain_increment;
  stress_new -= elasticity_tensor * creep_strain_increment;

  tangent_operator = elasticity_tensor;
}


