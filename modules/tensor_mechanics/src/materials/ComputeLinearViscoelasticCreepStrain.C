/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeLinearViscoelasticCreepStrain.h"

template <>
InputParameters
validParams<ComputeLinearViscoelasticCreepStrain>()
{
  InputParameters params = validParams<ComputeCreepStrainBase>();
  params.addClassDescription("Computes visco-elastic creep strain");
  params.addRequiredParam<std::string>("viscoelastic_model",
                                       "name of the viscoelastic model to use");
  params.addParam<std::string>("driving_eigenstrain",
                               "name of the eigenstrain that increases the creep strains");
  params.addParam<Real>("driving_eigenstrain_prefactor",
                        1,
                        "prefactor by which the driving eigenstrain is multiplied");
  return params;
}

ComputeLinearViscoelasticCreepStrain::ComputeLinearViscoelasticCreepStrain(
    const InputParameters & parameters)
  : ComputeCreepStrainBase(parameters),
    _viscoelastic_model_name(getParam<std::string>("viscoelastic_model")),
    _viscoelastic_model(getViscoelasticModel(_viscoelastic_model_name)),
    _viscous_strains(declareProperty<std::vector<RankTwoTensor>>(_base_name + "viscous_strains")),
    _viscous_strains_old(
        declarePropertyOld<std::vector<RankTwoTensor>>(_base_name + "viscous_strains")),
    _apparent_elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(
        _viscoelastic_model->getApparentElasticityTensorName())),
    _instantaneous_elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(
        _viscoelastic_model->getInstantaneousElasticityTensorName())),
    _has_driving_eigenstrain(isParamValid("driving_eigenstrain")),
    _driving_eigenstrain_prefactor(getParam<Real>("driving_eigenstrain_prefactor")),
    _driving_eigenstrain_name(
        _has_driving_eigenstrain ? getParam<std::string>("driving_eigenstrain") : ""),
    _driving_eigenstrain(_has_driving_eigenstrain
                             ? &getMaterialPropertyByName<RankTwoTensor>(_driving_eigenstrain_name)
                             : NULL)
{
}

void
ComputeLinearViscoelasticCreepStrain::updateQpViscousStrain(unsigned int qp,
                                                            const RankTwoTensor & strain,
                                                            const RankTwoTensor & stress)
{
  if (_has_driving_eigenstrain)
    _viscoelastic_model->updateQpViscousStrain(qp,
                                               _viscous_strains[qp],
                                               _viscous_strains_old[qp],
                                               strain,
                                               stress,
                                               true,
                                               (*_driving_eigenstrain)[qp] *
                                                   _driving_eigenstrain_prefactor);
  else
    _viscoelastic_model->updateQpViscousStrain(
        qp, _viscous_strains[qp], _viscous_strains_old[qp], strain, stress, false);
}

void
ComputeLinearViscoelasticCreepStrain::initQpStatefulProperties()
{
  ComputeCreepStrainBase::initQpStatefulProperties();

  unsigned int components = _viscoelastic_model->components(_qp);

  _viscous_strains[_qp].resize(components, RankTwoTensor());
  _viscous_strains_old[_qp].resize(components, RankTwoTensor());
}

void
ComputeLinearViscoelasticCreepStrain::computeQpCreepStrain()
{
  if (_has_driving_eigenstrain)
    _viscoelastic_model->accumulateQpViscousStrain(_qp,
                                                   _creep_strain[_qp],
                                                   _viscous_strains_old[_qp],
                                                   true,
                                                   (*_driving_eigenstrain)[_qp] *
                                                       _driving_eigenstrain_prefactor);
  else
    _viscoelastic_model->accumulateQpViscousStrain(
        _qp, _creep_strain[_qp], _viscous_strains_old[_qp], false);
}

MooseSharedPointer<LinearViscoelasticityBase>
ComputeLinearViscoelasticCreepStrain::getViscoelasticModel(
    const std::string & viscoelastic_model_name) const
{
  MooseSharedPointer<Material> test =
      _mi_feproblem.getMaterial(viscoelastic_model_name, _material_data_type, _mi_tid, true);

  if (!test)
    mooseError(viscoelastic_model_name + " does not exist");

  MooseSharedPointer<LinearViscoelasticityBase> viscoelastic_model =
      MooseSharedNamespace::dynamic_pointer_cast<LinearViscoelasticityBase>(test);

  if (!viscoelastic_model)
    mooseError(viscoelastic_model_name + " is not a LinearViscoelasticityBase object");

  return viscoelastic_model;
}
