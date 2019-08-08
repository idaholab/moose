//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ViscoPlasticityStressUpdate.h"

#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", ViscoPlasticityStressUpdate);

template <>
InputParameters
validParams<ViscoPlasticityStressUpdate>()
{
  InputParameters params = validParams<StressUpdateBase>();
  params += validParams<SingleVariableReturnMappingSolution>();
  params.addClassDescription(
      "This material computes the non-linear homogenized gauge stress in order to compute the "
      "viscoplastic responce due to creep in porous materials. This material must be used in "
      "conjunction with ComputeMultiplePorousInelasticStress");
  params.addParam<MooseEnum>("viscoplasticity_model",
                             ViscoPlasticityStressUpdate::getModelEnum(),
                             "Which viscoplastic model to use");
  params.addParam<MooseEnum>("pore_shape_model",
                             ViscoPlasticityStressUpdate::getPoreShapeEnum(),
                             "Which pore shape model to use");
  params.addParam<Real>("max_inelastic_increment",
                        1.0e-4,
                        "The maximum inelastic strain increment allowed in a time step");
  params.addParam<std::string>(
      "effective_inelastic_strain_name",
      "effective_viscoplasticity",
      "Name of the material property that stores the effective inelastic strain");
  params.addRequiredParam<MaterialPropertyName>(
      "coefficient", "Material property name for the leading coefficient for Norton power law");
  params.addRequiredRangeCheckedParam<Real>(
      "power", "power>=1.0", "Stress exponent for Norton power law");
  params.addParam<bool>("verbose", false, "Flag to output verbose information");
  params.addParam<MaterialPropertyName>(
      "porosity_name", "porosity", "Name of porosity material property");
  params.addParam<std::string>("total_strain_base_name", "Base name for the total strain");

  params.addParamNamesToGroup("effective_inelastic_strain_name", "Advanced");
  return params;
}

ViscoPlasticityStressUpdate::ViscoPlasticityStressUpdate(const InputParameters & parameters)
  : StressUpdateBase(parameters),
    SingleVariableReturnMappingSolution(parameters),
    _model(getParam<MooseEnum>("viscoplasticity_model")),
    _pore_shape(getParam<MooseEnum>("pore_shape_model")),
    _pore_shape_factor(_pore_shape == "spherical" ? 1.5 : std::sqrt(3.0)),
    _total_strain_base_name(isParamValid("total_strain_base_name")
                                ? getParam<std::string>("total_strain_base_name") + "_"
                                : ""),
    _strain_increment(
        getMaterialProperty<RankTwoTensor>(_total_strain_base_name + "strain_increment")),
    _effective_inelastic_strain(declareProperty<Real>(
        _base_name + getParam<std::string>("effective_inelastic_strain_name"))),
    _effective_inelastic_strain_old(getMaterialPropertyOld<Real>(
        _base_name + getParam<std::string>("effective_inelastic_strain_name"))),
    _creep_strain(declareProperty<RankTwoTensor>(_base_name + "creep_strain")),
    _creep_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "creep_strain")),
    _max_inelastic_increment(getParam<Real>("max_inelastic_increment")),
    _power(getParam<Real>("power")),
    _power_factor((_power - 1.0) / (_power + 1.0)),
    _coefficient(getMaterialProperty<Real>("coefficient")),
    _gauge_stress(declareProperty<Real>(_base_name + "gauge_stress")),
    _intermediate_porosity(0.0),
    _porosity_old(getMaterialPropertyOld<Real>(getParam<MaterialPropertyName>("porosity_name"))),
    _verbose(getParam<bool>("verbose")),
    _hydro_stress(0.0),
    _identity_two(RankTwoTensor::initIdentity),
    _dhydro_stress_dsigma(_identity_two / 3.0),
    _derivative(0.0)
{
}

MooseEnum
ViscoPlasticityStressUpdate::getModelEnum()
{
  return MooseEnum("LPS GTN", "LPS");
}

MooseEnum
ViscoPlasticityStressUpdate::getPoreShapeEnum()
{
  return MooseEnum("spherical cylindrical", "spherical");
}

void
ViscoPlasticityStressUpdate::initQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = 0.0;
  _creep_strain[_qp].zero();
}

void
ViscoPlasticityStressUpdate::propagateQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
  _creep_strain[_qp] = _creep_strain_old[_qp];
}

