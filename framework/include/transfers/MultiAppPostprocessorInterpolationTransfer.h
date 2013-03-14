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

#ifndef MULTIAPPPOSTPROCESSORINTERPOLATIONTRANSFER_H
#define MULTIAPPPOSTPROCESSORINTERPOLATIONTRANSFER_H

#include "MultiAppTransfer.h"

class MooseVariable;
class MultiAppPostprocessorInterpolationTransfer;

template<>
InputParameters validParams<MultiAppPostprocessorInterpolationTransfer>();

/**
 * Transfers from spatially varying PostprocessorInterpolations in a MultiApp to the "master" system.
 */
class MultiAppPostprocessorInterpolationTransfer :
  public MultiAppTransfer
{
public:
  MultiAppPostprocessorInterpolationTransfer(const std::string & name, InputParameters parameters);
  virtual ~MultiAppPostprocessorInterpolationTransfer() {}

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

  PostprocessorName _postprocessor;
  AuxVariableName _to_var_name;
};

#endif /* MULTIAPPPOSTPROCESSORINTERPOLATIONTRANSFER_H */
