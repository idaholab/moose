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
#include "ConsoleStreamInterface.h"
#include "MooseTypes.h"
#include "InputParameters.h"
#include "Syntax.h"
#include "GlobalParamsAction.h"

#include "hit.h"

#include <vector>
#include <string>
#include <iomanip>

// Forward declarations
class ActionWarehouse;
class SyntaxTree;
class MooseApp;
class Factory;
class ActionFactory;
class JsonSyntaxTree;

class FuncParseEvaler : public hit::Evaler
{
public:
  virtual std::string
  eval(hit::Field * n, const std::list<std::string> & args, hit::BraceExpander & exp)
  {
    std::string func_text;
    for (auto & s : args)
      func_text += s;
    auto n_errs = exp.errors.size();

    FunctionParser fp;
    fp.AddConstant("pi", libMesh::pi);
    fp.AddConstant("e", std::exp(Real(1)));
    std::vector<std::string> var_names;
    auto ret = fp.ParseAndDeduceVariables(func_text, var_names);
    if (ret != -1)
    {
      exp.errors.push_back(hit::errormsg(exp.fname, n, "fparse error: ", fp.ErrorMsg()));
      return n->val();
    }

    std::string errors;
    std::vector<double> var_vals;
    for (auto & var : var_names)
    {
      // recursively check all parent scopes for the needed variables
      hit::Node * curr = n;
      while ((curr = curr->parent()))
      {
        auto src = curr->find(var);
        if (src && src != n && src->type() == hit::NodeType::Field)
        {
          exp.used.push_back(hit::pathJoin({curr->fullpath(), var}));
          var_vals.push_back(curr->param<double>(var));
          break;
        }
      }

      if (curr == nullptr)
        exp.errors.push_back(hit::errormsg(exp.fname,
                                           n,
                                           "\n    no variable '",
                                           var,
                                           "' found for use in function parser expression"));
    }

    if (exp.errors.size() != n_errs)
      return n->val();

    std::stringstream ss;
    ss << std::setprecision(17) << fp.Eval(var_vals.data());

    // change kind only (not val)
    n->setVal(n->val(), hit::Field::Kind::Float);
    return ss.str();
  }
};

/**
 * Class for parsing input files. This class utilizes the GetPot library for actually tokenizing and
 * parsing files. It is not currently designed for extensibility. If you wish to build your own
 * parser, please contact the MOOSE team for guidance.
 */
class Parser : public ConsoleStreamInterface, public hit::Walker
{
public:
  enum SyntaxFormatterType
  {
    INPUT_FILE,
    YAML
  };

  Parser(MooseApp & app, ActionWarehouse & action_wh);

  virtual ~Parser();

  /**
   * Return the filename that was parsed
   */
  std::string getFileName(bool stripLeadingPath = true) const;

  /**
   * Parse an input file consisting of hit syntax and setup objects
   * in the MOOSE derived application
   */
  void parse(const std::string & input_filename);

  /**
   * This function attempts to extract values from the input file based on the contents of
   * the passed parameters objects.  It handles a number of various types with dynamic casting
   * including vector types
   */
  void extractParams(const std::string & prefix, InputParameters & p);

  /**
   * Creates a syntax formatter for printing
   */
  void initSyntaxFormatter(SyntaxFormatterType type, bool dump_mode);

  /**
   * Use MOOSE Factories to construct a full parse tree for documentation or echoing input.
   */
  void buildFullTree(const std::string & search_string);

  /**
   * Use MOOSE Factories to construct a parameter tree for documentation or echoing input.
   */
  void buildJsonSyntaxTree(JsonSyntaxTree & tree) const;

  void walk(const std::string & fullpath, const std::string & nodepath, hit::Node * n);

  void errorCheck(const Parallel::Communicator & comm, bool warn_unused, bool err_unused);

  std::vector<std::string> listValidParams(std::string & section_name);

  /**
   * Marks MOOSE hit syntax from supplied command-line arguments
   */
  std::string hitCLIFilter(std::string appname, const std::vector<std::string> & argv);

protected:
  template <typename T>
  bool toBool(const std::string & /*s*/, T & /*val*/)
  {
    return false;
  }

