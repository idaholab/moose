/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef STRESSDIVERGENCERSPHERICALTENSORS_H
#define STRESSDIVERGENCERSPHERICALTENSORS_H

#include "StressDivergenceTensors.h"

// Forward Declarations
class StressDivergenceRSphericalTensors;

template <>
InputParameters validParams<StressDivergenceRSphericalTensors>();

/**
 * StressDivergenceRSphericalTensors is a modification of StressDivergenceTensors
 * for 1D spherically symmetric problems.  The main modifications from the original
 * StressDivergenceTensors code are requirements from the dependence of stress in
 * the polar and azimuthal stresses on displacement and position in the radial
 * direction.  This kernel is for symmetrical loading only.
 * If solving an anisotropic material problem, recall that the orientation of
 * the basis vectors (\hat{e}_r) change with position, so the components of the
 * elasticity tensor are functions of position.
 * Reference: Bower, A.F. Applied Mechanics of Solids (2012). Chapter 4. Available
 * online at solidmechanics.org
 * Within this kernel, '_disp_x' refers to displacement in the radial direction.
 * The COORD_TYPE in the Problem block must be set to RSpherical.
 */
class StressDivergenceRSphericalTensors : public StressDivergenceTensors
{
public:
  StressDivergenceRSphericalTensors(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  Real calculateJacobian(unsigned int ivar, unsigned int jvar);
};

#endif // STRESSDIVERGENCERSPHERICALTENSORS_H
