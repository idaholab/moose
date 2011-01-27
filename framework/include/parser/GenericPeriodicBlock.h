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

#ifndef GENERICPERIODICBLOCK_H
#define GENERICPERIODICBLOCK_H

#include "ParserBlock.h"

class GenericPeriodicBlock;

template<>
InputParameters validParams<GenericPeriodicBlock>();

class GenericPeriodicBlock: public ParserBlock
{
public:
  GenericPeriodicBlock(const std::string & name, InputParameters params);

  virtual void execute();

  void setPeriodicVars(PeriodicBoundary & p, const std::vector<std::string> & var_names);

private:
  std::string _type;
};


  

#endif //GENERICPERIODICBLOCK_H
