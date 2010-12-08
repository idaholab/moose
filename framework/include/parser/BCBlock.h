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

#ifndef BCBLOCK_H
#define BCBLOCK_H

#include "ParserBlock.h"
#include "InputParameters.h"

//Forward Declarations
//class InputParameters;
class Parser;
class BCBlock;

template<>
InputParameters validParams<BCBlock>();

class BCBlock: public ParserBlock
{
public:
  BCBlock(const std::string & name, InputParameters params);

  virtual void execute();
};

  

#endif //BCBLOCK_H
