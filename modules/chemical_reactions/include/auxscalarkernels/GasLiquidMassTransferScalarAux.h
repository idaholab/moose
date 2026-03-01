#pragma once

#include "AuxScalarKernel.h"
#include "Function.h"

// Forward Declarations
class SinglePhaseFluidProperties;

class GasLiquidMassTransferScalarAux : public AuxScalarKernel
{
public:
  static InputParameters validParams();

  GasLiquidMassTransferScalarAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Pressure [Pa]
  const VariableValue & _pressure;

  /// Fluid temperature [K]
  const VariableValue & _temperature;

  /// Liquid velocity [m/s]
  const VariableValue & _fluid_velocity;

  /// Diameter of the flow channel [m]
  const Real _diameter;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;

  /// Enum used to select the type
  const enum class Equationlist { STOKESEINSTEIN, WILKECHANG } _equation_list;

  /// Particle radius [m]
  const Real _radius;

  /// Molecular weight of fluid [kg/mol]
  const Real _mw;

  /// Association parameter for Wilke-Chang model
  const Real _phi;

  /// Constant in the Wilke-Chang model
  const Real _wc;

  /// Dittus-Boelter leading coefficient
  const Real _db;
};
