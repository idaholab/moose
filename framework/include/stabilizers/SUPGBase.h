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

#ifndef SUPGBASE_H
#define SUPGBASE_H

#include "Moose.h"
#include "Stabilizer.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

//Forward Declarations
class SUPGBase;

template<>
InputParameters validParams<SUPGBase>();

/**
 * 
 */
class SUPGBase : public Stabilizer
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param moose_system The reference to the MooseSystem that this object is contained within
   * @param parameters The parameters object holding data for the class to use.
   */
  SUPGBase(std::string name,
           MooseSystem & moose_system,
           InputParameters parameters);

  /**
   * Compute the test functions.
   */
  virtual void computeTestFunctions();

protected:

  /**
   * This function MUST be overriden!
   *
   * Override it to fill the Tau and Velocity vectors for
   * each quadrature point.  Note that this means you MUST
   * do your own quadrature loop!
   */
  virtual void computeTausAndVelocities() = 0;
  
  /**
   * The velocity at each quadrature point.
   */
  std::vector<RealVectorValue> _velocity;

  /**
   * The Taus at each quadrature point.
   */
  std::vector<Real> _tau;
};

#endif //SUPGBASE_H
