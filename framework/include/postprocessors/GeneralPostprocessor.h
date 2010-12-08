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
#include "MooseObject.h"
#include "PostprocessorInterface.h"

//Forward Declarations
class GeneralPostprocessor;

template<>
InputParameters validParams<GeneralPostprocessor>();

/* This class is here to combine the Postprocessor interface and the
 * base class Postprocessor object along with adding MooseObject to the inheritence tree*/
class GeneralPostprocessor : public Postprocessor, public MooseObject, protected PostprocessorInterface
{
public:
  GeneralPostprocessor(const std::string & name, InputParameters parameters);
  
  virtual ~GeneralPostprocessor()
    {}

protected:
  MooseSystem & _moose_system;
};
 
#endif
