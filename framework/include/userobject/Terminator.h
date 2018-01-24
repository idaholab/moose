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
