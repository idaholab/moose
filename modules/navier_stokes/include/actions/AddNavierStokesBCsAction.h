//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDNAVIERSTOKESBCSACTION_H
#define ADDNAVIERSTOKESBCSACTION_H

#include "MooseObjectAction.h"

class AddNavierStokesBCsAction;

template <>
InputParameters validParams<AddNavierStokesBCsAction>();

/**
 * This class allows us to have a section of the input file like the
 * following which adds BC objects for each requested boundary condition.
 *
 * [NavierStokes]
 *   [./BCs]
 *     [./inlet]
 *       type = NSWeakStagnationInletBC
 *       boundary = 'inlet'
 *       stagnation_pressure = 120192.995549849
 *       stagnation_temperature = 315
 *       sx = 1.
 *       sy = 0.
 *       fluid_properties = ideal_gas
 *     [../]
 *
 *     [./solid_walls]
 *       type = NSNoPenetrationBC
 *       boundary = '3 4'
 *       fluid_properties = ideal_gas
 *     [../]
 *
 *     [./outlet]
 *       type = NSStaticPressureOutletBC
 *       boundary = '2' # 'Outflow'
 *       specified_pressure = 101325 # Pa
 *       fluid_properties = ideal_gas
 *     [../]
 *   [../]
 * []
 */
class AddNavierStokesBCsAction : public MooseObjectAction
{
public:
  AddNavierStokesBCsAction(InputParameters parameters);
  virtual ~AddNavierStokesBCsAction();

  virtual void act();

protected:
  unsigned int _dim;

  // Helper functions that add the various weak stagnation BCs.
  void addNSMassWeakStagnationBC();
  void addNSEnergyWeakStagnationBC();
  void addNSMomentumWeakStagnationBC(unsigned int component);

  // Helper function that adds the no-penetration BCs
  void addNoPenetrationBC(unsigned int component);

  // Helper function that adds the static pressure outlet BCs
  void addNSMassUnspecifiedNormalFlowBC();
  void addNSEnergyInviscidSpecifiedPressureBC();
  void addNSMomentumInviscidSpecifiedPressureBC(unsigned int component);

  // Helper function that sets the parameters which are common to all NSKernels.
  void setCommonParams(InputParameters & params);

  // Couple the appropriate number (depending on the _dim) of velocity/momentum
  // components into a Kernel.
  void coupleVelocities(InputParameters & params);
  void coupleMomentums(InputParameters & params);

  // Type that we use in Actions for declaring coupling
  typedef std::vector<VariableName> CoupledName;
};

#endif
