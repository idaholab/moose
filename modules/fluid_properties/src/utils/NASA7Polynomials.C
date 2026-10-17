//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NASA7Polynomials.h"
#include "MooseError.h"
#include "MooseUtils.h"

namespace
{
// GRI-Mech 3.0 data as distributed with Cantera 3.2.0 (gri30.yaml), with the source precision and
// temperature ranges. The molar masses are the ones Cantera computes from its atomic weights
// (H 1.008, C 12.011, N 14.007, O 15.999 g/mol).
const std::array<NASA7::SpeciesData, 7> species_data = {{
    {"H2",
     2.016e-3,
     200.0,
     1000.0,
     3500.0,
     {2.34433112,
      7.98052075e-3,
      -1.94781510e-5,
      2.01572094e-8,
      -7.37611761e-12,
      -9.17935173e2,
      6.83010238e-1},
     {3.33727920,
      -4.94024731e-5,
      4.99456778e-7,
      -1.79566394e-10,
      2.00255376e-14,
      -9.50158922e2,
      -3.20502331}},
    {"O2",
     31.998e-3,
     200.0,
     1000.0,
     3500.0,
     {3.78245636,
      -2.99673416e-3,
      9.84730201e-6,
      -9.68129509e-9,
      3.24372837e-12,
      -1.06394356e3,
      3.65767573},
     {3.28253784,
      1.48308754e-3,
      -7.57966669e-7,
      2.09470555e-10,
      -2.16717794e-14,
      -1.08845772e3,
      5.45323129}},
    {"H2O",
     18.015e-3,
     200.0,
     1000.0,
     3500.0,
     {4.19864056,
      -2.03643410e-3,
      6.52040211e-6,
      -5.48797062e-9,
      1.77197817e-12,
      -3.02937267e4,
      -8.49032208e-1},
     {3.03399249,
      2.17691804e-3,
      -1.64072518e-7,
      -9.70419870e-11,
      1.68200992e-14,
      -3.00042971e4,
      4.96677010}},
    {"CH4",
     16.043e-3,
     200.0,
     1000.0,
     3500.0,
     {5.14987613,
      -1.36709788e-2,
      4.91800599e-5,
      -4.84743026e-8,
      1.66693956e-11,
      -1.02466476e4,
      -4.64130376},
     {7.48514950e-2,
      1.33909467e-2,
      -5.73285809e-6,
      1.22292535e-9,
      -1.01815230e-13,
      -9.46834459e3,
      18.4373180}},
    {"CO",
     28.010e-3,
     200.0,
     1000.0,
     3500.0,
     {3.57953347,
      -6.10353680e-4,
      1.01681433e-6,
      9.07005884e-10,
      -9.04424499e-13,
      -1.43440860e4,
      3.50840928},
     {2.71518561,
      2.06252743e-3,
      -9.98825771e-7,
      2.30053008e-10,
      -2.03647716e-14,
      -1.41518724e4,
      7.81868772}},
    {"CO2",
     44.009e-3,
     200.0,
     1000.0,
     3500.0,
     {2.35677352,
      8.98459677e-3,
      -7.12356269e-6,
      2.45919022e-9,
      -1.43699548e-13,
      -4.83719697e4,
      9.90105222},
     {3.85746029,
      4.41437026e-3,
      -2.21481404e-6,
      5.23490188e-10,
      -4.72084164e-14,
      -4.87591660e4,
      2.27163806}},
    {"N2",
     28.014e-3,
     300.0,
     1000.0,
     5000.0,
     {3.29867700,
      1.40824040e-3,
      -3.96322200e-6,
      5.64151500e-9,
      -2.44485400e-12,
      -1.02089990e3,
      3.95037200},
     {2.92664000,
      1.48797680e-3,
      -5.68476000e-7,
      1.00970380e-10,
      -6.75335100e-15,
      -9.22797700e2,
      5.98052800}},
}};
}

namespace NASA7
{
const SpeciesData &
speciesData(const std::string & name)
{
  for (const auto & data : species_data)
    if (data.name == name)
      return data;

  mooseError("Unknown NASA-7 species '", name, "'.");
}

std::string
speciesNames()
{
  std::vector<std::string> names;
  for (const auto & data : species_data)
    names.push_back(data.name);
  return MooseUtils::join(names, " ");
}
}
