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
 * This vectorpostprocessor computes the average stress scalar normal to the crack front at points
 * along the crack front.
 */
class CrackFrontNonlocalStress : public ElementVectorPostprocessor
{
public:
  static InputParameters validParams();

  CrackFrontNonlocalStress(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /** dimensions of the box in front of the crack tip that the stress is averaged over
   * The box is centered in front of the crack tip
   * _box_length distance box extends in front of the crack tip
   * _box_width is tangent to the crack tip, centered on the crack point
   * _box_height is normal to the crack tip, centered on the crack point
   */
  ///@{
  Real _box_length;
  Real _box_width;
  Real _box_height;
  ///@}

  /// used to transform local coordinates to crack front coordinates
  const CrackFrontDefinition * _crack_front_definition;

  /// Base name of the material system
  const std::string _base_name;
  /// The stress tensor
  const MaterialProperty<RankTwoTensor> & _stress;

  // volume being integrated over for each crack front
  std::vector<Real> _volume;

  /// Vectors computed by this VectorPostprocessor:
  /// x,y,z coordinates, and position of nodes along crack front, and crack tip average scalar stress
  ///@{
  VectorPostprocessorValue & _x;
  VectorPostprocessorValue & _y;
  VectorPostprocessorValue & _z;
  VectorPostprocessorValue & _position;
  VectorPostprocessorValue & _avg_crack_tip_stress;
  ///@}

  /**
   * Determine whether a point is located within a specified crack front oriented box
   * @param crack_front_point_index Index of the point on the crack front that the box is based on
   * @param qp_coord Point object to determine whether it is inside the box
   * @return  1 if point is within the box, 0 otherwise
   */
  Real BoxWeightingFunction(std::size_t crack_front_point_index, const Point & qp_coord) const;
};
