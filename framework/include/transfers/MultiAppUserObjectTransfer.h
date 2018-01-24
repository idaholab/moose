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

#ifndef MULTIAPPUSEROBJECTTRANSFER_H
#define MULTIAPPUSEROBJECTTRANSFER_H

// MOOSE includes
#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppUserObjectTransfer;

template <>
InputParameters validParams<MultiAppUserObjectTransfer>();

/**
 * Samples a variable's value in the Master domain at the point where
 * the MultiApp is.  Copies that value into a postprocessor in the
 * MultiApp.
 */
class MultiAppUserObjectTransfer : public MultiAppTransfer
{
public:
  MultiAppUserObjectTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void execute() override;

protected:
  AuxVariableName _to_var_name;
  std::string _user_object_name;

  bool _displaced_target_mesh;
};

#endif // MULTIAPPVARIABLEVALUESAMPLEPOSTPROCESSORTRANSFER_H
