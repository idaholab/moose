//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SphericalGaussianMaterial.h"
#include "MooseMesh.h"
#include "MathUtils.h"
#include "GrainTrackerInterface.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>

registerMooseObject("PhaseFieldApp", SphericalGaussianMaterial);

InputParameters
SphericalGaussianMaterial::validParams()
{
  InputParameters params = ADMaterial::validParams();

  params.addClassDescription(
      "5D-gaussian-anisotropy material object for anisotropic grain growth. "
      "This material adds anisotropy to epsilon - also called kappa, m - also called mu, and L for "
      "epsilon model or adds anisotropy to gamma and L for gamma model");
  MooseEnum models("EPSILON GAMMA");
  params.addRequiredParam<MooseEnum>("model_type", models, "Epsilon or Gamma model?");
  params.addRequiredParam<FileName>("quaternion_file_name",
                                    "Name of the file containing orientation quaternions data");
  params.addRequiredParam<FileName>(
      "library_file_name",
      "Name of the file containing minima misorientations (minima library) data");
  params.addParam<MaterialPropertyName>(
      "epsilon_name", "kappa_op", "The name of the anisotropic epsilon");
  params.addParam<MaterialPropertyName>("m_name", "mu", "The name of the anisotropic m");
  params.addParam<MaterialPropertyName>("mob_L_name", "mob_L", "The name of the anisotropic L");
  params.addParam<MaterialPropertyName>("gamma_name", "gamma_asymm", "The name of isotropic gamma");
  params.addParam<Real>(
      "gamma_value",
      1.5,
      "Value of gamma used to specify an isotropic material propertie gamma (dimensionless)");
  params.addParam<Real>(
      "g_gamma_value", 0.471404, "g_gamma value associated to gamma_value (dimensionless)");
  params.addParam<Real>(
      "f_gamma_value", 0.124961, "f_gamma value associated to gamma_value (dimensionless)");
  params.addParam<Real>("grain_boundary_width", 40, "Grain boundary width (nm)");
  params.addParam<Real>("base_energy",
                        2.0,
                        "Base value of energy used to compute base value of epsilon from which "
                        "gaussian is either added or substracted (J/m^2)");
  params.addParam<Real>("base_gamma",
                        1.6,
                        "Base value of gamma used to specify an anisotropic material propertie "
                        "gamma (dimensionless)");
  params.addParam<Real>("base_g_gamma",
                        0.482570,
                        "Base value of g_gamma associated to base_gamma from which gaussian is "
                        "either added or substracted (dimensionless)");
  params.addParam<Real>(
      "base_f_gamma", 0.130925, "Base value of f_gamma associated to base_gamma (dimensionless)");
  params.addParam<Real>(
      "base_L",
      0.0007,
      "Base value of L from which gaussian is either added or substracted (nm^3/eV.ns)");
  params.addParam<Real>(
      "gaussian_tolerance", 1e-30, "Value used to control gaussian effect (dimensionless)");
  params.addParam<Real>(
      "axis_variance_range",
      3,
      "Acceptable range for axis variance for gaussian switch calculation (Radian)");
  params.addParam<Real>(
      "angle_variance_range",
      3,
      "Acceptable range for angle variance for gaussian switch calculation (Radian)");
  params.addParam<Real>("sharpness", 20, "Gaussian sharpness (dimensionless)");
  params.addParam<bool>("anisotropic_epsilon",
                        true,
                        "If true, use anisotropic epsilon; otherwise use isotropic epsilon");
  params.addParam<bool>(
      "anisotropic_m", true, "If true, use anisotropic m; otherwise use isotropic m");
  params.addParam<bool>(
      "anisotropic_gamma", true, "If true, use anisotropic gamma; otherwise use isotropic gamma");
  params.addParam<bool>(
      "anisotropic_L", true, "If true, use anisotropic L; otherwise use isotropic L");
  params.addParam<bool>("add_epsilon_gaussian",
                        true,
                        "If true, gaussian adds to the base value of epsilon, else subtract");
  params.addParam<bool>("add_g_gamma_gaussian",
                        true,
                        "If true, gaussian adds to the base value of g_gamma, else subtract");
  params.addParam<bool>(
      "add_L_gaussian", true, "If true, gaussian adds to the base value of L, else subtract");
  params.addParam<bool>(
      "compute_final_gaussian_direction",
      true,
      "If true, the final gaussian direction is computed using the orientation quaternions"
      "If false, it is taken directly from the minima library file");
  params.addParam<bool>(
      "set_bulk_to_base_values",
      true,
      "If true, the anisotropic material properties are at base values within the grains"
      "If false, they are at zero");
  params.addRequiredParam<int>(
      "library_misorientations_number",
      "Number of minima misorientations in the minima library file to use");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled order parameter variables");
  params.addRequiredParam<UserObjectName>("grain_tracker", "Grain tracker UserObject");
  params.addParam<Real>("length_scale", 1.0e-9, "Conversion for 1/m to 1/nm.");
  params.addParam<Real>("time_scale", 1.0e-9, "Time scale in s, where default is ns.");
  params.addParam<Real>("energy_scale", 6.24150974e18, "Joule to eV conversion.");
  return params;
}

