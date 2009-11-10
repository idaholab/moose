#ifndef INPUTPARAMETERS_H
#define INPUTPARAMETERS_H

#include <set>
#include <map>

// libMesh includes
#include "parameters.h"

class InputParameters : public Parameters
{
public:
  InputParameters() {}
  InputParameters(const InputParameters &rhs);
  InputParameters(const Parameters &rhs);
  
  virtual ~InputParameters() 
    {}

  template <typename T>
  void addParam(const std::string &name, const std::string &doc_string, bool required);

  const std::string &getDocString(const std::string &name) const;
  bool isRequired(const std::string &name) const;
  InputParameters & operator=(const InputParameters &rhs);
  InputParameters & operator=(const Parameters &rhs);
  
private:
  std::map<std::string, std::string> _doc_string;
  std::set<std::string> _required_params;
};


// Template and inline function implementations
template <typename T>
void InputParameters::addParam(const std::string &name, const std::string &doc_string, bool required) 
{
  Parameters::set<T>(name);
  if (required)
    _required_params.insert(name);
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
InputParameters::isRequired(const std::string &name) const
{
  return _required_params.find(name) != _required_params.end();
}


#endif //INPUTPARAMETERS_H
