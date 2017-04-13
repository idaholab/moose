/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GeneralizedPlaneStrainUserObject.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "Function.h"
#include "Assembly.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<GeneralizedPlaneStrainUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription(
      "Generalized Plane Strain UserObject to provide Residual and diagonal Jacobian entry");
  params.addParam<FunctionName>(
      "out_of_plane_pressure",
      "0",
      "Function used to prescribe pressure in the out-of-plane direction");
  params.addParam<Real>("factor", 1.0, "Scale factor applied to prescribed pressure");
  params.addParam<std::string>("base_name", "Material properties base name");
  MooseUtils::setExecuteOnFlags(params, 1, EXEC_LINEAR);

  return params;
}

GeneralizedPlaneStrainUserObject::GeneralizedPlaneStrainUserObject(
    const InputParameters & parameters)
  : ElementUserObject(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _Cijkl(getMaterialProperty<RankFourTensor>(_base_name + "elasticity_tensor")),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _out_of_plane_pressure(getFunction("out_of_plane_pressure")),
    _factor(getParam<Real>("factor"))
{
}

void
GeneralizedPlaneStrainUserObject::initialize()
{
  if (_assembly.coordSystem() == Moose::COORD_XYZ)
    _scalar_out_of_plane_strain_direction = 2;
  else if (_assembly.coordSystem() == Moose::COORD_RZ)
    _scalar_out_of_plane_strain_direction = 1;
  else
    mooseError("Unsupported coordinate system for generalized plane strain formulation");

  _residual = 0;
  _jacobian = 0;
}

void
GeneralizedPlaneStrainUserObject::execute()
{
  for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // residual, integral of stress_zz for COORD_XYZ
    _residual +=
        _JxW[_qp] * _coord[_qp] * (_stress[_qp](_scalar_out_of_plane_strain_direction,
                                                _scalar_out_of_plane_strain_direction) +
                                   _out_of_plane_pressure.value(_t, _q_point[_qp]) * _factor);
    // diagonal jacobian, integral of C(2, 2, 2, 2) for COORD_XYZ
    _jacobian += _JxW[_qp] * _coord[_qp] * _Cijkl[_qp](_scalar_out_of_plane_strain_direction,
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
  _residual += gpsuo._residual;
  _jacobian += gpsuo._jacobian;
}

void
GeneralizedPlaneStrainUserObject::finalize()
{
  gatherSum(_residual);
  gatherSum(_jacobian);
}

Real
GeneralizedPlaneStrainUserObject::returnResidual() const
{
  return _residual;
}

Real
GeneralizedPlaneStrainUserObject::returnJacobian() const
{
  return _jacobian;
}
