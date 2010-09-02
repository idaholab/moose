/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
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
   * @param moose_system The reference to the MooseSystem that this object is contained within
   * @param parameters The parameters object holding data for the class to use.
   */
  ConvectionDiffusionSUPG(std::string name,
                          MooseSystem & moose_system,
                          InputParameters parameters);
  
  virtual void computeTausAndVelocities();

protected:
  /**
   * Hyperbolic Cotangent
   */
  Real coth(Real x);

  /**
   * Diffusion coefficient.
   */
  Real _coef;

  /**
   * Reaction coefficient.
   */
  Real _rcoef;
  
  /**
   * A velocity vector that supports a dot product.
   */
  RealVectorValue _my_velocity;

  /**
   * Class variables to hold the components of velocity coming from the input parameters.
   */
  Real _x;
  Real _y;
  Real _z;

  /**
   * Velocity Magnitude
   */
  Real _vel_mag;
};

#endif //CONVECTIONDIFFUSIONSUPG_H
