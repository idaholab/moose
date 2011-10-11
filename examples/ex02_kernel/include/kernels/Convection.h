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

#ifndef CONVECTION_H
#define CONVECTION_H

#include "Kernel.h"

/**
 * The forward declaration is so that we can declare the validParams() function
 * before we actually define the class... that way the definition isn't lost
 * at the bottom of the file.
 */

// Forward Declarations
class Convection;

/**
 * validParams returns the parameters that this Kernel accepts / needs
 * The actual body of the function MUST be in the .C file.
 */
template<>
InputParameters validParams<Convection>();

/**
 * Define the Kernel for a convection operator that looks like:
 *
 * V . grad(u)
 *
 * where V is a given constant velocity field.
 */
class Convection : public Kernel
{
public:

  /**
   * This is the constructor declaration.  This class takes a
   * string and a InputParameters object, just like other 
   * Kernel-derived classes.
   */
  Convection(const std::string & name,
             InputParameters parameters);

protected:
  /**
   * Responsible for computing the residual at one quadrature point.
   * This function should always be defined in the .C file.
   */
  virtual Real computeQpResidual();

  /**
   * Responsible for computing the diagonal block of the preconditioning matrix.
   * This is essentially the partial derivative of the residual with respect to
   * the variable this kernel operates on ("u").
   *
   * Note that this can be an approximation or linearization.  In this case it's
   * not because the Jacobian of this operator is easy to calculate.
   *
   * This function should always be defined in the .C file.
   */
  virtual Real computeQpJacobian();

private:
  /**
   * A vector object for storing the velocity.  Convenient for
   * computing dot products.
   */
  RealVectorValue _velocity;

  /**
   * Class variables to hold the components of velocity coming from the input parameters.
   */
  Real _x;
  Real _y;
  Real _z;
};

#endif // CONVECTION_H
