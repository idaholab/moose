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

#ifndef DEBUGOUTPUT_H
#define DEBUGOUTPUT_H

// MOOSE includes
#include "PetscOutput.h"
#include "FEProblem.h"

// Forward declerations
class DebugOutput;

template<>
InputParameters validParams<DebugOutput>();

/**
 *
 */
class DebugOutput : public PetscOutput
{
public:

  /**
   * Class constructor
   * @param name
   * @param parameters
   */
  DebugOutput(const std::string & name, InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~DebugOutput();

protected:

  /**
   * Perform the debugging output
   */
  virtual void output();

  //@{
  /**
   * Individual component output is not supported for DebugOutput
   */
  std::string filename();
  virtual void outputNodalVariables();
  virtual void outputElementalVariables();
  virtual void outputPostprocessors();
  virtual void outputVectorPostprocessors();
  virtual void outputScalarVariables();
  //@}

  /// Flag for outputting variable norms
  bool _show_var_residual_norms;

  // Reference to libMesh system
  TransientNonlinearImplicitSystem & _sys;
};

#endif //DEBUGOUTPUT_H
