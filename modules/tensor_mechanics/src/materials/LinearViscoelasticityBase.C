/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "LinearViscoelasticityBase.h"

template <>
InputParameters
validParams<LinearViscoelasticityBase>()
{
  MooseEnum integration("backward-euler mid-point newmark zienkiewicz", "backward-euler");

  InputParameters params = validParams<Material>();
  params.addParam<std::string>("current_elasticity_tensor",
                               "name of the current instantaneous elasticity tensor");
  params.addParam<std::string>("base_name", "base name of the material");
  params.addParam<MooseEnum>("integration_rule",
                             integration,
                             "describes how the viscoelastic behavior is integrated through time");
  params.addRangeCheckedParam<Real>("theta",
                                    1,
                                    "theta > 0 & theta <= 1",
                                    "coefficient for newmark integration rule (between 0 and 1)");
  return params;
}

LinearViscoelasticityBase::LinearViscoelasticityBase(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _integration_rule(getParam<MooseEnum>("integration_rule")),
    _theta(getParam<Real>("theta")),
    _apparent_elasticity_tensor(
        declareProperty<RankFourTensor>(_base_name + "apparent_elasticity_tensor")),
    _instantaneous_elasticity_tensor(
        declareProperty<RankFourTensor>(_base_name + "instantaneous_elasticity_tensor")),
    _first_elasticity_tensor(
        declareProperty<RankFourTensor>(_base_name + "first_elasticity_tensor")),
    _springs_elasticity_tensors(
        declareProperty<std::vector<RankFourTensor>>(_base_name + "springs_elasticity_tensors")),
    _dashpot_viscosities(declareProperty<std::vector<Real>>(_base_name + "dashpot_viscosities")),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(declareProperty<RankFourTensor>(_elasticity_tensor_name)),
    _has_current_elasticity_tensor(isParamValid("current_elasticity_tensor")),
    _current_elasticity_tensor_name(
        _has_current_elasticity_tensor ? getParam<std::string>("current_elasticity_tensor") : ""),
    _current_elasticity_tensor(
        _has_current_elasticity_tensor
            ? &getMaterialPropertyByName<RankFourTensor>(_current_elasticity_tensor_name)
            : NULL)
{
  if (_theta < 0.5)
    mooseWarning("theta parameter for LinearViscoelasticityBase is below 0.5; time integration may "
                 "not converge!");
}

void
LinearViscoelasticityBase::computeQpProperties()
{
  computeQpViscoelasticProperties();
  computeQpApparentElasticityTensors();

  _elasticity_tensor[_qp] = _apparent_elasticity_tensor[_qp];
  if (_has_current_elasticity_tensor)
    _elasticity_tensor[_qp] = _elasticity_tensor[_qp] * (*_current_elasticity_tensor)[_qp] *
                              (_instantaneous_elasticity_tensor[_qp].invSymm());
}

Real
LinearViscoelasticityBase::computeTheta(Real dt, Real viscosity) const
{
  if (MooseUtils::absoluteFuzzyEqual(dt, 0.0))
    mooseError("linear viscoelasticity cannot be integrated over a dt of ", dt);

  switch (_integration_rule)
  {
    case 0:
      return 1.;
    case 1:
      return 0.5;
    case 2:
      return _theta;
    case 3:
      return 1. / (1. - std::exp(-dt / viscosity)) - viscosity / dt;
    default:
      return 1.;
  }
  return 1.;
}

void
LinearViscoelasticityBase::fillIsotropicElasticityTensor(RankFourTensor & tensor,
                                                         Real young_modulus,
                                                         Real poisson_ratio) const
{
  std::vector<Real> iso_const(2);
  iso_const[0] =
      young_modulus * poisson_ratio / ((1.0 + poisson_ratio) * (1.0 - 2.0 * poisson_ratio));
  iso_const[1] = young_modulus / (2.0 * (1.0 + poisson_ratio));

  tensor.fillFromInputVector(iso_const, RankFourTensor::symmetric_isotropic);
}

unsigned int
LinearViscoelasticityBase::components(unsigned qp) const
{
  return _dashpot_viscosities[qp].size();
}

bool
LinearViscoelasticityBase::hasLongtermDashpot(unsigned int qp) const
{
  return _dashpot_viscosities[qp].size() == (_springs_elasticity_tensors[qp].size() + 1);
}
