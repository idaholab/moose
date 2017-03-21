/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef STRESSDIVERGENCERZTENSORS_H
#define STRESSDIVERGENCERZTENSORS_H

#include "StressDivergenceTensors.h"

// Forward Declarations
class StressDivergenceRZTensors;

template <>
InputParameters validParams<StressDivergenceRZTensors>();

/**
 * StressDivergenceRZTensors is a modification of StressDivergenceTensors to
 * accommodate the Axisymmetric material models that use cylindrical coordinates.
 * This kernel is for symmetrical loading only.  The key modifications are a result
 * of the circumferential stress' dependence on displacement in the axial direction.
 * Reference: Cook et.al. Concepts and Applications of Finite Element Analysis,
 * 4th Ed. 2002. p 510.
 * Within this kernel, '_disp_x' refers to displacement in the radial direction,
 * u_r, and '_disp_y' refers to displacement in the axial direction, u_z.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class StressDivergenceRZTensors : public StressDivergenceTensors
{
public:
  StressDivergenceRZTensors(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  virtual void computeAverageGradientTest() override;
  virtual void computeAverageGradientPhi() override;

  Real calculateJacobian(unsigned int ivar, unsigned int jvar);
};

#endif // STRESSDIVERGENCERZTENSORS_H
