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

#ifndef NODALVARIABLEVECTORPOSTPROCESSOR_H
#define NODALVARIABLEVECTORPOSTPROCESSOR_H

#include "NodalVectorPostprocessor.h"
#include "Coupleable.h"

class MooseVariable;

//Forward Declarations
class NodalVariableVectorPostprocessor;

template<>
InputParameters validParams<NodalVariableVectorPostprocessor>();

class NodalVariableVectorPostprocessor : public NodalVectorPostprocessor
{
public:
  NodalVariableVectorPostprocessor(const std::string & name, InputParameters parameters);
};

#endif
