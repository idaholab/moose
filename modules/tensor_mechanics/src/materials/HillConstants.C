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

  return params;
}

HillConstants::HillConstants(const InputParameters & parameters)
  : ADMaterial(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _hill_constants_input(6),
    _hill_constants(6),
    _hill_constant_material(declareProperty<std::vector<Real>>(_base_name + "hill_constants")),
    _zyx_angles(isParamValid("rotation_angles") ? getParam<RealVectorValue>("rotation_angles")
                                                : RealVectorValue(0.0, 0.0, 0.0)),
    _transformation_tensor(6, 6)
{
  _hill_constants_input = getParam<std::vector<Real>>("hill_constants");
  rotateHillConstants(_hill_constants_input);
}

void
HillConstants::computeQpProperties()
{
  _hill_constant_material[_qp] = _hill_constants;
}

void
HillConstants::rotateHillConstants(std::vector<Real> & hill_constants_input)
{
  const Real sz = std::sin(_zyx_angles(0) * libMesh::pi / 180.0);
  const Real cz = std::cos(_zyx_angles(0) * libMesh::pi / 180.0);

  const Real sy = std::sin(_zyx_angles(1) * libMesh::pi / 180.0);
  const Real cy = std::cos(_zyx_angles(1) * libMesh::pi / 180.0);

  const Real sx = std::sin(_zyx_angles(2) * libMesh::pi / 180.0);
  const Real cx = std::cos(_zyx_angles(2) * libMesh::pi / 180.0);

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
}
