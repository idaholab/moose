/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef NSNAMES_H
#define NSNAMES_H

#include <string>

namespace NS
{
const std::string density = "rho";
const std::string momentum_x = "rhou";
const std::string momentum_y = "rhov";
const std::string momentum_z = "rhow";
const std::string total_energy = "rhoE";

const std::string velocity_x = "vel_x";
const std::string velocity_y = "vel_y";
const std::string velocity_z = "vel_z";
const std::string pressure = "pressure";
const std::string temperature = "temperature";
const std::string enthalpy = "enthalpy";
const std::string mach_number = "Mach";
const std::string internal_energy = "internal_energy";
const std::string specific_volume = "specific_volume";
}

#endif
