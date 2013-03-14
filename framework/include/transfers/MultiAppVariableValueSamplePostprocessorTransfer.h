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

#ifndef MULTIAPPVARIABLEVALUESAMPLEPOSTPROCESSORTRANSFER_H
#define MULTIAPPVARIABLEVALUESAMPLEPOSTPROCESSORTRANSFER_H

#include "MultiAppTransfer.h"

class MooseVariable;
class MultiAppVariableValueSamplePostprocessorTransfer;

template<>
InputParameters validParams<MultiAppVariableValueSamplePostprocessorTransfer>();

/**
 * Samples a variable's value in the Master domain at the point where the MultiApp is.
 * Copies that value into a postprocessor in the MultiApp.
 */
class MultiAppVariableValueSamplePostprocessorTransfer :
  public MultiAppTransfer
{
public:
  MultiAppVariableValueSamplePostprocessorTransfer(const std::string & name, InputParameters parameters);
  virtual ~MultiAppVariableValueSamplePostprocessorTransfer() {}

  virtual void execute();

protected:
  AuxVariableName _postprocessor_name;
  PostprocessorName _from_var_name;
};

#endif /* MULTIAPPVARIABLEVALUESAMPLEPOSTPROCESSORTRANSFER_H */
