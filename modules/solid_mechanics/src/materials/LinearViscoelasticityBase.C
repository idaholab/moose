//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearViscoelasticityBase.h"
#include "Conversion.h"

InputParameters
LinearViscoelasticityBase::validParams()
{
  MooseEnum integration("backward-euler mid-point newmark zienkiewicz", "backward-euler");

  InputParameters params = ComputeElasticityTensorBase::validParams();
  params.addParam<MooseEnum>("integration_rule",
                             integration,
                             "describes how the viscoelastic behavior is integrated through time");
  params.addRangeCheckedParam<Real>("theta",
                                    1,
                                    "theta > 0 & theta <= 1",
                                    "coefficient for Newmark integration rule (between 0 and 1)");
  params.addParam<std::string>("driving_eigenstrain",
                               "name of the eigenstrain that increases the creep strains");
  params.addParam<std::string>(
      "elastic_strain_name", "elastic_strain", "name of the true elastic strain of the material");
  params.addParam<std::string>("creep_strain_name",
                               "creep_strain",
                               "name of the true creep strain of the material"
                               "(computed by LinearViscoelasticStressUpdate or"
                               "ComputeLinearViscoelasticStress)");
  params.addParam<bool>("force_recompute_properties",
                        false,
                        "forces the computation of the viscoelastic properties at each step of"
                        "the solver (default: false)");
  params.addParam<bool>(
      "need_viscoelastic_properties_inverse",
      false,
      "checks whether the model requires the computation of the inverse viscoelastic"
      "properties (default: false)");
  params.suppressParameter<FunctionName>("elasticity_tensor_prefactor");
  return params;
}

LinearViscoelasticityBase::LinearViscoelasticityBase(const InputParameters & parameters)
  : ComputeElasticityTensorBase(parameters),
    _integration_rule(getParam<MooseEnum>("integration_rule").getEnum<IntegrationRule>()),
    _theta(getParam<Real>("theta")),
    _apparent_elasticity_tensor(
        declareProperty<RankFourTensor>(_base_name + "apparent_elasticity_tensor")),
    _apparent_elasticity_tensor_inv(
        declareProperty<RankFourTensor>(_base_name + "apparent_elasticity_tensor_inv")),
    _elasticity_tensor_inv(declareProperty<RankFourTensor>(_elasticity_tensor_name + "_inv")),
    _need_viscoelastic_properties_inverse(getParam<bool>("need_viscoelastic_properties_inverse")),
    _has_longterm_dashpot(false),
    _components(0),
    _first_elasticity_tensor(
        declareProperty<RankFourTensor>(_base_name + "spring_elasticity_tensor_0")),
    _first_elasticity_tensor_inv(
        _need_viscoelastic_properties_inverse
            ? &declareProperty<RankFourTensor>(_base_name + "spring_elasticity_tensor_0_inv")
            : nullptr),
    _apparent_creep_strain(declareProperty<RankTwoTensor>(_base_name + "apparent_creep_strain")),
    _apparent_creep_strain_old(
        getMaterialPropertyOld<RankTwoTensor>(_base_name + "apparent_creep_strain")),
    _elastic_strain_old(
        getMaterialPropertyOld<RankTwoTensor>(getParam<std::string>("elastic_strain_name"))),
    _creep_strain_old(
        getMaterialPropertyOld<RankTwoTensor>(getParam<std::string>("creep_strain_name"))),
    _has_driving_eigenstrain(isParamValid("driving_eigenstrain")),
    _driving_eigenstrain_name(
        _has_driving_eigenstrain ? getParam<std::string>("driving_eigenstrain") : ""),
    _driving_eigenstrain(_has_driving_eigenstrain
                             ? &getMaterialPropertyByName<RankTwoTensor>(_driving_eigenstrain_name)
                             : nullptr),
    _driving_eigenstrain_old(_has_driving_eigenstrain
                                 ? &getMaterialPropertyOld<RankTwoTensor>(_driving_eigenstrain_name)
                                 : nullptr),
    _force_recompute_properties(getParam<bool>("force_recompute_properties")),
    _step_zero(declareRestartableData<bool>("step_zero", true))
{
  if (_theta < 0.5)
    mooseWarning("theta parameter for LinearViscoelasticityBase is below 0.5; time integration may "
                 "not converge!");

  // force material properties to be considered stateful
  getMaterialPropertyOld<RankFourTensor>(_base_name + "apparent_elasticity_tensor");
  getMaterialPropertyOld<RankFourTensor>(_base_name + "apparent_elasticity_tensor_inv");
  getMaterialPropertyOld<RankFourTensor>(_elasticity_tensor_name);
  getMaterialPropertyOld<RankFourTensor>(_elasticity_tensor_name + "_inv");
  getMaterialPropertyOld<RankFourTensor>(_base_name + "spring_elasticity_tensor_0");
  if (_need_viscoelastic_properties_inverse)
    getMaterialPropertyOld<RankFourTensor>(_base_name + "spring_elasticity_tensor_0_inv");
}