  /**
   * Helper functions for setting parameters of arbitrary types
   */
  /// Template method for setting any scalar type parameter read from the input file or command line
  template <typename T, typename Base>
  void setScalarParameter(const std::string & full_name,
                          const std::string & short_name,
                          InputParameters::Parameter<T> * param,
                          bool in_global,
                          GlobalParamsAction * global_block);

  template <typename T, typename UP_T, typename Base>
  void setScalarValueTypeParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<T> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

  /// Template method for setting any vector type parameter read from the input file or command line
  template <typename T, typename Base>
  void setVectorParameter(const std::string & full_name,
                          const std::string & short_name,
                          InputParameters::Parameter<std::vector<T>> * param,
                          bool in_global,
                          GlobalParamsAction * global_block);

  /**
   * Sets an input parameter representing a file path using input file data.  The file path is
   * modified to be relative to the directory this application's input file is in.
   */
  template <typename T>
  void setFilePathParam(const std::string & full_name,
                        const std::string & short_name,
                        InputParameters::Parameter<T> * param,
                        InputParameters & params,
                        bool in_global,
                        GlobalParamsAction * global_block);

  /**
   * Sets an input parameter representing a vector of file paths using input file data.  The file
   * paths are modified to be relative to the directory this application's input file is in.
   */
  template <typename T>
  void setVectorFilePathParam(const std::string & full_name,
                              const std::string & short_name,
                              InputParameters::Parameter<std::vector<T>> * param,
                              InputParameters & params,
                              bool in_global,
                              GlobalParamsAction * global_block);
  /**
   * Template method for setting any double indexed type parameter read from the input file or
   * command line.
   */
  template <typename T>
  void setDoubleIndexParameter(const std::string & full_name,
                               const std::string & short_name,
                               InputParameters::Parameter<std::vector<std::vector<T>>> * param,
                               bool in_global,
                               GlobalParamsAction * global_block);

  /**
   * Template method for setting any multivalue "scalar" type parameter read from the input file or
   * command line.  Examples include "Point" and "RealVectorValue".
   */
  template <typename T>
  void setScalarComponentParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<T> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

  /**
   * Template method for setting several multivalue "scalar" type parameter read from the input
   * file or command line.  Examples include "Point" and "RealVectorValue".
   */
  template <typename T>
  void setVectorComponentParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<std::vector<T>> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

  /**
   * Extract custom application parameters
   */
  virtual bool extractCustomParams(const std::string & full_name,
                                   const std::string & short_name,
                                   Parameters::Value * par,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

  std::unique_ptr<hit::Node> _cli_root = nullptr;
  std::unique_ptr<hit::Node> _root = nullptr;
  std::vector<std::string> _secs_need_first;

  /// The MooseApp this Parser is part of
  MooseApp & _app;
  /// The Factory associated with that MooseApp
  Factory & _factory;
  /// Action warehouse that will be filled by actions
  ActionWarehouse & _action_wh;
  /// The Factory that builds actions
  ActionFactory & _action_factory;
  /// Reference to an object that defines input file syntax
  Syntax & _syntax;

  /// Object for holding the syntax parse tree
  std::unique_ptr<SyntaxTree> _syntax_formatter;

  /// The input file name that is used for parameter extraction
  std::string _input_filename;

  /// The set of all variables extracted from the input file
  std::set<std::string> _extracted_vars;

  /// Boolean to indicate whether parsing has started (sections have been extracted)
  bool _sections_read;

  /// The current parameter object for which parameters are being extracted
  InputParameters * _current_params;

  /// The current stream object used for capturing errors during extraction
  std::ostringstream * _current_error_stream;

private:
  std::string _errmsg;
  std::string _warnmsg;
  void walkRaw(std::string fullpath, std::string nodepath, hit::Node * n);
};

// Template Specializations for retrieving special types from the input file

template <typename T, typename Base>
void
Parser::setScalarParameter(const std::string & full_name,
                           const std::string & short_name,
                           InputParameters::Parameter<T> * param,
                           bool in_global,
                           GlobalParamsAction * global_block)
{
  try
  {
    param->set() = _root->param<Base>(full_name);
  }
  catch (hit::Error & err)
  {
    auto strval = _root->param<std::string>(full_name);

    // handle the case where the user put a number inside quotes
    auto & t = typeid(T);
    if (t == typeid(int) || t == typeid(unsigned int) || t == typeid(SubdomainID) ||
        t == typeid(BoundaryID) || t == typeid(double))
    {
      try
      {
        param->set() = MooseUtils::convert<T>(strval, true);
      }
      catch (std::invalid_argument & /*e*/)
      {
        const std::string format_type = (t == typeid(double)) ? "float" : "integer";
        _errmsg += hit::errormsg(_input_filename,
                                 _root->find(full_name),
                                 "invalid ",
                                 format_type,
                                 " syntax for parameter: ",
                                 full_name,
                                 "=",
                                 strval) +
                   "\n";
      }
    }
    else if (t == typeid(bool))
    {
      bool isbool = toBool(strval, param->set());
      if (!isbool)
        _errmsg += hit::errormsg(_input_filename,
                                 _root->find(full_name),
                                 "invalid boolean syntax for parameter: ",
                                 full_name,
                                 "=",
                                 strval) +
                   "\n";
    }
    else
      throw;
  }

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<T>(short_name) = param->get();
  }
}

