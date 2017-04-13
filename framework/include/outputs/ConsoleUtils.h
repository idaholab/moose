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

#ifndef CONSOLEUTILS_H
#define CONSOLEUTILS_H

// MOOSE includes
#include "Moose.h"

// Forward declarations
class MooseApp;
class FEProblemBase;

// libMesh forward declarations
namespace libMesh
{
class System;
}

namespace ConsoleUtils
{

/// Width used for printing simulation information
static const unsigned int console_field_width = 27;

/// Line length for printing simulation information
static const unsigned int console_line_length = 100;

/**
 * Create empty string for indenting
 */
std::string indent(unsigned int spaces);

/**
 * Outputs framework information
 *
 * This includes the versions and timestamps
 */
std::string outputFrameworkInformation(MooseApp & app);

/**
 * Output the mesh information
 */
std::string outputMeshInformation(FEProblemBase & problem, bool verbose = true);

/**
 * Output the Auxiliary system information
 */
std::string outputAuxiliarySystemInformation(FEProblemBase & problem);

/**
 * Output the Nonlinear system information
 */
std::string outputNonlinearSystemInformation(FEProblemBase & problem);

/**
 * Output execution information
 */
std::string outputExecutionInformation(MooseApp & app, FEProblemBase & problem);

/**
 * Output the output information
 */
std::string outputOutputInformation(MooseApp & app);

/**
 * Output system information
 * @param system The libMesh system to output
 * @see outputAuxiliarySystemInformation outputNonlinearSystemInformation
 */
std::string outputSystemInformationHelper(const System & system);

/**
 * Helper function function for stringstream formatting
 */
void insertNewline(std::stringstream & oss, std::streampos & begin, std::streampos & curr);

} // ConsoleUtils namespace

#endif
