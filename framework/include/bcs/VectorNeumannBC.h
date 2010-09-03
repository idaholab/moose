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

#ifndef VECTORNEUMANNBC_H
#define VECTORNEUMANNBC_H

#include "BoundaryCondition.h"

//libMesh includes
#include "vector_value.h"


//Forward Declarations
class VectorNeumannBC;

template<>
InputParameters validParams<VectorNeumannBC>();

/**
 * Implements a simple constant VectorNeumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class VectorNeumannBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  VectorNeumannBC(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
 virtual ~VectorNeumannBC(){}

protected:
  virtual Real computeQpResidual();
  
private:
  /**
   * Vector to dot with the normal.
   */
  VectorValue<Real> _value;
};

#endif //NEUMANNBC_H