void
ViscoPlasticityStressUpdate::updateState(RankTwoTensor & elastic_strain_increment,
                                         RankTwoTensor & inelastic_strain_increment,
                                         const RankTwoTensor & /*rotation_increment*/,
                                         RankTwoTensor & stress,
                                         const RankTwoTensor & /*stress_old*/,
                                         const RankFourTensor & elasticity_tensor,
                                         const RankTwoTensor & elastic_strain_old,
                                         bool /*compute_full_tangent_operator*/,
                                         RankFourTensor & /*tangent_operator*/)
{
  // Compute initial hydrostatic stress and porosity
  if (_pore_shape == "cylindrical")
    _hydro_stress = (stress(0, 0) + stress(1, 1)) / 2.0;
  else
    _hydro_stress = stress.trace() / 3.0;

  // Subtract elastic strain from strain increment to find all inelastic strain increments
  // calculated so far except the one that we're about to calculate
  const RankTwoTensor inelastic_volumetric_increment =
      _strain_increment[_qp] - elastic_strain_increment;
  // Calculate intermdiate porosity from all inelastic strain increments calculated so far except
  // the one that we're about to calculate
  _intermediate_porosity =
      (1.0 - _porosity_old[_qp]) * inelastic_volumetric_increment.trace() + _porosity_old[_qp];

  // Compute intermediate equivalent stress
  const RankTwoTensor dev_stress = stress.deviatoric();
  const Real dev_stress_squared = dev_stress.doubleContraction(dev_stress);
  const Real equiv_stress = MooseUtils::absoluteFuzzyEqual(dev_stress_squared, 0.0)
                                ? 0.0
                                : std::sqrt(1.5 * dev_stress_squared);

  computeStressInitialize(equiv_stress, elasticity_tensor);

  // Prepare values
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
  _creep_strain[_qp] = _creep_strain_old[_qp];
  inelastic_strain_increment.zero();

  // If equivalent stress or hydrostatic stress is present, calculate creep strain increment
  if (equiv_stress || _hydro_stress)
  {
    // Initalize stress potential
    Real dpsi_dgauge(0);
    computeInelasticStrainIncrement(_gauge_stress[_qp],
                                    dpsi_dgauge,
                                    inelastic_strain_increment,
                                    equiv_stress,
                                    dev_stress,
                                    stress);

    // Update elastic strain increment due to inelastic strain calculated here
    elastic_strain_increment -= inelastic_strain_increment;
    // Update stress due to new strain
    stress = elasticity_tensor * (elastic_strain_old + elastic_strain_increment);
    // Compute effective strain from the stress potential
    _effective_inelastic_strain[_qp] += dpsi_dgauge * _dt;
    // Update creep strain due to currently computed inelastic strain
    _creep_strain[_qp] += inelastic_strain_increment;
  }

  computeStressFinalize(inelastic_strain_increment);
}

Real
ViscoPlasticityStressUpdate::computeReferenceResidual(const Real /*effective_trial_stress*/,
                                                      const Real gauge_stress)
{
  // Use gauge stress for relative tolerance criteria, defined as:
  // std::abs(residual / gauge_stress) <= _relative_tolerance
  return gauge_stress;
}

Real
ViscoPlasticityStressUpdate::initialGuess(const Real effective_trial_stress)
{
  // Use equilvalent stress as an initial guess for the gauge stress
  return effective_trial_stress;
}

Real
ViscoPlasticityStressUpdate::maximumPermissibleValue(const Real effective_trial_stress) const
{
  return effective_trial_stress * 1.0e6;
}

Real
ViscoPlasticityStressUpdate::minimumPermissibleValue(const Real effective_trial_stress) const
{
  return effective_trial_stress;
}

Real
ViscoPlasticityStressUpdate::computeTimeStepLimit()
{
  const Real scalar_inelastic_strain_incr =
      _effective_inelastic_strain[_qp] - _effective_inelastic_strain_old[_qp];

  if (MooseUtils::absoluteFuzzyEqual(scalar_inelastic_strain_incr, 0.0))
    return std::numeric_limits<Real>::max();

  return _dt * _max_inelastic_increment / scalar_inelastic_strain_incr;
}

void
ViscoPlasticityStressUpdate::outputIterationSummary(std::stringstream * iter_output,
                                                    const unsigned int total_it)
{
  if (iter_output)
  {
    *iter_output << "At element " << _current_elem->id() << " _qp=" << _qp << " Coordinates "
                 << _q_point[_qp] << " block=" << _current_elem->subdomain_id() << '\n';
  }
  SingleVariableReturnMappingSolution::outputIterationSummary(iter_output, total_it);
}

Real
ViscoPlasticityStressUpdate::computeResidual(const Real equiv_stress, const Real trial_gauge)
{
  const Real M = std::abs(_hydro_stress) / trial_gauge;
  const Real dM_dtrial_gauge = -M / trial_gauge;

  const Real residual_left = Utility::pow<2>(equiv_stress / trial_gauge);
  const Real dresidual_left_dtrial_gauge = -2.0 * residual_left / trial_gauge;

  Real residual = residual_left;
  _derivative = dresidual_left_dtrial_gauge;

  if (_model == "GTN")
  {
    residual += 2.0 * _intermediate_porosity * std::cosh(_pore_shape_factor * M) - 1.0 -
                Utility::pow<2>(_intermediate_porosity);
    _derivative += 2.0 * _intermediate_porosity * std::sinh(_pore_shape_factor * M) *
                   _pore_shape_factor * dM_dtrial_gauge;
  }
  else
  {
    const Real h = computeH(_power, M);
    const Real dh_dM = computeH(_power, M, true);

    residual += _intermediate_porosity * (h + _power_factor / h) - 1.0 -
                _power_factor * Utility::pow<2>(_intermediate_porosity);
    const Real dresidual_dh = _intermediate_porosity * (1.0 - _power_factor / Utility::pow<2>(h));
    _derivative += dresidual_dh * dh_dM * dM_dtrial_gauge;
  }

  // Compute the derivative of the residual with respect to the trial gauge

  if (_verbose)
  {
    Moose::out << "in computeResidual:\n"
               << "  position: " << _q_point[_qp] << " hydro_stress: " << _hydro_stress
               << " equiv_stress: " << equiv_stress << " trial_grage: " << trial_gauge
               << " M: " << M << std::endl;
    Moose::out << "  residual: " << residual << "  derivative: " << _derivative << std::endl;
  }

  return residual;
}

