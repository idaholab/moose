#ifndef INPUTPARAMETERS_H
#define INPUTPARAMETERS_H

//local includes
//#include "ValidParams.h"

#include <set>
#include <map>

// libMesh includes
#include "parameters.h"

class InputParameters;
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

  const std::string &getDocString(const std::string &name) const;
  bool isParamRequired(const std::string &name) const;
  bool isParamValid(const std::string &name) const;
  InputParameters & operator=(const InputParameters &rhs);
  InputParameters & operator+=(const InputParameters &rhs);
  void seenInInputFile(const std::string &name);

  // These are the only objects allowed to _create_ InputParameters
  friend InputParameters validParams<Kernel>();
  friend InputParameters validParams<AuxKernel>();
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

inline const std::string &
InputParameters::getDocString(const std::string &name) const
{
  static std::string empty;
  std::map<std::string, std::string>::const_iterator doc_string = _doc_string.find(name);
  return doc_string != _doc_string.end() ? doc_string->second : empty;
}

inline bool
InputParameters::isParamRequired(const std::string &name) const
{
  return _required_params.find(name) != _required_params.end();
}

inline bool
InputParameters::isParamValid(const std::string &name) const
{
  return _valid_params.find(name) != _valid_params.end();
}



#endif //INPUTPARAMETERS_H
