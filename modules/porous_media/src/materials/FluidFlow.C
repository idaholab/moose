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

#include "FluidFlow.h"
#include "WaterSteamEOS.h"   

template<>
InputParameters validParams<FluidFlow>()
{
  InputParameters params = validParams<PorousMedia>(); 
  params.addCoupledVar("pressure", "Use pressure here to calculate Darcy Flux and Pore Velocity, [Pa]");
  params.addCoupledVar("enthalpy", "Use enthalpy here to calculate Darcy Flux and Pore Velocity for two-phase flow, [J]");
  params.addCoupledVar("temperature", "Use temperature to calculate Darcy Flux and Pore Velocity for single-phase flow, [K]");
  params.addParam<bool>("temp_dependent_fluid_props", true, "flag true if single-phase and fluid properties are temperature dependent, default = true");
  params.addParam<Real>("constant_density", 1000, "Use to set value of constant density, [kg/m^3]");
  params.addParam<Real>("constant_viscosity", 0.12e-3, "Use to set value of constant viscosity, [Pa.s]");
  params.addParam<UserObjectName>("water_steam_properties", "EOS functions, calculates water steam properties, provide if doing two-phase or if temp_dependent_fluid_props = true");
  return params;
}

FluidFlow::FluidFlow(const std::string & name, InputParameters parameters) :
    PorousMedia (name, parameters),
    //UserObject used to calculate two-phase flow or temp-depnt fluid properties and their derivatives
    _water_steam_properties(parameters.isParamValid("water_steam_properties") ? &getUserObject<WaterSteamEOS>("water_steam_properties") : NULL),

    //Three main varaibles (Pressure, Temperature, Enthalpy)
    //used for either PT (single phase) or PH (two phase) problems
    _has_pressure(isCoupled("pressure")),
    _grad_p(_has_pressure ? coupledGradient("pressure") : _grad_zero),
    _pressure(_has_pressure ? coupledValue("pressure")  : _zero),
    _pressure_old(_has_pressure && _is_transient ? coupledValueOld("pressure") : _zero),

    _has_temp(isCoupled("temperature")),
    _temp_dependent_fluid_props(getParam<bool>("temp_dependent_fluid_props")),
    _temperature(_has_temp ? coupledValue("temperature")  : _zero),
    _temperature_old(_has_temp && _is_transient ? coupledValueOld("temperature") : _zero),
    
    _has_enthalpy(isCoupled("enthalpy")), 
    _enthalpy(_has_enthalpy ? coupledValue("enthalpy")  : _zero),
    _enthalpy_old(_has_enthalpy && _is_transient ? coupledValueOld("enthalpy") : _zero),

    //Fluid velocity terms Tau and Darcy Flux (water and steam) and their derivatives
    _tau_water(declareProperty<Real>("tau_water")),
    _darcy_flux_water(declareProperty<RealGradient>("darcy_flux_water")),
    _darcy_mass_flux_water(declareProperty<RealGradient>("darcy_mass_flux_water")),
    _darcy_mass_flux_water_pressure(declareProperty<RealGradient>("darcy_mass_flux_water_pressure")),
    _darcy_mass_flux_water_elevation(declareProperty<RealGradient>("darcy_mass_flux_water_elevation")),
    _Dtau_waterDP(declareProperty<Real>("Dtau_waterDP")),
    _Dtau_waterDH(declareProperty<Real>("Dtau_waterDH")),

    _tau_steam(declareProperty<Real>("tau_steam")),
    _darcy_flux_steam(declareProperty<RealGradient>("darcy_flux_steam")),
    _darcy_mass_flux_steam(declareProperty<RealGradient>("darcy_mass_flux_steam")),
    _darcy_mass_flux_steam_pressure(declareProperty<RealGradient>("darcy_mass_flux_steam_pressure")),
    _darcy_mass_flux_steam_elevation(declareProperty<RealGradient>("darcy_mass_flux_steam_elevation")),
    _Dtau_steamDP(declareProperty<Real>("Dtau_steamDP")),
    _Dtau_steamDH(declareProperty<Real>("Dtau_steamDH")),

    //Equation_of_State_Properties - Pressure/Enthalpy based, Non-Derivative Outputs
    _temp_out(declareProperty<Real>("material_temperature")),
    _sat_fraction_out(declareProperty<Real>("saturation_water")),
    _dens_out(declareProperty<Real>("density")),
    _dens_water_out(declareProperty<Real>("density_water")),
    _dens_steam_out(declareProperty<Real>("density_steam")),
    _enth_water_out(declareProperty<Real>("enthalpy_water")),
    _enth_steam_out(declareProperty<Real>("enthalpy_steam")),
    _visc_water_out(declareProperty<Real>("viscosity_water")),
    _visc_steam_out(declareProperty<Real>("viscosity_steam")),

    //Equation_of_State_Properites - Pressure/Enthalpy based, Derivative Outputs
    _d_dens_d_enth(declareProperty<Real>("ddensitydH_P")),
    _d_dens_d_press(declareProperty<Real>("ddensitydp_H")),
    _d_enth_water_d_enth(declareProperty<Real>("denthalpy_waterdH_P")),
    _d_enth_steam_d_enth(declareProperty<Real>("denthalpy_steamdH_P")),
    _d_temp_d_enth(declareProperty<Real>("dTdH_P")),
    _d_sat_fraction_d_enth(declareProperty<Real>("dswdH")),
    _d_enth_water_d_press(declareProperty<Real>("denthalpy_waterdP_H")),
    _d_enth_steam_d_press(declareProperty<Real>("denthalpy_steamdP_H")),
    _d_temp_d_press(declareProperty<Real>("dTdP_H")),
    
    //Equation_of_State_Properties - Temperature/Pressure based, Derivative Outputs
    _d_dens_d_temp_PT(declareProperty<Real>("dwdt")),
    _d_dens_d_press_PT(declareProperty<Real>("dwdp")),

    //Time Derivative Equation_of_State_Properties
    //Equation_of_State_Properties - Non-Derivative_PH Outputs
    _time_old_temp_out(declareProperty<Real>("time_old_material_temperature")),
    _time_old_dens_out(declareProperty<Real>("time_old_density")),
    _time_old_dens_water_out(declareProperty<Real>("time_old_density_water")),
    _time_old_dens_steam_out(declareProperty<Real>("time_old_density_steam")),
    _time_old_visc_water_out(declareProperty<Real>("time_old_viscosity_water")),
    _time_old_visc_steam_out(declareProperty<Real>("time_old_viscosity_steam")),

    //Equation of State Properties - Temperature/Pressure based, constant density and viscosity inputs
    _constant_density(getParam<Real>("constant_density")),
    _constant_viscosity(getParam<Real>("constant_viscosity"))

    //_permeability(getMaterialProperty<Real>("permeability"))
{ }



