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

#ifndef CONSTANTVECTORPOSTPROCESSOR_H
#define CONSTANTVECTORPOSTPROCESSOR_H

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class ConstantVectorPostprocessor;

template <>
InputParameters validParams<ConstantVectorPostprocessor>();

class ConstantVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  ConstantVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

protected:
  VectorPostprocessorValue & _value;
};

#endif
