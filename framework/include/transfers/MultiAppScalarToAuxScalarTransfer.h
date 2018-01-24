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

#ifndef MULTIAPPSCALARTOAUXSCALARTRANSFER_H
#define MULTIAPPSCALARTOAUXSCALARTRANSFER_H

// MOOSE includes
#include "MultiAppTransfer.h"

// Forward declerations
class MultiAppScalarToAuxScalarTransfer;

template <>
InputParameters validParams<MultiAppScalarToAuxScalarTransfer>();

/**
 * Copies the value of a SCALAR variable from one App to another.
 */
class MultiAppScalarToAuxScalarTransfer : public MultiAppTransfer
{
public:
  MultiAppScalarToAuxScalarTransfer(const InputParameters & parameters);

  /**
   * Execute the transfer
   */
  virtual void execute() override;

protected:
  /// The name of the variable from which the values are being transfered
  VariableName _from_variable_name;

  /// The name of the variable to which the scalar values are being transfered
  VariableName _to_aux_name;
};

#endif /* MULTIAPPSCALARTOAUXSCALARTRANSFER_H */
