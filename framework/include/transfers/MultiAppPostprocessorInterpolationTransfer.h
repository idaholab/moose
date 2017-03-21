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

// MOOSE includes
#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppPostprocessorInterpolationTransfer;

template <>
InputParameters validParams<MultiAppPostprocessorInterpolationTransfer>();

/**
 * Transfers from spatially varying PostprocessorInterpolations in a MultiApp to the "master"
 * system.
 */
class MultiAppPostprocessorInterpolationTransfer : public MultiAppTransfer
{
public:
  MultiAppPostprocessorInterpolationTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  PostprocessorName _postprocessor;
  AuxVariableName _to_var_name;

  unsigned int _num_points;
  Real _power;
  MooseEnum _interp_type;
  Real _radius;
};

#endif /* MULTIAPPPOSTPROCESSORINTERPOLATIONTRANSFER_H */
