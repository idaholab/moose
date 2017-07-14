/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef STRESSDIVERGENCERZTENSORS_H
#define STRESSDIVERGENCERZTENSORS_H

#include "StressDivergence2DTensors.h"

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
class StressDivergenceRZTensors : public StressDivergence2DTensors
{
public:
  StressDivergenceRZTensors(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  virtual void computeAverageGradientZZTest() override;
  virtual void computeAverageGradientZZPhi() override;
  virtual Real getGradientZZTest() override;
  virtual Real getGradientZZPhi() override;

  /// Whether the mesh is made of first order elements
  const bool _first_order;
};

#endif // STRESSDIVERGENCERZTENSORS_H
