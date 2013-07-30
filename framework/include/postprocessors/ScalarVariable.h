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

#ifndef SCALARVARIABLE_H
#define SCALARVARIABLE_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class ScalarVariable;

template<>
InputParameters validParams<ScalarVariable>();

/**
 *
 */
class ScalarVariable : public GeneralPostprocessor
{
public:
  ScalarVariable(const std::string & name, InputParameters parameters);
  virtual ~ScalarVariable();

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

protected:
  MooseVariableScalar & _var;
  unsigned int _idx;
};



#endif /* SCALARVARIABLE_H */