void
LinearViscoelasticityBase::declareViscoelasticProperties()
{
  for (unsigned int i = 0; i < _components; ++i)
  {
    std::string ith = Moose::stringify(i + 1);

    if (!_has_longterm_dashpot || (_components > 0 && i < _components - 1))
    {
      _springs_elasticity_tensors.push_back(
          &declareProperty<RankFourTensor>(_base_name + "spring_elasticity_tensor_" + ith));
      getMaterialPropertyOld<RankFourTensor>(_base_name + "spring_elasticity_tensor_" + ith);
    }

    _dashpot_viscosities.push_back(&declareProperty<Real>(_base_name + "dashpot_viscosity_" + ith));
    _dashpot_viscosities_old.push_back(
        &getMaterialPropertyOld<Real>(_base_name + "dashpot_viscosity_" + ith));

    _viscous_strains.push_back(
        &declareProperty<RankTwoTensor>(_base_name + "viscous_strain_" + ith));
    _viscous_strains_old.push_back(
        &getMaterialPropertyOld<RankTwoTensor>(_base_name + "viscous_strain_" + ith));

    if (_need_viscoelastic_properties_inverse)
    {
      _springs_elasticity_tensors_inv.push_back(&declareProperty<RankFourTensor>(
          _base_name + "spring_elasticity_tensor_" + ith + "_inv"));
      _springs_elasticity_tensors_inv_old.push_back(&getMaterialPropertyOld<RankFourTensor>(
          _base_name + "spring_elasticity_tensor_" + ith + "_inv"));
    }
  }
}

void
LinearViscoelasticityBase::initQpStatefulProperties()
{
  if (_components != _viscous_strains.size())
    mooseError(
        "inconsistent numbers of dashpots and viscous strains in LinearViscoelasticityBase;"
        " Make sure declareViscoelasticProperties has been called in the viscoelastic model");

  _apparent_creep_strain[_qp].zero();
  _apparent_elasticity_tensor[_qp].zero();
  _apparent_elasticity_tensor_inv[_qp].zero();
  _elasticity_tensor_inv[_qp].zero();
  _first_elasticity_tensor[_qp].zero();
  if (_need_viscoelastic_properties_inverse)
    (*_first_elasticity_tensor_inv)[_qp].zero();

  for (unsigned int i = 0; i < _components; ++i)
  {
    if (!_has_longterm_dashpot || (_components > 0 && i < _components - 1))
    {
      (*_springs_elasticity_tensors[i])[_qp].zero();
      if (_need_viscoelastic_properties_inverse)
        (*_springs_elasticity_tensors_inv[i])[_qp].zero();
    }

    (*_dashpot_viscosities[i])[_qp] = 0.0;
    (*_viscous_strains[i])[_qp].zero();
  }
}

void
LinearViscoelasticityBase::recomputeQpApparentProperties(unsigned int qp)
{
  unsigned int qp_prev = _qp;
  _qp = qp;

  if (_t_step >= 1)
    _step_zero = false;

  // 1. we get the viscoelastic properties and their inverse if needed
  computeQpViscoelasticProperties();
  if (_need_viscoelastic_properties_inverse)
    computeQpViscoelasticPropertiesInv();

  // 2. we update the internal viscous strains from the previous time step
  updateQpViscousStrains();

  // 3. we compute the apparent elasticity tensor
  computeQpApparentElasticityTensors();

  // 4. we transform the internal viscous strains in an apparent creep strain
  if (!_step_zero)
    computeQpApparentCreepStrain();

  _qp = qp_prev;
}

void
LinearViscoelasticityBase::computeQpElasticityTensor()
{
  if (_force_recompute_properties)
    recomputeQpApparentProperties(_qp);
}

void
LinearViscoelasticityBase::computeQpViscoelasticPropertiesInv()
{
  if (MooseUtils::absoluteFuzzyEqual(_first_elasticity_tensor[_qp].L2norm(), 0.0))
    (*_first_elasticity_tensor_inv)[_qp].zero();
  else
    (*_first_elasticity_tensor_inv)[_qp] = _first_elasticity_tensor[_qp].invSymm();

  for (unsigned int i = 0; i < _springs_elasticity_tensors.size(); ++i)
  {
    if (MooseUtils::absoluteFuzzyEqual((*_springs_elasticity_tensors[i])[_qp].L2norm(), 0.0))
      (*_springs_elasticity_tensors_inv[i])[_qp].zero();
    else
      (*_springs_elasticity_tensors_inv[i])[_qp] = (*_springs_elasticity_tensors[i])[_qp].invSymm();
  }
}

Real
LinearViscoelasticityBase::computeTheta(Real dt, Real viscosity) const
{
  if (MooseUtils::absoluteFuzzyEqual(dt, 0.0))
    mooseError("linear viscoelasticity cannot be integrated over a dt of ", dt);

  switch (_integration_rule)
  {
    case IntegrationRule::BackwardEuler:
      return 1.;
    case IntegrationRule::MidPoint:
      return 0.5;
    case IntegrationRule::Newmark:
      return _theta;
    case IntegrationRule::Zienkiewicz:
      return 1. / (1. - std::exp(-dt / viscosity)) - viscosity / dt;
    default:
      return 1.;
  }
  return 1.;
}
