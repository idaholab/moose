//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/**
 *  FormulationEnums contains the enumeration used to determine between
 *  time or frequency (i.e., time-harmonic) formulations involving electromagnetic electrical fields
 *  and used to determine between solving for an electrostatic or
 *  electromagnetic electric field
 */
namespace ElectromagneticFormulation
{
/// Enum used when determining the domain of the formulation
enum FormEnum
{
  TIME,
  FREQUENCY,
};

/// Enum used when determining the sover
enum SolverEnum
{
  ELECTROSTATIC,
  ELECTROMAGNETIC
};
} // namespace FM
