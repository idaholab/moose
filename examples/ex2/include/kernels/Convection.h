#include "Kernel.h"

#ifndef CONVECTION_H
#define CONVECTION_H

/**
 * The forward declaration is so that we can declare the valid_params function
 * before we actually define the class... that way the definition isn't lost
 * at the bottom of the file.
 */

//Forward Declarations
class Convection;

/**
 * valid_params returns the parameters that this Kernel accepts / needs
 * The actual body of the function MUST be in the .C file.
 */
template<>
InputParameters valid_params<Convection>();

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
  Convection(std::string name,
            InputParameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to,
            std::vector<std::string> coupled_as)

    // You must call the constructor of the base class first
    // The "true" here specifies that this Kernel is to be integrated
    // over the domain.
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as),

    // This is the "Intialization List" it sets the values of class variables
    // Here we are grabbing the values of Parameters to use for a velocity vector
    _x(_parameters.get<Real>("x")),
    _y(_parameters.get<Real>("y")),
    _z(_parameters.get<Real>("z"))
  {
    // Build a velocity vector to use in the residual / jacobian computations.
    // We do this here so that it's only done once and then we just reuse it.
    // Note that RealVectorValues ALWAYS have 3 components... even when running in
    // 2D or 1D.  This makes the code simpler...
    velocity(0)=_x;
    velocity(1)=_y;
    velocity(2)=_z;
  }

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
  RealVectorValue velocity;

  /**
   * Class variables to hold the components of velocity coming from the input parameters.
   */
  Real _x;
  Real _y;
  Real _z;
};
#endif //CONVECTION_H
