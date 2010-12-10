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

#ifndef SLAVECONSTRAINT_H
#define SLAVECONSTRAINT_H

// Moose Includes
#include "DiracKernel.h"

//Forward Declarations
class SlaveConstraint;

template<>
InputParameters validParams<SlaveConstraint>();

class SlaveConstraint : public DiracKernel
{
public:
  SlaveConstraint(const std::string & name, InputParameters parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
protected:
  NumericVector<Number> & _residual_copy;

  Node * _node;
};
 
#endif //SLAVECONSTRAINT_H
