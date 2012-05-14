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

#ifndef ACTION_H
#define ACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "MooseObject.h"

#include <string>
#include <ostream>

class Action;

template<>
InputParameters validParams<Action>();

class Action
{
public:
  Action(const std::string & name, InputParameters params);

  virtual ~Action() {}                  // empty virtual destructor for proper memory release

  virtual void act() = 0;

  const std::string & name() { return _name; }

  InputParameters & parameters() { return _pars; }

  const std::string & getAction() { return _action; }

  template <typename T>
  const T & getParam(const std::string & name) { return _pars.get<T>(name); }

  template <typename T>
  const T & getParam(const std::string & name) const { return _pars.get<T>(name); }

  inline bool isParamValid(const std::string &name) const { return _pars.isParamValid(name); }

  inline InputParameters & getParams() { return _pars; }

  /**
   * Returns the short name which is the final string after the last delimiter for the
   * current ParserBlock
   */
  std::string getShortName() const;

protected:

  std::string _name;
  InputParameters _pars;

  std::string _action;
  Parser & _parser_handle;

  /// Convenience reference to a problem this action works on
  FEProblem * & _problem;
};

#endif // ACTION_H
