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

#ifndef SIMULATIONINFORMATION_H
#define SIMULATIONINFORMATION_H

// MOOSE includes
#include "BasicOutput.h"

// Forward declarations
class SimulationInformation;

template<>
InputParameters validParams<SimulationInformation>();

/**
 * Class for outputting simulations information to the Console output object
 */
class SimulationInformation :
  public BasicOutput<Output>
{
public:

  /**
   * Class constructor
   */
  SimulationInformation(const std::string & name, InputParameters);

  /**
   * Helper function function for stringstream formatting
   */
  static void insertNewline(std::stringstream & oss, std::streampos & begin, std::streampos & curr);

  /// Width used for printing simulation information
  static const unsigned int _field_width = 27;

  /// Line length for printing simulation information
  static const unsigned int _line_length = 100;

protected:

  /**
   * Overload the Output::output method, this is required for SimulationInformation
   * output due to the method utilized for outputting
   */
  virtual void output(const ExecFlagType & type);

  /**
   * Outputs the simulation information for output_on = 'initial'
   *
   * This allows for this information to print prior to all other output
   */
  virtual void init();

private:

  /**
   * Outputs framework information
   *
   * This includes the versions and timestamps as well as legacy flag status
   */
  void outputFrameworkInformation();

  /**
   * Output the mesh information
   */
  void outputMeshInformation();

  /**
   * Output the Auxiliary system information
   */
  void outputAuxiliarySystemInformation();

  /**
   * Output the Nonlinear system information
   */
  void outputNonlinearSystemInformation();

  /**
   * Output execution information
   */
  void outputExecutionInformation();

  /**
   * Output the output information
   */
  void outputOutputInformation();

  /**
   * Output system information
   * @param system The libMesh system to output
   * @see outputAuxiliarySystemInformation outputNonlinearSystemInformation
   */
  std::string outputSystemInformationHelper(const System & system);

  /// Flags for controlling the what simulations information is shown
  MultiMooseEnum _flags;

};

#endif // SIMULATIONINFORMATION_H
