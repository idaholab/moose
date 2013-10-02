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

#ifndef MATTESTNEUMANNBC_H
#define MATTESTNEUMANNBC_H

#include "NeumannBC.h"

class MatTestNeumannBC;

template<>
InputParameters validParams<MatTestNeumannBC>();

/**
 * Neumann boundary condition for testing BoundaryRestrictable class
 */
class MatTestNeumannBC : public NeumannBC
{
public:
  MatTestNeumannBC(const std::string & name, InputParameters parameters);

protected:

  virtual Real computeQpResidual();

  const std::string _prop_name;

  MaterialProperty<Real> * _value;
};

#endif /* MATTESTNEUMANNBC_H */
