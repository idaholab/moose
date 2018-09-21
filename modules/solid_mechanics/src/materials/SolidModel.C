//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidModel.h"
#include "AxisymmetricRZ.h"
#include "NonlinearRZ.h"
#include "SphericalR.h"
#include "Linear.h"
#include "Nonlinear3D.h"
#include "PlaneStrain.h"
#include "NonlinearPlaneStrain.h"
#include "VolumetricModel.h"
#include "ConstitutiveModel.h"
#include "SymmIsotropicElasticityTensor.h"
#include "MooseApp.h"
#include "Problem.h"
#include "PiecewiseLinear.h"

#include "libmesh/quadrature.h"

registerMooseObject("SolidMechanicsApp", SolidModel);

template <>
InputParameters
validParams<SolidModel>()
{
  MooseEnum formulation(
      "Nonlinear3D NonlinearRZ AxisymmetricRZ SphericalR Linear PlaneStrain NonlinearPlaneStrain");
  MooseEnum compute_method("NoShearRetention ShearRetention");

  InputParameters params = validParams<Material>();
  params.addParam<std::string>(
      "appended_property_name", "", "Name appended to material properties to make them unique");
  params.addParam<Real>("bulk_modulus", "The bulk modulus for the material.");
  params.addParam<Real>("lambda", "Lame's first parameter for the material.");
  params.addParam<Real>("poissons_ratio", "Poisson's ratio for the material.");
  params.addParam<FunctionName>("poissons_ratio_function",
                                "Poisson's ratio as a function of temperature.");
  params.addParam<Real>("shear_modulus", "The shear modulus of the material.");
  params.addParam<Real>("youngs_modulus", "Young's modulus of the material.");
  params.addParam<FunctionName>("youngs_modulus_function",
                                "Young's modulus as a function of temperature.");
  params.addParam<Real>("thermal_expansion", "The thermal expansion coefficient.");
  params.addParam<FunctionName>("thermal_expansion_function",
                                "Thermal expansion coefficient as a function of temperature.");
  params.addCoupledVar("temp", "Coupled Temperature");
  params.addParam<Real>(
      "stress_free_temperature",
      "The stress-free temperature.  If not specified, the initial temperature is used.");
  params.addParam<Real>("thermal_expansion_reference_temperature",
                        "Reference temperature for mean thermal expansion function.");
  MooseEnum cte_function_type("instantaneous mean");
  params.addParam<MooseEnum>("thermal_expansion_function_type",
                             cte_function_type,
                             "Type of thermal expansion function.  Choices are: " +
                                 cte_function_type.getRawNames());
  params.addParam<std::vector<Real>>("initial_stress",
                                     "The initial stress tensor (xx, yy, zz, xy, yz, zx)");
  params.addParam<std::string>(
      "cracking_release",
      "abrupt",
      "The cracking release type.  Choices are abrupt (default) and exponential.");
  params.addParam<Real>("cracking_stress",
                        0.0,
                        "The stress threshold beyond which cracking occurs.  Must be positive.");
  params.addParam<Real>(
      "cracking_residual_stress",
      0.0,
      "The fraction of the cracking stress allowed to be maintained following a crack.");
  params.addParam<Real>("cracking_beta", 1.0, "The coefficient used in the exponetional model.");
  params.addParam<MooseEnum>(
      "compute_method", compute_method, "The method  used in the stress calculation.");
  params.addParam<FunctionName>(
      "cracking_stress_function", "", "The cracking stress as a function of time and location");
  params.addParam<std::vector<unsigned int>>(
      "active_crack_planes", "Planes on which cracks are allowed (0,1,2 -> x,z,theta in RZ)");
  params.addParam<unsigned int>(
      "max_cracks", 3, "The maximum number of cracks allowed at a material point.");
  params.addParam<Real>("cracking_neg_fraction",
                        "The fraction of the cracking strain at which a "
                        "transitition begins during decreasing strain to "
                        "the original stiffness.");
  params.addParam<MooseEnum>("formulation",
                             formulation,
                             "Element formulation.  Choices are: " + formulation.getRawNames());
  params.addParam<std::string>("increment_calculation",
                               "RashidApprox",
                               "The algorithm to use when computing the "
                               "incremental strain and rotation (RashidApprox or "
                               "Eigen). For use with Nonlinear3D/RZ formulation.");
  params.addParam<bool>("large_strain",
                        false,
                        "Whether to include large strain terms in "
                        "AxisymmetricRZ, SphericalR, and PlaneStrain "
                        "formulations.");
  params.addParam<bool>("compute_JIntegral", false, "Whether to compute the J Integral.");
  params.addParam<bool>(
      "compute_InteractionIntegral", false, "Whether to compute the Interaction Integral.");
  params.addParam<bool>("store_stress_older",
                        false,
                        "Parameter which indicates whether the older "
                        "stress state, required for HHT time "
                        "integration, needs to be stored");
  params.addCoupledVar("disp_r", "The r displacement");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("strain_zz", "The zz strain");
  params.addCoupledVar("scalar_strain_zz", "The zz strain (scalar variable)");
  params.addParam<std::vector<std::string>>(
      "dep_matl_props", "Names of material properties this material depends on.");
  params.addParam<std::string>("constitutive_model", "ConstitutiveModel to use (optional)");
  params.addParam<bool>("volumetric_locking_correction",
                        true,
                        "Set to false to turn off volumetric locking correction");
  return params;
}

namespace
{
SolidModel::CRACKING_RELEASE
getCrackingModel(const std::string & name)
{
  std::string n(name);
  std::transform(n.begin(), n.end(), n.begin(), ::tolower);
  SolidModel::CRACKING_RELEASE cm(SolidModel::CR_UNKNOWN);
  if (n == "abrupt")
    cm = SolidModel::CR_ABRUPT;
  else if (n == "exponential")
    cm = SolidModel::CR_EXPONENTIAL;
  else if (n == "power")
    cm = SolidModel::CR_POWER;
  if (cm == SolidModel::CR_UNKNOWN)
    mooseError("Unknown cracking model");
  return cm;
}
}

