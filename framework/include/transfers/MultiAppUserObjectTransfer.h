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

#include "MultiAppTransfer.h"

class MooseVariable;
class MultiAppUserObjectTransfer;

template<>
InputParameters validParams<MultiAppUserObjectTransfer>();

/**
 * Samples a variable's value in the Master domain at the point where the MultiApp is.
 * Copies that value into a postprocessor in the MultiApp.
 */
class MultiAppUserObjectTransfer :
  public MultiAppTransfer
{
public:
  MultiAppUserObjectTransfer(const std::string & name, InputParameters parameters);
  virtual ~MultiAppUserObjectTransfer() {}

  virtual void execute();

protected:
  AuxVariableName _to_var_name;
  std::string _user_object_name;

  bool _displaced_target_mesh;
};

#endif /* MULTIAPPVARIABLEVALUESAMPLEPOSTPROCESSORTRANSFER_H */
