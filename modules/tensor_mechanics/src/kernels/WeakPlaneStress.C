//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WeakPlaneStress.h"

#include "Material.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

registerMooseObject("TensorMechanicsApp", WeakPlaneStress);

InputParameters
WeakPlaneStress::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Plane stress kernel to provide out-of-plane strain contribution.");
  params.addCoupledVar("displacements",
                       "The string of displacements suitable for the problem statement");
  params.addCoupledVar("temperature",
                       "The name of the temperature variable used in the "
                       "ComputeThermalExpansionEigenstrain.  (Not required for "
                       "simulations without temperature coupling.)");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names",
      "List of eigenstrains used in the strain calculation. Used for computing their derivaties "
      "for off-diagonal Jacobian terms.");
  params.addParam<std::string>("base_name", "Material property base name");

  MooseEnum direction("x y z", "z");
  params.addParam<MooseEnum>("out_of_plane_strain_direction",
                             direction,
                             "The direction of the out-of-plane strain variable");

  params.set<bool>("use_displaced_mesh") = false;

  return params;
}

WeakPlaneStress::WeakPlaneStress(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _Jacobian_mult(getMaterialProperty<RankFourTensor>(_base_name + "Jacobian_mult")),
    _direction(getParam<MooseEnum>("out_of_plane_strain_direction")),
    _disp_coupled(isCoupled("displacements")),
    _ndisp(_disp_coupled ? coupledComponents("displacements") : 0),
    _disp_var(_ndisp),
    _temp_coupled(isCoupled("temperature")),
    _temp_var(_temp_coupled ? coupled("temperature") : 0)
{
  if (_disp_coupled)
    for (unsigned int i = 0; i < _ndisp; ++i)
      _disp_var[i] = coupled("displacements", i);

  if (_temp_coupled)
  {
    for (auto eigenstrain_name : getParam<std::vector<MaterialPropertyName>>("eigenstrain_names"))
      _deigenstrain_dT.push_back(&getMaterialPropertyDerivative<RankTwoTensor>(
          eigenstrain_name, coupledName("temperature", 0)));
  }

  // Checking for consistency between mesh size and length of the provided displacements vector
  if (_disp_coupled && _ndisp != _mesh.dimension())
    mooseError("The number of displacement variables supplied must match the mesh dimension.");
}

Real
WeakPlaneStress::computeQpResidual()
{
  return _stress[_qp](_direction, _direction) * _test[_i][_qp];
}

Real
WeakPlaneStress::computeQpJacobian()
{
  return _Jacobian_mult[_qp](_direction, _direction, _direction, _direction) * _test[_i][_qp] *
         _phi[_j][_qp];
}

Real
WeakPlaneStress::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real val = 0.0;

  // off-diagonal Jacobian with respect to a coupled displacement component
  if (_disp_coupled)
  {
    for (unsigned int coupled_direction = 0; coupled_direction < _ndisp; ++coupled_direction)
    {
      if (jvar == _disp_var[coupled_direction])
      {
        unsigned int coupled_direction_index = 0;
        switch (_direction)
        {
          case 0: // x
          {
            if (coupled_direction == 0)
              coupled_direction_index = 1;
            else
              coupled_direction_index = 2;
            break;
          }
          case 1: // y
          {
            if (coupled_direction == 0)
              coupled_direction_index = 0;
            else
              coupled_direction_index = 2;
            break;
          }
          default: // z
          {
            coupled_direction_index = coupled_direction;
            break;
          }
        }

        val = _Jacobian_mult[_qp](
                  _direction, _direction, coupled_direction_index, coupled_direction_index) *
              _test[_i][_qp] * _grad_phi[_j][_qp](coupled_direction_index);
      }
    }
  }

  // off-diagonal Jacobian with respect to a coupled temperature variable
  if (_temp_coupled && jvar == _temp_var)
  {
    RankTwoTensor total_deigenstrain_dT;
    for (const auto deigenstrain_dT : _deigenstrain_dT)
      total_deigenstrain_dT += (*deigenstrain_dT)[_qp];

    Real sum = 0.0;
    for (unsigned int i = 0; i < 3; ++i)
      sum += _Jacobian_mult[_qp](_direction, _direction, i, i) * total_deigenstrain_dT(i, i);

    val = -sum * _test[_i][_qp] * _phi[_j][_qp];
  }

  return val;
}
