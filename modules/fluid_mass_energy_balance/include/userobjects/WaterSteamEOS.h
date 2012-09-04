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

#ifndef WATERSTEAMEOS_H
#define WATERSTEAMEOS_H

#include "GeneralUserObject.h"

class WaterSteamEOS;

template<>
InputParameters validParams<WaterSteamEOS>();

class WaterSteamEOS : public GeneralUserObject
{
public:
  WaterSteamEOS(const std::string & name, InputParameters params);
    
    virtual ~WaterSteamEOS();
    
    virtual void initialize(){}
    
    virtual void execute(){}
    virtual void finalize(){}
    
    virtual void destroy();

    Real phase_determ(Real _enth_in, Real _press_in, Real& _phase, Real& _temp_sat, Real& _enth_water_sat, Real& _enth_steam_sat, Real& _dens_water_sat, Real& _dens_steam_sat) const;
    
    Real water_eq_of_state(Real _enth_in, Real _press_in, Real _temp_sat, Real& _temp1, Real& _dens1, Real& _enth1) const;
    
    Real steam_eq_of_state(Real _enth_in, Real _press_in, Real _temp_sat, Real& _temp2, Real& _dens2, Real& _enth2) const;
    
    Real viscosity(Real _density, Real _temp, Real& _viscosity) const;
    
    Real water_EOS_deriv_init_values(Real _press_in, Real _temp_0, Real& _enth_water_0, Real& _dens_0) const;
    
    Real steam_EOS_deriv_init_values(Real _press_in, Real _temp_0, Real& _enth_steam_0, Real& _dens_0) const;
    
    Real Equations_of_State_Properties(Real _enth_in, Real _press_in, Real& _phase, Real& _temp_out, Real& _temp_sat, Real& _sat_fraction_out, Real& _dens_out, Real& _dens_water_out, Real& _dens_steam_out, Real& _enth_water_out, Real& _enth_steam_out, Real& _visc_water_out, Real& _visc_steam_out, Real& _del_press, Real& _del_enth) const;    
    
    Real Equations_of_State_Derivative_Properties(Real _enth_in, Real _press_in, Real& _temp_out, Real& _sat_fraction_out, Real& _dens_out, Real& _dens_water_out, Real& _dens_steam_out, Real& _enth_water_out, Real& _enth_steam_out, Real& _visc_water_out, Real& _visc_steam_out, Real& _del_press, Real& _del_enth, Real& _d_enth_water_d_press, Real& _d_enth_steam_d_press, Real& _d_dens_d_press, Real& _d_temp_d_press, Real& _d_enth_water_d_enth, Real& _d_enth_steam_d_enth, Real& _d_dens_d_enth, Real& _d_temp_d_enth, Real& _d_sat_fraction_d_enth) const;    
};

#endif /* WATERSTEAMEOS_H */
