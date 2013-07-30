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

#ifndef NUMNODES_H
#define NUMNODES_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class NumNodes;

template<>
InputParameters validParams<NumNodes>();

class NumNodes : public GeneralPostprocessor
{
public:
  NumNodes(const std::string & name, InputParameters parameters);

  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the number of nodes in the system
   */
  virtual Real getValue();
};

#endif // NUMNODES_H
