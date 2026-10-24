//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GasTransport.h"
#include "MooseError.h"

#include <array>
#include <utility>

namespace
{
// GRI-Mech 3.0 transport data as distributed with Cantera 3.2.0 (gri30.yaml), the same source as
// the NASA 7-coefficient data. The parameters of H2O are meant for a Stockmayer potential with a
// dipole correction, which the Lennard-Jones collision integrals here do not apply.
const std::array<std::pair<std::string, GasTransport::LennardJonesParameters>, 7>
    lennard_jones_data = {{{"H2", {2.920e-10, 38.00}},
                           {"O2", {3.458e-10, 107.40}},
                           {"H2O", {2.605e-10, 572.40}},
                           {"CH4", {3.746e-10, 141.40}},
                           {"CO", {3.650e-10, 98.10}},
                           {"CO2", {3.763e-10, 244.00}},
                           {"N2", {3.621e-10, 97.53}}}};
}

namespace GasTransport
{
const LennardJonesParameters &
lennardJonesParameters(const std::string & name)
{
  for (const auto & [species, parameters] : lennard_jones_data)
    if (species == name)
      return parameters;

  mooseError("No Lennard-Jones parameters for species '", name, "'.");
}
}