void FluidFlow::computeProperties()
{
    if (!areParentPropsComputed())
        PorousMedia::computeProperties();
    
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {  
    Real temp_out;
    Real sat_fraction_out;
    Real dens_out, dens_water_out, dens_steam_out;
    Real enth_water_out, enth_steam_out;
    Real visc_water_out, visc_steam_out;
    Real d_enth_water_d_press, d_enth_steam_d_press;
    Real d_dens_d_press, d_temp_d_press;
    Real d_enth_water_d_enth, d_enth_steam_d_enth;
    Real d_dens_d_enth, d_temp_d_enth, d_sat_fraction_d_enth;

    //TWO PHASE PROBLEMS:
    //pressure-enthalpy based
    //If enthalpy IS a provided coupled variable in material property block
    //of input file, it will run this loop for material fluid properties
    
    if (_has_enthalpy)
    {
        if (_water_steam_properties == NULL)
            mooseError("You forgot to provide a UserObject for the EOS fluid property calculations... water_steam_properties in the material block of your input file needs an entry");
        
      //Calling EOS function and assigning fluid properties (sat_fraction, density, visosity, ..., and their derivatives) to output material values
        _water_steam_properties->waterAndSteamEquationOfStatePropertiesWithDerivativesPH (_enthalpy[qp], _pressure[qp], _temperature[qp], temp_out, sat_fraction_out, dens_out, dens_water_out, dens_steam_out, enth_water_out, enth_steam_out, visc_water_out, visc_steam_out, d_enth_water_d_press, d_enth_steam_d_press, d_dens_d_press, d_temp_d_press, d_enth_water_d_enth, d_enth_steam_d_enth, d_dens_d_enth, d_temp_d_enth, d_sat_fraction_d_enth);
                        
      _temp_out[qp] = temp_out;
      _sat_fraction_out[qp] = sat_fraction_out;
      _dens_out[qp] = dens_out;
      _dens_water_out[qp] = dens_water_out;
      _dens_steam_out[qp] = dens_steam_out;
      _enth_water_out[qp] = enth_water_out;
      _enth_steam_out[qp] = enth_steam_out;
      _visc_water_out[qp] = visc_water_out;
      _visc_steam_out[qp] = visc_steam_out;
      _d_enth_water_d_press[qp] = d_enth_water_d_press;
      _d_enth_steam_d_press[qp] = d_enth_steam_d_press;
      _d_dens_d_press[qp] = d_dens_d_press;
      _d_temp_d_press[qp] = d_temp_d_press;
      _d_enth_water_d_enth[qp] = d_enth_water_d_enth;
      _d_enth_steam_d_enth[qp] = d_enth_steam_d_enth;
      _d_dens_d_enth[qp] = d_dens_d_enth;
      _d_temp_d_enth[qp] = d_temp_d_enth;
      _d_sat_fraction_d_enth[qp] = d_sat_fraction_d_enth;

      //If problem is transient, this loop will provide "stateful" material properties by
      //recalling EOS function and passing in old pressure and enthalpy values.
      //This circumvents issues with real stateful materials and mesh adaptivity
      if (_is_transient)
      {
        Real time_old_temp_out;
        Real time_old_dens_out, time_old_dens_water_out, time_old_dens_steam_out;
        Real time_old_visc_water_out, time_old_visc_steam_out;
        Real var[7];
                
        _water_steam_properties->waterAndSteamEquationOfStatePropertiesPH (_enthalpy_old[qp], _pressure_old[qp], _temperature_old[qp], var[0], time_old_temp_out, var[1], var[2], time_old_dens_out, time_old_dens_water_out, time_old_dens_steam_out, var[3], var[4], time_old_visc_water_out, time_old_visc_steam_out, var[5], var[6]);
                                
        _time_old_temp_out[qp] = time_old_temp_out;
        _time_old_dens_out[qp] = time_old_dens_out;
        _time_old_dens_water_out[qp] = time_old_dens_water_out;
        _time_old_dens_steam_out[qp] = time_old_dens_steam_out;
        _time_old_visc_water_out[qp] = time_old_visc_water_out;
        _time_old_visc_steam_out[qp] = time_old_visc_steam_out;
                                    
      }
                
      //Determining tau and darcy_flux fluid properties 
      Real _tau_water0,  _tau_water1,  _tau_water2;  
      Real _tau_steam0,  _tau_steam1,  _tau_steam2;
      Real _sat_fraction;
      Real _dens_water, _dens_steam;
      Real _visc_water, _visc_steam;
      Real _var[8];
      Real del_p=1.0, del_h=1e-7;
                
      //Obtaining properties to compute derivative of tau_water/steam w.r.t. a pressure step (enthalpy held constant)
      _water_steam_properties->waterAndSteamEquationOfStatePropertiesPH (_enthalpy[qp], (_pressure[qp] + del_p), _temperature[qp], _var[0], _var[1], _var[2], _sat_fraction, _var[3], _dens_water, _dens_steam, _var[4], _var[5], _visc_water, _visc_steam, _var[6], _var[7]);
                
      FluidFlow::compute2PhProperties0( _permeability[qp], _sat_fraction, _dens_water, _dens_steam, _visc_water, _visc_steam, _tau_water[qp],_tau_steam[qp]);
                
      _tau_water1 = _tau_water[qp];
      _tau_steam1 = _tau_steam[qp];
                
                
      //Obtaining properties to compute derivative of tau_water/steam w.r.t. a enthalpy step (pressure held constant)
      _water_steam_properties->waterAndSteamEquationOfStatePropertiesPH ((_enthalpy[qp] + del_h), _pressure[qp], _temperature[qp], _var[0], _var[1], _var[2], _sat_fraction, _var[3], _dens_water, _dens_steam, _var[4], _var[5], _visc_water, _visc_steam, _var[6], _var[7]);
                
      FluidFlow::compute2PhProperties0( _permeability[qp], _sat_fraction, _dens_water, _dens_steam, _visc_water, _visc_steam, _tau_water[qp],_tau_steam[qp]);
                
      _tau_water2 = _tau_water[qp];
      _tau_steam2 =  _tau_steam[qp];
                
                
      //Obtaining properties to compute darcy_mass_flux, darcy_flux, and tau_water/_steam
      FluidFlow:: compute2PhProperties0( _permeability[qp], sat_fraction_out, dens_water_out, dens_steam_out, visc_water_out, visc_steam_out, _tau_water[qp], _tau_steam[qp]);
                
      _tau_water0 = _tau_water[qp];
      _tau_steam0 = _tau_steam[qp];
                
                
      //Calculating darcy_mass_flux, darcy_flux, and tau_water/_steam derivatives
      _darcy_mass_flux_water[qp] = -_tau_water0 * (_grad_p[qp] + _dens_water * _gravity[qp] * _gravity_vector[qp]);
      _darcy_mass_flux_steam[qp] = -_tau_steam0 * (_grad_p[qp] + _dens_steam * _gravity[qp] * _gravity_vector[qp]);
      _darcy_mass_flux_water_pressure[qp] =  -_tau_water0 * _grad_p[qp];
      _darcy_mass_flux_water_elevation[qp] = -_tau_water0 * _gravity[qp] * _gravity_vector[qp] * _dens_water;
      _darcy_mass_flux_steam_pressure[qp] =  -_tau_steam0 * _grad_p[qp];
      _darcy_mass_flux_steam_elevation[qp] = -_tau_steam0 * _gravity[qp] * _gravity_vector[qp] * _dens_steam; 
      _darcy_flux_steam[qp] = _darcy_mass_flux_steam[qp] / _dens_steam;
      _darcy_flux_water[qp] = _darcy_mass_flux_water[qp] / _dens_water;
                
      _Dtau_waterDP[qp] = (_tau_water1 - _tau_water0) / del_p;
      _Dtau_steamDP[qp] = (_tau_steam1 - _tau_steam0) / del_p;
      _Dtau_waterDH[qp] = (_tau_water2 - _tau_water0) / del_h;
      _Dtau_steamDH[qp] = (_tau_steam2 - _tau_steam0) / del_h;
            
    }
    //SINGLE PHASE PROBLEMS:
    //pressure-temperature based
    //If enthalpy IS NOT a provided coupled variable in the material block
    //of the input file, this loop will provide material fluid properties
    else 
    {
      //For when fluid properties are temperature dependant (ie. not constant).
      //If temperature IS a provided coupled variable and _temp_dependent_fluid_props = true
      //in the material block of the input file, this loop will execute and give temperature dependent
      //fluid properties and their derivatives
      if (_temp_dependent_fluid_props && _has_temp)
      {
        if (_water_steam_properties == NULL)
          mooseError("You forgot to provide a UserObject for the EOS fluid property calculations... water_steam_properties in the material block of your input file needs an entry");
          
        Real _dens_water_PT;
        Real _visc_water_PT;
        Real _time_old_dens_water_PT;
        Real _var;
        Real _density_with_temperature_step;
        Real _density_with_pressure_step;
                
        //Obtaining value for density when given parameters are temperature and pressure (no enthalpy)
        //mooseAssert(_temperature[qp] > 0 && _temperature[qp] < 1000, "Temperature is not a real number!!");
										
        _water_steam_properties->waterEquationOfStatePT (_pressure[qp], _temperature[qp], _var, _dens_water_PT);
					                
        _dens_water_out[qp] = _dens_water_PT;
                         
		if (_is_transient)
        {
            //Obtaining value for density_old when given parameters are temperature and pressure (no enthalpy)
            _water_steam_properties->waterEquationOfStatePT (_pressure_old[qp], _temperature_old[qp], _var, _time_old_dens_water_PT);
                
            _time_old_dens_water_out[qp] = _time_old_dens_water_PT;
        }
          
                
        //Obtaining value for viscosity when given parameters are temperature and pressure (no enthalpy)
        _water_steam_properties->viscosity (_dens_water_PT, _temperature[qp], _visc_water_PT);
                
        _visc_water_out[qp] = _visc_water_PT;
                
    
        //Obtaining numerical derivative of water density w.r.t. a pressure step (temperature held constant)
        _water_steam_properties->waterEquationOfStatePT ((_pressure[qp] + 0.1), _temperature[qp], _var, _density_with_pressure_step);
                
        _d_dens_d_press_PT[qp] = ((_density_with_pressure_step - _dens_water_PT) / 0.1);
                
                
        //Obtaining numerical derivative of water density w.r.t. a temperature step (pressure held constant)
        _water_steam_properties->waterEquationOfStatePT (_pressure[qp], (_temperature[qp] + 1.0e-6), _var, _density_with_temperature_step);
                
        _d_dens_d_temp_PT[qp] = ((_density_with_temperature_step - _dens_water_PT) / 1.0e-6);
                
                
        //Determining tau_water and darcy_flux fluid properties 
        Real _dens_water0 = 1E3;
        Real _visc_water0 = 5E-4;
                
        _dens_water0 =  _dens_water_out[qp];
        _visc_water0 =  _visc_water_out[qp];
                
        _tau_water[qp] = _permeability[qp] * _dens_water0 / _visc_water0;
        _darcy_mass_flux_water[qp] = -_tau_water[qp] * (_grad_p[qp] + _dens_water0  * _gravity[qp] * _gravity_vector[qp]);
        _darcy_mass_flux_water_pressure[qp] =  (-_tau_water[qp] * _grad_p[qp]);
        _darcy_mass_flux_water_elevation[qp] = (-_tau_water[qp] * _gravity[qp] * _gravity_vector[qp] * _dens_water0);
        _darcy_flux_water[qp] = _darcy_mass_flux_water[qp] / _dens_water0;    
                
      }
      //For when fluid properties are temperature INdependent (ie. constant).
      //If temperature IS NOT a provided coupled variable or if _temp_dependent_fluid_props = false
      //in the material block of the input file, this loop will execute and give constant single phase
      //fluid properties.  Input params constant_density and constant_viscosity can
      //be set in material block of input file to desired constant values, if not they
      //are initialized to 1000 kg/m3 and 0.12e-3 Pa.s
      else
      {
        _dens_water_out[qp] = _constant_density;
        _time_old_dens_water_out[qp] = _constant_density;
        _visc_water_out[qp] = _constant_viscosity;
        _d_dens_d_press_PT[qp] = 0.0;
        _d_dens_d_temp_PT[qp] = 0.0;
                    
        Real _dens_water0;
        Real _visc_water0;
        _dens_water0 =  _dens_water_out[qp];
        _visc_water0 =  _visc_water_out[qp];
                    
        _tau_water[qp] = _permeability[qp] * _dens_water0 / _visc_water0;
        _darcy_mass_flux_water[qp] = -_tau_water[qp] * (_grad_p[qp] + _dens_water0 * _gravity[qp] * _gravity_vector[qp]);
        _darcy_mass_flux_water_pressure[qp] =  (-_tau_water[qp] * _grad_p[qp]);
        _darcy_mass_flux_water_elevation[qp] = (-_tau_water[qp] * _gravity[qp] * _gravity_vector[qp] * _dens_water0);
        _darcy_flux_water[qp] = _darcy_mass_flux_water[qp] / _dens_water0;
      }
    }
            
  }
}
    
