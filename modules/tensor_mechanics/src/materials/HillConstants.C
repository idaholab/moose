//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HillConstants.h"

registerMooseObject("TensorMechanicsApp", HillConstants);

InputParameters
HillConstants::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addClassDescription("Build and rotate the Hill Tensor. It can be used with other Hill "
                             "plasticity and creep materials.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addRequiredRangeCheckedParam<std::vector<Real>>("hill_constants",
                                                         "hill_constants_size = 6",
                                                         "Hill material constants in order: F, "
                                                         "G, H, L, M, N");
  params.addParam<RealVectorValue>("rotation_angles",
                                   "Provide the rotation angles for the transformation matrix. "
                                   "This should be a vector that provides "
                                   "the rotation angles about z-, y-, and x-axis, respectively.");
  params.addParam<std::vector<FunctionName>>(
      "function_names",
      "A set of functions that describe the evolution of anisotropy with temperature");
  params.addParam<bool>(
      "use_large_rotation",
      false,
      "Whether to rotate the Hill tensor (anisotropic parameters) to account for large kinematic "
      "rotations. It's recommended to set it to true if large displacements are to be expected.");
  params.addCoupledVar("temperature", "Coupled temperature");
  return params;
}

HillConstants::HillConstants(const InputParameters & parameters)
  : ADMaterial(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _use_large_rotation(getParam<bool>("use_large_rotation")),
    _rotation_total_hill(_use_large_rotation
                             ? &declareADProperty<RankTwoTensor>(_base_name + "rotation_total_hill")
                             : nullptr),
    _rotation_total_hill_old(_use_large_rotation ? &getMaterialPropertyOldByName<RankTwoTensor>(
                                                       _base_name + "rotation_total_hill")
                                                 : nullptr),
    _rotation_increment(_use_large_rotation ? &getADMaterialProperty<RankTwoTensor>(
                                                  _base_name + "rotation_increment")
                                            : nullptr),
    _hill_constants_input(6),
    _hill_constants(6),
    _hill_constant_material(declareProperty<std::vector<Real>>(_base_name + "hill_constants")),
    _zyx_angles(isParamValid("rotation_angles") ? getParam<RealVectorValue>("rotation_angles")
                                                : RealVectorValue(0.0, 0.0, 0.0)),
    _transformation_tensor(6, 6),
    _has_temp(isParamValid("temperature")),
    _temperature(_has_temp ? coupledValue("temperature") : _zero),
    _function_names(getParam<std::vector<FunctionName>>("function_names")),
    _num_functions(_function_names.size()),
    _functions(_num_functions)
{
  _hill_constants_input = getParam<std::vector<Real>>("hill_constants");
  if (_has_temp && _num_functions != 6)
    paramError("function_names",
               "Six functions need to be provided to determine the evolution of Hill's "
               "coefficients F, G, H, L, M, and N, when temperature dependency is selected.");

  for (unsigned int i = 0; i < _num_functions; i++)
  {
    _functions[i] = &getFunctionByName(_function_names[i]);
    if (_functions[i] == nullptr)
      paramError("function_names", "Function names provided cannot retrieve a function.");
  }

  if (!_has_temp && !_use_large_rotation)
    rotateHillConstants(_hill_constants_input);

  if (_use_large_rotation && isParamValid("rotation_angles"))
    paramError(
        "rotation_angles",
        "Use of both updates of anisotropic coefficients through rotation_angles (initial rigid "
        "body rotation) and finite strain rotation kinematics are not supported at this time.");
}

void
HillConstants::initQpStatefulProperties()
{
  if (_use_large_rotation)
  {
    RankTwoTensor identity_rotation(RankTwoTensor::initIdentity);
    (*_rotation_total_hill)[_qp] = identity_rotation;
  }
}

void
HillConstants::computeQpProperties()
{
  // Account for finite strain rotation influence on anisotropic coefficients
  if (_use_large_rotation)
  {
    (*_rotation_total_hill)[_qp] = (*_rotation_increment)[_qp] * (*_rotation_total_hill_old)[_qp];
    std::array<Real, 3> angles_zyx =
        computeZYXAngles(MetaPhysicL::raw_value((*_rotation_total_hill)[_qp]));
    _zyx_angles(0) = angles_zyx[0] / libMesh::pi * 180.0;
    _zyx_angles(1) = angles_zyx[1] / libMesh::pi * 180.0;
    _zyx_angles(2) = angles_zyx[2] / libMesh::pi * 180.0;

    Moose::out << "Z angle: " << _zyx_angles(0) << "\n";
    Moose::out << "Y angle: " << _zyx_angles(1) << "\n";
    Moose::out << "X angle: " << _zyx_angles(2) << "\n";
    // Make sure to provide the original coefficients to the orientation transformation
    _hill_constant_material[_qp].resize(6);
    _hill_constant_material[_qp] = _hill_constants_input;
  }

  // Account for temperature dependency
  if (_has_temp)
  {
    _hill_constant_material[_qp].resize(6);

    const Point p;
    for (unsigned int i = 0; i < 6; i++)
      _hill_constant_material[_qp][i] =
          _functions[i]->value(MetaPhysicL::raw_value(_temperature[_qp]), p);
  }

  // We need to update the coefficients whether we use temperature dependency or large rotation
  // kinematics (or both)
  if (_has_temp || _use_large_rotation)
    rotateHillConstants(_hill_constant_material[_qp]);

  // Update material coefficients whether or not they are temperature-dependent
  _hill_constant_material[_qp] = _hill_constants;
}

std::array<Real, 3>
HillConstants::computeZYXAngles(const RankTwoTensor & rotation_matrix)
{
  std::array<Real, 3> zyx_array;

  //  const double rotation_00 = rotation_matrix(0, 0);
  //  const double rotation_10 = rotation_matrix(1, 0);
  //
  //  if (std::abs(rotation_00) == 0.0 && std::abs(rotation_10) == 0.0)
  //  {
  //    // z angle
  //    zyx_array[0] = std::atan2(rotation_matrix(0, 1), rotation_matrix(1, 1));
  //    // y angle
  //    zyx_array[1] = libMesh::pi / 2.0;
  //    // x angle
  //    zyx_array[2] = 0.0;
  //  }
  //  else
  //  {
  //    // z angle
  //    zyx_array[0] = std::atan2(rotation_matrix(2, 1), rotation_matrix(2, 2));
  //    // y angle
  //    zyx_array[1] = std::atan2(-rotation_matrix(2, 0),
  //                              std::sqrt(rotation_matrix(0, 0) * rotation_matrix(0, 0) +
  //                                        rotation_matrix(1, 0) * rotation_matrix(1, 0)));
  //    // x angle
  //    zyx_array[2] = std::atan2(rotation_matrix(1, 0), rotation_matrix(0, 0));
  //  }

  //  const double rotation_02 = rotation_matrix(0, 2);
  //  if (rotation_02 < 1.0)
  //  {
  //    if (rotation_02 > -1.0)
  //    {
  //      // z angle
  //      zyx_array[0] = std::atan2(-rotation_matrix(0, 1), rotation_matrix(0, 0));
  //      // y angle
  //      zyx_array[1] = std::asin(rotation_matrix(0, 2));
  //      // x angle
  //      zyx_array[2] = std::atan2(-rotation_matrix(1, 2), rotation_matrix(2, 2));
  //    }
  //    else
  //    {
  //      // z angle
  //      zyx_array[0] = 0.0;
  //      // y angle
  //      zyx_array[1] = -libMesh::pi / 2.0;
  //      // x angle
  //      zyx_array[2] = -std::atan2(rotation_matrix(1, 0), rotation_matrix(1, 1));
  //    }
  //  }
  //  else
  //  {
  //    // z angle
  //    zyx_array[0] = 0.0;
  //    // y angle
  //    zyx_array[1] = libMesh::pi / 2.0;
  //    // x angle
  //    zyx_array[2] = std::atan2(rotation_matrix(1, 0), rotation_matrix(1, 1));
  //  }

  const double rotation_20 = rotation_matrix(2, 0);
  if (rotation_20 < 1.0)
  {
    if (rotation_20 > -1.0)
    {
      // z angle
      zyx_array[0] = std::atan2(rotation_matrix(1, 0), rotation_matrix(0, 0));
      // y angle
      zyx_array[1] = std::asin(-rotation_matrix(2, 0));
      // x angle
      zyx_array[2] = std::atan2(rotation_matrix(2, 1), rotation_matrix(2, 2));
    }
    else
    {
      // z angle
      zyx_array[0] = -std::atan2(-rotation_matrix(1, 2), rotation_matrix(1, 1));
      // y angle
      zyx_array[1] = libMesh::pi / 2.0;
      // x angle
      zyx_array[2] = 0.0;
    }
  }
  else
  {
    // z angle
    zyx_array[0] = std::atan2(-rotation_matrix(1, 2), rotation_matrix(1, 1));
    // y angle
    zyx_array[1] = -libMesh::pi / 2.0;
    // x angle
    zyx_array[2] = 0.0;
  }

  if (std::abs(zyx_array[1] - libMesh::pi / 2.0) < TOLERANCE * TOLERANCE)
    mooseDoOnce(mooseWarning("Euler angles used to define rotation of anisotropic parameters face "
                             "a singularity. The definition of Hill coefficients F, G, H, L, M, "
                             "and N may not be accurate. This message is printed once."));

  return zyx_array;
}

void
HillConstants::rotateHillConstants(const std::vector<Real> & hill_constants_input)
{
  const Real sz = std::sin(_zyx_angles(0) * libMesh::pi / 180.0);
  const Real cz = std::cos(_zyx_angles(0) * libMesh::pi / 180.0);

  const Real sy = std::sin(_zyx_angles(1) * libMesh::pi / 180.0);
  const Real cy = std::cos(_zyx_angles(1) * libMesh::pi / 180.0);

  const Real sx = std::sin(_zyx_angles(2) * libMesh::pi / 180.0);
  const Real cx = std::cos(_zyx_angles(2) * libMesh::pi / 180.0);

  //  Moose::out << "Inside: _zyx_angles(0) Z: " << _zyx_angles(0) << "\n";
  //  Moose::out << "Inside: _zyx_angles(1) Y: " << _zyx_angles(1) << "\n";
  //  Moose::out << "Inside: _zyx_angles(2) X: " << _zyx_angles(2) << "\n";

  // transformation matrix is formed by performing the ZYX rotation
  _transformation_tensor(0, 0) = cy * cy * cz * cz;
  _transformation_tensor(0, 1) = sz * sz * cy * cy;
  _transformation_tensor(0, 2) = sy * sy;
  _transformation_tensor(0, 3) = -2.0 * sy * sz * cy;
  _transformation_tensor(0, 4) = 2.0 * sy * cy * cz;
  _transformation_tensor(0, 5) = 2.0 * sz * cy * cy * cz;

  _transformation_tensor(1, 0) =
      sx * sx * sy * sy * cz * cz + 2.0 * sx * sy * sz * cx * cz + sz * sz * cx * cx;
  _transformation_tensor(1, 1) =
      sx * sx * sz * sz * sy * sy - 2.0 * sx * sy * sz * cx * cz + cx * cx * cz * cz;
  _transformation_tensor(1, 2) = sx * sx * cy * cy;
  _transformation_tensor(1, 3) = 2.0 * sx * sx * sz * sy * cy + 2.0 * sx * cx * cy * cz;
  _transformation_tensor(1, 4) = -2.0 * sx * sx * sy * cy * cz + 2.0 * sx * sz * cx * cz;
  _transformation_tensor(1, 5) = -2.0 * (-sz * sz + cz * cz) * sx * sy * cx +
                                 2.0 * sx * sx * sy * sy * sz * cz - 2.0 * sz * cx * cx * cz;

  _transformation_tensor(2, 0) =
      sx * sx * sz * sz - 2.0 * sx * sy * sz * cx * cz + sy * sy * cx * cx * cz * cz;
  _transformation_tensor(2, 1) =
      sx * sx * cz * cz + 2.0 * sx * sy * sz * cx * cz + sy * sy * sz * sz * cx * cx;
  _transformation_tensor(2, 2) = cx * cx * cy * cy;
  _transformation_tensor(2, 3) = -2.0 * sx * cx * cy * cz + 2.0 * sy * sz * cx * cx * cy;
  _transformation_tensor(2, 4) = -2.0 * sx * sz * cx * cy - 2.0 * sy * cx * cx * cy * cz;
  _transformation_tensor(2, 5) = 2.0 * (-sz * sz + cz * cz) * sx * sy * cx -
                                 2.0 * sx * sx * sz * cz + 2.0 * sy * sy * sz * cx * cx * cz;

  _transformation_tensor(3, 0) =
      (-sx * sx + cx * cx) * sy * sz * cz + sx * sy * sy * cx * cz * cz - sx * sz * sz * cx;
  _transformation_tensor(3, 1) =
      -(-sx * sx + cx * cx) * sy * sz * cz + sx * sy * sy * sz * sz * cx - sx * cx * cz * cz;
  _transformation_tensor(3, 2) = sx * cx * cy * cy;
  _transformation_tensor(3, 3) = (-sx * sx + cx * cx) * cy * cz + 2.0 * sx * sy * sz * cx * cy;
  _transformation_tensor(3, 4) = (-sx * sx + cx * cx) * sz * cy - 2.0 * sx * sy * cx * cy * cz;
  _transformation_tensor(3, 5) = -(-sx * sx + cx * cx) * (-sz * sz + cz * cz) * sy +
                                 2.0 * sx * sy * sy * sz * cx * cz + 2.0 * sx * sz * cx * cz;

  _transformation_tensor(4, 0) = sx * sz * cy * cz - sy * cx * cy * cz * cz;
  _transformation_tensor(4, 1) = -sx * sz * cy * cz - sy * sz * sz * cx * cy;
  _transformation_tensor(4, 2) = sy * cx * cy;
  _transformation_tensor(4, 3) = -(-sy * sy + cy * cy) * sz * cx - sx * sy * cz;
  _transformation_tensor(4, 4) = (-sy * sy + cy * cy) * cx * cz - sx * sy * sz;
  _transformation_tensor(4, 5) = -(-sz * sz + cz * cz) * sx * cy - 2.0 * sy * sz * cx * cy * cz;

  _transformation_tensor(5, 0) = -sx * sy * cy * cz * cz - sz * cx * cy * cz;
  _transformation_tensor(5, 1) = -sx * sy * sz * sz * cy + sz * cx * cy * cz;
  _transformation_tensor(5, 2) = sx * sy * cy;
  _transformation_tensor(5, 3) = -(-sy * sy + cy * cy) * sx * sz + sy * cx * cz;
  _transformation_tensor(5, 4) = (-sy * sy + cy * cy) * sx * cz + sy * sz * cx;
  _transformation_tensor(5, 5) = (-sz * sz + cz * cz) * cx * cy - 2.0 * sx * sy * sz * cy * cz;

  // store hill constants
  const Real & F = hill_constants_input[0];
  const Real & G = hill_constants_input[1];
  const Real & H = hill_constants_input[2];
  const Real & L = hill_constants_input[3];
  const Real & M = hill_constants_input[4];
  const Real & N = hill_constants_input[5];

  //  Moose::out << "Before, F: " << F << "\n";
  //  Moose::out << "Before, G: " << G << "\n";
  //  Moose::out << "Before, H: " << H << "\n";
  //  Moose::out << "Before, L: " << L << "\n";
  //  Moose::out << "Before, M: " << M << "\n";
  //  Moose::out << "Before, N: " << N << "\n";

  // rotated hill constants are calculated from rotated hill tensor, Hill_rot = Tm*Hill*Tm^T
  _hill_constants[0] = -_transformation_tensor(1, 0) *
                           (-G * _transformation_tensor(2, 2) - H * _transformation_tensor(2, 1) +
                            _transformation_tensor(2, 0) * (G + H)) -
                       _transformation_tensor(1, 1) *
                           (-F * _transformation_tensor(2, 2) - H * _transformation_tensor(2, 0) +
                            _transformation_tensor(2, 1) * (F + H)) -
                       _transformation_tensor(1, 2) *
                           (-F * _transformation_tensor(2, 1) - G * _transformation_tensor(2, 0) +
                            _transformation_tensor(2, 2) * (F + G)) -
                       2.0 * L * _transformation_tensor(1, 4) * _transformation_tensor(2, 4) -
                       2.0 * M * _transformation_tensor(1, 5) * _transformation_tensor(2, 5) -
                       2.0 * N * _transformation_tensor(1, 3) * _transformation_tensor(2, 3);

  _hill_constants[1] = -_transformation_tensor(0, 0) *
                           (-G * _transformation_tensor(2, 2) - H * _transformation_tensor(2, 1) +
                            _transformation_tensor(2, 0) * (G + H)) -
                       _transformation_tensor(0, 1) *
                           (-F * _transformation_tensor(2, 2) - H * _transformation_tensor(2, 0) +
                            _transformation_tensor(2, 1) * (F + H)) -
                       _transformation_tensor(0, 2) *
                           (-F * _transformation_tensor(2, 1) - G * _transformation_tensor(2, 0) +
                            _transformation_tensor(2, 2) * (F + G)) -
                       2.0 * L * _transformation_tensor(0, 4) * _transformation_tensor(2, 4) -
                       2.0 * M * _transformation_tensor(0, 5) * _transformation_tensor(2, 5) -
                       2.0 * N * _transformation_tensor(0, 3) * _transformation_tensor(2, 3);

  _hill_constants[2] = -_transformation_tensor(0, 0) *
                           (-G * _transformation_tensor(1, 2) - H * _transformation_tensor(1, 1) +
                            _transformation_tensor(1, 0) * (G + H)) -
                       _transformation_tensor(0, 1) *
                           (-F * _transformation_tensor(1, 2) - H * _transformation_tensor(1, 0) +
                            _transformation_tensor(1, 1) * (F + H)) -
                       _transformation_tensor(0, 2) *
                           (-F * _transformation_tensor(1, 1) - G * _transformation_tensor(1, 0) +
                            _transformation_tensor(1, 2) * (F + G)) -
                       2.0 * L * _transformation_tensor(0, 4) * _transformation_tensor(1, 4) -
                       2.0 * M * _transformation_tensor(0, 5) * _transformation_tensor(1, 5) -
                       2.0 * N * _transformation_tensor(0, 3) * _transformation_tensor(1, 3);

  _hill_constants[3] = 0.5 * _transformation_tensor(4, 0) *
                           (-G * _transformation_tensor(4, 2) - H * _transformation_tensor(4, 1) +
                            _transformation_tensor(4, 0) * (G + H)) +
                       0.5 * _transformation_tensor(4, 1) *
                           (-F * _transformation_tensor(4, 2) - H * _transformation_tensor(4, 0) +
                            _transformation_tensor(4, 1) * (F + H)) +
                       0.5 * _transformation_tensor(4, 2) *
                           (-F * _transformation_tensor(4, 1) - G * _transformation_tensor(4, 0) +
                            _transformation_tensor(4, 2) * (F + G)) +
                       L * _transformation_tensor(4, 4) * _transformation_tensor(4, 4) +
                       M * _transformation_tensor(4, 5) * _transformation_tensor(4, 5) +
                       N * _transformation_tensor(4, 3) * _transformation_tensor(4, 3);

  _hill_constants[4] = 0.5 * _transformation_tensor(5, 0) *
                           (-G * _transformation_tensor(5, 2) - H * _transformation_tensor(5, 1) +
                            _transformation_tensor(5, 0) * (G + H)) +
                       0.5 * _transformation_tensor(5, 1) *
                           (-F * _transformation_tensor(5, 2) - H * _transformation_tensor(5, 0) +
                            _transformation_tensor(5, 1) * (F + H)) +
                       0.5 * _transformation_tensor(5, 2) *
                           (-F * _transformation_tensor(5, 1) - G * _transformation_tensor(5, 0) +
                            _transformation_tensor(5, 2) * (F + G)) +
                       L * _transformation_tensor(5, 4) * _transformation_tensor(5, 4) +
                       M * _transformation_tensor(5, 5) * _transformation_tensor(5, 5) +
                       N * _transformation_tensor(5, 3) * _transformation_tensor(5, 3);

  _hill_constants[5] = 0.5 * _transformation_tensor(3, 0) *
                           (-G * _transformation_tensor(3, 2) - H * _transformation_tensor(3, 1) +
                            _transformation_tensor(3, 0) * (G + H)) +
                       0.5 * _transformation_tensor(3, 1) *
                           (-F * _transformation_tensor(3, 2) - H * _transformation_tensor(3, 0) +
                            _transformation_tensor(3, 1) * (F + H)) +
                       0.5 * _transformation_tensor(3, 2) *
                           (-F * _transformation_tensor(3, 1) - G * _transformation_tensor(3, 0) +
                            _transformation_tensor(3, 2) * (F + G)) +
                       L * _transformation_tensor(3, 4) * _transformation_tensor(3, 4) +
                       M * _transformation_tensor(3, 5) * _transformation_tensor(3, 5) +
                       N * _transformation_tensor(3, 3) * _transformation_tensor(3, 3);

  //  Moose::out << "After, F: " << _hill_constants[0] << "\n";
  //  Moose::out << "After, G: " << _hill_constants[1] << "\n";
  //  Moose::out << "After, H: " << _hill_constants[2] << "\n";
  //  Moose::out << "After, L: " << _hill_constants[3] << "\n";
  //  Moose::out << "After, M: " << _hill_constants[4] << "\n";
  //  Moose::out << "After, N: " << _hill_constants[5] << "\n";
}
