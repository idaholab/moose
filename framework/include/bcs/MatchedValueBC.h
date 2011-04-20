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

#ifndef MATCHEDVALUEBC_H
#define MATCHEDVALUEBC_H

#include "NodalBC.h"

//Forward Declarations
class MatchedValueBC;

template<>
InputParameters validParams<MatchedValueBC>();

/**
 * Implements a simple coupled boundary condition where u=v on the boundary.
 */
class MatchedValueBC : public NodalBC
{
public:
  MatchedValueBC(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  VariableValue & _v_face;
};

#endif //MATCHEDVALUEBC_H
