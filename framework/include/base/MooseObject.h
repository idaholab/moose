#ifndef MOOSEOBJECT_H
#define MOOSEOBJECT_H

#include "InputParameters.h"

/**
 * Every object that can be built by the factory should be derived from this class.
 */
class MooseObject
{
public:
  MooseObject(const std::string & name, InputParameters parameters);

  const std::string & name() { return _name; }

  InputParameters & parameters() { return _pars; }

  template <typename T>
  const T & getParam(const std::string & name) { return _pars.get<T>(name); }

  template <typename T>
  const T & getParam(const std::string & name) const { return _pars.get<T>(name); }
  
  /**
   * This virtual gets called every time the subdomain changes.  This is useful for doing pre-calcualtions
   * that should only be done once per subdomain.  In particular this is where references to material
   * property vectors should be initialized.
   */
  virtual void subdomainSetup();

protected:
  std::string _name;
  InputParameters _pars;
};

#endif /* MOOSEOBJECT_H_*/
