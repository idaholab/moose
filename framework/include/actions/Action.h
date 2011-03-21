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

class Action : public MooseObject
{
public:
  Action(const std::string & name, InputParameters params);

  virtual void act() = 0;

  const std::string & getAction() { return _action; }

  inline bool isParamValid(const std::string &name) const { return _pars.isParamValid(name); }

  inline InputParameters & getParams() { return _pars; }

  /**
   * Returns the short name which is the final string after the last delimiter for the
   * current ParserBlock
   */
  std::string getShortName() const;

  void printInputFile(const std::string * prev_name);
  
protected:
  /// Helper method for adding Params pointers to be printed out in syntax dumps
  virtual void addParamsPtrs(std::vector<InputParameters *> & param_ptrs);
  
  ///Output stream which the print_* functions print to. Defaults to std::cout 
  static std::ostream * _out;
  
  std::string _action;
  Parser & _parser_handle;

private:
  /// Helper method for printing the parts of the InputFile Syntax
  void printCloseAndOpen(const std::string * prev_name, const std::string & curr_name) const;
};

template<>
InputParameters validParams<Action>();

#endif // ACTION_H
