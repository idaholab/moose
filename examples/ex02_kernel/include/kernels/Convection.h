#ifndef CONVECTION_H
#define CONVECTION_H

#include "Kernel.h"

/**
 * The forward declaration is so that we can declare the validParams function
 * before we actually define the class... that way the definition isn't lost
 * at the bottom of the file.
 */

//Forward Declarations
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
 * velocity dot u'
 * 
 * This first line is defining the name and inheriting from Kernel.
 */
class Convection : public Kernel
{
public:

  /**
   * This is the Constructor declaration AND definition.
   * It is ok to have the definition in the .h if the function body
   * is really small.  Otherwise it should be in the .C
   */
  Convection(const std::string & name,
             MooseSystem &sys,
             InputParameters parameters);

protected:
  /**
   * Responsible for computing the residual at one quadrature point
   *
   * This should always be defined in the .C
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
   * This should always be defined in the .C
   */
  virtual Real computeQpJacobian();

private:
  /**
   * A velocity vector that supports a dot product.
   */
  RealVectorValue _velocity;

  /**
   * Class variables to hold the components of velocity coming from the input parameters.
   */
  Real _x;
  Real _y;
  Real _z;
};

#endif //CONVECTION_H
