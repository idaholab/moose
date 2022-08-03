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
#include "UserObjectInterface.h"

#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", GeneralizedPlaneStrainUserObject);

InputParameters
GeneralizedPlaneStrainUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription(
      "Generalized plane strain UserObject to provide residual and diagonal Jacobian entries.");
  params.addParam<UserObjectName>("subblock_index_provider",
                                  "SubblockIndexProvider user object name");
  params.addParam<FunctionName>("out_of_plane_pressure_function",
                                "Function used to prescribe pressure (applied toward the body) in "
                                "the out-of-plane direction");
  params.addDeprecatedParam<FunctionName>(
      "out_of_plane_pressure",
      "Function used to prescribe pressure (applied toward the body) in the out-of-plane direction "
      "(y for 1D Axisymmetric or z for 2D Cartesian problems)",
      "This has been replaced by 'out_of_plane_pressure_function'");
  params.addParam<MaterialPropertyName>("out_of_plane_pressure_material",
                                        "0",
                                        "Material used to prescribe pressure (applied toward the "
                                        "body) in the out-of-plane direction");
  MooseEnum outOfPlaneDirection("x y z", "z");
  params.addParam<MooseEnum>(
      "out_of_plane_direction", outOfPlaneDirection, "The direction of the out-of-plane strain.");
  params.addDeprecatedParam<Real>(
      "factor",
      "Scale factor applied to prescribed out-of-plane pressure (both material and function)",
      "This has been replaced by 'pressure_factor'");
  params.addParam<Real>(
      "pressure_factor",
      "Scale factor applied to prescribed out-of-plane pressure (both material and function)");
  params.addParam<std::string>("base_name", "Material properties base name");
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_NONLINEAR};

  return params;
}

GeneralizedPlaneStrainUserObject::GeneralizedPlaneStrainUserObject(
    const InputParameters & parameters)
  : ElementUserObject(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _Jacobian_mult(getMaterialProperty<RankFourTensor>(_base_name + "Jacobian_mult")),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _subblock_id_provider(nullptr),
    _out_of_plane_pressure_function(parameters.isParamSetByUser("out_of_plane_pressure_function")
                                        ? &getFunction("out_of_plane_pressure_function")
                                    : parameters.isParamSetByUser("out_of_plane_pressure")
                                        ? &getFunction("out_of_plane_pressure")
                                        : nullptr),
    _out_of_plane_pressure_material(getMaterialProperty<Real>("out_of_plane_pressure_material")),
    _pressure_factor(parameters.isParamSetByUser("pressure_factor")
                         ? getParam<Real>("pressure_factor")
                     : parameters.isParamSetByUser("factor") ? getParam<Real>("factor")
                                                             : 1.0)
{
  if (parameters.isParamSetByUser("out_of_plane_pressure_function") &&
      parameters.isParamSetByUser("out_of_plane_pressure"))
    mooseError("Cannot specify both 'out_of_plane_pressure_function' and 'out_of_plane_pressure'");
  if (parameters.isParamSetByUser("pressure_factor") && parameters.isParamSetByUser("factor"))
    mooseError("Cannot specify both 'pressure_factor' and 'factor'");
}

void
GeneralizedPlaneStrainUserObject::initialize()
{
  if (isParamValid("subblock_index_provider"))
    _subblock_id_provider = &getUserObject<SubblockIndexProvider>("subblock_index_provider");
  if (_assembly.coordSystem() == Moose::COORD_XYZ)
    _scalar_out_of_plane_strain_direction = getParam<MooseEnum>("out_of_plane_direction");
  else if (_assembly.coordSystem() == Moose::COORD_RZ)
    _scalar_out_of_plane_strain_direction = 1;
  else
    mooseError("Unsupported coordinate system for generalized plane strain formulation");

  unsigned int max_size = _subblock_id_provider ? _subblock_id_provider->getMaxSubblockIndex() : 1;
  _residual.assign(max_size, 0.0);
  _reference_residual.assign(max_size, 0.0);
  _jacobian.assign(max_size, 0.0);
}

void
GeneralizedPlaneStrainUserObject::execute()
{
  const unsigned int subblock_id =
      _subblock_id_provider ? _subblock_id_provider->getSubblockIndex(*_current_elem) : 0;

  for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    const Real out_of_plane_pressure =
        ((_out_of_plane_pressure_function
              ? _out_of_plane_pressure_function->value(_t, _q_point[_qp])
              : 0.0) +
         _out_of_plane_pressure_material[_qp]) *
        _pressure_factor;

    // residual, integral of stress_zz for COORD_XYZ
    _residual[subblock_id] += _JxW[_qp] * _coord[_qp] *
                              (_stress[_qp](_scalar_out_of_plane_strain_direction,
                                            _scalar_out_of_plane_strain_direction) +
                               out_of_plane_pressure);

    _reference_residual[subblock_id] += std::abs(
        _JxW[_qp] * _coord[_qp] *
        _stress[_qp](_scalar_out_of_plane_strain_direction, _scalar_out_of_plane_strain_direction));

    // diagonal jacobian, integral of C(2, 2, 2, 2) for COORD_XYZ
    _jacobian[subblock_id] += _JxW[_qp] * _coord[_qp] *
                              _Jacobian_mult[_qp](_scalar_out_of_plane_strain_direction,
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
    _reference_residual[i] += gpsuo._reference_residual[i];
    _jacobian[i] += gpsuo._jacobian[i];
  }
}

void
GeneralizedPlaneStrainUserObject::finalize()
{
  gatherSum(_residual);
  gatherSum(_reference_residual);
  gatherSum(_jacobian);
}

Real
GeneralizedPlaneStrainUserObject::returnResidual(unsigned int scalar_var_id) const
{
  if (_residual.size() <= scalar_var_id)
    mooseError("Index out of bounds!");

  return _residual[scalar_var_id];
}

Real
GeneralizedPlaneStrainUserObject::returnReferenceResidual(unsigned int scalar_var_id) const
{
  // At startup, the GeneralizedPlaneStrainReferenceResidual class can ask for this value
  // before it has been computed.  Return 0.0 in this case.  The only way size will stay
  // zero is if initialize is never called.
  if (_reference_residual.size() == 0)
    return 0.0;

  if (_residual.size() <= scalar_var_id)
    mooseError("Index out of bounds!");

  return _reference_residual[scalar_var_id];
}

Real
GeneralizedPlaneStrainUserObject::returnJacobian(unsigned int scalar_var_id) const
{
  if (_jacobian.size() <= scalar_var_id)
    mooseError("Index out of bounds!");

  return _jacobian[scalar_var_id];
}
