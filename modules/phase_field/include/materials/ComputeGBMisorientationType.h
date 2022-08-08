//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "GrainTracker.h"
#include "EBSDReader.h"

/**
 * Visualize the location of grain boundaries in a polycrystalline simulation.
 */
class ComputeGBMisorientationType : public Material
{
public:
  static InputParameters validParams();

  ComputeGBMisorientationType(const InputParameters & parameters);

protected:
  /// Necessary override. This is where the property values are set.
  virtual void computeQpProperties() override;

  /// Function to obtain the total line number in misorientation angle file
  virtual unsigned int getTotalLineNum() const;
  /// Function to obtain line number for a given grain pair
  virtual unsigned int getLineNum(unsigned int grain_i, unsigned int grain_j);
  /// Function to get the GB type for triple junctions
  virtual Real getTripleJunctionType(std::vector<unsigned int> gb_pairs,
                                     std::vector<Real> gb_op_pairs);
  /// Function to convert symmetry matrix to quaternion form
  void rotationSymmetryToQuaternion(const double O[3][3], double q[4]);
  /// Function to define the symmetry operator
  void defineSymmetryOperator(double *** sym);
  /// Function to convert grain Euler Angle orientation to quaternion vector
  void eulerOrientationToQuaternion(int grain_id);
  /// Function to multiply quaternions and update
  void getQuaternionProduct(const double qi[4], const double qj[4], double q[4]);
  /// Function to return the misorientation of two quaternions
  double getMisorientationFromQuaternion(const double qi[4], const double qj[4]);
  /// Get the Misorientation angle
  void getMisorientationAngles();

  /// Grain tracker object
  const GrainTracker & _grain_tracker;

  /// EBSD reader user object
  const EBSDReader & _ebsd_reader;

  /// Parameters to calculate the Misorientation angle file
  std::vector<double> _misorientation_angles;

  /// order parameters
  const unsigned int _op_num;
  const std::vector<const VariableValue *> _vals;

  /// the max value of LAGB
  const Real _angle_threshold;

  /// The parameters to calculate the misorientation
  double ** sym_quat;
  int o_sym = 24;
  std::vector<std::vector<double>> euler_angle;
  std::vector<std::vector<double>> quat_angle;

  /// precalculated element value
  ADMaterialProperty<Real> & _gb_type;
};
