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

#ifndef NEUMANNBC_H
#define NEUMANNBC_H

#include "IntegratedBC.h"


class NeumannBC;

template<>
InputParameters validParams<NeumannBC>();

/**
 * Implements a simple constant Neumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class NeumannBC : public IntegratedBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NeumannBC(const std::string & name, InputParameters parameters);


protected:
  virtual Real computeQpResidual();

  /// Value of grad(u) on the boundary.
  const Real & _value;
};


#endif //NEUMANNBC_H
