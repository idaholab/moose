#include "NavierStokesModule.h"
#include "Factory.h"
#include "ActionFactory.h"

// navier_stokes
#include "NSMassInviscidFlux.h"
#include "NSMomentumInviscidFlux.h"
#include "NSEnergyInviscidFlux.h"
#include "NSGravityPower.h"
#include "NSGravityForce.h"
#include "NSThermalBC.h"
#include "NSVelocityAux.h"
#include "NSImposedVelocityBC.h"
#include "NSTemperatureAux.h"
#include "NSTemperatureL2.h"
#include "NSPressureAux.h"
#include "NSEnthalpyAux.h"
#include "NSEnergyThermalFlux.h"
#include "NSMomentumViscousFlux.h"
#include "NSEnergyViscousFlux.h"
#include "NSMomentumInviscidFluxWithGradP.h"
#include "NSSUPGMomentum.h"
#include "NSSUPGMass.h"
#include "NSSUPGEnergy.h"
#include "NSMassSpecifiedNormalFlowBC.h"
#include "NSMassUnspecifiedNormalFlowBC.h"
#include "NSInflowThermalBC.h"
#include "NSMomentumInviscidSpecifiedPressureBC.h"
#include "NSMomentumInviscidSpecifiedNormalFlowBC.h"
#include "NSMomentumViscousBC.h"
#include "NSEnergyInviscidSpecifiedPressureBC.h"
#include "NSEnergyInviscidSpecifiedNormalFlowBC.h"
#include "NSEnergyInviscidUnspecifiedBC.h"
#include "NSEnergyInviscidSpecifiedBC.h"
#include "NSEnergyInviscidSpecifiedDensityAndVelocityBC.h"
#include "NSEnergyViscousBC.h"
#include "NSStagnationPressureBC.h"
#include "NSStagnationTemperatureBC.h"
#include "NSImposedVelocityDirectionBC.h"
#include "NSMassWeakStagnationBC.h"
#include "NSMomentumConvectiveWeakStagnationBC.h"
#include "NSMomentumPressureWeakStagnationBC.h"
#include "NSEnergyWeakStagnationBC.h"
#include "NSPenalizedNormalFlowBC.h"
#include "NSMomentumInviscidNoPressureImplicitFlowBC.h"

void
Elk::NavierStokes::registerObjects(Factory & factory)
{
  // navier_stokes
  registerKernel(NSMassInviscidFlux);
  registerKernel(NSMomentumInviscidFlux);
  registerKernel(NSEnergyInviscidFlux);
  registerKernel(NSGravityPower);
  registerKernel(NSGravityForce);
  registerKernel(NSTemperatureL2);
  registerBoundaryCondition(NSThermalBC);
  registerAux(NSVelocityAux);
  registerBoundaryCondition(NSImposedVelocityBC);
  registerAux(NSTemperatureAux);
  registerAux(NSPressureAux);
  registerAux(NSEnthalpyAux);
  registerKernel(NSEnergyThermalFlux);
  registerKernel(NSMomentumViscousFlux);
  registerKernel(NSEnergyViscousFlux);
  registerKernel(NSMomentumInviscidFluxWithGradP);
  registerKernel(NSSUPGMomentum);
  registerKernel(NSSUPGMass);
  registerKernel(NSSUPGEnergy);
  registerBoundaryCondition(NSMassSpecifiedNormalFlowBC);
  registerBoundaryCondition(NSMassUnspecifiedNormalFlowBC);
  registerBoundaryCondition(NSInflowThermalBC);
  registerBoundaryCondition(NSMomentumInviscidSpecifiedPressureBC);
  registerBoundaryCondition(NSMomentumInviscidSpecifiedNormalFlowBC);
  registerBoundaryCondition(NSMomentumViscousBC);
  registerBoundaryCondition(NSEnergyInviscidSpecifiedPressureBC);
  registerBoundaryCondition(NSEnergyInviscidSpecifiedNormalFlowBC);
  registerBoundaryCondition(NSEnergyInviscidUnspecifiedBC);
  registerBoundaryCondition(NSEnergyInviscidSpecifiedBC);
  registerBoundaryCondition(NSEnergyInviscidSpecifiedDensityAndVelocityBC);
  registerBoundaryCondition(NSEnergyViscousBC);
  registerBoundaryCondition(NSStagnationPressureBC);
  registerBoundaryCondition(NSStagnationTemperatureBC);
  registerBoundaryCondition(NSImposedVelocityDirectionBC);
  registerBoundaryCondition(NSMassWeakStagnationBC);
  registerBoundaryCondition(NSMomentumConvectiveWeakStagnationBC);
  registerBoundaryCondition(NSMomentumPressureWeakStagnationBC);
  registerBoundaryCondition(NSEnergyWeakStagnationBC);
  registerBoundaryCondition(NSPenalizedNormalFlowBC);
  registerBoundaryCondition(NSMomentumInviscidNoPressureImplicitFlowBC);
}
