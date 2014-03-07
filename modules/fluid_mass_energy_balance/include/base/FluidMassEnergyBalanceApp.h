#ifndef FLUID_MASS_ENERGY_BALANCEAPP_H
#define FLUID_MASS_ENERGY_BALANCEAPP_H

#include "MooseApp.h"

class FluidMassEnergyBalanceApp;

template<>
InputParameters validParams<FluidMassEnergyBalanceApp>();

class FluidMassEnergyBalanceApp : public MooseApp
{
public:
  FluidMassEnergyBalanceApp(const std::string & name, InputParameters parameters);
  virtual ~FluidMassEnergyBalanceApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* FLUID_MASS_ENERGY_BALANCEAPP_H */
