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

#include <vector>
#include <set>
#include <map>

// libMesh
#include "parameters.h"

#include "Moose.h"
#include "MooseTypes.h"
#include "MooseEnum.h"

class MooseObject;
class GlobalParamsAction;
class Action;
class Parser;
class Problem;

class InputParameters;

template<class T>
InputParameters validParams();


/**
 *
 */
class InputParameters : public Parameters
{
public:
  InputParameters(const InputParameters &rhs);
  InputParameters(const Parameters &rhs);

  virtual ~InputParameters()
    {}

  virtual void clear();

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
   * Override from libMesh to set user-defined attributes on our parameter
   */
  virtual void set_attributes(const std::string & name, bool inserted_only);

  /**
   * This method adds a parameter and documentation string to the InputParameters
   * object that will be extracted from the input file.  If the parameter is
   * missing in the input file, and error will be thrown
   */
  template <typename T>
  void addRequiredParam(const std::string &name, const std::string &doc_string);

  /**
   * This version of addRequiredParam is here for a consistent use with MooseEnum.  Use of
   * this function for any other type will throw an error.
   */
  template <typename T>
  void addRequiredParam(const std::string &name, const T &moose_enum, const std::string &doc_string);

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
   * These methods add an option parameter and with a customer type to the InputParameters object.  The custom
   * type will be output in YAML dumps and can be used within the GUI application.
   */
  template <typename T>
  void addRequiredCustomTypeParam(const std::string &name, const std::string &custom_type, const std::string &doc_string);
  template <typename T>
  void addCustomTypeParam(const std::string &name, const T &value, const std::string &custom_type, const std::string &doc_string);
  template <typename T>
  void addCustomTypeParam(const std::string &name, const std::string &custom_type, const std::string &doc_string);

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
   * Utility functions for retrieving one of the MooseTypes variables into the common "string" base class.
   * Scalar and Vector versions are supplied
   */
  std::string getMooseType(const std::string &name) const;
  std::vector<std::string> getVecMooseType(const std::string &name) const;

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
  std::string getDocString(const std::string &name) const;

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
   * This method returns a truth value to indicate whether a parameter was seen in the input
   * file or not
   */
  bool wasSeenInInput(const std::string &name) const;

  /**
   * This method returns true if all of the parameters in this object are valid
   * (i.e. isParamValid(name) == true - for all parameters)
   */
  bool areAllRequiredParamsValid() const;

  /**
   * Prints the type of the requested parameter by name
   */
  std::string type(const std::string &name);

  /**
   * Returns a boolean indicating whether the specified parameter is private or not
   */
  bool isPrivate(const std::string &name) const;

  /**
   * Copy and Copy/Add operators for the InputParameters object
   */
  using Parameters::operator=;
  using Parameters::operator+=;
  InputParameters & operator=(const InputParameters &rhs);
  InputParameters & operator+=(const InputParameters &rhs);

  /**
   * This function is called from the parser to indicate which parameters are seen
   * as they are read from the input file.
   */
  void seenInInputFile(const std::string &name);

  /**
   * This function checks parameters stored in the object to make sure they are in the correct
   * state as the user expects:
   *   Required parameters are verified as valid meaning that they were either initialized when
   *   they were created, or were read from an inputfile or some other valid source
   */
  void checkParams(const std::string & prefix) const;

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
  friend InputParameters validParams<Action>();
  friend InputParameters validParams<Problem>();
  friend InputParameters emptyInputParameters();

private:
  // Private constructor so that InputParameters can only be created in certain places.
  InputParameters() {}

private:
  /// The documentation strings for each parameter
  std::map<std::string, std::string> _doc_string;

  /// The custom type that will be printed in the YAML dump for a parameter if supplied
  std::map<std::string, std::string> _custom_type;

  /// The set of parameters that are required (i.e. will cause an abort if not supplied)
  std::set<std::string> _required_params;

  /**
   * The set of parameters either seen in the input file or provided a default value when added
   * Note: We do not store MooseEnum names in valid params, instead we asked MooseEnums whether
   *       they are valid or not.
   */
  std::set<std::string> _valid_params;

  /// The set of parameters that will NOT appear in the the dump of the parser tree
  std::set<std::string> _private_params;

