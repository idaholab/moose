//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Moose.h"

// Forward declarations
class MooseApp;
class FEProblemBase;
class MooseObject;

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
std::string outputFrameworkInformation(const MooseApp & app);

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
std::string outputNonlinearSystemInformation(FEProblemBase & problem,
                                             const unsigned int nl_sys_num);

/**
 * Output action RelationshipManager information
 */
std::string outputRelationshipManagerInformation(const MooseApp & app);

/**
 * Output execution information
 */
std::string outputExecutionInformation(const MooseApp & app, FEProblemBase & problem);

/**
 * Output the output information
 */
std::string outputOutputInformation(MooseApp & app);

/**
 * Output system information
 * @param system The libMesh system to output
 * @see outputAuxiliarySystemInformation outputNonlinearSystemInformation
 */
std::string outputSystemInformationHelper(libMesh::System & system);

/**
 * Output the information about pre-SMO residual evaluation
 */
std::string outputPreSMOResidualInformation();

/**
 * Output the legacy flag information
 */
std::string outputLegacyInformation(MooseApp & app);

/**
 * Output the registered data paths for searching
 */
std::string outputDataFilePaths();

/**
 * Output the (param path = value) pairs for each DataFileName parameter
 */
std::string outputDataFileParams(MooseApp & app);

/**
 * Helper function function for stringstream formatting
 */
void insertNewline(std::stringstream & oss, std::streampos & begin, std::streampos & curr);

/**
 * Add new lines and prefixes to a string for pretty display in output
 * NOTE: This makes a copy of the string, on purpose, to be able to return
 *       a modified copy
 * @return the formatted string
 */
std::string formatString(std::string message, const std::string & prefix);

/**
 * Routine to output the name of MooseObjects in a string
 * @param objs the vector with all the MooseObjects
 * @param sep a separator in between each object's name
 */
std::string mooseObjectVectorToString(const std::vector<MooseObject *> & objs,
                                      const std::string & sep = " ");
} // ConsoleUtils namespace
