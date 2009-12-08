#include "Kernel.h"

#ifndef CONVECTION_H
#define CONVECTION_H

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
 * grad_some_var dot u'
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

    // coupledGrad will give us a reference to the gradient of another
    // variable in the computation.  We are going to use that gradient
    // as our velocity vector.
    //
    // Note that "some_var" is the name this Kernel expects... ie
    // what should be in "coupled_as"
    // 
    // You can also use coupledVal() and coupledValOld() if you want
    // values
    _grad_some_var(coupledGrad("some_var"))
  {
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
   * Coupled things come through as std::vector _refernces_.
   *
   * Since this is a reference it MUST be set in the Initialization List of the
   * constructor!
   */
  std::vector<RealGradient> & _grad_some_var;
};
#endif //CONVECTION_H
