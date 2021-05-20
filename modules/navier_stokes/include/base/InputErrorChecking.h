//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/**
 * Checks whether the user has specified an input parameter that will go un-used, printing
 * an error message to help in the debugging process.
 * @param p input parameters object
 * @param name name of the input parameter
 * @param explanation short explanation of the reason why parameter is unused
 */
void checkUnusedInputParameter(const InputParameters & p,
                               const std::string & name,
                               const std::string & explanation);

/**
 * Prints a warning message that the object only should be used for testing
 * @param p input parameters object
 */
void checkTestOnlyParameter(const InputParameters & p);

/**
 * Prints a general error message
 * @param p input parameters object
 * @param message error message
 */
void errorMessage(const InputParameters & p, const std::string & message);