SphericalGaussianMaterial::SphericalGaussianMaterial(const InputParameters & parameters)
  : ADMaterial(parameters),
    _model_type(getParam<MooseEnum>("model_type").getEnum<ModelType>()),
    _quaternion_file_name(getParam<FileName>("quaternion_file_name")),
    _library_file_name(getParam<FileName>("library_file_name")),
    _epsilon(declareADProperty<Real>(getParam<MaterialPropertyName>("epsilon_name"))),
    _m(declareADProperty<Real>(getParam<MaterialPropertyName>("m_name"))),
    _mob_L(declareADProperty<Real>(getParam<MaterialPropertyName>("mob_L_name"))),
    _gamma(declareADProperty<Real>(getParam<MaterialPropertyName>("gamma_name"))),
    _depsilon_plus(declareADProperty<RealGradient>("depsilon_plus")),
    _depsilon_minus(declareADProperty<RealGradient>("depsilon_minus")),
    _dm_plus(declareADProperty<RealGradient>("dm_plus")),
    _dm_minus(declareADProperty<RealGradient>("dm_minus")),
    _dgamma_plus(declareADProperty<RealGradient>("dgamma_plus")),
    _dgamma_minus(declareADProperty<RealGradient>("dgamma_minus")),
    _gamma_value(getParam<Real>("gamma_value")),
    _g_gamma_value(getParam<Real>("g_gamma_value")),
    _f_gamma_value(getParam<Real>("f_gamma_value")),
    _grain_boundary_width(getParam<Real>("grain_boundary_width")),
    _base_energy(getParam<Real>("base_energy")),
    _base_gamma(getParam<Real>("base_gamma")),
    _base_g_gamma(getParam<Real>("base_g_gamma")),
    _base_f_gamma(getParam<Real>("base_f_gamma")),
    _base_L(getParam<Real>("base_L")),
    _gaussian_tolerance(getParam<Real>("gaussian_tolerance")),
    _axis_variance_range(getParam<Real>("axis_variance_range")),
    _angle_variance_range(getParam<Real>("angle_variance_range")),
    _sharpness(getParam<Real>("sharpness")),
    _anisotropic_epsilon(getParam<bool>("anisotropic_epsilon")),
    _anisotropic_m(getParam<bool>("anisotropic_m")),
    _anisotropic_gamma(getParam<bool>("anisotropic_gamma")),
    _anisotropic_L(getParam<bool>("anisotropic_L")),
    _add_epsilon_gaussian(getParam<bool>("add_epsilon_gaussian")),
    _add_g_gamma_gaussian(getParam<bool>("add_g_gamma_gaussian")),
    _add_L_gaussian(getParam<bool>("add_L_gaussian")),
    _compute_final_gaussian_direction(getParam<bool>("compute_final_gaussian_direction")),
    _set_bulk_to_base_values(getParam<bool>("set_bulk_to_base_values")),
    _library_misorientations_number(getParam<int>("library_misorientations_number")),
    _op_num(coupledComponents("v")),
    _vals(adCoupledValues("v")),
    _grad_vals(adCoupledGradients("v")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker")),
    _length_scale(getParam<Real>("length_scale")),
    _time_scale(getParam<Real>("time_scale")),
    _energy_scale(getParam<Real>("energy_scale"))
{
  // Verify that the number of coupled order parameter variables is not 0
  if (_op_num == 0)
    paramError("op_num", "op_num must be greater than 0");

  // Read data files during initialization
  readQuaternionFile();
  readLibraryFile();
}

// Read and store grains orientation quaternions data from text file
void
SphericalGaussianMaterial::readQuaternionFile()
{
  std::ifstream File1(_quaternion_file_name.c_str());
  // Account for opening error
  if (!File1.is_open())
    mooseError("Unable to open quaternion file");

  Real quat0, quat1, quat2, quat3;
  while (File1 >> quat0 >> quat1 >> quat2 >> quat3)
  {
    // Calculate the quaternion norm
    ADReal qcomp0 = quat0;
    ADReal qcomp1 = quat1;
    ADReal qcomp2 = quat2;
    ADReal qcomp3 = quat3;
    ADReal qcomp_norm =
        std::sqrt(qcomp0 * qcomp0 + qcomp1 * qcomp1 + qcomp2 * qcomp2 + qcomp3 * qcomp3);
    // Normalize the quaternion components
    ADReal qcomp0_normalized = qcomp0 / qcomp_norm;
    ADReal qcomp1_normalized = qcomp1 / qcomp_norm;
    ADReal qcomp2_normalized = qcomp2 / qcomp_norm;
    ADReal qcomp3_normalized = qcomp3 / qcomp_norm;
    // Store the normalized quaternion components
    _q0.push_back(qcomp0_normalized);
    _q1.push_back(qcomp1_normalized);
    _q2.push_back(qcomp2_normalized);
    _q3.push_back(qcomp3_normalized);
  }

  File1.close();
}

// Read and store minima misorientations (minima library) data from text file
void
SphericalGaussianMaterial::readLibraryFile()
{
  std::ifstream File2(_library_file_name.c_str());
  // Account for opening error
  if (!File2.is_open())
    mooseError("Unable to open library file");

  Real lib1, lib2, lib3, lib4, lib5, lib6, lib7, lib8, lib9;
  while (File2 >> lib1 >> lib2 >> lib3 >> lib4 >> lib5 >> lib6 >> lib7 >> lib8 >> lib9)
  {
    // Create vectors and normalize: Axis of rotation and Gaussian direction for library
    // misorientation
    ADRealVectorValue library_rotation_axis_value(lib1, lib2, lib3);
    ADRealVectorValue library_gaussian_direction_value(lib5, lib6, lib7);
    library_rotation_axis_value = library_rotation_axis_value.unit();
    library_gaussian_direction_value = library_gaussian_direction_value.unit();
    // Verify that grain boundary energy and pre-determined value of L for library misorientation
    // are positive
    if (lib8 <= 0)
      mooseError("library_minima_energy must be positive. Found value: ", lib8);
    if (lib9 <= 0)
      mooseError("library_minima_L must be positive. Found value: ", lib9);
    // Store the values in sequence
    _library_rotation_axis.push_back(library_rotation_axis_value);
    _library_rotation_angle.push_back(lib4);
    _library_gaussian_direction.push_back(library_gaussian_direction_value);
    _library_minima_energy.push_back(lib8);
    _library_minima_L.push_back(lib9);
  }

  File2.close();
}

// Compute material properties and derivatives
void
SphericalGaussianMaterial::computeQpProperties()
{
  // Determine number of grains
  _grain_num = _grain_tracker.getTotalFeatureCount();
  // Determine order parameters to grain index
  const auto & op_to_grains = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  // Initialize values
  ADReal aniso_epsilon = 0.0;
  ADRealVectorValue anisoplus_epsilon(0.0, 0.0, 0.0);
  ADRealVectorValue anisominus_epsilon(0.0, 0.0, 0.0);
  ADReal sum_epsilon = 0.0;
  ADRealVectorValue sum_epsilon_plus(0.0, 0.0, 0.0);
  ADRealVectorValue sum_epsilon_minus(0.0, 0.0, 0.0);
  ADRealVectorValue finalgaussianplus_epsilon(0.0, 0.0, 0.0);
  ADRealVectorValue finalgaussianminus_epsilon(0.0, 0.0, 0.0);
  // Compute base value of epsilon from base value of energy
  const ADReal epsilon_base = 3.0 / 4.0 * _base_energy * _energy_scale * _length_scale *
                              _length_scale * _grain_boundary_width;

  ADReal aniso_g_gamma = 0.0;
  ADRealVectorValue anisoplus_g_gamma(0.0, 0.0, 0.0);
  ADRealVectorValue anisominus_g_gamma(0.0, 0.0, 0.0);
  ADReal sum_g_gamma = 0.0;
  ADRealVectorValue sum_g_gamma_plus(0.0, 0.0, 0.0);
  ADRealVectorValue sum_g_gamma_minus(0.0, 0.0, 0.0);
  ADRealVectorValue finalgaussianplus_g_gamma(0.0, 0.0, 0.0);
  ADRealVectorValue finalgaussianminus_g_gamma(0.0, 0.0, 0.0);
  ADReal g_gamma = 0.0;
  ADRealVectorValue dg_gamma_plus(0.0, 0.0, 0.0);
  ADRealVectorValue dg_gamma_minus(0.0, 0.0, 0.0);

  // Compute isotropic values of epsilon and m
  const ADReal converted_base_energy = _base_energy * _energy_scale * _length_scale * _length_scale;
  const ADReal m_value = std::sqrt((converted_base_energy * converted_base_energy) /
                                   (_grain_boundary_width * _grain_boundary_width * _base_f_gamma *
                                    _base_g_gamma * _base_g_gamma));
  const ADReal epsilon_value =
      (converted_base_energy * converted_base_energy) / (_base_g_gamma * _base_g_gamma * m_value);

  ADReal aniso_L = 0.0;
  ADReal sum_L = 0.0;

  ADRealVectorValue gaussianplus(0.0, 0.0, 0.0);
  ADRealVectorValue gaussianminus(0.0, 0.0, 0.0);
  ADReal continuity_term = 0.0;
  ADReal parts_continuity_term = 0.0;
  ADReal sum_continuity_term = 0.0;

  // Term used to combine all the individually computed anisotropic properties between each pair of
  // grains into single continuous functions
  for (std::size_t m : make_range(std::size_t(0), _op_num - 1))
    for (std::size_t n : make_range(m + 1, _op_num))
    {
      parts_continuity_term = (100000.0 * ((*_vals[m])[_qp]) * ((*_vals[m])[_qp]) + 0.01) *
                              (100000.0 * ((*_vals[n])[_qp]) * ((*_vals[n])[_qp]) + 0.01);
      sum_continuity_term += parts_continuity_term;
    }

  for (std::size_t b : make_range(std::size_t(0), _grain_num - 1))
    for (std::size_t m : make_range(std::size_t(0), _op_num - 1))
      for (std::size_t a : make_range(b + 1, _grain_num))
        for (std::size_t n : make_range(m + 1, _op_num))
          // Compute individual anisotropic properties between each pairs of grains
          if (b == op_to_grains[m])
            if (a == op_to_grains[n])
            {
              // Reset the values for new pairs of grains
              aniso_epsilon = epsilon_base;
              anisoplus_epsilon = {0.0, 0.0, 0.0};
              anisominus_epsilon = {0.0, 0.0, 0.0};

              aniso_g_gamma = _base_g_gamma;
              anisoplus_g_gamma = {0.0, 0.0, 0.0};
              anisominus_g_gamma = {0.0, 0.0, 0.0};

              aniso_L = _base_L;

              // Compute normalized gradient vector of the order parameters that specifies the
              // orientation of the grain boundary normal between grains A and B in the current
              // simulation domain
              continuity_term = (100000.0 * ((*_vals[m])[_qp]) * ((*_vals[m])[_qp]) + 0.01) *
                                (100000.0 * ((*_vals[n])[_qp]) * ((*_vals[n])[_qp]) + 0.01);
              ADRealVectorValue simulation_boundary_normal =
                  (*_grad_vals[m])[_qp] - (*_grad_vals[n])[_qp];
              ADReal normvsmallbavalue = simulation_boundary_normal.norm_sq();
              ADReal normvsmallba = simulation_boundary_normal.norm();
              ADRealVectorValue normalized_simulation_boundary_normal =
                  simulation_boundary_normal.unit();

              // Compute the simulation normalized misorientation quaternion between the two grains
              // using the orientation quaternions from the text file
              const ADReal qba0_init = (_q0[a] * _q0[b]) - (_q1[a] * -1.0 * _q1[b]) -
                                       (_q2[a] * -1.0 * _q2[b]) - (_q3[a] * -1.0 * _q3[b]);
              const ADReal qba1_init = (_q0[a] * _q1[b]) + (_q1[a] * -1.0 * _q0[b]) +
                                       (_q2[a] * -1.0 * _q3[b]) - (_q3[a] * -1.0 * _q2[b]);
              const ADReal qba2_init = (_q0[a] * _q2[b]) - (_q1[a] * -1.0 * _q3[b]) +
                                       (_q2[a] * -1.0 * _q0[b]) + (_q3[a] * -1.0 * _q1[b]);
              const ADReal qba3_init = (_q0[a] * _q3[b]) + (_q1[a] * -1.0 * _q2[b]) -
                                       (_q2[a] * -1.0 * _q1[b]) + (_q3[a] * -1.0 * _q0[b]);
              const ADReal norm_qba = std::sqrt(qba0_init * qba0_init + qba1_init * qba1_init +
                                                qba2_init * qba2_init + qba3_init * qba3_init);
              const ADReal qba0 = qba0_init / norm_qba;
              const ADReal qba1 = qba1_init / norm_qba;
              const ADReal qba2 = qba2_init / norm_qba;
              const ADReal qba3 = qba3_init / norm_qba;

              // Convert simulation normalized misorientation quaternion to simulation normalized
              // axis angle notation
              const ADReal simulation_rotation_angle = 2.0 * std::acos(qba0);
              ADRealVectorValue simulation_rotation_axis(
                  qba1 / std::sin(simulation_rotation_angle / 2.0),
                  qba2 / std::sin(simulation_rotation_angle / 2.0),
                  qba3 / std::sin(simulation_rotation_angle / 2.0));
              simulation_rotation_axis = simulation_rotation_axis.unit();

              // Loop through the stored minima library data to compute gaussian for each minima
              // library misorientation
              for (std::size_t l : make_range(std::size_t(0), _library_misorientations_number))
              {
                // Compute the gaussian switch by comparing the axis angle components of the minima
                // library misorientation to the simulation misorientation
                if (_library_rotation_angle[l] < 0.0)
                  _library_rotation_angle[l] =
                      (2 * libMesh::pi) +
                      _library_rotation_angle
                          [l]; // _library_rotation_angle is in radians. (2 * libMesh::pi) radians
                               // is equivalent to 360 degrees. If the angle is negative, this
                               // converts it to its equivalent positive angle. -60deg around <111>
                               // = (360-60 = 300deg) around <111>.
                const ADReal thetadtheta = simulation_rotation_angle - _library_rotation_angle[l];
                const ADReal normvba = simulation_rotation_axis.norm();
                _library_rotation_axis[l] = _library_rotation_axis[l].unit();
                ADReal library_rotation_axis_norm = _library_rotation_axis[l].norm();
                const ADReal cosinevalue1 = _library_rotation_axis[l] * simulation_rotation_axis;
                const ADReal cosinevalue2 = library_rotation_axis_norm * normvba;
                const ADReal cosinevalue3 = cosinevalue1 / cosinevalue2;
                const ADReal thetadv = std::acos(cosinevalue3);
                const ADReal switchvalue =
                    -100.0 *
                    ((thetadv / _axis_variance_range) * (thetadv / _axis_variance_range) +
                     (thetadtheta / _angle_variance_range) * (thetadtheta / _angle_variance_range));
                ADReal exp_switch_value = std::exp(switchvalue);
                ADReal s_switch = exp_switch_value;
                if (exp_switch_value < 0.001) // 0.001 is an arbitrary threshold below which the
                                              // switch is assumed negligible.
                  s_switch = 0.0;

                // Retrive and assign the library gaussian direction for the specific minima library
                // misorientation
                _library_gaussian_direction[l] = _library_gaussian_direction[l].unit();
                // Determine whether the library gaussian direction needs to be rotated into the
                // simulation reference frame using quaternion orientations or left as such
                ADRealVectorValue final_gaussian_direction(0.0, 0.0, 0.0);
                if (_compute_final_gaussian_direction)
                {
                  const ADReal miuo0 = (_q0[a] * 0.0) -
                                       (_q1[a] * _library_gaussian_direction[l](0)) -
                                       (_q2[a] * _library_gaussian_direction[l](1)) -
                                       (_q3[a] * _library_gaussian_direction[l](2));
                  const ADReal miuo1 = (_q0[a] * _library_gaussian_direction[l](0)) +
                                       (_q1[a] * 0.0) +
                                       (_q2[a] * _library_gaussian_direction[l](2)) -
                                       (_q3[a] * _library_gaussian_direction[l](1));
                  const ADReal miuo2 = (_q0[a] * _library_gaussian_direction[l](1)) -
                                       (_q1[a] * _library_gaussian_direction[l](2)) +
                                       (_q2[a] * 0.0) +
                                       (_q3[a] * _library_gaussian_direction[l](0));
                  const ADReal miuo3 = (_q0[a] * _library_gaussian_direction[l](2)) +
                                       (_q1[a] * _library_gaussian_direction[l](1)) -
                                       (_q2[a] * _library_gaussian_direction[l](0)) +
                                       (_q3[a] * 0.0);
                  final_gaussian_direction(0) = miuo0 * _q1[a] * -1.0 + miuo1 * _q0[a] +
                                                miuo2 * _q3[a] * -1.0 - miuo3 * _q2[a] * -1.0;
                  final_gaussian_direction(1) = miuo0 * _q2[a] * -1.0 - miuo1 * _q3[a] * -1.0 +
                                                miuo2 * _q0[a] + miuo3 * _q1[a] * -1.0;
                  final_gaussian_direction(2) = miuo0 * _q3[a] * -1.0 + miuo1 * _q2[a] * -1.0 -
                                                miuo2 * _q1[a] * -1.0 + miuo3 * _q0[a];
                }
                else
                {
                  final_gaussian_direction = _library_gaussian_direction[l];
                }
                // Set values at 0.0 within the grains
                if (sum_continuity_term == 0.0)
                {
                  final_gaussian_direction = 0.0;
                }

                // Compute the gaussian and its derivatives by adding or substracting adequate
                // values from base values based on the gaussian switch and directions
                // at/surrounding library gaussian direction
                const ADReal dot_product =
                    final_gaussian_direction * normalized_simulation_boundary_normal;
                const ADReal exponent = _sharpness * (dot_product - 1.0);
                const ADReal tol = _gaussian_tolerance;
                if (std::abs(normvsmallba) > tol)
                {
                  const ADReal s_switch_value = s_switch;
                  const ADReal exp_value = std::exp(exponent);
                  gaussianplus = (_sharpness / normvsmallba) *
                                 (final_gaussian_direction -
                                  (dot_product * normalized_simulation_boundary_normal));
                  gaussianminus = (-1) * (_sharpness / normvsmallba) *
                                  (final_gaussian_direction -
                                   (dot_product * normalized_simulation_boundary_normal));

                  // Update values with computed gaussian and associated derivatives
                  if (_model_type == ModelType::EPSILON) // If model_type is "EPSILON"
                  {
                    const ADReal amplitudes_epsilon = std::abs(
                        (_base_energy - _library_minima_energy[l]) * _grain_boundary_width / 1.0e9 *
                        std::sqrt(_f_gamma_value) / _g_gamma_value);
                    const ADReal common_epsilon = s_switch_value * exp_value * amplitudes_epsilon *
                                                  _energy_scale * _length_scale;
                    const ADReal finalgaussian_epsilon =
                        _add_epsilon_gaussian ? common_epsilon : -common_epsilon;
                    finalgaussianplus_epsilon = _add_epsilon_gaussian
                                                    ? (gaussianplus * common_epsilon)
                                                    : -(gaussianplus * common_epsilon);
                    finalgaussianminus_epsilon = _add_epsilon_gaussian
                                                     ? (gaussianminus * common_epsilon)
                                                     : -(gaussianminus * common_epsilon);
                    aniso_epsilon += finalgaussian_epsilon;
                    anisoplus_epsilon += finalgaussianplus_epsilon;
                    anisominus_epsilon += finalgaussianminus_epsilon;
                  }
                  else if (_model_type == ModelType::GAMMA) // If model_type is "GAMMA"
                  {
                    const ADReal library_minima_g_gamma =
                        (_library_minima_energy[l] * _energy_scale * _length_scale *
                         _length_scale) /
                        (std::sqrt(epsilon_value * m_value));
                    const ADReal amplitudes_g_gamma =
                        std::abs(_base_g_gamma - library_minima_g_gamma);
                    const ADReal common_g_gamma = s_switch_value * exp_value * amplitudes_g_gamma;
                    const ADReal finalgaussian_g_gamma =
                        _add_g_gamma_gaussian ? common_g_gamma : -common_g_gamma;
                    finalgaussianplus_g_gamma = _add_g_gamma_gaussian
                                                    ? (gaussianplus * common_g_gamma)
                                                    : -(gaussianplus * common_g_gamma);
                    finalgaussianminus_g_gamma = _add_g_gamma_gaussian
                                                     ? (gaussianminus * common_g_gamma)
                                                     : -(gaussianminus * common_g_gamma);
                    aniso_g_gamma += finalgaussian_g_gamma;
                    anisoplus_g_gamma += finalgaussianplus_g_gamma;
                    anisominus_g_gamma += finalgaussianminus_g_gamma;
                  }
                  else
                  {
                    mooseError(
                        "Invalid model_type value"); // Handle unexpected values of _model_type
                  }
                  const ADReal amplitudes_L = std::abs(_base_L - _library_minima_L[l]);
                  const ADReal gaussian_L = amplitudes_L * s_switch_value * exp_value;
                  const ADReal finalgaussian_L = _add_L_gaussian ? gaussian_L : -gaussian_L;
                  aniso_L += finalgaussian_L;
                }
              }
              // Sum up all gaussians and associated derivatives after looping through all the
              // stored minima library data to make a continuous function
              sum_epsilon += aniso_epsilon * continuity_term;
              sum_epsilon_plus += anisoplus_epsilon * continuity_term;
              sum_epsilon_minus += anisominus_epsilon * continuity_term;

              sum_g_gamma += aniso_g_gamma * continuity_term;
              sum_g_gamma_plus += anisoplus_g_gamma * continuity_term;
              sum_g_gamma_minus += anisominus_g_gamma * continuity_term;

              sum_L += aniso_L * continuity_term;
            }

  // Compute final material properties
  if (_model_type == ModelType::EPSILON) // If model_type is "EPSILON"
  {
    // Compute continuous epsilon, m and L and associated derivatives at grain boundaries by
    // specifying anisotropy or isotropy
    if (_anisotropic_epsilon)
    {
      // Anisotropic epsilon
      _epsilon[_qp] = std::abs(sum_epsilon / sum_continuity_term);
      _depsilon_plus[_qp] = sum_epsilon_plus / sum_continuity_term;
      _depsilon_minus[_qp] = sum_epsilon_minus / sum_continuity_term;
    }
    else
    {
      // Isotropic epsilon
      _epsilon[_qp] = epsilon_base;
      _depsilon_plus[_qp] = {0.0, 0.0, 0.0};
      _depsilon_minus[_qp] = {0.0, 0.0, 0.0};
    }
    if (_anisotropic_m)
    {
      // Anisotropic m
      _m[_qp] = _epsilon[_qp] / (_grain_boundary_width * _grain_boundary_width) / _f_gamma_value;
      _dm_plus[_qp] =
          _depsilon_plus[_qp] / (_grain_boundary_width * _grain_boundary_width * _f_gamma_value);
      _dm_minus[_qp] =
          _depsilon_minus[_qp] / (_grain_boundary_width * _grain_boundary_width * _f_gamma_value);
    }
    else
    {
      // Isotropic m
      _m[_qp] = epsilon_base / (_grain_boundary_width * _grain_boundary_width) / _f_gamma_value;
      _dm_plus[_qp] = {0.0, 0.0, 0.0};
      _dm_minus[_qp] = {0.0, 0.0, 0.0};
    }
    if (_anisotropic_L)
    {
      // Anisotropic L
      _mob_L[_qp] = std::abs(sum_L / sum_continuity_term);
    }
    else
    {
      // Isotropic L
      _mob_L[_qp] = _base_L;
    }

    // Set values of epsilon, m and L at base values within the grains
    if (_set_bulk_to_base_values)
    {
      if (_epsilon[_qp] == 0.0)
      {
        _epsilon[_qp] = epsilon_base;
        _m[_qp] = epsilon_base / (_grain_boundary_width * _grain_boundary_width) / _f_gamma_value;
        _mob_L[_qp] = _base_L;
      }
    }

    // Compute isotropic gamma
    _gamma[_qp] = _gamma_value;
  }
  else if (_model_type == ModelType::GAMMA) // If model_type is "GAMMA"
  {
    // Compute continuous gamma and L and associated derivatives at grain boundaries by specifying
    // anisotropy or isotropy
    g_gamma = std::abs(sum_g_gamma / sum_continuity_term);
    dg_gamma_plus = sum_g_gamma_plus / sum_continuity_term;
    dg_gamma_minus = sum_g_gamma_minus / sum_continuity_term;
    if (_anisotropic_gamma)
    {
      // Anisotropic gamma
      const ADReal g2 = g_gamma * g_gamma;
      const ADReal p = -(3.0944 * (g2 * g2 * g2 * g2)) - (1.8169 * (g2 * g2 * g2)) +
                       (10.323 * (g2 * g2)) - (8.1819 * (g2)) + 2.0033;
      const ADReal dpdg2 = -((3.0944 * 4.0) * (g2 * g2 * g2)) - ((1.8169 * 3.0) * (g2 * g2)) +
                           ((10.323 * 2.0) * (g2)) - 8.1819;
      _gamma[_qp] = 1.0 / p;
      _dgamma_plus[_qp] = -2.0 * g_gamma * _gamma[_qp] * _gamma[_qp] * dpdg2 * dg_gamma_plus;
      _dgamma_minus[_qp] = -2.0 * g_gamma * _gamma[_qp] * _gamma[_qp] * dpdg2 * dg_gamma_minus;
    }
    else
    {
      // Isotropic gamma
      _gamma[_qp] = _base_gamma;
      _dgamma_plus[_qp] = {0.0, 0.0, 0.0};
      _dgamma_minus[_qp] = {0.0, 0.0, 0.0};
    }
    if (_anisotropic_L)
    {
      // Anisotropic L
      _mob_L[_qp] = std::abs(sum_L / sum_continuity_term);
    }
    else
    {
      // Isotropic L
      _mob_L[_qp] = _base_L;
    }

    // Set values of gamma and L at base values within the grains
    if (_set_bulk_to_base_values)
    {
      if (g_gamma == 0.0)
      {
        _gamma[_qp] = _base_gamma;
        _mob_L[_qp] = _base_L;
      }
    }

    // Compute isotropic epsilon and m
    _epsilon[_qp] = epsilon_value;
    _m[_qp] = m_value;
  }
  else
  {
    mooseError("Invalid model_type value"); // Handle unexpected values of _model_type
  }
}
