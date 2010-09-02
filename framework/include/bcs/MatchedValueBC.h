/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
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

#include "BoundaryCondition.h"

//Forward Declarations
class MatchedValueBC;

template<>
InputParameters validParams<MatchedValueBC>();

/**
 * Implements a simple coupled boundary condition where u=v on the boundary.
 */
class MatchedValueBC : public BoundaryCondition
{
public:
  MatchedValueBC(std::string name, MooseSystem & moose_system, InputParameters parameters);
    
  virtual ~MatchedValueBC() {}

protected:
  virtual Real computeQpResidual();

private:
  VariableValue & _v_face;
};

#endif //MATCHEDVALUEBC_H