  /// The set of parameters seen in the input file only
  std::set<std::string> _seen_in_input;

  /// The coupled variables set
  std::set<std::string> _coupled_vars;
};

// Template and inline function implementations
template <typename T>
void InputParameters::addRequiredParam(const std::string &name, const std::string &doc_string)
{
  Parameters::insert<T>(name);
  _required_params.insert(name);
  _doc_string[name] = doc_string;
}

template <typename T>
void InputParameters::addRequiredParam(const std::string &name, const T &value, const std::string &doc_string)
{
  mooseError("You cannont call addRequiredParam and supply a default value for this type, please use addParam instead");
}

template <typename T>
void InputParameters::addParam(const std::string &name, const T &value, const std::string &doc_string)
{
  Parameters::set<T>(name) = value;                    // valid parameter is set by set_attributes
  _doc_string[name] = doc_string;
}

template <typename T>
void InputParameters::addParam(const std::string &name, const std::string &doc_string)
{
  Parameters::insert<T>(name);
  _doc_string[name] = doc_string;
}

template <typename T>
void InputParameters::addRequiredCustomTypeParam(const std::string &name, const std::string &custom_type, const std::string &doc_string)
{
  addRequiredParam<T>(name, doc_string);
  _custom_type[name] = custom_type;
}

template <typename T>
void InputParameters::addCustomTypeParam(const std::string &name, const T &value, const std::string &custom_type, const std::string &doc_string)
{
  addParam<T>(name, value, doc_string);
  _custom_type[name] = custom_type;
}

template <typename T>
void InputParameters::addCustomTypeParam(const std::string &name, const std::string &custom_type, const std::string &doc_string)
{
  addParam<T>(name, doc_string);
  _custom_type[name] = custom_type;
}

template <typename T>
void InputParameters::addPrivateParam(const std::string &name)
{
  Parameters::insert<T>(name);
  _private_params.insert(name);
}

template <typename T>
void InputParameters::addPrivateParam(const std::string &name, const T &value)
{
  Parameters::set<T>(name) = value;
  _private_params.insert(name);
}

// Specializations for MooseEnum
template <>
inline
void InputParameters::addRequiredParam<MooseEnum>(const std::string &name, const MooseEnum &moose_enum, const std::string &doc_string)
{
  Parameters::set<MooseEnum>(name) = moose_enum;                    // valid parameter is set by set_attributes
  _required_params.insert(name);
  _doc_string[name] = doc_string;
}

/**
 * Generic valid params

template<class T>
InputParameters validParams()
{
  InputParameters params;
  return params;
}
*/

InputParameters emptyInputParameters();

namespace libMesh
{
  template<>
  inline
  void InputParameters::Parameter<std::vector<short int> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }

  template<>
  inline
  void InputParameters::Parameter<std::vector<int> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }

  template<>
  inline
  void InputParameters::Parameter<std::vector<std::string> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }

  template<>
  inline
  void InputParameters::Parameter<std::vector<VariableName> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }

  template<>
  inline
  void InputParameters::Parameter<std::vector<NonlinearVariableName> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }

  template<>
  inline
  void InputParameters::Parameter<std::vector<AuxVariableName> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }

  template<>
  inline
  void InputParameters::Parameter<std::vector<IndicatorName> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }

  template<>
  inline
  void InputParameters::Parameter<std::vector<MarkerName> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }

  template<>
  inline
  void InputParameters::Parameter<std::vector<bool> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }

  template<>
  inline
  void InputParameters::Parameter<std::vector<float> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }

  template<>
  inline
  void InputParameters::Parameter<std::map<std::string, unsigned int> >::print (std::ostream& /*os*/) const
  {
  }

  template<>
  inline
  void InputParameters::Parameter<std::vector<Real> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }

  template<>
  inline
  void InputParameters::Parameter<std::vector<unsigned int> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }

  template<>
  inline
  void InputParameters::Parameter<std::vector<SubdomainID> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }

  template<>
  inline
  void InputParameters::Parameter<std::vector<SubdomainName> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }

  template<>
  inline
  void InputParameters::Parameter<std::vector<BoundaryName> >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      os << _value[i] << " ";
  }
} // libMesh

#endif /* INPUTPARAMETERS_H */