template <typename T>
void
Parser::setFilePathParam(const std::string & full_name,
                         const std::string & short_name,
                         InputParameters::Parameter<T> * param,
                         InputParameters & params,
                         bool in_global,
                         GlobalParamsAction * global_block)
{
  std::string prefix;
  std::string postfix = _root->param<std::string>(full_name);
  size_t pos = _input_filename.find_last_of('/');
  if (pos != std::string::npos && postfix[0] != '/' && !postfix.empty())
    prefix = _input_filename.substr(0, pos + 1);

  params.rawParamVal(short_name) = postfix;
  param->set() = prefix + postfix;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<T>(short_name) = param->get();
  }
}

template <typename T, typename UP_T, typename Base>
void
Parser::setScalarValueTypeParameter(const std::string & full_name,
                                    const std::string & short_name,
                                    InputParameters::Parameter<T> * param,
                                    bool in_global,
                                    GlobalParamsAction * global_block)
{
  setScalarParameter<T, Base>(full_name, short_name, param, in_global, global_block);

  // If this is a range checked param, we need to make sure that the value falls within the
  // requested range
  mooseAssert(_current_params, "Current params is nullptr");

  _current_params->rangeCheck<T, UP_T>(full_name, short_name, param, *_current_error_stream);
}

template <typename T, typename Base>
void
Parser::setVectorParameter(const std::string & full_name,
                           const std::string & short_name,
                           InputParameters::Parameter<std::vector<T>> * param,
                           bool in_global,
                           GlobalParamsAction * global_block)
{
  std::vector<T> vec;
  if (_root->find(full_name))
  {
    try
    {
      auto tmp = _root->param<std::vector<Base>>(full_name);
      for (auto val : tmp)
        vec.push_back(val);
    }
    catch (hit::Error & err)
    {
      _errmsg += hit::errormsg(_input_filename, _root->find(full_name), err.what()) + "\n";
      return;
    }
  }

  param->set() = vec;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<T>(short_name).resize(param->get().size());
    for (unsigned int i = 0; i < vec.size(); ++i)
      global_block->setVectorParam<T>(short_name)[i] = param->get()[i];
  }
}

template <typename T>
void
Parser::setVectorFilePathParam(const std::string & full_name,
                               const std::string & short_name,
                               InputParameters::Parameter<std::vector<T>> * param,
                               InputParameters & params,
                               bool in_global,
                               GlobalParamsAction * global_block)
{
  std::vector<T> vec;
  std::vector<std::string> rawvec;
  if (_root->find(full_name))
  {
    auto tmp = _root->param<std::vector<std::string>>(full_name);
    params.rawParamVal(short_name) = _root->param<std::string>(full_name);
    for (auto val : tmp)
    {
      std::string prefix;
      std::string postfix = val;
      size_t pos = _input_filename.find_last_of('/');
      if (pos != std::string::npos && postfix[0] != '/')
        prefix = _input_filename.substr(0, pos + 1);
      rawvec.push_back(postfix);
      vec.push_back(prefix + postfix);
    }
  }

  param->set() = vec;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<T>(short_name).resize(param->get().size());
    for (unsigned int i = 0; i < vec.size(); ++i)
      global_block->setVectorParam<T>(short_name)[i] = param->get()[i];
  }
}

