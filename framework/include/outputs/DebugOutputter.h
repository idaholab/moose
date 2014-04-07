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

#ifndef DEBUGOUTPUTTER_H
#define DEBUGOUTPUTTER_H

// MOOSE includes
#include "PetscOutputter.h"
#include "FEProblem.h"

// Forward declerations
class DebugOutputter;

template<>
InputParameters validParams<DebugOutputter>();

/**
 *
 */
class DebugOutputter : public PetscOutputter
{
public:

  /**
   * Class constructor
   * @param name
   * @param parameters
   */
  DebugOutputter(const std::string & name, InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~DebugOutputter();

protected:

  /**
   * Perform the debugging output
   */
  virtual void output();

  //@{
  /**
   * Individual component output is not supported for DebugOutputter
   */
  std::string filename();
  virtual void outputNodalVariables();
  virtual void outputElementalVariables();
  virtual void outputPostprocessors();
  virtual void outputScalarVariables();
  //@}

  /// Flag for outputting variable norms
  bool _show_var_residual_norms;

  // Reference to libMesh system
  TransientNonlinearImplicitSystem & _sys;
};

#endif //DEBUGOUTPUTTER_H
