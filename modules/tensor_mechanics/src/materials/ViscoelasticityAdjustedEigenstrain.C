/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ViscoelasticityAdjustedEigenstrain.h"

template <>
InputParameters
validParams<ViscoelasticityAdjustedEigenstrain>()
{
  InputParameters params = validParams<ComputeLinearViscoelasticCreepStrain>();

  params.addClassDescription("Adjusts an eigenstrain for internal creep strain");
  params.addRequiredParam<std::string>("source_eigenstrain", "name of the eigenstrain to adjust");
  params.addRequiredParam<std::string>(
      "base_name",
      "Required parameter that distinguishes the current creep strain from the true creep strain");
  return params;
}

ViscoelasticityAdjustedEigenstrain::ViscoelasticityAdjustedEigenstrain(
    const InputParameters & parameters)
  : ComputeLinearViscoelasticCreepStrain(parameters),
    _source_eigenstrain_name(getParam<std::string>("source_eigenstrain")),
    _source_eigenstrain(getMaterialPropertyByName<RankTwoTensor>(_source_eigenstrain_name))
{
  if (_has_driving_eigenstrain)
    mooseError("ViscoelasticityAdjustedEigenstrain cannot use a driving eigenstrain!");
}

void
ViscoelasticityAdjustedEigenstrain::updateQpViscousStrain(unsigned int qp,
                                                          const RankTwoTensor & /*strain*/,
                                                          const RankTwoTensor & /*stress*/)
{
  RankTwoTensor effective_stress = _instantaneous_elasticity_tensor[qp] * _creep_strain[qp];
  _viscoelastic_model->updateQpViscousStrain(qp,
                                             _viscous_strains[qp],
                                             _viscous_strains_old[qp],
                                             _source_eigenstrain[qp],
                                             effective_stress,
                                             false);
}

void
ViscoelasticityAdjustedEigenstrain::computeQpCreepStrain()
{
  _viscoelastic_model->accumulateQpViscousStrain(
      _qp, _creep_strain[_qp], _viscous_strains_old[_qp], false);
  _creep_strain[_qp] =
      (_apparent_elasticity_tensor[_qp] * _instantaneous_elasticity_tensor[_qp].invSymm()) *
      (_source_eigenstrain[_qp] - _creep_strain[_qp]);
}
