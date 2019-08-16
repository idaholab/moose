/****************************************************************/
/*                  DO NOT MODIFY THIS HEADER                   */
/*                           Marmot                             */
/*                                                              */
/*            (c) 2017 Battelle Energy Alliance, LLC            */
/*                     ALL RIGHTS RESERVED                      */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*             Under Contract No. DE-AC07-05ID14517             */
/*             With the U. S. Department of Energy              */
/*                                                              */
/*             See COPYRIGHT for full restrictions              */
/****************************************************************/
#pragma once

#include "EulerAngleProvider.h"
#include "RotationMatrix.h"
#include "RotationTensor.h"

#include "libmesh/tensor_value.h"
#include "libmesh/vector_value.h"

// Forward Declarations
class AnisoTwoGrainEulerAngleProvider;
class FEProblemBase;

template <>
InputParameters validParams<AnisoTwoGrainEulerAngleProvider>();

/**
 * Class that inherits from the EulerAngleProvider base class.
 * Is designed to provide euler angles based on time. time = angle in degrees.
 * Note that this can only be used for 2 grain simulations and is designed for testing only.
 */
class AnisoTwoGrainEulerAngleProvider : public EulerAngleProvider
{
public:
  AnisoTwoGrainEulerAngleProvider(const InputParameters & parameters);

  /// Taken from EulerAngleFileReader.C
  virtual const EulerAngles & getEulerAngles(unsigned int) const;

  /// Return the number of grains.
  virtual unsigned int getGrainNum() const;

  virtual void initialize() {}
  virtual void execute();
  virtual void finalize() {}

protected:
  std::vector<EulerAngles> _angles;
  RealVectorValue _rotation_axis;
  unsigned int _axis_type;
  FEProblemBase & _fe_problem;
  RealTensorValue _gb1_orientation_matrix;
  RealTensorValue _gb2_orientation_matrix;
  RealTensorValue _rotation_matrix_gb1;
  RealTensorValue _rotation_matrix_gb2;
  RealTensorValue _rotate_to_axis;

  /// Update the EulerAngles based on the time step.
  void updateEulerAngles();

  /// This is for creating the axis we want to rotate about.
  void createRotationAxis();

  /**
   * Create euler angles from a rotation tensor.
   * See "Consistent representations of and conversions between 3D rotations Rowenhorst et al"
   */
  EulerAngles rotationMatrixToEuler(RealTensorValue m);
};
