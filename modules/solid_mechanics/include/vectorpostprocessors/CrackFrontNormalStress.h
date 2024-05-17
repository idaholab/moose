//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementVectorPostprocessor.h"

// Forward Declarations
class CrackFrontDefinition;
/**
 * This vectorpostprocessor computes the J-Integral, which is a measure of
 * the strain energy release rate at a crack tip, which can be used as a
 * criterion for fracture growth.
 */
class CrackFrontNormalStress : public ElementVectorPostprocessor
{
public:
  static InputParameters validParams();

  CrackFrontNormalStress(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  Real _box_length;
  Real _box_width;
  Real _box_height;

  /// something about this
  const CrackFrontDefinition * _crack_front_definition;

  /// Base name of the material system
  const std::string _base_name;
  /// The stress tensor
  const MaterialProperty<RankTwoTensor> & _stress;

  /// Whether to treat a 3D model as 2D for computation of fracture integrals
  bool _treat_as_2d;
  /// Whether the crack is defined by an XFEM cutter mesh
  bool _using_mesh_cutter;

  // volume being integrated over for each crack front
  std::vector<Real> _volume;

  /// Vectors computed by this VectorPostprocessor:
  /// x,y,z coordinates, position of nodes along crack front, and
  /// crack tip principal stress
  ///@{
  VectorPostprocessorValue & _x;
  VectorPostprocessorValue & _y;
  VectorPostprocessorValue & _z;
  VectorPostprocessorValue & _position;
  VectorPostprocessorValue & _avg_crack_tip_stress;
  ///@}

  /**
   * Compute region within specified crack front oriented box, return 1 if it is inside
   * @param crack_front_point_index Index of the point on the crack front
   * @param current_node Node at which q is evaluated
   * @return q
   */
  Real CrackFrontBox(std::size_t crack_front_point_index, const Point & qp_coord) const;
};
