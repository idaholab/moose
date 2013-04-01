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
  PostprocessorName _postprocessor;
  AuxVariableName _to_var_name;

  unsigned int _num_points;
  Real _power;
  MooseEnum _interp_type;
  Real _radius;
};

#endif /* MULTIAPPPOSTPROCESSORINTERPOLATIONTRANSFER_H */