template <typename T>
void
Parser::setDoubleIndexParameter(const std::string & full_name,
                                const std::string & short_name,
                                InputParameters::Parameter<std::vector<std::vector<T>>> * param,
                                bool in_global,
                                GlobalParamsAction * global_block)
{
  // Get the full string assigned to the variable full_name
  std::string buffer = _root->param<std::string>(full_name);

  // split vector at delim ;
  // NOTE: the substrings are _not_ of type T yet
  std::vector<std::string> first_tokenized_vector;
  MooseUtils::tokenize(buffer, first_tokenized_vector, 1, ";");
  param->set().resize(first_tokenized_vector.size());

  for (unsigned j = 0; j < first_tokenized_vector.size(); ++j)
    if (!MooseUtils::tokenizeAndConvert<T>(first_tokenized_vector[j], param->set()[j]))
    {
      _errmsg +=
          hit::errormsg(
              _input_filename, _root->find(full_name), "invalid format for parameter ", full_name) +
          "\n";
      return;
    }

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setDoubleIndexParam<T>(short_name).resize(first_tokenized_vector.size());
    for (unsigned j = 0; j < first_tokenized_vector.size(); ++j)
    {
      global_block->setDoubleIndexParam<T>(short_name)[j].resize(param->get()[j].size());
      for (unsigned int i = 0; i < param->get()[j].size(); ++i)
        global_block->setDoubleIndexParam<T>(short_name)[j][i] = param->get()[j][i];
    }
  }
}

template <typename T>
void
Parser::setScalarComponentParameter(const std::string & full_name,
                                    const std::string & short_name,
                                    InputParameters::Parameter<T> * param,
                                    bool in_global,
                                    GlobalParamsAction * global_block)
{
  std::vector<double> vec;
  try
  {
    vec = _root->param<std::vector<double>>(full_name);
  }
  catch (hit::Error & err)
  {
    _errmsg += hit::errormsg(_input_filename, _root->find(full_name), err.what()) + "\n";
    return;
  }

  if (vec.size() != LIBMESH_DIM)
  {
    _errmsg += hit::errormsg(_input_filename,
                             _root->find(full_name),
                             "wrong number of values in scalar component parameter ",
                             full_name,
                             ": size ",
                             vec.size(),
                             " is not a multiple of ",
                             LIBMESH_DIM) +
               "\n";
    return;
  }

  T value;
  for (unsigned int i = 0; i < vec.size(); ++i)
    value(i) = Real(vec[i]);

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<T>(short_name) = value;
  }
}

template <typename T>
void
Parser::setVectorComponentParameter(const std::string & full_name,
                                    const std::string & short_name,
                                    InputParameters::Parameter<std::vector<T>> * param,
                                    bool in_global,
                                    GlobalParamsAction * global_block)
{
  std::vector<double> vec;
  try
  {
    vec = _root->param<std::vector<double>>(full_name);
  }
  catch (hit::Error & err)
  {
    _errmsg += hit::errormsg(_input_filename, _root->find(full_name), err.what()) + "\n";
    return;
  }

  if (vec.size() % LIBMESH_DIM)
  {
    _errmsg += hit::errormsg(_input_filename,
                             _root->find(full_name),
                             "wrong number of values in vector component parameter ",
                             full_name,
                             ": size ",
                             vec.size(),
                             " is not a multiple of ",
                             LIBMESH_DIM) +
               "\n";
    return;
  }

  std::vector<T> values;
  for (unsigned int i = 0; i < vec.size() / LIBMESH_DIM; ++i)
  {
    T value;
    for (int j = 0; j < LIBMESH_DIM; ++j)
      value(j) = Real(vec[i * LIBMESH_DIM + j]);
    values.push_back(value);
  }

  param->set() = values;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<T>(short_name).resize(vec.size(), values[0]);
    for (unsigned int i = 0; i < vec.size() / LIBMESH_DIM; ++i)
      global_block->setVectorParam<T>(short_name)[i] = values[0];
  }
}
