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

#ifndef LINEARCOMBINATIONAUX_H
#define LINEARCOMBINATIONAUX_H

#include "AuxKernel.h"

class LinearCombinationAux;

template<>
InputParameters validParams<LinearCombinationAux>();

/**
 * Compute a linear combination of variables
 *
 * \$ u = \Sum v_i * w_i\$
 */
class LinearCombinationAux : public AuxKernel
{
public:
  LinearCombinationAux(const std::string & name, InputParameters parameters);
  virtual ~LinearCombinationAux();

protected:
  virtual Real computeValue();

  /// The number of entries (i.e. the length of _v and _w)
  unsigned int _n;
  /// First variable
  std::vector<VariableValue *> _v;
  /// Second variable
  std::vector<VariableValue *> _w;
};

#endif /* LINEARCOMBINATIONAUX_H */
