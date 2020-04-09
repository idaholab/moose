//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolutionUserObject.h"

// Forward Declarations

/**
 * This Userobject is the base class of Userobjects that generate one
 * random number per timestep and quadrature point in a way that the integral
 * over all random numbers is zero. This can be used for a concentration fluctuation
 * kernel such as ConservedLangevinNoise, that keeps the total concenration constant.
 *
 * \see ConservedUniformNoise
 */
class SolutionRasterizer : public SolutionUserObject
{
public:
  static InputParameters validParams();

  SolutionRasterizer(const InputParameters & parameters);

  virtual ~SolutionRasterizer() {}

  /// Initialize the System and Mesh objects for the solution being read
  virtual void initialSetup();

protected:
  FileName _xyz_input;
  FileName _xyz_output;

  std::string _variable;

  MooseEnum _raster_mode;

  Real _threshold;
};
