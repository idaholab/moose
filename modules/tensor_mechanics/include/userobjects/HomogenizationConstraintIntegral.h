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
namespace HomogenizationConstants
{
/// Which tensor indices correspond to each constraint number
//    These are different for large displacements (n^2 constraints)
//    and small displacements (n(n-1)/2 constraints)
//    and different depending on the problem dimension
typedef std::vector<std::pair<unsigned int, unsigned int>> index_list;
const std::map<bool, std::vector<index_list>> indices{
    {true,
     {{{0, 0}},
      {{0, 0}, {1, 1}, {1, 0}, {0, 1}},
      {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1}, {0, 2}, {1, 2}, {2, 2}}}},
    {false,
     {{{0, 0}}, {{0, 0}, {1, 1}, {0, 1}}, {{0, 0}, {1, 1}, {2, 2}, {1, 2}, {0, 2}, {0, 1}}}}};

/// Total number of required constraints for large/small deformations and the
/// problem dimension
//    Used to validate user input
const std::map<bool, std::vector<unsigned int>> required{{true, {1, 4, 9}}, {false, {1, 3, 6}}};
/// Moose constraint type, for input
const MultiMooseEnum mooseConstraintType("strain stress");
/// Constraint type: stress/PK stress or strain/deformation gradient
enum class ConstraintType
{
  Strain,
  Stress
};
}

/// Computes $\int_{V}\left(X_{ij}-\hat{X}_{ij}\right)dV$
//    Calculates the volume integral of the difference between some
//    quantity and a target, given as a MOOSE function
//
//    This is used by the HomogenizationConstraintScalarKernel
//    to enforce cell-average constraints on a simulation domain
//
class HomogenizationConstraintIntegral : public ElementUserObject
{
public:
  static InputParameters validParams();

  HomogenizationConstraintIntegral(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

  virtual const RankTwoTensor & getResidual() const;
  virtual const RankFourTensor & getJacobian() const;

protected:
  /// Total residual assembled as a rank two tensor
  virtual RankTwoTensor computeResidual();
  /// Total Jacobian assembled as a rank two tensor
  virtual RankFourTensor computeJacobian();

  /// Stress contribution jacobian entry
  Real stressJacobian(unsigned int i, unsigned int j, unsigned int a, unsigned int b);
  /// Small deformation strain contribution jacobian entry
  Real sdStrainJacobian(unsigned int i, unsigned int j, unsigned int a, unsigned int b);
  /// Large deformation deformation gradient contribution jacobian entry
  Real ldStrainJacobian(unsigned int i, unsigned int j, unsigned int a, unsigned int b);

protected:
  /// If true use large displacement kinematics
  const bool _ld;
  /// Problem dimension
  unsigned int _ndisp;
  /// Number of constraints
  unsigned int _ncomps;

  /// Deformation gradient
  const MaterialProperty<RankTwoTensor> & _F;
  /// 1st PK (or small) stress
  const MaterialProperty<RankTwoTensor> & _pk1_stress;
  /// PK derivative
  const MaterialProperty<RankFourTensor> & _pk1_jacobian;

  /// List of functions giving the targets for each constraint
  std::vector<const Function *> _targets;
  /// Type of each constraint (stress or strain)
  std::vector<HomogenizationConstants::ConstraintType> _ctypes;

  /// Used to loop through quadrature points
  unsigned int _qp;
  /// Used to loop through scalar variable entries
  //    residual and 1st index of jacobian
  unsigned int _h;
  /// Used to loop through scalar variable entries
  //    2nd index of Jacobian
  unsigned int _hh;

  /// Map between the flat list of constraints and the tensor index
  HomogenizationConstants::index_list _indices;

  /// The assembled tensor residual
  RankTwoTensor _residual;
  /// The assembled tensor jacobian
  RankFourTensor _jacobian;
};