void FluidFlow::compute2PhProperties0(Real _per, Real  _Sw, Real _Denw, Real _Dens, Real _visw, Real _viss, Real& _watertau, Real  &_steamtau)
{
  Real _krw=1.0, _krs=0.0;
  Real _swe;  
// Brooks and Corey  
  _swe=(_Sw-0.3)/0.65;
  if(_swe <= 0.0)
  { _krw= 0.0; }
  else
  { if(_swe >= 1.0)
    { _krw= 1.0; }
    else
    { _krw= _swe*_swe*_swe*_swe; }
  }
    
  if(_swe <= 0.0)
  { _krs= 1.0; }
  else
  { if(_swe >= 1.0)
    {  _krs= 0.0;}
    else
    { _krs= (1-_swe*_swe)*(1-_swe)*(1-_swe); }
  }

// VanG
  /*    _swe=(_Sw-0.3)/0.7; 
        if(_swe <= 0.0)
        {_krw= 0.0;}
        else
        {if(_swe >= 1.0)
        {_krw= 1.0;}
        else
        {_krw= std::sqrt(_swe)*(1-std::sqrt((1-_swe*_swe)))*(1-std::sqrt((1-_swe*_swe))); }
        }
      
        if(_swe <= 0.0)
        {_krs= 1.0;}
        else
        {if(_swe >= 1.0)
        {_krs= 0.0;}
        else
        {_krs= std::sqrt(1-_swe)*(1-_swe*_swe); }
        }
  */
// Sorey and others, 1980
  /*  _swe=(_Sw-0.3)/0.65; 
      if(_swe <= 0.0)
      {_krw= 0.0;}
      else
      {if(_swe >= 1.0)
      {_krw= 1.0;}
      else
      {_krw= _swe*_swe*_swe*_swe; }
      } 
      if(_swe <= 0.0)
      {_krs= 1.0;}
      else
      {if(_swe >= 1.0)
      {_krs= 0.0;}
      else
      {_krs= 1-_krw; }
      }
  */
// Verma ,1986
  /*  _swe=(_Sw-0.2)/0.695;
      if(_swe <= 0.0)
      {_krw= 0.0;}
      else
      {if(_swe >= 1.0)
      {_krw= 1.0;}
      else
      {_krw= _swe*_swe*_swe;}
      }
      
      if(_swe <= 0.0)
      {_krs= 1.0;}
      else
      {if(_swe >= 1.0)
      {_krs= 0.0; }
      else
      {_krs=1.2984-1.9832*_swe+0.7432* _swe*_swe; }
      }

  */
      
    
  _watertau = _per* _Denw / _visw * _krw;  
  _steamtau = _per* _Dens / _viss * _krs;
     
}