SolidModel::SolidModel(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _appended_property_name(getParam<std::string>("appended_property_name")),
    _bulk_modulus_set(parameters.isParamValid("bulk_modulus")),
    _lambda_set(parameters.isParamValid("lambda")),
    _poissons_ratio_set(parameters.isParamValid("poissons_ratio")),
    _shear_modulus_set(parameters.isParamValid("shear_modulus")),
    _youngs_modulus_set(parameters.isParamValid("youngs_modulus")),
    _bulk_modulus(_bulk_modulus_set ? getParam<Real>("bulk_modulus") : -1),
    _lambda(_lambda_set ? getParam<Real>("lambda") : -1),
    _poissons_ratio(_poissons_ratio_set ? getParam<Real>("poissons_ratio") : -1),
    _shear_modulus(_shear_modulus_set ? getParam<Real>("shear_modulus") : -1),
    _youngs_modulus(_youngs_modulus_set ? getParam<Real>("youngs_modulus") : -1),
    _youngs_modulus_function(
        isParamValid("youngs_modulus_function") ? &getFunction("youngs_modulus_function") : NULL),
    _poissons_ratio_function(
        isParamValid("poissons_ratio_function") ? &getFunction("poissons_ratio_function") : NULL),
    _cracking_release(getCrackingModel(getParam<std::string>("cracking_release"))),
    _cracking_stress(
        parameters.isParamValid("cracking_stress")
            ? (getParam<Real>("cracking_stress") > 0 ? getParam<Real>("cracking_stress") : -1)
            : -1),
    _cracking_residual_stress(getParam<Real>("cracking_residual_stress")),
    _cracking_beta(getParam<Real>("cracking_beta")),
    _compute_method(getParam<MooseEnum>("compute_method")),
    _cracking_stress_function(getParam<FunctionName>("cracking_stress_function") != ""
                                  ? &getFunction("cracking_stress_function")
                                  : NULL),
    _cracking_alpha(0),
    _active_crack_planes(3, 1),
    _max_cracks(getParam<unsigned int>("max_cracks")),
    _cracking_neg_fraction(
        isParamValid("cracking_neg_fraction") ? getParam<Real>("cracking_neg_fraction") : 0),
    _has_temp(isCoupled("temp")),
    _temperature(_has_temp ? coupledValue("temp") : _zero),
    _temperature_old(_has_temp ? coupledValueOld("temp") : _zero),
    _temp_grad(coupledGradient("temp")),
    _alpha(parameters.isParamValid("thermal_expansion") ? getParam<Real>("thermal_expansion") : 0.),
    _alpha_function(parameters.isParamValid("thermal_expansion_function")
                        ? &getFunction("thermal_expansion_function")
                        : NULL),
    _piecewise_linear_alpha_function(NULL),
    _has_stress_free_temp(false),
    _stress_free_temp(0.0),
    _ref_temp(0.0),
    _volumetric_models(),
    _dep_matl_props(),
    _stress(createProperty<SymmTensor>("stress")),
    _stress_old_prop(getPropertyOld<SymmTensor>("stress")),
    _stress_old(0),
    _total_strain(createProperty<SymmTensor>("total_strain")),
    _total_strain_old(getPropertyOld<SymmTensor>("total_strain")),
    _elastic_strain(createProperty<SymmTensor>("elastic_strain")),
    _elastic_strain_old(getPropertyOld<SymmTensor>("elastic_strain")),
    _crack_flags(NULL),
    _crack_flags_old(NULL),
    _crack_flags_local(),
    _crack_count(NULL),
    _crack_count_old(NULL),
    _crack_rotation(NULL),
    _crack_rotation_old(NULL),
    _crack_strain(NULL),
    _crack_strain_old(NULL),
    _crack_max_strain(NULL),
    _crack_max_strain_old(NULL),
    _principal_strain(3, 1),
    _elasticity_tensor(createProperty<SymmElasticityTensor>("elasticity_tensor")),
    _Jacobian_mult(createProperty<SymmElasticityTensor>("Jacobian_mult")),
    _d_strain_dT(),
    _d_stress_dT(createProperty<SymmTensor>("d_stress_dT")),
    _total_strain_increment(0),
    _mechanical_strain_increment(0),
    _strain_increment(0),
    _compute_JIntegral(getParam<bool>("compute_JIntegral")),
    _compute_InteractionIntegral(getParam<bool>("compute_InteractionIntegral")),
    _store_stress_older(getParam<bool>("store_stress_older")),
    _SED(NULL),
    _SED_old(NULL),
    _Eshelby_tensor(NULL),
    _J_thermal_term_vec(NULL),
    _current_instantaneous_thermal_expansion_coef(NULL),
    _block_id(std::vector<SubdomainID>(blockIDs().begin(), blockIDs().end())),
    _constitutive_active(false),
    _step_zero(declareRestartableData<bool>("step_zero", true)),
    _step_one(declareRestartableData<bool>("step_one", true)),
    _element(NULL),
    _local_elasticity_tensor(NULL)
{
  bool same_coord_type = true;

  for (unsigned int i = 1; i < _block_id.size(); ++i)
    same_coord_type &=
        (_subproblem.getCoordSystem(_block_id[0]) == _subproblem.getCoordSystem(_block_id[i]));
  if (!same_coord_type)
    mooseError("Material '",
               name(),
               "' was specified on multiple blocks that do not have the same coordinate system");
  // Use the first block to figure out the coordinate system (the above check ensures that they are
  // the same)
  _coord_type = _subproblem.getCoordSystem(_block_id[0]);
  _element = createElement();

  const std::vector<std::string> & dmp = getParam<std::vector<std::string>>("dep_matl_props");
  _dep_matl_props.insert(dmp.begin(), dmp.end());
  for (std::set<std::string>::const_iterator i = _dep_matl_props.begin();
       i != _dep_matl_props.end();
       ++i)
  {
    // Tell MOOSE that we need this MaterialProperty.  This enables dependency checking.
    getMaterialProperty<Real>(*i);
  }

  _cracking_alpha = -_youngs_modulus;

  if (_cracking_stress > 0)
  {
    _crack_flags = &createProperty<RealVectorValue>("crack_flags");
    _crack_flags_old = &getPropertyOld<RealVectorValue>("crack_flags");
    if (_cracking_release == CR_POWER)
    {
      _crack_count = &createProperty<RealVectorValue>("crack_count");
      _crack_count_old = &getPropertyOld<RealVectorValue>("crack_count");
    }
    _crack_rotation = &createProperty<ColumnMajorMatrix>("crack_rotation");
    _crack_rotation_old = &getPropertyOld<ColumnMajorMatrix>("crack_rotation");
    _crack_max_strain = &createProperty<RealVectorValue>("crack_max_strain");
    _crack_max_strain_old = &getPropertyOld<RealVectorValue>("crack_max_strain");
    _crack_strain = &createProperty<RealVectorValue>("crack_strain");
    _crack_strain_old = &getPropertyOld<RealVectorValue>("crack_strain");

    if (parameters.isParamValid("active_crack_planes"))
    {
      const std::vector<unsigned int> & planes =
          getParam<std::vector<unsigned>>("active_crack_planes");
      for (unsigned i(0); i < 3; ++i)
        _active_crack_planes[i] = 0;

      for (unsigned i(0); i < planes.size(); ++i)
      {
        if (planes[i] > 2)
          mooseError("Active planes must be 0, 1, or 2");
        _active_crack_planes[planes[i]] = 1;
      }
    }
    if (_cracking_residual_stress < 0 || _cracking_residual_stress > 1)
    {
      mooseError("cracking_residual_stress must be between 0 and 1");
    }
    if (isParamValid("cracking_neg_fraction") &&
        (_cracking_neg_fraction <= 0 || _cracking_neg_fraction > 1))
    {
      mooseError("cracking_neg_fraction must be > zero and <= 1");
    }
  }

  if (parameters.isParamValid("stress_free_temperature"))
  {
    _has_stress_free_temp = true;
    _stress_free_temp = getParam<Real>("stress_free_temperature");
    if (!_has_temp)
      mooseError("Cannot specify stress_free_temperature without coupling to temperature");
  }

  if (parameters.isParamValid("thermal_expansion_function_type"))
  {
    if (!_alpha_function)
      mooseError("thermal_expansion_function_type can only be set when thermal_expansion_function "
                 "is used");
    MooseEnum tec = getParam<MooseEnum>("thermal_expansion_function_type");
    if (tec == "mean")
      _mean_alpha_function = true;
    else if (tec == "instantaneous")
      _mean_alpha_function = false;
    else
      mooseError("Invalid option for thermal_expansion_function_type");
  }
  else
    _mean_alpha_function = false;

  if (parameters.isParamValid("thermal_expansion_reference_temperature"))
  {
    if (!_alpha_function)
      mooseError("thermal_expansion_reference_temperature can only be set when "
                 "thermal_expansion_function is used");
    if (!_mean_alpha_function)
      mooseError("thermal_expansion_reference_temperature can only be set when "
                 "thermal_expansion_function_type = mean");
    _ref_temp = getParam<Real>("thermal_expansion_reference_temperature");
    if (!_has_temp)
      mooseError(
          "Cannot specify thermal_expansion_reference_temperature without coupling to temperature");
  }

  if (_mean_alpha_function)
  {
    if (!parameters.isParamValid("thermal_expansion_reference_temperature") ||
        !_has_stress_free_temp)
      mooseError(
          "Must specify both stress_free_temperature and thermal_expansion_reference_temperature "
          "if thermal_expansion_function_type = mean");
  }

  if (parameters.isParamValid("thermal_expansion") &&
      parameters.isParamValid("thermal_expansion_function"))
    mooseError("Cannot specify both thermal_expansion and thermal_expansion_function");

  if (_compute_JIntegral)
  {
    _SED = &declareProperty<Real>("strain_energy_density");
    _SED_old = &getMaterialPropertyOld<Real>("strain_energy_density");
    _Eshelby_tensor = &declareProperty<RankTwoTensor>("Eshelby_tensor");
    _J_thermal_term_vec = &declareProperty<RealVectorValue>("J_thermal_term_vec");
    _current_instantaneous_thermal_expansion_coef =
        &declareProperty<Real>("current_instantaneous_thermal_expansion_coef");
  }

  if (_compute_InteractionIntegral &&
      !hasMaterialProperty<Real>("current_instantaneous_thermal_expansion_coef"))
    _current_instantaneous_thermal_expansion_coef =
        &declareProperty<Real>("current_instantaneous_thermal_expansion_coef");
}

////////////////////////////////////////////////////////////////////////

SolidModel::~SolidModel()
{
  delete _local_elasticity_tensor;
  delete _element;
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::checkElasticConstants()
{
  int num_elastic_constants = _bulk_modulus_set + _lambda_set + _poissons_ratio_set +
                              _shear_modulus_set + _youngs_modulus_set;

  if (num_elastic_constants != 2)
  {
    std::string err("Exactly two elastic constants must be defined for material '");
    err += name();
    err += "'.";
    mooseError(err);
  }

  if (_bulk_modulus_set && _bulk_modulus <= 0)
  {
    std::string err("Bulk modulus must be positive in material '");
    err += name();
    err += "'.";
    mooseError(err);
  }
  if (_poissons_ratio_set && (_poissons_ratio <= -1.0 || _poissons_ratio >= 0.5))
  {
    std::string err("Poissons ratio must be greater than -1 and less than 0.5 in material '");
    err += name();
    err += "'.";
    mooseError(err);
  }
  if (_shear_modulus_set && _shear_modulus < 0)
  {
    std::string err("Shear modulus must not be negative in material '");
    err += name();
    err += "'.";
    mooseError(err);
  }
  if (_youngs_modulus_set && _youngs_modulus <= 0)
  {
    std::string err("Youngs modulus must be positive in material '");
    err += name();
    err += "'.";
    mooseError(err);
  }

  // Calculate lambda, the shear modulus, and Young's modulus
  if (_lambda_set && _shear_modulus_set) // First and second Lame
  {
    _youngs_modulus =
        _shear_modulus * (3 * _lambda + 2 * _shear_modulus) / (_lambda + _shear_modulus);
    _poissons_ratio = 0.5 * _lambda / (_lambda + _shear_modulus);
  }
  else if (_lambda_set && _poissons_ratio_set)
  {
    _shear_modulus = (_lambda * (1.0 - 2.0 * _poissons_ratio)) / (2.0 * _poissons_ratio);
    _youngs_modulus =
        _shear_modulus * (3 * _lambda + 2 * _shear_modulus) / (_lambda + _shear_modulus);
  }
  else if (_lambda_set && _bulk_modulus_set)
  {
    _shear_modulus = 3.0 * (_bulk_modulus - _lambda) / 2.0;
    _youngs_modulus =
        _shear_modulus * (3 * _lambda + 2 * _shear_modulus) / (_lambda + _shear_modulus);
    _poissons_ratio = _lambda / (3 * _bulk_modulus - _lambda);
  }
  else if (_lambda_set && _youngs_modulus_set)
  {
    _shear_modulus =
        ((_youngs_modulus - 3.0 * _lambda) / 4.0) +
        (std::sqrt((_youngs_modulus - 3.0 * _lambda) * (_youngs_modulus - 3.0 * _lambda) +
                   8.0 * _lambda * _youngs_modulus) /
         4.0);
    _poissons_ratio = _lambda / (3 * _bulk_modulus - _lambda);
  }
  else if (_shear_modulus_set && _poissons_ratio_set)
  {
    _lambda = (2.0 * _shear_modulus * _poissons_ratio) / (1.0 - 2.0 * _poissons_ratio);
    _youngs_modulus =
        _shear_modulus * (3 * _lambda + 2 * _shear_modulus) / (_lambda + _shear_modulus);
  }
  else if (_shear_modulus_set && _bulk_modulus_set)
  {
    _lambda = _bulk_modulus - 2.0 * _shear_modulus / 3.0;
    _youngs_modulus =
        _shear_modulus * (3 * _lambda + 2 * _shear_modulus) / (_lambda + _shear_modulus);
    _poissons_ratio =
        (3 * _bulk_modulus - 2 * _shear_modulus) / (2 * (3 * _bulk_modulus + _shear_modulus));
  }
  else if (_shear_modulus_set && _youngs_modulus_set)
  {
    _lambda = ((2.0 * _shear_modulus - _youngs_modulus) * _shear_modulus) /
              (_youngs_modulus - 3.0 * _shear_modulus);
    _poissons_ratio = 0.5 * _youngs_modulus / _shear_modulus - 1;
  }
  else if (_poissons_ratio_set && _bulk_modulus_set)
  {
    _lambda = (3.0 * _bulk_modulus * _poissons_ratio) / (1.0 + _poissons_ratio);
    _shear_modulus =
        (3.0 * _bulk_modulus * (1.0 - 2.0 * _poissons_ratio)) / (2.0 * (1.0 + _poissons_ratio));
    _youngs_modulus =
        _shear_modulus * (3 * _lambda + 2 * _shear_modulus) / (_lambda + _shear_modulus);
  }
  else if (_youngs_modulus_set && _poissons_ratio_set) // Young's Modulus and Poisson's Ratio
  {
    _lambda = (_poissons_ratio * _youngs_modulus) /
              ((1.0 + _poissons_ratio) * (1 - 2.0 * _poissons_ratio));
    _shear_modulus = _youngs_modulus / (2.0 * (1.0 + _poissons_ratio));
  }
  else if (_youngs_modulus_set && _bulk_modulus_set)
  {
    _lambda = 3.0 * _bulk_modulus * (3.0 * _bulk_modulus - _youngs_modulus) /
              (9.0 * _bulk_modulus - _youngs_modulus);
    _shear_modulus =
        3.0 * _youngs_modulus * _bulk_modulus / (9.0 * _bulk_modulus - _youngs_modulus);
    _poissons_ratio = (3 * _bulk_modulus - _youngs_modulus) / (6 * _bulk_modulus);
  }

  _lambda_set = true;
  _shear_modulus_set = true;
  _youngs_modulus_set = true;
  _poissons_ratio_set = true;
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::createElasticityTensor()
{
  bool constant(true);

  if (_cracking_stress > 0 || _youngs_modulus_function || _poissons_ratio_function ||
      _cracking_stress_function)
  {
    constant = false;
  }

  SymmIsotropicElasticityTensor * iso = new SymmIsotropicElasticityTensor(constant);
  mooseAssert(_youngs_modulus_set, "Internal error:  Youngs modulus not set");
  mooseAssert(_poissons_ratio_set, "Internal error:  Poissons ratio not set");
  iso->setYoungsModulus(_youngs_modulus);
  iso->setPoissonsRatio(_poissons_ratio);
  iso->calculate(0);
  elasticityTensor(iso);
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::timestepSetup()
{
  // if (_cracking_stress > 0)
  // {
  //   _cracked_this_step_count.clear();
  // }
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::jacobianSetup()
{
  // if (_cracking_stress > 0)
  // {
  //   for (std::map<Point, unsigned>::iterator i = _cracked_this_step.begin();
  //        i != _cracked_this_step.end(); ++i)
  //   {
  //     if (i->second)
  //     {
  //       ++_cracked_this_step_count[i->first];
  //     }
  //   }
  // }
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::elasticityTensor(SymmElasticityTensor * e)
{
  delete _local_elasticity_tensor;
  _local_elasticity_tensor = e;
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::modifyStrainIncrement()
{
  bool modified = false;
  _d_strain_dT.zero();

  const SubdomainID current_block = _current_elem->subdomain_id();
  if (_constitutive_active)
  {
    MooseSharedPointer<ConstitutiveModel> cm = _constitutive_model[current_block];

    // Let's be a little careful and check for a non-existent
    // ConstitutiveModel, which could be returned as a default value
    // from std::map::operator[]
    if (!cm)
      mooseError("ConstitutiveModel not available for block ", current_block);

    cm->setQp(_qp);
    modified |= cm->modifyStrainIncrement(*_current_elem, _strain_increment, _d_strain_dT);
  }

  if (!modified)
  {
    applyThermalStrain();
  }

  applyVolumetricStrain();
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::applyThermalStrain()
{
  if (_has_temp && !_step_zero)
  {
    Real inc_thermal_strain;
    Real d_thermal_strain_d_temp;

    Real old_temp;
    if (_step_one && _has_stress_free_temp)
      old_temp = _stress_free_temp;
    else
      old_temp = _temperature_old[_qp];

    Real current_temp = _temperature[_qp];

    Real delta_t = current_temp - old_temp;

    Real alpha = _alpha;

    if (_alpha_function)
    {
      Point p;
      Real alpha_current_temp = _alpha_function->value(current_temp, p);
      Real alpha_old_temp = _alpha_function->value(old_temp, p);

      if (_mean_alpha_function)
      {
        Real alpha_stress_free_temperature = _alpha_function->value(_stress_free_temp, p);
        Real small(1e-6);

        Real numerator = alpha_current_temp * (current_temp - _ref_temp) -
                         alpha_old_temp * (old_temp - _ref_temp);
        Real denominator = 1.0 + alpha_stress_free_temperature * (_stress_free_temp - _ref_temp);
        if (denominator < small)
          mooseError("Denominator too small in thermal strain calculation");
        inc_thermal_strain = numerator / denominator;
        d_thermal_strain_d_temp = alpha_current_temp * (current_temp - _ref_temp);
      }
      else
      {
        inc_thermal_strain = delta_t * 0.5 * (alpha_current_temp + alpha_old_temp);
        d_thermal_strain_d_temp = alpha_current_temp;
      }
    }
    else
    {
      inc_thermal_strain = delta_t * alpha;
      d_thermal_strain_d_temp = alpha;
    }

    _strain_increment.addDiag(-inc_thermal_strain);
    _d_strain_dT.addDiag(-d_thermal_strain_d_temp);
  }
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::applyVolumetricStrain()
{
  const Real V0Vold = 1 / _element->volumeRatioOld(_qp);
  const SubdomainID current_block = _current_elem->subdomain_id();
  const std::vector<MooseSharedPointer<VolumetricModel>> & vm(_volumetric_models[current_block]);
  for (unsigned int i(0); i < vm.size(); ++i)
  {
    vm[i]->modifyStrain(_qp, V0Vold, _strain_increment, _d_strain_dT);
  }
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::rotateSymmetricTensor(const ColumnMajorMatrix & R,
                                  const SymmTensor & T,
                                  SymmTensor & result)
{

  //     R           T         Rt
  //  00 01 02   00 01 02   00 10 20
  //  10 11 12 * 10 11 12 * 01 11 21
  //  20 21 22   20 21 22   02 12 22
  //
  const Real T00 = R(0, 0) * T.xx() + R(0, 1) * T.xy() + R(0, 2) * T.zx();
  const Real T01 = R(0, 0) * T.xy() + R(0, 1) * T.yy() + R(0, 2) * T.yz();
  const Real T02 = R(0, 0) * T.zx() + R(0, 1) * T.yz() + R(0, 2) * T.zz();

  const Real T10 = R(1, 0) * T.xx() + R(1, 1) * T.xy() + R(1, 2) * T.zx();
  const Real T11 = R(1, 0) * T.xy() + R(1, 1) * T.yy() + R(1, 2) * T.yz();
  const Real T12 = R(1, 0) * T.zx() + R(1, 1) * T.yz() + R(1, 2) * T.zz();

  const Real T20 = R(2, 0) * T.xx() + R(2, 1) * T.xy() + R(2, 2) * T.zx();
  const Real T21 = R(2, 0) * T.xy() + R(2, 1) * T.yy() + R(2, 2) * T.yz();
  const Real T22 = R(2, 0) * T.zx() + R(2, 1) * T.yz() + R(2, 2) * T.zz();

  result.xx(T00 * R(0, 0) + T01 * R(0, 1) + T02 * R(0, 2));
  result.yy(T10 * R(1, 0) + T11 * R(1, 1) + T12 * R(1, 2));
  result.zz(T20 * R(2, 0) + T21 * R(2, 1) + T22 * R(2, 2));
  result.xy(T00 * R(1, 0) + T01 * R(1, 1) + T02 * R(1, 2));
  result.yz(T10 * R(2, 0) + T11 * R(2, 1) + T12 * R(2, 2));
  result.zx(T00 * R(2, 0) + T01 * R(2, 1) + T02 * R(2, 2));
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::initQpStatefulProperties()
{
  if (isParamValid("initial_stress"))
  {
    const std::vector<Real> & s = getParam<std::vector<Real>>("initial_stress");
    if (6 != s.size())
    {
      mooseError("initial_stress must give six values");
    }
    _stress[_qp].fillFromInputVector(s);
  }

  if (_cracking_stress_function != NULL)
  {
    _cracking_stress = _cracking_stress_function->value(_t, _q_point[_qp]);
  }
  if (_cracking_stress > 0)
  {
    (*_crack_flags)[_qp](0) = (*_crack_flags)[_qp](1) = (*_crack_flags)[_qp](2) = 1;
    if (_crack_count)
    {
      (*_crack_count)[_qp](0) = (*_crack_count)[_qp](1) = (*_crack_count)[_qp](2) = 0;
    }

    (*_crack_rotation)[_qp].identity();
  }
  if (_SED)
    (*_SED)[_qp] = 0;
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::computeProperties()
{
  if (_t_step >= 1)
    _step_zero = false;

  if (_t_step >= 2)
    _step_one = false;

  elementInit();
  _element->init();

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _element->computeStrain(_qp, _total_strain_old[_qp], _total_strain[_qp], _strain_increment);
    _total_strain_increment = _strain_increment;

    modifyStrainIncrement();
    _mechanical_strain_increment = _strain_increment;

    computeElasticityTensor();

    if (!_constitutive_active)
      computeStress();
    else
      computeConstitutiveModelStress();

    if (_compute_JIntegral)
      computeStrainEnergyDensity();

    _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment;

    crackingStressRotation();

    finalizeStress();

    if (_compute_JIntegral)
      computeEshelby();

    if (_compute_JIntegral && _has_temp)
      computeThermalJvec();

    if (_compute_InteractionIntegral && _has_temp)
      computeCurrentInstantaneousThermalExpansionCoefficient();

    computePreconditioning();
  }
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::computeStrainEnergyDensity()
{
  mooseAssert(_SED, "_SED not initialized");
  mooseAssert(_SED_old, "_SED_old not initialized");
  (*_SED)[_qp] = (*_SED_old)[_qp] +
                 _stress[_qp].doubleContraction(_mechanical_strain_increment) / 2 +
                 _stress_old_prop[_qp].doubleContraction(_mechanical_strain_increment) / 2;
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::computeEshelby()
{
  mooseAssert(_SED, "_SED not initialized");
  mooseAssert(_Eshelby_tensor, "_Eshelby_tensor not initialized");
  // Cauchy stress (sigma) in a colum major matrix:
  ColumnMajorMatrix stress_CMM;
  stress_CMM(0, 0) = _stress[_qp].xx();
  stress_CMM(0, 1) = _stress[_qp].xy();
  stress_CMM(0, 2) = _stress[_qp].xz();
  stress_CMM(1, 0) = _stress[_qp].xy();
  stress_CMM(1, 1) = _stress[_qp].yy();
  stress_CMM(1, 2) = _stress[_qp].yz();
  stress_CMM(2, 0) = _stress[_qp].xz();
  stress_CMM(2, 1) = _stress[_qp].yz();
  stress_CMM(2, 2) = _stress[_qp].zz();

  // Deformation gradient (F):
  ColumnMajorMatrix F;
  _element->computeDeformationGradient(_qp, F);
  // Displacement gradient (H):
  ColumnMajorMatrix H(F);
  H.addDiag(-1.0);
  Real detF = _element->detMatrix(F);
  ColumnMajorMatrix Finv;
  _element->invertMatrix(F, Finv);
  ColumnMajorMatrix FinvT;
  FinvT = Finv.transpose();
  ColumnMajorMatrix HT;
  HT = H.transpose();

  // 1st Piola-Kirchoff Stress (P):
  ColumnMajorMatrix piola;
  piola = stress_CMM * FinvT;
  piola *= detF;

  // HTP = H^T * P = H^T * detF * sigma * FinvT;
  ColumnMajorMatrix HTP;
  HTP = HT * piola;

  ColumnMajorMatrix WI;
  WI.identity();
  WI *= (*_SED)[_qp];
  WI *= detF;
  (*_Eshelby_tensor)[_qp] = WI - HTP;
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::computeConstitutiveModelStress()
{
  // Given the stretching, compute the stress increment and add it to the old stress. Also update
  // the creep strain
  // stress = stressOld + stressIncrement

  const SubdomainID current_block = _current_elem->subdomain_id();
  MooseSharedPointer<ConstitutiveModel> cm = _constitutive_model[current_block];

  mooseAssert(_constitutive_active, "Logic error.  ConstitutiveModel not active.");

  // Let's be a little careful and check for a non-existent
  // ConstitutiveModel, which could be returned as a default value
  // from std::map::operator[]
  if (!cm)
    mooseError("Logic error.  No ConstitutiveModel for current_block=", current_block, ".");

  cm->setQp(_qp);
  cm->computeStress(
      *_current_elem, *elasticityTensor(), _stress_old, _strain_increment, _stress[_qp]);
}

////////////////////////////////////////////////////////////////////////
void
SolidModel::computeElasticityTensor()
{
  if (_cracking_stress_function != NULL)
  {
    _cracking_stress = _cracking_stress_function->value(_t, _q_point[_qp]);
  }

  _stress_old = _stress_old_prop[_qp];

  bool changed = updateElasticityTensor(*_local_elasticity_tensor);

  _local_elasticity_tensor->calculate(_qp);

  _elasticity_tensor[_qp] = *_local_elasticity_tensor;

  crackingStrainDirections();

  if (changed || _cracking_stress > 0)
  {
    _stress_old = _elasticity_tensor[_qp] * _elastic_strain_old[_qp];
  }
}

////////////////////////////////////////////////////////////////////////

bool
SolidModel::updateElasticityTensor(SymmElasticityTensor & tensor)
{
  bool changed(false);
  if (_constitutive_active)
  {
    const SubdomainID current_block = _current_elem->subdomain_id();
    MooseSharedPointer<ConstitutiveModel> cm = _constitutive_model[current_block];

    // Let's be a little careful and check for a non-existent
    // ConstitutiveModel, which could be returned as a default value
    // from std::map::operator[]
    if (!cm)
      mooseError("ConstitutiveModel not available for block ", current_block);

    cm->setQp(_qp);
    changed |= cm->updateElasticityTensor(tensor);
  }

  if (!changed && (_youngs_modulus_function || _poissons_ratio_function))
  {
    SymmIsotropicElasticityTensor * t = dynamic_cast<SymmIsotropicElasticityTensor *>(&tensor);
    if (!t)
    {
      mooseError("Cannot use Youngs modulus or Poissons ratio functions");
    }
    t->unsetConstants();
    Point p;
    t->setYoungsModulus((_youngs_modulus_function
                             ? _youngs_modulus_function->value(_temperature[_qp], p)
                             : _youngs_modulus));
    t->setPoissonsRatio((_poissons_ratio_function
                             ? _poissons_ratio_function->value(_temperature[_qp], p)
                             : _poissons_ratio));
    changed = true;
  }
  return changed;
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::finalizeStress()
{
  std::vector<SymmTensor *> t(3);
  t[0] = &_elastic_strain[_qp];
  t[1] = &_total_strain[_qp];
  t[2] = &_stress[_qp];
  _element->finalizeStress(t);
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::computePreconditioning()
{
  mooseAssert(_local_elasticity_tensor, "null elasticity tensor");

  //  _Jacobian_mult[_qp] = *_local_elasticity_tensor;
  //  _d_stress_dT[_qp] = *_local_elasticity_tensor * _d_strain_dT;
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
  _d_stress_dT[_qp] = _elasticity_tensor[_qp] * _d_strain_dT;
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::initialSetup()
{

  checkElasticConstants();

  createElasticityTensor();

  // Load in the volumetric models and constitutive models
  bool set_constitutive_active = false;
  for (unsigned i(0); i < _block_id.size(); ++i)
  {

    //    const std::vector<Material*> * mats_p;
    std::vector<MooseSharedPointer<Material>> const * mats_p;
    std::string suffix;
    if (_bnd)
    {
      mats_p = &_fe_problem.getMaterialWarehouse()[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(
          _block_id[i], _tid);
      suffix = "_face";
    }
    else
      mats_p = &_fe_problem.getMaterialWarehouse().getActiveBlockObjects(_block_id[i], _tid);

    const std::vector<MooseSharedPointer<Material>> & mats = *mats_p;

    for (unsigned int j = 0; j < mats.size(); ++j)
    {
      MooseSharedPointer<VolumetricModel> vm =
          MooseSharedNamespace::dynamic_pointer_cast<VolumetricModel>(mats[j]);
      if (vm)
      {
        const std::vector<std::string> & dep_matl_props = vm->getDependentMaterialProperties();
        for (unsigned k = 0; k < dep_matl_props.size(); ++k)
        {
          if ("" != dep_matl_props[k] &&
              _dep_matl_props.find(dep_matl_props[k]) == _dep_matl_props.end())
          {
            mooseError("A VolumetricModel depends on " + dep_matl_props[k] +
                       ", but that material property was not given in the dep_matl_props line.");
          }
        }
        _volumetric_models[_block_id[i]].push_back(vm);
      }
    }

    for (std::map<SubdomainID, MooseSharedPointer<ConstitutiveModel>>::iterator iter =
             _constitutive_model.begin();
         iter != _constitutive_model.end();
         ++iter)
    {
      iter->second->initialSetup();
    }

    if (isParamValid("constitutive_model") && !_constitutive_active)
    {
      // User-defined name of the constitutive model (a Material object)
      std::string constitutive_model = getParam<std::string>("constitutive_model") + suffix;

      for (unsigned int j = 0; j < mats.size(); ++j)
      {
        MooseSharedPointer<ConstitutiveModel> cm =
            MooseSharedNamespace::dynamic_pointer_cast<ConstitutiveModel>(mats[j]);

        if (cm && cm->name() == constitutive_model)
        {
          _constitutive_model[_block_id[i]] = cm;
          set_constitutive_active = true;
          break;
        }
      }

      if (!set_constitutive_active)
        mooseError("Unable to find constitutive model " + constitutive_model);
    }
  }
  if (set_constitutive_active)
    _constitutive_active = true;

  if (_compute_JIntegral && _alpha_function)
  {
    // Make sure that timeDerivative is supported for _alpha_function.  If not, it will error out.
    Point dummy_point;
    Real dummy_temp = 0;
    _alpha_function->timeDerivative(dummy_temp, dummy_point);
  }
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::crackingStrainDirections()
{
  bool cracking_locally_active(false);
  if (_cracking_stress > 0)
  {
    // Compute whether cracking has occurred
    (*_crack_rotation)[_qp] = (*_crack_rotation_old)[_qp];

    ColumnMajorMatrix RT((*_crack_rotation)[_qp].transpose());
    SymmTensor ePrime;
    rotateSymmetricTensor(RT, _elastic_strain[_qp], ePrime);

    for (unsigned int i(0); i < 3; ++i)
    {
      (*_crack_max_strain)[_qp](i) = (*_crack_max_strain_old)[_qp](i);

      if (_cracking_neg_fraction == 0 && ePrime(i, i) < 0)
      {
        _crack_flags_local(i) = 1;
      }
      else if (_cracking_neg_fraction > 0 &&
               (*_crack_strain)[_qp](i) * _cracking_neg_fraction > ePrime(i, i))
      {
        if (-(*_crack_strain)[_qp](i) * _cracking_neg_fraction > ePrime(i, i))
        {
          _crack_flags_local(i) = 1;
        }
        else
        {
          // s = a*e^2 + b*e + c
          // a = (Ec-Eo)/(4etr)
          // b = (Ec+Eo)/2
          // c = (Ec-Eo)*etr/4
          // etr = _cracking_neg_fraction * strain when crack occurred
          const Real etr = _cracking_neg_fraction * (*_crack_strain)[_qp](i);
          const Real Eo = _cracking_stress / (*_crack_strain)[_qp](i);
          const Real Ec = Eo * (*_crack_flags_old)[_qp](i);
          const Real a = (Ec - Eo) / (4 * etr);
          const Real b = (Ec + Eo) / 2;
          // Compute the ratio of the current transition stiffness to the original stiffness
          _crack_flags_local(i) = (2 * a * etr + b) / Eo;
          cracking_locally_active = true;
        }
      }
      else
      {
        _crack_flags_local(i) = (*_crack_flags_old)[_qp](i);
        if (_crack_flags_local(i) < 1)
        {
          cracking_locally_active = true;
        }
      }
    }
  }
  if (cracking_locally_active)
  {
    // Adjust the elasticity matrix for cracking.  This must be used by the
    // constitutive law.
    if (_compute_method == "ShearRetention")
      _local_elasticity_tensor->adjustForCracking(_crack_flags_local);
    else
      _local_elasticity_tensor->adjustForCrackingWithShearRetention(_crack_flags_local);

    ColumnMajorMatrix R_9x9(9, 9);
    const ColumnMajorMatrix & R((*_crack_rotation)[_qp]);
    _local_elasticity_tensor->form9x9Rotation(R, R_9x9);
    _local_elasticity_tensor->rotateFromLocalToGlobal(R_9x9);

    _elasticity_tensor[_qp] = *_local_elasticity_tensor;
  }
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::applyCracksToTensor(SymmTensor & tensor, const RealVectorValue & sigma)
{
  // Form transformation matrix R*E*R^T
  const ColumnMajorMatrix & R((*_crack_rotation)[_qp]);

  // Rotate to crack frame
  rotateSymmetricTensor(R.transpose(), tensor, tensor);

  // Reset stress if cracked
  if ((*_crack_flags)[_qp](0) < 1)
  {
    tensor(0, 0) = sigma(0);
  }
  if ((*_crack_flags)[_qp](1) < 1)
  {
    tensor(1, 1) = sigma(1);
  }
  if ((*_crack_flags)[_qp](2) < 1)
  {
    tensor(2, 2) = sigma(2);
  }

  // Rotate back to global frame
  rotateSymmetricTensor(R, tensor, tensor);
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::computeCrackStrainAndOrientation(ColumnMajorMatrix & principal_strain)
{
  // The rotation tensor is ordered such that known dirs appear last in the list of
  // columns.  So, if one dir is known, it corresponds with the last column in the
  // rotation tensor.
  //
  // This convention is based on the eigen routine returning eigen values in
  // ascending order.
  const unsigned int numKnownDirs = getNumKnownCrackDirs();

  _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment;

  (*_crack_rotation)[_qp] = (*_crack_rotation_old)[_qp];

  if (numKnownDirs == 0)
  {
    ColumnMajorMatrix e_vec(3, 3);
    _elastic_strain[_qp].columnMajorMatrix().eigen(principal_strain, e_vec);
    // If the elastic strain is beyond the cracking strain, save the eigen vectors as
    // the rotation tensor.
    (*_crack_rotation)[_qp] = e_vec;
  }
  else if (numKnownDirs == 1)
  {
    // This is easily the most complicated case.
    // 1.  Rotate the elastic strain to the orientation associated with the known
    //     crack.
    // 2.  Extract the upper 2x2 diagonal block into a separate tensor.
    // 3.  Run the eigen solver on the result.
    // 4.  Update the rotation tensor to reflect the effect of the 2 eigenvectors.

    // 1.
    ColumnMajorMatrix RT((*_crack_rotation)[_qp].transpose());
    SymmTensor ePrime;
    rotateSymmetricTensor(RT, _elastic_strain[_qp], ePrime);

    // 2.
    ColumnMajorMatrix e2x2(2, 2);
    e2x2(0, 0) = ePrime(0, 0);
    e2x2(1, 0) = ePrime(1, 0);
    e2x2(0, 1) = ePrime(0, 1);
    e2x2(1, 1) = ePrime(1, 1);

    // 3.
    ColumnMajorMatrix e_val2x1(2, 1);
    ColumnMajorMatrix e_vec2x2(2, 2);
    e2x2.eigen(e_val2x1, e_vec2x2);

    // 4.
    ColumnMajorMatrix e_vec(3, 3);
    e_vec(0, 0) = e_vec2x2(0, 0);
    e_vec(1, 0) = e_vec2x2(1, 0);
    e_vec(2, 0) = 0;
    e_vec(0, 1) = e_vec2x2(0, 1);
    e_vec(1, 1) = e_vec2x2(1, 1);
    e_vec(2, 1) = 0;
    e_vec(2, 0) = 0;
    e_vec(2, 1) = 0;
    e_vec(2, 2) = 1;
    (*_crack_rotation)[_qp] = (*_crack_rotation_old)[_qp] * e_vec;

    principal_strain(0, 0) = e_val2x1(0, 0);
    principal_strain(1, 0) = e_val2x1(1, 0);
    principal_strain(2, 0) = ePrime(2, 2);
  }
  else if (numKnownDirs == 2 || numKnownDirs == 3)
  {
    // Rotate to cracked orientation and pick off the strains in the rotated
    // coordinate directions.
    ColumnMajorMatrix RT((*_crack_rotation)[_qp].transpose());
    SymmTensor ePrime;
    rotateSymmetricTensor(RT, _elastic_strain[_qp], ePrime);
    principal_strain(0, 0) = ePrime.xx();
    principal_strain(1, 0) = ePrime.yy();
    principal_strain(2, 0) = ePrime.zz();
  }
  else
  {
    mooseError("Invalid number of known crack directions");
  }
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::crackingStressRotation()
{
  if (_cracking_stress_function != NULL)
  {
    _cracking_stress = _cracking_stress_function->value(_t, _q_point[_qp]);
  }

  if (_cracking_stress > 0)
  {

    computeCrackStrainAndOrientation(_principal_strain);

    for (unsigned i(0); i < 3; ++i)
    {
      if (_principal_strain(i, 0) > (*_crack_max_strain_old)[_qp](i))
      {
        (*_crack_max_strain)[_qp](i) = _principal_strain(i, 0);
      }
    }

    // Check for new cracks.
    // This must be done in the crack-local coordinate frame.

    // Rotate stress to cracked orientation.
    ColumnMajorMatrix R((*_crack_rotation)[_qp]);
    ColumnMajorMatrix RT((*_crack_rotation)[_qp].transpose());
    SymmTensor sigmaPrime;
    rotateSymmetricTensor(RT, _stress[_qp], sigmaPrime);

    unsigned int num_cracks(0);
    for (unsigned i(0); i < 3; ++i)
    {
      _crack_flags_local(i) = 1;
      (*_crack_strain)[_qp](i) = (*_crack_strain_old)[_qp](i);
      if ((*_crack_flags_old)[_qp](i) < 1)
      {
        ++num_cracks;
      }
    }

    bool new_crack(false);
    bool cracked(false);
    RealVectorValue sigma;
    for (unsigned i(0); i < 3; ++i)
    {
      sigma(i) = sigmaPrime(i, i);
      (*_crack_flags)[_qp](i) = (*_crack_flags_old)[_qp](i);
      if (sigma(i) <= 1e-4)
      {
        if ((*_crack_flags)[_qp](i) == 1)
        {
          (*_crack_max_strain)[_qp](i) = _principal_strain(i, 0);
        }
      }

      // _cracked_this_step[_q_point[_qp]] = 0;
      Real crackFactor(1);
      if (_cracking_release == CR_POWER)
      {
        (*_crack_count)[_qp](i) = (*_crack_count_old)[_qp](i);
      }
      if ((_cracking_release == CR_POWER && sigma(i) > _cracking_stress &&
           _active_crack_planes[i] == 1
           // && (*_crack_count)[_qp](i) == 0
           )
          // || _cracked_this_step_count[_q_point[_qp]] > 5
      )
      {
        cracked = true;
        ++((*_crack_count)[_qp](i));
        // _cracked_this_step[_q_point[_qp]] = 1;
        // Assume Poisson's ratio drops to zero for this direction.  Stiffness is then Young's
        // modulus.
        const Real stiff = _youngs_modulus_function
                               ? _youngs_modulus_function->value(_temperature[_qp], Point())
                               : _youngs_modulus;

        if ((*_crack_count_old)[_qp](i) == 0)
        {
          new_crack = true;
          ++num_cracks;

          (*_crack_strain)[_qp](i) = _cracking_stress / stiff;
        }
        // Compute stress, factor....
        (*_crack_flags)[_qp](i) *= 1. / 3.;

        if ((*_crack_max_strain)[_qp](i) < (*_crack_strain)[_qp](i))
        {
          (*_crack_max_strain)[_qp](i) = (*_crack_strain)[_qp](i);
        }
        sigma(i) = (*_crack_flags)[_qp](i) * stiff * _principal_strain(i, 0);
      }
      else if ((_cracking_release != CR_POWER && (*_crack_flags_old)[_qp](i) == 1 &&
                sigma(i) > _cracking_stress && num_cracks < _max_cracks &&
                _active_crack_planes[i] == 1)
               // || _cracked_this_step_count[_q_point[_qp]] > 5
      )
      {
        // A new crack
        // _cracked_this_step[_q_point[_qp]] = 1;

        cracked = true;
        new_crack = true;
        ++num_cracks;

        // Assume Poisson's ratio drops to zero for this direction.  Stiffness is then Young's
        // modulus.
        const Real stiff = _youngs_modulus_function
                               ? _youngs_modulus_function->value(_temperature[_qp], Point())
                               : _youngs_modulus;

        (*_crack_strain)[_qp](i) = _cracking_stress / stiff;
        if ((*_crack_max_strain)[_qp](i) < (*_crack_strain)[_qp](i))
        {
          (*_crack_max_strain)[_qp](i) = (*_crack_strain)[_qp](i);
        }

        crackFactor = computeCrackFactor(i, sigma(i), (*_crack_flags)[_qp](i));

        (*_crack_flags)[_qp](i) = crackFactor;
        _crack_flags_local(i) = crackFactor;

        // May want to set the old value.  This may help with the nonlinear solve
        // since the stress cannot bounce between just below the critical stress and
        // effectively zero.  However, this may set allow cracking prematurely.
        //        (*_crack_flags_old)[_qp](i) = crackFactor;
        //        (*_crack_strain_old)[_qp](i) = _principal_strain(i,0);
      }
      else if (_cracking_release != CR_POWER && (*_crack_flags_old)[_qp](i) < 1 &&
               std::abs(_principal_strain(i, 0) - (*_crack_max_strain)[_qp](i)) < 1e-10)
      {
        // Previously cracked,
        // Crack opening
        cracked = true;
        crackFactor = computeCrackFactor(i, sigma(i), (*_crack_flags)[_qp](i));
        (*_crack_flags)[_qp](i) = crackFactor;
        _crack_flags_local(i) = crackFactor;
      }
      else if (_cracking_neg_fraction > 0 &&
               (*_crack_strain)[_qp](i) * _cracking_neg_fraction > _principal_strain(i, 0) &&
               -(*_crack_strain)[_qp](i) * _cracking_neg_fraction < _principal_strain(i, 0))
      {
        // s = a*e^2 + b*e + c
        // a = (Ec-Eo)/(4etr)
        // b = (Ec+Eo)/2
        // c = (Ec-Eo)*etr/4
        // etr = _cracking_neg_fraction * strain when crack occurred
        cracked = true;
        const Real etr = _cracking_neg_fraction * (*_crack_strain)[_qp](i);
        const Real Eo = _cracking_stress / (*_crack_strain)[_qp](i);
        const Real Ec = Eo * (*_crack_flags_old)[_qp](i);
        const Real a = (Ec - Eo) / (4 * etr);
        const Real b = (Ec + Eo) / 2;
        const Real c = (Ec - Eo) * etr / 4;
        sigma(i) = (a * _principal_strain(i, 0) + b) * _principal_strain(i, 0) + c;
      }
    }

    if (!new_crack)
    {
      (*_crack_rotation)[_qp] = (*_crack_rotation_old)[_qp];
    }
    if (cracked)
    {
      applyCracksToTensor(_stress[_qp], sigma);
    }
  }
}

Real
SolidModel::computeCrackFactor(int i, Real & sigma, Real & flagVal)
{
  if (_cracking_release == CR_EXPONENTIAL)
  {
    if ((*_crack_max_strain)[_qp](i) < (*_crack_strain)[_qp](i))
    {
      std::stringstream err;
      err << "Max strain less than crack strain: " << i << " " << sigma << ", "
          << (*_crack_max_strain)[_qp](i) << ", " << (*_crack_strain)[_qp](i) << ", "
          << _principal_strain(0, 0) << ", " << _principal_strain(1, 0) << ", "
          << _principal_strain(2, 0) << _elastic_strain[_qp] << std::endl;
      mooseError(err.str());
    }
    const Real crackMaxStrain((*_crack_max_strain)[_qp](i));
    // Compute stress that follows exponental curve
    sigma = _cracking_stress * (_cracking_residual_stress +
                                (1 - _cracking_residual_stress) *
                                    std::exp(_cracking_alpha * _cracking_beta / _cracking_stress *
                                             (crackMaxStrain - (*_crack_strain)[_qp](i))));
    // Compute ratio of current stiffness to original stiffness
    flagVal = sigma * (*_crack_strain)[_qp](i) / (crackMaxStrain * _cracking_stress);
  }
  else
  {
    if (_cracking_residual_stress == 0)
    {
      const Real tiny(1e-16);
      flagVal = tiny;
      sigma = tiny * (*_crack_strain)[_qp](i) * _youngs_modulus;
    }
    else
    {
      sigma = _cracking_residual_stress * _cracking_stress;
      flagVal = sigma / ((*_crack_max_strain)[_qp](i) * _youngs_modulus);
    }
  }
  if (flagVal < 0)
  {
    std::stringstream err;
    err << "Negative crack flag found: " << i << " " << flagVal << ", "
        << (*_crack_max_strain)[_qp](i) << ", " << (*_crack_strain)[_qp](i) << ", " << std::endl;
    mooseError(err.str());
  }
  return flagVal;
}

unsigned int
SolidModel::getNumKnownCrackDirs() const
{
  const unsigned fromElement = _element->getNumKnownCrackDirs();
  unsigned int retVal(0);
  for (unsigned int i(0); i < 3 - fromElement; ++i)
  {
    retVal += ((*_crack_flags_old)[_qp](i) < 1);
  }
  return retVal + fromElement;
}

SolidMechanics::Element *
SolidModel::createElement()
{
  std::string mat_name = name();
  InputParameters parameters = emptyInputParameters();
  parameters += this->parameters();

  SolidMechanics::Element * element(NULL);

  std::string formulation = getParam<MooseEnum>("formulation");
  std::transform(formulation.begin(), formulation.end(), formulation.begin(), ::tolower);
  if (formulation == "nonlinear3d")
  {
    if (!isCoupled("disp_x") || !isCoupled("disp_y") || !isCoupled("disp_z"))
      mooseError("Nonlinear3D requires all three displacements");

    if (isCoupled("disp_r"))
      mooseError("Linear must not define disp_r");

    if (_coord_type == Moose::COORD_RZ)
      mooseError("Nonlinear3D formulation requested for coord_type = RZ problem");

    element = new SolidMechanics::Nonlinear3D(*this, mat_name, parameters);
  }
  else if (formulation == "nonlinearrz")
  {
    if (!isCoupled("disp_r") || !isCoupled("disp_z"))
      mooseError("NonlinearRZ must define disp_r and disp_z");

    element = new SolidMechanics::NonlinearRZ(*this, mat_name, parameters);
  }
  else if (formulation == "axisymmetricrz")
  {
    if (!isCoupled("disp_r") || !isCoupled("disp_z"))
      mooseError("AxisymmetricRZ must define disp_r and disp_z");
    element = new SolidMechanics::AxisymmetricRZ(*this, mat_name, parameters);
  }
  else if (formulation == "sphericalr")
  {
    if (!isCoupled("disp_r"))
      mooseError("SphericalR must define disp_r");
    element = new SolidMechanics::SphericalR(*this, mat_name, parameters);
  }
  else if (formulation == "planestrain")
  {
    if (!isCoupled("disp_x") || !isCoupled("disp_y"))
      mooseError("PlaneStrain must define disp_x and disp_y");
    element = new SolidMechanics::PlaneStrain(*this, mat_name, parameters);
  }
  else if (formulation == "nonlinearplanestrain")
  {
    if (!isCoupled("disp_x") || !isCoupled("disp_y"))
      mooseError("NonlinearPlaneStrain must define disp_x and disp_y");
    element = new SolidMechanics::NonlinearPlaneStrain(*this, mat_name, parameters);
  }
  else if (formulation == "linear")
  {
    if (isCoupled("disp_r"))
      mooseError("Linear must not define disp_r");
    if (_coord_type == Moose::COORD_RZ)
      mooseError("Linear formulation requested for coord_type = RZ problem");
    element = new SolidMechanics::Linear(*this, mat_name, parameters);
  }
  else if (formulation != "")
    mooseError("Unknown formulation: " + formulation);

  if (!element && _coord_type == Moose::COORD_RZ)
  {
    if (!isCoupled("disp_r") || !isCoupled("disp_z"))
    {
      std::string err(name());
      err += ": RZ coord sys requires disp_r and disp_z for AxisymmetricRZ formulation";
      mooseError(err);
    }
    element = new SolidMechanics::AxisymmetricRZ(*this, mat_name, parameters);
  }
  else if (!element && _coord_type == Moose::COORD_RSPHERICAL)
  {
    if (!isCoupled("disp_r"))
    {
      std::string err(name());
      err += ": RSPHERICAL coord sys requires disp_r for SphericalR formulation";
      mooseError(err);
    }
    element = new SolidMechanics::SphericalR(*this, mat_name, parameters);
  }

  if (!element)
  {
    if (isCoupled("disp_x") && isCoupled("disp_y") && isCoupled("disp_z"))
    {
      if (isCoupled("disp_r"))
        mooseError("Error with displacement specification in material " + mat_name);
      element = new SolidMechanics::Nonlinear3D(*this, mat_name, parameters);
    }
    else if (isCoupled("disp_x") && isCoupled("disp_y"))
    {
      if (isCoupled("disp_r"))
        mooseError("Error with displacement specification in material " + mat_name);
      element = new SolidMechanics::PlaneStrain(*this, mat_name, parameters);
    }
    else if (isCoupled("disp_r") && isCoupled("disp_z"))
    {
      if (_coord_type != Moose::COORD_RZ)
        mooseError("RZ coord system not specified, but disp_r and disp_z are");
      element = new SolidMechanics::AxisymmetricRZ(*this, mat_name, parameters);
    }
    else if (isCoupled("disp_r"))
    {
      if (_coord_type != Moose::COORD_RSPHERICAL)
        mooseError("RSPHERICAL coord system not specified, but disp_r is");
      element = new SolidMechanics::SphericalR(*this, mat_name, parameters);
    }
    else if (isCoupled("disp_x"))
      element = new SolidMechanics::Linear(*this, mat_name, parameters);
    else
      mooseError("Unable to determine formulation for material " + mat_name);
  }

  mooseAssert(element, "No Element created for material " + mat_name);

  return element;
}

void
SolidModel::createConstitutiveModel(const std::string & cm_name)
{

  Factory & factory = _app.getFactory();
  InputParameters params = factory.getValidParams(cm_name);

  params += parameters();
  MooseSharedPointer<ConstitutiveModel> cm =
      factory.create<ConstitutiveModel>(cm_name, name() + "Model", params, _tid);

  _models_to_free.insert(
      cm); // Keep track of the dynamic memory that is created internally to this object

  _constitutive_active = true;
  for (unsigned i(0); i < _block_id.size(); ++i)
  {
    _constitutive_model[_block_id[i]] = cm;
  }
}

void
SolidModel::initStatefulProperties(unsigned n_points)
{
  for (_qp = 0; _qp < n_points; ++_qp)
  {
    initQpStatefulProperties();
  }
  if (_constitutive_active)
  {
    const SubdomainID current_block = _current_elem->subdomain_id();
    MooseSharedPointer<ConstitutiveModel> cm = _constitutive_model[current_block];
    cm->initStatefulProperties(n_points);
  }
}

void
SolidModel::computeThermalJvec()
{
  mooseAssert(_J_thermal_term_vec, "_J_thermal_term_vec not initialized");

  Real stress_trace = _stress[_qp].xx() + _stress[_qp].yy() + _stress[_qp].zz();

  computeCurrentInstantaneousThermalExpansionCoefficient();
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
  {
    Real dthermstrain_dx =
        _temp_grad[_qp](i) * (*_current_instantaneous_thermal_expansion_coef)[_qp];
    (*_J_thermal_term_vec)[_qp](i) = stress_trace * dthermstrain_dx;
  }
}

void
SolidModel::computeCurrentInstantaneousThermalExpansionCoefficient()
{
  mooseAssert(_current_instantaneous_thermal_expansion_coef,
              "_current_instantaneous_thermal_expansion_coef not initialized");

  (*_current_instantaneous_thermal_expansion_coef)[_qp] = 0.0;

  if (_alpha_function)
  {
    Point p;
    Real current_temp = _temperature[_qp];

    if (!_mean_alpha_function)
    {
      Real alpha = _alpha_function->value(current_temp, p);
      (*_current_instantaneous_thermal_expansion_coef)[_qp] = alpha;
    }
    else
    {
      Real small(1e-6);
      Real dalphabar_dT = _alpha_function->timeDerivative(current_temp, p);
      Real alphabar_Tsf = _alpha_function->value(_stress_free_temp, p);
      Real alphabar = _alpha_function->value(current_temp, p);
      Real numerator = dalphabar_dT * (current_temp - _ref_temp) + alphabar;
      Real denominator = 1.0 + alphabar_Tsf * (_stress_free_temp - _ref_temp);
      if (denominator < small)
        mooseError("Denominator too small in thermal strain calculation");
      (*_current_instantaneous_thermal_expansion_coef)[_qp] = numerator / denominator;
    }
  }
  else
    (*_current_instantaneous_thermal_expansion_coef)[_qp] = _alpha;
}
