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
 * This vectorpostprocessor computes the average stress scalar normal to the crack front.
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
  /*** dimensions of the box in front of the crack tip that the stress is averaged over
   * box is centered in front of the crack tip
   * _box_length distance box extends in front of the crack tip
   * _box_width is tangent to the crack tip, centered on the crack point
   * _box_height is normal to the crack tip, centered on the crack point
   */
  ///@{
  Real _box_length;
  Real _box_width;
  Real _box_height;
  ///@}

  /// something about this
  const CrackFrontDefinition * _crack_front_definition;

  /// Base name of the material system
  const std::string _base_name;
  /// The stress tensor
  const MaterialProperty<RankTwoTensor> & _stress;

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
