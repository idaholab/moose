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

    Real phaseDetermine (Real enth_in, Real press_in, Real& phase, Real& temp_sat, Real& enth_water_sat, Real& enth_steam_sat, Real& dens_water_sat, Real& dens_steam_sat) const;

    Real waterEquationOfStatePH (Real enth_in, Real press_in, Real temp_in, Real temp_sat, Real& temp1, Real& dens1, Real& enth1) const;

    Real steamEquationOfStatePH (Real enth_in, Real press_in, Real temp_in, Real temp_sat, Real& temp2, Real& dens2, Real& enth2) const;

    Real viscosity (Real density, Real temp, Real& viscosity) const;

    Real waterEquationOfStatePT (Real press_in, Real temp, Real& enth_water, Real& dens) const;

    Real steamEquationOfStatePT (Real press_in, Real temp, Real& enth_steam, Real& dens) const;

    Real waterAndSteamEquationOfStatePropertiesPH (Real enth_in, Real press_in, Real temp_in, Real& phase, Real& temp_out, Real& temp_sat, Real& sat_fraction_out, Real& dens_out, Real& dens_water_out, Real& dens_steam_out, Real& enth_water_out, Real& enth_steam_out, Real& visc_water_out, Real& visc_steam_out, Real& del_press, Real& del_enth) const;

    Real waterAndSteamEquationOfStatePropertiesWithDerivativesPH (Real enth_in, Real press_in, Real temp_in, Real& temp_out, Real& sat_fraction_out, Real& dens_out, Real& dens_water_out, Real& dens_steam_out, Real& enth_water_out, Real& enth_steam_out, Real& visc_water_out, Real& visc_steam_out, Real& d_enth_water_d_press, Real& d_enth_steam_d_press, Real& d_dens_d_press, Real& d_temp_d_press, Real& d_enth_water_d_enth, Real& d_enth_steam_d_enth, Real& d_dens_d_enth, Real& d_temp_d_enth, Real& d_sat_fraction_d_enth) const;
};

#endif /* WATERSTEAMEOS_H */
