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

#ifndef MULTIAPPPOSTPROCESSORTOAUXSCALARTRANSFER_H
#define MULTIAPPPOSTPROCESSORTOAUXSCALARTRANSFER_H

// MOOSE includes
#include "MultiAppTransfer.h"

// Forward declerations
class MooseVariable;
class MultiAppPostprocessorToAuxScalarTransfer;

template<>
InputParameters validParams<MultiAppPostprocessorToAuxScalarTransfer>();

/**
 * Copies the value of a Postprocessor from one app to a scalar AuxVariable in another.
 */
class MultiAppPostprocessorToAuxScalarTransfer :
  public MultiAppTransfer
{
public:

  /**
   * Class constructor.
   */
  MultiAppPostprocessorToAuxScalarTransfer(const std::string & name, InputParameters parameters);

  /**
   * Classs destructor.
   */
  virtual ~MultiAppPostprocessorToAuxScalarTransfer() {}

  /**
   * Execute the transfer
   */
  virtual void execute();

protected:

  /// The name of the postprocessor that the transfer originates
  PostprocessorName _from_pp_name;

  /// The name of the variable to which the postprocessor is being transfered
  VariableName _to_aux_name;
};

#endif /* MULTIAPPPOSTPROCESSORTOAUXSCALARTRANSFER_H */
