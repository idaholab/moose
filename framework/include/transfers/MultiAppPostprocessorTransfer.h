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

#ifndef MULTIAPPPOSTPROCESSORTRANSFER_H
#define MULTIAPPPOSTPROCESSORTRANSFER_H

#include "MultiAppTransfer.h"

class MooseVariable;
class MultiAppPostprocessorTransfer;

template<>
InputParameters validParams<MultiAppPostprocessorTransfer>();

/**
 * Copies the value of a Postprocessor from the Master to a MultiApp
 */
class MultiAppPostprocessorTransfer :
  public MultiAppTransfer
{
public:
  MultiAppPostprocessorTransfer(const std::string & name, InputParameters parameters);
  virtual ~MultiAppPostprocessorTransfer() {}

  virtual void execute();

protected:
  PostprocessorName _from_pp_name;
  PostprocessorName _to_pp_name;
};

#endif /* MULTIAPPPOSTPROCESSORTRANSFER_H */
