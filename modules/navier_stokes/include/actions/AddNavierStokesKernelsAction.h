/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ADDNAVIERSTOKESKERNELSACTION_H
#define ADDNAVIERSTOKESKERNELSACTION_H

#include "NSAction.h"

class AddNavierStokesKernelsAction;

template <>
InputParameters validParams<AddNavierStokesKernelsAction>();

/**
 * This class allows us to have a section of the input file like the
 * following which automatically adds Kernels and AuxKernels for all
 * the required nonlinear and auxiliary variables.
 *
 * [NavierStokes]
 *   [./Kernels]
 *   [../]
 * []
 */
class AddNavierStokesKernelsAction : public NSAction
{
public:
  AddNavierStokesKernelsAction(InputParameters parameters);
  virtual ~AddNavierStokesKernelsAction();

  virtual void act();

protected:
  // Helper function that sets the parameters which are common to all NSKernels.
  void setCommonParams(InputParameters & params);

  // Couple the appropriate number (depending on the _dim) of velocity/momentum
  // components into a Kernel.
  void coupleVelocities(InputParameters & params);
  void coupleMomentums(InputParameters & params);

  // Helper functions that add various inviscid flux Kernels.
  void addNSMassInviscidFlux();
  void addNSMomentumInviscidFlux(unsigned int component);
  void addNSEnergyInviscidFlux();

  // Helper functions that add SUPG Kernels
  void addNSSUPGMass();
  void addNSSUPGMomentum(unsigned int component);
  void addNSSUPGEnergy();

  // Helper functions that add AuxKernels
  void addPressureOrTemperatureAux(const std::string & kernel_type);
  void addNSVelocityAux(unsigned int component);
  void addNSEnthalpyAux();
  void addNSMachAux();
  void addNSInternalEnergyAux();
  void addNSSpecificVolumeAux();

  // Name of the FluidProperties object to pass on to Kernels
  UserObjectName _fp_name;
};

#endif
