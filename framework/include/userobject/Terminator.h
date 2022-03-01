//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_FPARSER

#include "GeneralUserObject.h"
#include "libmesh/fparser.hh"

/**
 * This Userobject requests termination of the current solve based on
 * the values of Postprocessors (and a logical expression testing them)
 *
 *                     <((((((\\\
 *                     /      . }\
 *                     ;--..--._|}
 *  (\                 '--/\--'  )
 *   \\                | '-'  :'|
 *    \\               . -==- .-|
 *     \\               \.__.'   \--._
 *     [\\          __.--|       //  _/'--.
 *     \ \\       .'-._ ('-----'/ __/      \
 *      \ \\     /   __>|      | '--.       |
 *       \ \\   |   \   |     /    /       /
 *        \ '\ /     \  |     |  _/       /
 *         \  \       \ |     | /        /
 *          \  \      \        /
 */
class Terminator : public GeneralUserObject
{
public:
  static InputParameters validParams();

  Terminator(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// handle output of the optional message
  void handleMessage();

  const enum class FailMode { HARD, SOFT } _fail_mode;
  const enum class ErrorLevel { INFO, WARNING, ERROR, NONE } _error_level;

  /// Postprocessor names
  std::vector<std::string> _pp_names;

  // Number of postprocessors the expression depends on
  unsigned int _pp_num;

  /// Postprocessor values
  std::vector<const PostprocessorValue *> _pp_values;

  /// Expression of the criterion, to be parsed for evaluation
  std::string _expression;

  /// Fparser object
  FunctionParserBase<Real> _fp;

  /// Fparser parameter buffer
  std::vector<Real> _params;
};

#endif // LIBMESH_HAVE_FPARSER
