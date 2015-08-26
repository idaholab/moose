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

#ifndef CONTROL_H
#define CONTROL_H

// MOOSE includes
#include "GeneralUserObject.h"
#include "ControlInterface.h"

// Forward declarations
class Control;

template<>
InputParameters validParams<Control>();

/**
 * Base class for Control objects
 *
 * Control objects are simply GeneralUserObjects with an additional interface
 * for accessing InputParameters via the InputParameterWarehouse. These objects
 * are create by the [Controls] block in the input file after all other MooseObjects
 * are created, so they have access to parameters in all other MooseObjects.
 */
class Control :
  public GeneralUserObject,
  public ControlInterface
{
public:

  /**
   * Class constructor
   * @param parameters The input parameters for this control object
   */
  Control(const InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~Control(){}
};

#endif //FUNCTIONCONTROL_H
