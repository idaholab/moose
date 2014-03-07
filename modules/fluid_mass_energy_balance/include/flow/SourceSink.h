/****************************************************************/
/*             DO NOT MODIFY OR REMOVE THIS HEADER              */
/*          FALCON - Fracturing And Liquid CONvection           */
/*                                                              */
/*       (c) pending 2012 Battelle Energy Alliance, LLC         */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "Kernel.h"

#ifndef SOURCESINK_H
#define SOURCESINK_H

// Forward Declaration
class SourceSink;

template<>
InputParameters validParams<SourceSink>();

/**
 * Define the Kernel for a SourceSink operator that looks like:
 *
 * grad_some_var dot u'
 *
 * This first line is defining the name and inheriting from Kernel.
 */
class SourceSink : public Kernel
{
public:

  SourceSink(const std::string & name, InputParameters parameters);

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

  Real _value;
  std::vector<Real> _point_param;
  std::vector<Real> _range_param;
  Point _p;

};
#endif //SOURCESINK_H
