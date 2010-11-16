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

#ifndef INPUTPARAMETERS_H
#define INPUTPARAMETERS_H

#include <set>

// libMesh includes
#include "parameters.h"

class InputParameters;
class MooseObject;
class ParserBlock;
class InitialCondition;
class Executioner;
class GlobalParamsBlock;

template<class KernelType>
InputParameters validParams();

class InputParameters : public Parameters
{
public:
  InputParameters(const InputParameters &rhs);
  InputParameters(const Parameters &rhs);
  
  virtual ~InputParameters() 
    {}

  /**
   * This method adds a description of the class that will be displayed
   * in the input file syntax dump
   */
  void addClassDescription(const std::string &doc_string);

  /**
   * Returns the class description
   */
  std::string getClassDescription() const;

  /**
   * This method adds a parameter and documentation string to the InputParameters
   * object that will be extracted from the input file.  If the parameter is
   * missing in the input file, and error will be thrown
   */
  template <typename T>
  void addRequiredParam(const std::string &name, const std::string &doc_string);

  /**
   * These methods add an option parameter and a documentation string to the InputParameters object.
   * The first version of this function takes a default value which is used if the parameter
   * is not found in the input file.  The second method will leave the parameter uninitialized
   * but can be checked with "isValid" before use
   */
  template <typename T>
  void addParam(const std::string &name, const T &value, const std::string &doc_string);
  template <typename T>
  void addParam(const std::string &name, const std::string &doc_string);

  /**
   * These method adds a parameter to the InputParameters object which can be retrieved
   * like any other parameter.  This parameter however is not printed in the Input file syntax
   * dump or web page dump so does not take a documentation string.  The first version
   * of this function takes an optional default value.
   */
  template <typename T>
  void addPrivateParam(const std::string &name, const T &value);
  template <typename T>
  void addPrivateParam(const std::string &name);

  /**
   * This method adds a coupled variable name pair.  The parser will look for variable
   * name pair in the input file and can return a reference to the storage location
   * for the coupled variable if found
   */
  void addCoupledVar(const std::string &name, const std::string &doc_string);

  /**
   * This method adds a coupled variable name pair.  The parser will look for variable
   * name pair in the input file and can return a reference to the storage location
   * for the coupled variable.  If the coupled variable is not supplied in the input
   * file, and error is thrown
   */
  void addRequiredCoupledVar(const std::string &name, const std::string &doc_string);

  /**
   * Returns the documentation string for the specified parameter name
   */
  const std::string &getDocString(const std::string &name) const;

  /**
   * Returns a boolean indicating whether the specified parameter is required or not
   */
  bool isParamRequired(const std::string &name) const;

  /**
   * This method returns parameters that have been initialized in one fashion or another,
   * i.e. The value was supplied as a default argument or read and properly converted from
   * the input file
   */
  bool isParamValid(const std::string &name) const;

  /**
   * Returns a boolean indicating whether the specified parameter is private or not
   */
  bool isPrivate(const std::string &name) const;

  /**
   * Copy and Copy/Add operators for the InputParameters object
   */
  InputParameters & operator=(const InputParameters &rhs);
  InputParameters & operator+=(const InputParameters &rhs);

  /**
   * This function is called from the parser to indicate which parameters are seen
   * as they are read from the input file.
   */
  void seenInInputFile(const std::string &name);

  /**
   * Methods returning iterators to the coupled variables names stored in this
   * InputParameters object
   */
  inline std::set<std::string>::const_iterator coupledVarsBegin()
  {
    return _coupled_vars.begin();
  }
  inline std::set<std::string>::const_iterator coupledVarsEnd()
  {
    return _coupled_vars.end();
  }

  // These are the only objects allowed to _create_ InputParameters
  friend InputParameters validParams<MooseObject>();
  friend InputParameters validParams<ParserBlock>();
  friend InputParameters validParams<InitialCondition>();
  friend InputParameters validParams<Executioner>();
  friend InputParameters validParams<GlobalParamsBlock>();
  friend class ParserBlock;
  
private:
  // Private constructor so that InputParameters can only be created in certain places.
  InputParameters() {}
  
  std::map<std::string, std::string> _doc_string;
  std::set<std::string> _required_params;

  /**
   * The set of parameters either seen in the input file or provided a default value when added
   */
  std::set<std::string> _valid_params;

  /**
   * The set of parameters that will NOT appear in the the dump of the parser tree
   */
  std::set<std::string> _private_params;

  /**
   * The coupled variables set 
   */
  std::set<std::string> _coupled_vars;
};


// Template and inline function implementations
template <typename T>
void InputParameters::addRequiredParam(const std::string &name, const std::string &doc_string) 
{
  Parameters::set<T>(name);
  _required_params.insert(name);
  _doc_string[name] = doc_string;
}

template <typename T>
void InputParameters::addParam(const std::string &name, const T &value, const std::string &doc_string) 
{
  Parameters::set<T>(name) = value;
  _valid_params.insert(name);
  _doc_string[name] = doc_string;
}

template <typename T>
void InputParameters::addParam(const std::string &name, const std::string &doc_string) 
{
  Parameters::set<T>(name);
  _doc_string[name] = doc_string;
}

template <typename T>
void InputParameters::addPrivateParam(const std::string &name)
{
  Parameters::set<T>(name);
  _private_params.insert(name);
}

template <typename T>
void InputParameters::addPrivateParam(const std::string &name, const T &value)
{
  Parameters::set<T>(name) = value;
  _private_params.insert(name);
}

#endif //INPUTPARAMETERS_H
