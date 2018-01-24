//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
