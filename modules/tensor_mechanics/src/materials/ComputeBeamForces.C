/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeBeamForces.h"

template <>
InputParameters
validParams<ComputeBeamForces>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Compute forces and moments using elasticity");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  return params;
}

ComputeBeamForces::ComputeBeamForces(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _disp_strain_increment(
        getMaterialPropertyByName<RealVectorValue>(_base_name + "disp_strain_increment")),
    _rot_strain_increment(
        getMaterialPropertyByName<RealVectorValue>(_base_name + "rot_strain_increment")),
    _material_stiffness(
        getMaterialPropertyByName<RealVectorValue>(_base_name + "material_stiffness")),
    _material_flexure(getMaterialPropertyByName<RealVectorValue>(_base_name + "material_flexure")),
    _total_rotation(getMaterialPropertyByName<RankTwoTensor>(_base_name + "total_rotation")),
    _force(declareProperty<RealVectorValue>(_base_name + "forces")),
    _moment(declareProperty<RealVectorValue>(_base_name + "moments")),
    _force_old(getMaterialPropertyOld<RealVectorValue>(_base_name + "forces")),
    _moment_old(getMaterialPropertyOld<RealVectorValue>(_base_name + "moments"))
{
}

void
ComputeBeamForces::initQpStatefulProperties()
{
  _force[_qp].zero();
  _moment[_qp].zero();
}

void
ComputeBeamForces::computeQpProperties()
{
  // force = R^T * _material_stiffness * strain_increment + force_old
  RealVectorValue force_increment;
  force_increment(0) = _material_stiffness[_qp](0) * _disp_strain_increment[_qp](0);
  force_increment(1) = _material_stiffness[_qp](1) * _disp_strain_increment[_qp](1);
  force_increment(2) = _material_stiffness[_qp](2) * _disp_strain_increment[_qp](2);

  _force[_qp] = _total_rotation[0].transpose() * force_increment + _force_old[_qp];

  // moment = R^T * _material_flexure * rotation_increment + moment_old
  RealVectorValue moment_increment;
  moment_increment(0) = _material_flexure[_qp](0) * _rot_strain_increment[_qp](0);
  moment_increment(1) = _material_flexure[_qp](1) * _rot_strain_increment[_qp](1);
  moment_increment(2) = _material_flexure[_qp](2) * _rot_strain_increment[_qp](2);

  _moment[_qp] = _total_rotation[0].transpose() * moment_increment + _moment_old[_qp];
}
