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

#ifndef CONTROLOUTPUT_H
#define CONTROLOUTPUT_H

// MOOSE includes
#include "BasicOutput.h"
#include "Output.h"

// Forward declarations
class ControlOutput;

template <>
InputParameters validParams<ControlOutput>();

/**
 * Class for output information regarding Controls to the screen
 */
class ControlOutput : public BasicOutput<Output>
{
public:
  /**
   * Class constructor
   */
  ControlOutput(const InputParameters & parameters);

protected:
  /**
   * Perform the output of control information
   */
  virtual void output(const ExecFlagType & type) override;

private:
  /**
   * Output a list of active MooseObjects
   */
  void outputActiveObjects();

  /**
   * Output list of controllable parameters
   */
  void outputControls();

  /**
   * Output list of parameters that have been controlled
   */
  void outputChangedControls();

  /// Flag for clearing the controlled parameters after they are output
  bool _clear_after_output;

  /// Flag for showing active objects
  bool _show_active_objects;
};

#endif /* CONTROLOUTPUT_H */
