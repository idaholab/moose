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

#ifndef MULTIAPPMESHFUNCTIONTRANSFER_H
#define MULTIAPPMESHFUNCTIONTRANSFER_H

#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppMeshFunctionTransfer;

template <>
InputParameters validParams<MultiAppMeshFunctionTransfer>();

/**
 * Samples a variable's value in the Master domain at the point where
 * the MultiApp is.  Copies that value into a postprocessor in the
 * MultiApp.
 */
class MultiAppMeshFunctionTransfer : public MultiAppTransfer
{
public:
  MultiAppMeshFunctionTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void execute() override;

protected:
  AuxVariableName _to_var_name;
  VariableName _from_var_name;
  bool _error_on_miss;
};

#endif /* MULTIAPPMESHFUNCTIONTRANSFER_H */
