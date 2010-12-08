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

#ifndef GENERICBCBLOCK_H
#define GENERICBCBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericBCBlock;

template<>
InputParameters validParams<GenericBCBlock>();

class GenericBCBlock: public ParserBlock
{
public:
  GenericBCBlock(const std::string & name, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICBCBLOCK_H
