/****************************************************************/
/*             DO NOT MODIFY OR REMOVE THIS HEADER              */
/*          FALCON - Fracturing And Liquid CONvection           */
/*                                                              */
/*       (c) pending 2012 Battelle Energy Alliance, LLC         */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef FLUIDFLOW_H
#define FLUIDFLOW_H

#include "PorousMedia.h"
#include "WaterSteamEOS.h"


//Forward Declarations
class FluidFlow;

template<>
InputParameters validParams<FluidFlow>();

/**
 * Simple material with FluidFlow properties.
 */
class FluidFlow : virtual public PorousMedia
{
public:
  FluidFlow(const std::string & name,
            InputParameters parameters);
  
protected:
  virtual void computeProperties();
  virtual void compute2PhProperties0(Real _per, Real _Sw, Real _Denw, Real _Dens, Real _visw, Real _viss, Real &_watertau, Real  &_steamtau);

  const WaterSteamEOS * _water_steam_properties;
    
  bool _has_pressure;
  VariableGradient & _grad_p;
  VariableValue  & _pressure;
  VariableValue & _pressure_old;

  bool _has_temp;
  bool _temp_dependent_fluid_props;
  VariableValue & _temperature;
  VariableValue & _temperature_old;
    
  bool _has_enthalpy;
  VariableValue & _enthalpy;
  VariableValue & _enthalpy_old;
    
  bool _if_transient;

  MaterialProperty<Real> & _tau_water;
  MaterialProperty<RealGradient> & _darcy_flux_water;
  MaterialProperty<RealGradient> & _darcy_mass_flux_water;
  MaterialProperty<RealGradient> & _darcy_mass_flux_water_pressure;
  MaterialProperty<RealGradient> & _darcy_mass_flux_water_elevation;
  MaterialProperty<Real> & _Dtau_waterDP;
  MaterialProperty<Real> & _Dtau_waterDH;
  
  MaterialProperty<Real> & _tau_steam;
  MaterialProperty<RealGradient> & _darcy_flux_steam;
  MaterialProperty<RealGradient> & _darcy_mass_flux_steam;
  MaterialProperty<RealGradient> & _darcy_mass_flux_steam_pressure;
  MaterialProperty<RealGradient> & _darcy_mass_flux_steam_elevation;
  MaterialProperty<Real> & _Dtau_steamDP;
  MaterialProperty<Real> & _Dtau_steamDH;
    
  //Equation_of_State_Properties - Non-Derivative Material Outputs
  MaterialProperty<Real> & _temp_out;
  MaterialProperty<Real> & _sat_fraction_out;
  MaterialProperty<Real> & _dens_out;
  MaterialProperty<Real> & _dens_water_out;
  MaterialProperty<Real> & _dens_steam_out;
  MaterialProperty<Real> & _enth_water_out;
  MaterialProperty<Real> & _enth_steam_out;
  MaterialProperty<Real> & _visc_water_out;
  MaterialProperty<Real> & _visc_steam_out;
    
  //Equations_of_State_Properties - Derivative Material Ouptuts
  MaterialProperty<Real> & _d_dens_d_enth;
  MaterialProperty<Real> & _d_dens_d_press;
  MaterialProperty<Real> & _d_enth_water_d_enth;
  MaterialProperty<Real> & _d_enth_steam_d_enth;
  MaterialProperty<Real> & _d_temp_d_enth;
  MaterialProperty<Real> & _d_sat_fraction_d_enth;
  MaterialProperty<Real> & _d_enth_water_d_press;
  MaterialProperty<Real> & _d_enth_steam_d_press;
  MaterialProperty<Real> & _d_temp_d_press;
    
  //Equations_of_State_Properties - Temperature/Pressure based
  MaterialProperty<Real> & _d_dens_d_temp_PT;
  MaterialProperty<Real> & _d_dens_d_press_PT;

  //Time Derivative Equation_of_State_Properties
  //Equation_of_State_Properties - Non-PH-Derivative Outputs
  MaterialProperty<Real> & _time_old_temp_out;
  MaterialProperty<Real> & _time_old_dens_out;
  MaterialProperty<Real> & _time_old_dens_water_out;
  MaterialProperty<Real> & _time_old_dens_steam_out;
  MaterialProperty<Real> & _time_old_visc_water_out;
  MaterialProperty<Real> & _time_old_visc_steam_out;

  //Equation_of_State_Properties - Temperature/Pressure based, constant density and viscosity inputs
  Real _constant_density;
  Real _constant_viscosity;
    
    //MaterialProperty<Real> & _permeability;

};

#endif //FLUIDFLOW_H
