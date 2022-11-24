//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

// Helpers common to the whole homogenization system
namespace Homogenization
{
/// Moose constraint type, for input
const MultiMooseEnum constraintType("strain stress none");
/// Constraint type: stress/PK stress or strain/deformation gradient
enum class ConstraintType
{
  Strain,
  Stress,
  None
};
typedef std::map<std::pair<unsigned int, unsigned int>, std::pair<ConstraintType, const Function *>>
    ConstraintMap;
}

/// Computes $\int_{V}\left(X_{ij}-\hat{X}_{ij}\right)dV$
///
/// Calculates the volume integral of the difference between some
/// quantity and a target, given as a MOOSE function
///
/// This is used by the HomogenizationConstraintScalarKernel
/// to enforce cell-average constraints on a simulation domain
///
class HomogenizationConstraint : public ElementUserObject
{
public:
  static InputParameters validParams();

  HomogenizationConstraint(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

  virtual const RankTwoTensor & getResidual() const { return _residual; }
  virtual const RankFourTensor & getJacobian() const { return _jacobian; }
  virtual const Homogenization::ConstraintMap & getConstraintMap() const { return _cmap; }

protected:
  /// Total residual assembled as a rank two tensor
  virtual RankTwoTensor computeResidual();
  /// Total Jacobian assembled as a rank two tensor
  virtual RankFourTensor computeJacobian();

protected:
  /// If true use large displacement kinematics
  const bool _large_kinematics;

  /// Prepend to the material properties
  const std::string _base_name;

  /// Deformation gradient
  const MaterialProperty<RankTwoTensor> & _F;
  /// 1st PK (or small) stress
  const MaterialProperty<RankTwoTensor> & _pk1_stress;
  /// PK derivative
  const MaterialProperty<RankFourTensor> & _pk1_jacobian;

  /// Type of each constraint (stress or strain) for each component
  Homogenization::ConstraintMap _cmap;

private:
  /// Used to loop through quadrature points
  unsigned int _qp;

  /// The assembled tensor residual
  RankTwoTensor _residual;
  /// The assembled tensor jacobian
  RankFourTensor _jacobian;
};
