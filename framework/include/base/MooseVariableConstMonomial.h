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

#ifndef MOOSEVARIABLECONSTMONOMIAL_H
#define MOOSEVARIABLECONSTMONOMIAL_H

#include "MooseTypes.h"
#include "MooseVariable.h"

class MooseVariableConstMonomial : public MooseVariable
{
public:
  MooseVariableConstMonomial(unsigned int var_num,
                             const FEType & fe_type,
                             SystemBase & sys,
                             Assembly & assembly,
                             Moose::VarKindType var_kind);

  virtual void computeElemValues() override;
};

#endif /* MOOSEVARIABLECONSTMONOMIAL_H */
