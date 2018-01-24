//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TERMINATOR_H
#define TERMINATOR_H

#include "GeneralUserObject.h"
#include "libmesh/fparser.hh"

// Forward Declarations
class Terminator;

template <>
InputParameters validParams<Terminator>();

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
  Terminator(const InputParameters & parameters);
  /// The Terminator DEFINITELY needs a destructor!
  virtual ~Terminator();

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// Postprocessor names
  std::vector<std::string> _pp_names;

  // Number of postprocessors the expression depends on
  unsigned int _pp_num;

  /// Postprocessor values
  std::vector<const PostprocessorValue *> _pp_values;

  std::string _expression;

  /// Fparser object
  FunctionParserBase<Real> _fp;

  /// Fparser parameter buffer
  Real * _params;
};

#endif // TERMINATOR_H
