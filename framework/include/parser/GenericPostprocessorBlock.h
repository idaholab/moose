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

#ifndef GENERICPOSTPROCESSORBLOCK_H
#define GENERICPOSTPROCESSORBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericPostprocessorBlock;

template<>
InputParameters validParams<GenericPostprocessorBlock>();

class GenericPostprocessorBlock: public ParserBlock
{
public:
  GenericPostprocessorBlock(const std::string & name, InputParameters params);

  virtual void execute();

protected:
  Moose::PostprocessorType _pps_type;

private:
  std::string _type;
};

  

#endif //GENERICPOSTPROCESSORBLOCK_H
