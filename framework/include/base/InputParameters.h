#ifndef INPUTPARAMETERS_H
#define INPUTPARAMETERS_H

//local includes
//#include "ValidParams.h"

#include <set>
#include <map>

// libMesh includes
#include "parameters.h"

class InputParameters;
class PDEBase;
class Kernel;
class AuxKernel;
class BoundaryCondition;
class Stabilizer;
class ParserBlock;
class Material;
class InitialCondition;
class Executioner;

template<class KernelType>
InputParameters validParams();

class InputParameters : public Parameters
{
public:
  InputParameters(const InputParameters &rhs);
  InputParameters(const Parameters &rhs);
  
  virtual ~InputParameters() 
    {}

  template <typename T>
  void addRequiredParam(const std::string &name, const std::string &doc_string);
  
  template <typename T>
  void addParam(const std::string &name, const T &value, const std::string &doc_string);

  template <typename T>
  void addParam(const std::string &name, const std::string &doc_string);

  template <typename T>
  void addPrivateParam(const std::string &name);

  template <typename T>
  void addPrivateParam(const std::string &name, const T &value);

  void addCoupledVar(const std::string &name, const std::string &doc_string);

  void addRequiredCoupledVar(const std::string &name, const std::string &doc_string);

  const std::string &getDocString(const std::string &name) const;
  bool isParamRequired(const std::string &name) const;
  bool isParamValid(const std::string &name) const;
  bool isPrivate(const std::string &name) const;
  InputParameters & operator=(const InputParameters &rhs);
  InputParameters & operator+=(const InputParameters &rhs);
  void seenInInputFile(const std::string &name);

  inline std::set<std::string>::const_iterator coupledVarsBegin()
  {
    return _coupled_vars.begin();
  }
  
  inline std::set<std::string>::const_iterator coupledVarsEnd()
  {
    return _coupled_vars.end();
  }

  // These are the only objects allowed to _create_ InputParameters
  friend InputParameters validParams<PDEBase>();
  friend InputParameters validParams<Kernel>();
  friend InputParameters validParams<BoundaryCondition>();
  friend InputParameters validParams<Stabilizer>();
  friend InputParameters validParams<ParserBlock>();
  friend InputParameters validParams<Material>();
  friend InputParameters validParams<InitialCondition>();
  friend InputParameters validParams<Executioner>();
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
