/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "InputParameters.h"

/**
 * To be called in the validParams functions of classes that need to
 * operate on ranges of files.  Adds several non-required parameters
 * that are parsed in the parseFileRange function.
 */
void addFileRangeParams(InputParameters & params);

/**
 * Augments an InputParameters object with file range information.
 * Creates and adds a vector<string> with the list of filenames to the
 * params object for use by the calling object.  The params object
 * passed in must contain suitable information for building the list
 * of filenames in the range.  Returns a non-zero error code if there
 * is an error while parsing.
 */
int parseFileRange(InputParameters & params);

/**
 * Returns a string containing a description of the error code.
 */
std::string getFileRangeErrorMessage(int code);
