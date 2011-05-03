/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef CONVECTIONDIFFUSIONSUPG_H
#define CONVECTIONDIFFUSIONSUPG_H

#include "Moose.h"
#include "SUPGBase.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

//Forward Declarations
class ConvectionDiffusionSUPG;

template<>
InputParameters validParams<ConvectionDiffusionSUPG>();

/**
 * 
 */
class ConvectionDiffusionSUPG : public SUPGBase
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   */
  ConvectionDiffusionSUPG(const std::string & name, InputParameters parameters);
  
  virtual void computeTausAndVelocities();

protected:
  /**
   * Hyperbolic Cotangent
   */
  Real coth(Real x);

  Real _coef;                           ///< Diffusion coefficient
  Real _rcoef;                          ///< Reaction coefficient
  RealVectorValue _my_velocity;         ///< A velocity vector that supports a dot product

  /**
   * Class variables to hold the components of velocity coming from the input parameters.
   */
  Real _x;
  Real _y;
  Real _z;

  Real _vel_mag;                        ///< Velocity Magnitude
};

#endif //CONVECTIONDIFFUSIONSUPG_H
