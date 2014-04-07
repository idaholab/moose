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

#ifndef EIGENKERNEL_H
#define EIGENKERNEL_H

#include "Kernel.h"

//Forward Declarations
class EigenKernel;

template<>
InputParameters validParams<EigenKernel>();

/*
 * The behavior of this kernel is controlled by one problem-wise global parameter
 *    eigen_on_current - bool, to indicate if this kernel is operating on the current solution or old solution
 * This kernel also obtain the postprocessor for eigenvalue by one problem-wise global parameter
 *    eigen_postprocessor - string, the name of the postprocessor to obtain the eigenvalue
 */
class EigenKernel : public Kernel
{
public:

  EigenKernel(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  VariableValue & _u_old;

  const bool & _current;
  const PostprocessorName & _eigen_pp;
  const Real & _eigen;
  const Real & _eigen_old;
};
#endif //EIGENKERNEL_H
