/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECAPPEDWEAKINCLINEDPLANESTRESS_H
#define COMPUTECAPPEDWEAKINCLINEDPLANESTRESS_H

#include "ComputeCappedWeakPlaneStress.h"

class ComputeCappedWeakInclinedPlaneStress;

template<>
InputParameters validParams<ComputeCappedWeakInclinedPlaneStress>();

/**
 * ComputeCappedWeakInclinedPlaneStress performs the return-map
 * algorithm and associated stress updates for plastic
 * models that describe capped inclined-weak-plane plasticity
 *
 * It assumes various things about the elasticity tensor, viz
 * in the reference frame where the normal to the inclined plane
 * is rotated equal to the "z" axis, it assumes:
 * E(i,i,j,k) = 0 except if k=j
 * E(0,0,i,j) = E(1,1,i,j)
 */
class ComputeCappedWeakInclinedPlaneStress :
  public ComputeCappedWeakPlaneStress
{
public:
  ComputeCappedWeakInclinedPlaneStress(const InputParameters & parameters);

protected:
  virtual void computeQpStress() override;
  virtual void initQpStatefulProperties() override;
  virtual void errorHandler(const std::string & message) override;

  /**
   * Rotate relevant vectors and tensors to the frame where the
   * inclined plane's normal coincides with the "z" axis
   */
  void rotate();

  /**
   * Rotate relevant vectors and tensors from the frame where the
   * inclined plane's normal coincides with the "z" axis
   * back to the original reference frame
   */
  void unrotate();

  /// Normal to the inclined weak plane
  RealVectorValue _n_input;

  /// Current value of the normal
  MaterialProperty<RealVectorValue> & _n;

  /// Old value of the normal
  MaterialProperty<RealVectorValue> & _n_old;

  /// Rotation matrix that rotates _n to "z" and vice-versa
  RealTensorValue _rot;
};

#endif //COMPUTECAPPEDWEAKINCLINEDPLANESTRESS_H
