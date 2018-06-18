/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "DisplacementJumpBasedCohesiveInterfaceMaterial.h"
#include "Assembly.h"
#include "MooseMesh.h"
#include "TractionSeparationUOBase.h"
#include "RotationMatrix.h"

registerMooseObject("TensorMechanicsApp", DisplacementJumpBasedCohesiveInterfaceMaterial);

template <>
InputParameters
validParams<DisplacementJumpBasedCohesiveInterfaceMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("disp_x",
                               "variable containing the X component of"
                               "the dispalcement on the master side");
  params.addRequiredCoupledVar("disp_x_neighbor",
                               "variable containing the X component"
                               " of the dispalcement on the slave side");
  params.addCoupledVar("disp_y",
                       "variable containing the Y component of"
                       "the dispalcement on the master side");
  params.addCoupledVar("disp_y_neighbor",
                       "variable containing the Y "
                       "component of the dispalcement on the slave side");
  params.addCoupledVar("disp_z",
                       "variable containing the Z component of"
                       "the dispalcement on the master side");
  params.addCoupledVar("disp_z_neighbor",
                       "variable containing the Z "
                       "component of the dispalcement on the slave side");

  params.addRequiredParam<UserObjectName>(
      "uo_TractionSeparationLaw",
      "the name of the user object including the traction separation law");
  params.addClassDescription("this material class is used when defining a "
                             "cohesive zone model");
  return params;
}

DisplacementJumpBasedCohesiveInterfaceMaterial::DisplacementJumpBasedCohesiveInterfaceMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _disp_x(coupledValue("disp_x")),
    _disp_x_neighbor(coupledNeighborValue("disp_x_neighbor")),
    _disp_y(_mesh.dimension() >= 2 ? coupledValue("disp_y") : _zero),
    _disp_y_neighbor(_mesh.dimension() >= 2 ? coupledNeighborValue("disp_y_neighbor") : _zero),
    _disp_z(_mesh.dimension() >= 3 ? coupledValue("disp_z") : _zero),
    _disp_z_neighbor(_mesh.dimension() >= 3 ? coupledNeighborValue("disp_z_neighbor") : _zero),

    _normals(_assembly.normals()),

    _Jump(declareProperty<RealVectorValue>("Jump")),
    _JumpLocal(declareProperty<RealVectorValue>("JumpLocal")),
    _Traction(declareProperty<RealVectorValue>("Traction")),
    _TractionLocal(&declareProperty<RealVectorValue>("TractionLocal")),
    _TractionSpatialDerivative(declareProperty<RankTwoTensor>("TractionSpatialDerivative")),
    _TractionSpatialDerivativeLocal(
        &declareProperty<RankTwoTensor>("TractionSpatialDerivativeLocal")),

    _RotationGlobal2Local(RealTensorValue()),
    _RotationLocal2Global(RealTensorValue())

{
  // assign user object
  _uo_tractionSeparation = &getUserObjectByName<TractionSeparationUOBase>(
      parameters.get<UserObjectName>("uo_TractionSeparationLaw"));

  // get stateful material property number and names
  _uo_tractionSeparation->statefulMaterialPropertyNames(_materialPropertyNames);
  _num_stateful_material_properties = _materialPropertyNames.size();

  if (_num_stateful_material_properties > 0)
  {
    // initialize the stateful material property values container
    _materialPropertyValues.resize(_num_stateful_material_properties);
    _materialPropertyValues_old.resize(_num_stateful_material_properties);

    // declare properties
    for (unsigned int i = 0; i < _num_stateful_material_properties; ++i)
    {
      _materialPropertyValues[i] = &declareProperty<std::vector<Real>>(_materialPropertyNames[i]);

      _materialPropertyValues_old[i] =
          &getMaterialPropertyOld<std::vector<Real>>(_materialPropertyNames[i]);
    }
  }
}

void
DisplacementJumpBasedCohesiveInterfaceMaterial::computeQpProperties()
{
  // compute the jump on the interface in global coordinates
  _Jump[_qp](0) = _disp_x_neighbor[_qp] - _disp_x[_qp];
  _Jump[_qp](1) = _disp_y_neighbor[_qp] - _disp_y[_qp];
  _Jump[_qp](2) = _disp_z_neighbor[_qp] - _disp_z[_qp];

  // transform from global to loval cooridnates
  moveToLocalFrame();

  // compute tractions and traction derivatives in localc coordinates
  _uo_tractionSeparation->computeTractionLocal(_qp, (*_TractionLocal)[_qp]);

  _uo_tractionSeparation->computeTractionSpatialDerivativeLocal(
      _qp, (*_TractionSpatialDerivativeLocal)[_qp]);

  // move results back to global coordinates
  moveBackToGlobalFrame();
}

void
DisplacementJumpBasedCohesiveInterfaceMaterial::initQpStatefulProperties()
{

  if (_num_stateful_material_properties > 0)
  {
    // initialize stateful material material properties
    for (unsigned int i = 0; i < _num_stateful_material_properties; i++)
    {
      // resize material properties
      (*_materialPropertyValues[i])[_qp].resize(
          _uo_tractionSeparation->statefulMaterialPropertySize(i));

      // resize old state
      // TODO: remove this nasty const_cast if you can figure out how
      const_cast<MaterialProperty<std::vector<Real>> &>(*_materialPropertyValues_old[i])[_qp]
          .resize(_uo_tractionSeparation->statefulMaterialPropertySize(i));

      // fill in intial values
      _uo_tractionSeparation->initStatefulMaterialProperty(i, (*_materialPropertyValues[i])[_qp]);
      // TODO: remove this nasty const_cast if you can figure out how
      const_cast<MaterialProperty<std::vector<Real>> &>(*_materialPropertyValues_old[i])[_qp] =
          (*_materialPropertyValues[i])[_qp];
    }
  }
}

void
DisplacementJumpBasedCohesiveInterfaceMaterial::moveToLocalFrame()
{
  // this is a rotation matrix that will rotate _n to the "x" axis such that the
  // the first compoenent in the local frame represent the opening displacment
  _RotationGlobal2Local = RotationMatrix::rotVec1ToVec2(_normals[_qp], RealVectorValue(1, 0, 0));

  // compute the jump in the lcoal coordinate system
  for (unsigned int i = 0; i < 3; i++)
  {
    _JumpLocal[_qp](i) = 0; // just to be sure that it reset at each iteration
    for (unsigned int j = 0; j < 3; j++)
    {
      _JumpLocal[_qp](i) += _RotationGlobal2Local(i, j) * _Jump[_qp](j);
    }
  }
}

void
DisplacementJumpBasedCohesiveInterfaceMaterial::moveBackToGlobalFrame()
{

  _RotationLocal2Global = _RotationGlobal2Local.transpose();
  // rotate traction in the global frame
  for (unsigned int i = 0; i < 3; i++)
  {
    _Traction[_qp](i) = 0;
    for (unsigned int j = 0; j < 3; j++)
    {
      _Traction[_qp](i) += _RotationLocal2Global(i, j) * (*_TractionLocal)[_qp](j);
    }
  }

  // rotate traction derivatives in the global frame
  _TractionSpatialDerivative[_qp] = (*_TractionSpatialDerivativeLocal)[_qp];
  _TractionSpatialDerivative[_qp].rotate(_RotationLocal2Global);
}
