/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SOLUTIONRASTERIZER_H
#define SOLUTIONRASTERIZER_H

#include "SolutionUserObject.h"

// Forward Declarations
class SolutionRasterizer;

template <>
InputParameters validParams<SolutionRasterizer>();

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

#endif // SOLUTIONRASTERIZER_H
