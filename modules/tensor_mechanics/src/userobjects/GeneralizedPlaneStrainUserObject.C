//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedPlaneStrainUserObject.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "Function.h"
#include "Assembly.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<GeneralizedPlaneStrainUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription(
      "Generalized Plane Strain UserObject to provide Residual and diagonal Jacobian entry");
  params.addParam<UserObjectName>("subblock_index_provider",
                                  "SubblockIndexProvider user object name");
  params.addParam<FunctionName>(
      "out_of_plane_pressure",
      "0",
      "Function used to prescribe pressure in the out-of-plane direction");
  params.addParam<Real>("factor", 1.0, "Scale factor applied to prescribed pressure");
  params.addParam<std::string>("base_name", "Material properties base name");
  params.set<ExecFlagEnum>("execute_on") = EXEC_LINEAR;

  return params;
}

GeneralizedPlaneStrainUserObject::GeneralizedPlaneStrainUserObject(
    const InputParameters & parameters)
  : ElementUserObject(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _Cijkl(getMaterialProperty<RankFourTensor>(_base_name + "elasticity_tensor")),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _subblock_id_provider(nullptr),
    _out_of_plane_pressure(getFunction("out_of_plane_pressure")),
    _factor(getParam<Real>("factor"))
{
}

void
GeneralizedPlaneStrainUserObject::initialize()
{
  if (isParamValid("subblock_index_provider"))
    _subblock_id_provider = &getUserObject<SubblockIndexProvider>("subblock_index_provider");
  if (_assembly.coordSystem() == Moose::COORD_XYZ)
    _scalar_out_of_plane_strain_direction = 2;
  else if (_assembly.coordSystem() == Moose::COORD_RZ)
    _scalar_out_of_plane_strain_direction = 1;
  else
    mooseError("Unsupported coordinate system for generalized plane strain formulation");

  unsigned int max_size = _subblock_id_provider ? _subblock_id_provider->getMaxSubblockIndex() : 1;
  _residual.assign(max_size, 0.0);
  _jacobian.assign(max_size, 0.0);
}

void
GeneralizedPlaneStrainUserObject::execute()
{
  const unsigned int subblock_id =
      _subblock_id_provider ? _subblock_id_provider->getSubblockIndex(*_current_elem) : 0;

  for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // residual, integral of stress_zz for COORD_XYZ
    _residual[subblock_id] +=
        _JxW[_qp] * _coord[_qp] * (_stress[_qp](_scalar_out_of_plane_strain_direction,
                                                _scalar_out_of_plane_strain_direction) +
                                   _out_of_plane_pressure.value(_t, _q_point[_qp]) * _factor);
    // diagonal jacobian, integral of C(2, 2, 2, 2) for COORD_XYZ
    _jacobian[subblock_id] +=
        _JxW[_qp] * _coord[_qp] * _Cijkl[_qp](_scalar_out_of_plane_strain_direction,
                                              _scalar_out_of_plane_strain_direction,
                                              _scalar_out_of_plane_strain_direction,
                                              _scalar_out_of_plane_strain_direction);
  }
}

void
GeneralizedPlaneStrainUserObject::threadJoin(const UserObject & uo)
{
  const GeneralizedPlaneStrainUserObject & gpsuo =
      static_cast<const GeneralizedPlaneStrainUserObject &>(uo);
  for (unsigned int i = 0; i < _residual.size(); ++i)
  {
    _residual[i] += gpsuo._residual[i];
    _jacobian[i] += gpsuo._jacobian[i];
  }
}

void
GeneralizedPlaneStrainUserObject::finalize()
{
  gatherSum(_residual);
  gatherSum(_jacobian);
}

Real
GeneralizedPlaneStrainUserObject::returnResidual(unsigned int scalar_var_id) const
{
  if (_residual.size() < scalar_var_id)
    mooseError("Index out of bounds!");

  return _residual[scalar_var_id];
}

Real
GeneralizedPlaneStrainUserObject::returnJacobian(unsigned int scalar_var_id) const
{
  if (_jacobian.size() < scalar_var_id)
    mooseError("Index out of bounds!");

  return _jacobian[scalar_var_id];
}
