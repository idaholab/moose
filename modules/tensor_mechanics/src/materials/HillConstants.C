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
registerMooseObject("TensorMechanicsApp", ADHillConstants);

template <bool is_ad>
InputParameters
HillConstantsTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
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
  params.addParam<RealVectorValue>(
      "rotation_angles",
      "Provide the rotation angles for the transformation matrix. "
      "This should be a vector that provides "
      "the rotation angles about z-, x-, and z-axis, respectively in degrees.");
  params.addParam<std::vector<FunctionName>>(
      "function_names",
      "A set of functions that describe the evolution of anisotropy with temperature");
  params.addParam<bool>(
      "use_large_rotation",
      true,
      "Whether to rotate the Hill tensor (anisotropic parameters) to account for large kinematic "
      "rotations. It's recommended to set it to true if large displacements are to be expected.");
  params.addParam<bool>("use_automatic_differentiation",
                        false,
                        "Whether thermal contact depends on automatic differentiation materials.");
  params.addCoupledVar("temperature", "Coupled temperature");
  return params;
}

template <bool is_ad>
HillConstantsTempl<is_ad>::HillConstantsTempl(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _use_large_rotation(getParam<bool>("use_large_rotation")),
    _rotation_total_hill(_use_large_rotation ? &declareGenericProperty<RankTwoTensor, is_ad>(
                                                   _base_name + "rotation_total_hill")
                                             : nullptr),
    _rotation_total_hill_old(_use_large_rotation
                                 ? &this->template getMaterialPropertyOldByName<RankTwoTensor>(
                                       _base_name + "rotation_total_hill")
                                 : nullptr),
    _rotation_increment(_use_large_rotation ? &getGenericMaterialProperty<RankTwoTensor, is_ad>(
                                                  "rotation_increment")
                                            : nullptr),
    _hill_constants_input(getParam<std::vector<Real>>("hill_constants")),
    _hill_tensor(6, 6),
    _hill_constant_material(declareProperty<std::vector<Real>>(_base_name + "hill_constants")),
    _hill_tensor_material(_use_large_rotation
                              ? &declareProperty<DenseMatrix<Real>>(_base_name + "hill_tensor")
                              : nullptr),
    _zxz_angles(isParamValid("rotation_angles") ? getParam<RealVectorValue>("rotation_angles")
                                                : RealVectorValue(0.0, 0.0, 0.0)),
    _transformation_tensor(6, 6),
    _has_temp(isParamValid("temperature")),
    _temperature(_has_temp ? coupledValue("temperature") : _zero),
    _function_names(getParam<std::vector<FunctionName>>("function_names")),
    _num_functions(_function_names.size()),
    _functions(_num_functions),
    _rigid_body_rotation_tensor(_zxz_angles)
{
  // Transform to radians, used throughout the class.
  for (unsigned i = 0; i < 3; i++)
    _zxz_angles(i) *= libMesh::pi / 180.0;

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
}

template <bool is_ad>
void
HillConstantsTempl<is_ad>::initQpStatefulProperties()
{
  if (_use_large_rotation)
  {
    RankTwoTensor identity_rotation(RankTwoTensor::initIdentity);
    (*_rotation_total_hill)[_qp] = identity_rotation;
  }
}

template <bool is_ad>
void
HillConstantsTempl<is_ad>::computeQpProperties()
{
  // Account for finite strain rotation influence on anisotropic coefficients
  if (_use_large_rotation)
  {
    (*_rotation_total_hill)[_qp] = (*_rotation_increment)[_qp] * (*_rotation_total_hill_old)[_qp];

    // Make sure to provide the original coefficients to the orientation transformation
    _hill_constant_material[_qp].resize(6);
    _hill_constant_material[_qp] = _hill_constants_input;
  }

  // Account for temperature dependency
  if (_has_temp)
  {
    _hill_constant_material[_qp].resize(6);

    for (unsigned int i = 0; i < 6; i++)
      _hill_constant_material[_qp][i] = _functions[i]->value(_temperature[_qp]);
  }

  // We need to update the coefficients whether we use temperature dependency or large rotation
  // kinematics (or both)
  if (_has_temp || _use_large_rotation)
    rotateHillConstants(_hill_constant_material[_qp]);

  // Update material coefficients whether or not they are temperature-dependent
  if (_use_large_rotation)
    (*_hill_tensor_material)[_qp] = _hill_tensor;

  // To be used only for simple cases (axis-aligned, small deformation)
  if (!_use_large_rotation)
  {
    _hill_constant_material[_qp].resize(6);
    _hill_constant_material[_qp][0] = -_hill_tensor(1, 2);      // F
    _hill_constant_material[_qp][1] = -_hill_tensor(0, 2);      // G
    _hill_constant_material[_qp][2] = -_hill_tensor(0, 1);      // H
    _hill_constant_material[_qp][3] = _hill_tensor(4, 4) / 2.0; // L
    _hill_constant_material[_qp][4] = _hill_tensor(5, 5) / 2.0; // M
    _hill_constant_material[_qp][5] = _hill_tensor(3, 3) / 2.0; // N
  }
}

template <bool is_ad>
void
HillConstantsTempl<is_ad>::rotateHillConstants(const std::vector<Real> & hill_constants_input)
{
  // Rotation due to rigid body motion and large deformation
  RankTwoTensor total_rotation_matrix;

  if (_use_large_rotation)
    total_rotation_matrix =
        MetaPhysicL::raw_value((*_rotation_total_hill)[_qp]) * _rigid_body_rotation_tensor;
  else
    total_rotation_matrix = _rigid_body_rotation_tensor;

  const RankTwoTensor & trm = total_rotation_matrix;

  const Real & F = hill_constants_input[0];
  const Real & G = hill_constants_input[1];
  const Real & H = hill_constants_input[2];
  const Real & L = hill_constants_input[3];
  const Real & M = hill_constants_input[4];
  const Real & N = hill_constants_input[5];

  _hill_tensor.zero();

  _hill_tensor(0, 0) = G + H;
  _hill_tensor(1, 1) = F + H;
  _hill_tensor(2, 2) = F + G;
  _hill_tensor(0, 1) = _hill_tensor(1, 0) = -H;
  _hill_tensor(0, 2) = _hill_tensor(2, 0) = -G;
  _hill_tensor(1, 2) = _hill_tensor(2, 1) = -F;

  _hill_tensor(3, 3) = 2.0 * N;
  _hill_tensor(4, 4) = 2.0 * L;
  _hill_tensor(5, 5) = 2.0 * M;

  // Transformed the Hill tensor given the total rotation matrix
  // MEHRABADI, MORTEZA M.; COWIN, STEPHEN C.  (1990). EIGENTENSORS OF LINEAR ANISOTROPIC ELASTIC
  // MATERIALS. The Quarterly Journal of Mechanics and Applied Mathematics, 43(1), 15-41.
  // doi:10.1093/qjmam/43.1.15
  DenseMatrix<Real> transformation_matrix_n(6, 6);
  const static std::array<std::size_t, 3> a = {{1, 0, 0}};
  const static std::array<std::size_t, 3> b = {{2, 2, 1}};
  for (std::size_t i = 0; i < 3; ++i)
    for (std::size_t j = 0; j < 3; ++j)
    {
      transformation_matrix_n(i, j) = trm(i, j) * trm(i, j);
      transformation_matrix_n(i + 3, j) = 2.0 * trm((i + 1) % 3, j) * trm((i + 2) % 3, j);
      transformation_matrix_n(j, i + 3) = trm(j, (i + 1) % 3) * trm(j, (i + 2) % 3);
      transformation_matrix_n(i + 3, j + 3) =
          trm(a[i], a[j]) * trm(b[i], b[j]) + trm(a[i], b[j]) * trm(b[i], a[j]);
    }

  _hill_tensor.left_multiply(transformation_matrix_n);
  _hill_tensor.right_multiply_transpose(transformation_matrix_n);
}

template class HillConstantsTempl<false>;
template class HillConstantsTempl<true>;
