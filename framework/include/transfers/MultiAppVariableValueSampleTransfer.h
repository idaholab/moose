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

#ifndef MULTIAPPVARIABLEVALUESAMPLETRANSFER_H
#define MULTIAPPVARIABLEVALUESAMPLETRANSFER_H

#include "MultiAppTransfer.h"

class MooseVariable;
class MultiAppVariableValueSampleTransfer;

template<>
InputParameters validParams<MultiAppVariableValueSampleTransfer>();

/**
 * Samples a variable's value in the Master domain at the point where the MultiApp is.
 * Copies that value into a field in the MultiApp.
 */
class MultiAppVariableValueSampleTransfer :
  public MultiAppTransfer
{
public:
  MultiAppVariableValueSampleTransfer(const std::string & name, InputParameters parameters);
  virtual ~MultiAppVariableValueSampleTransfer() {}

  virtual void execute();

protected:
  /**
   * Small helper function for finding the system containing the variable.
   *
   * Note that this implies that variable names are unique across all systems!
   *
   * @param es The EquationSystems object to be searched.
   * @param var_name The name of the variable you are looking for.
   */
  System * find_sys(EquationSystems & es, std::string & var_name);

  AuxVariableName _to_var_name;
  VariableName _from_var_name;
};

#endif /* MULTIAPPVARIABLEVALUESAMPLETRANSFER_H */