Real
ViscoPlasticityStressUpdate::computeH(const Real n, const Real & M, const bool derivative)
{
  const Real mod = std::pow(M * _pore_shape_factor, (n + 1.0) / n);

  // Calculate derivative with respect to M
  if (derivative)
  {
    const Real dmod_dM = (n + 1.0) / n * mod / M;
    return dmod_dM * std::pow(1.0 + mod / n, n - 1.0);
  }

  return std::pow(1.0 + mod / n, n);
}

RankTwoTensor
ViscoPlasticityStressUpdate::computeDGaugeDSigma(const Real & gauge_stress,
                                                 const Real & equiv_stress,
                                                 const RankTwoTensor & dev_stress,
                                                 const RankTwoTensor & stress)
{
  // Compute the derivative of the gauge stress with respect to the equilvalent and hydrostatic
  // stress components
  const Real M = std::abs(_hydro_stress) / gauge_stress;
  const Real h = computeH(_power, M);

  // Compute the derviative of the residual with respect to the hydrostatic stress
  Real dresidual_dhydro_stress = 0.0;
  if (_hydro_stress)
  {
    const Real dM_dhydro_stress = M / _hydro_stress;
    if (_model == "GTN")
    {
      dresidual_dhydro_stress = 2.0 * _intermediate_porosity * std::sinh(_pore_shape_factor * M) *
                                _pore_shape_factor * dM_dhydro_stress;
    }
    else
    {
      const Real dresidual_dh = _intermediate_porosity * (1.0 - _power_factor / Utility::pow<2>(h));
      const Real dh_dM = computeH(_power, M, true);
      dresidual_dhydro_stress = dresidual_dh * dh_dM * dM_dhydro_stress;
    }
  }

  // Compute the derivative of the residual with respect to the equilvalent stress
  Real dresidual_dequiv_stress = 0.0;
  if (equiv_stress)
    dresidual_dequiv_stress = 2.0 * equiv_stress / Utility::pow<2>(gauge_stress);

  // Compute the derivative of the equilvalent stress to the deviatoric stress
  RankTwoTensor dequiv_stress_dsigma;
  if (equiv_stress)
    dequiv_stress_dsigma = 1.5 * dev_stress / equiv_stress;

  // Compute the derivative of the residual with the stress
  const RankTwoTensor dresidual_dsigma = dresidual_dhydro_stress * _dhydro_stress_dsigma +
                                         dresidual_dequiv_stress * dequiv_stress_dsigma;

  // Compute the deritative of the gauge stress with respect to the stress
  const RankTwoTensor dgauge_dsigma =
      gauge_stress * dresidual_dsigma / dresidual_dsigma.doubleContraction(stress);

  return dgauge_dsigma;
}

void
ViscoPlasticityStressUpdate::computeInelasticStrainIncrement(
    Real & gauge_stress,
    Real & dpsi_dgauge,
    RankTwoTensor & inelastic_strain_increment,
    const Real & equiv_stress,
    const RankTwoTensor & dev_stress,
    const RankTwoTensor & stress)
{
  // If hydrostatic stress and porosity present, compute non-linear gauge stress
  if (_hydro_stress && _intermediate_porosity)
    returnMappingSolve(equiv_stress, gauge_stress, _console);
  // If equivalent stress present, compute traditional creep strain
  else if (equiv_stress)
  {
    gauge_stress = equiv_stress;
    if (_intermediate_porosity)
    {
      if (_model == "GTN")
        gauge_stress /= std::sqrt(1.0 + Utility::pow<2>(_intermediate_porosity));
      else
        gauge_stress /= std::sqrt(1.0 + _power_factor * Utility::pow<2>(_intermediate_porosity));
    }
  }
  else
    mooseError("In ",
               _name,
               ": Zero stress state computed or porosity is zero. Unable to compute gauge stress");

  if (gauge_stress < 0.0)
    mooseException(
        "In ", _name, ": Gauge stress calculated in inner Newton solve is less than zero.");

  // Compute stress potential
  dpsi_dgauge = _coefficient[_qp] * std::pow(gauge_stress, _power);

  // Compute strain increment from stress potential and the gauge stress derivative with respect
  // to the stress stress. The current form is explicit, and should eventually be changed
  inelastic_strain_increment +=
      _dt * dpsi_dgauge * computeDGaugeDSigma(gauge_stress, equiv_stress, dev_stress, stress);
}
