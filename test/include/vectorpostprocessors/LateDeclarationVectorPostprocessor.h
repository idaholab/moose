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

#ifndef LATEDECLARATIONVECTORPOSTPROCESSOR_H
#define LATEDECLARATIONVECTORPOSTPROCESSOR_H

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class LateDeclarationVectorPostprocessor;

template <>
InputParameters validParams<LateDeclarationVectorPostprocessor>();

class LateDeclarationVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  LateDeclarationVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

protected:
  VectorPostprocessorValue * _value;
};

#endif
