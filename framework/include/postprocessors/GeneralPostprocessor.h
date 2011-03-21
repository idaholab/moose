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

#ifndef GENERALPOSTPROCESSOR_H
#define GENERALPOSTPROCESSOR_H

#include "Postprocessor.h"
#include "PostprocessorInterface.h"
#include "Problem.h"


//Forward Declarations
class GeneralPostprocessor;

template<>
InputParameters validParams<GeneralPostprocessor>();

/* This class is here to combine the Postprocessor interface and the
 * base class Postprocessor object along with adding MooseObject to the inheritance tree*/
class GeneralPostprocessor :
  public Postprocessor,
  protected PostprocessorInterface
{
public:
  GeneralPostprocessor(const std::string & name, InputParameters parameters);
  
  virtual ~GeneralPostprocessor() {}
};
 
#endif
