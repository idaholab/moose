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

#ifndef BOUNDSAUX_H
#define BOUNDSAUX_H

#include "AuxKernel.h"


//Forward Declarations
class BoundsAux;

template<>
InputParameters validParams<BoundsAux>();

/**
 * Fills in the "bounds vectors" to provide an upper and lower bound for the variable that is coupled in.
 * Doesn't actually calculate an auxiliary value although it must take an auxiliary variable as input.
 *
 * This MUST be run on a Nodal Auxiliary Variable!
 */
class BoundsAux : public AuxKernel
{
public:

  /**
   * Factory constructor.
   */
  BoundsAux(const std::string & name, InputParameters parameters);

protected:
  /**
   * Will store both the upper and lower bound into their respective vectors.
   */
  virtual Real computeValue();

  NumericVector<Number> & _upper_vector;
  NumericVector<Number> & _lower_vector;
  unsigned int _bounded_variable_id;
  bool _upper_valid;
  bool _lower_valid;
  Real _upper;
  Real _lower;


};

#endif //BOUNDSAUX_H
