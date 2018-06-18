/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef DISPALCEMENTJUMPBASEDCOHESIVEINTERFACEMATERIAL_H
#define DISPALCEMENTJUMPBASEDCOHESIVEINTERFACEMATERIAL_H

#include "Material.h"
#include "TractionSeparationUOBase.h"

class DisplacementJumpBasedCohesiveInterfaceMaterial;

template <>
InputParameters validParams<DisplacementJumpBasedCohesiveInterfaceMaterial>();

/**
 *
 */
class DisplacementJumpBasedCohesiveInterfaceMaterial : public Material
{
public:
  DisplacementJumpBasedCohesiveInterfaceMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;

  const VariableValue & _disp_x;
  const VariableValue & _disp_x_neighbor;
  const VariableValue & _disp_y;
  const VariableValue & _disp_y_neighbor;
  const VariableValue & _disp_z;
  const VariableValue & _disp_z_neighbor;

  const MooseArray<Point> & _normals;

  /// User objects that define the slip rate
  const TractionSeparationUOBase * _uo_tractionSeparation;

  std::vector<std::string> _materialPropertyNames;
  unsigned int _num_stateful_material_properties;

  std::vector<MaterialProperty<std::vector<Real>> *> _materialPropertyValues;
  std::vector<const MaterialProperty<std::vector<Real>> *> _materialPropertyValues_old;

  /// the disaplcement jump in global coordiantes
  MaterialProperty<RealVectorValue> & _Jump;

  /// the disaplcement jump in natural element coordiantes
  MaterialProperty<RealVectorValue> & _JumpLocal;

  /// the value of the Traction in global coordiantes
  MaterialProperty<RealVectorValue> & _Traction;

  /// the value of the Traction in natural element coordiantes
  MaterialProperty<RealVectorValue> * _TractionLocal;

  /// the value of the Traction Derivatives in global coordiantes
  MaterialProperty<RankTwoTensor> & _TractionSpatialDerivative;

  /// the value of the Traction Derivatives in natural element coordiantes
  MaterialProperty<RankTwoTensor> * _TractionSpatialDerivativeLocal;

  /// rotation matrix that takes _n to (0, 0, 1) i.e. _nLocal = R*_n
  /// global to local and viceversa
  RealTensorValue _RotationGlobal2Local;
  RealTensorValue _RotationLocal2Global;

  /// compute the displacmenet Jump in natural element coordinates (N,T,S)
  virtual void moveToLocalFrame();

  /// Transform Travtion and Traction derivatives from the local back to global
  virtual void moveBackToGlobalFrame();
};

#endif // DISPALCEMENTJUMPBASEDCOHESIVEINTERFACEMATERIAL_H
