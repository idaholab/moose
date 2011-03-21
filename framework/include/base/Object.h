#ifndef OBJECT_H_
#define OBJECT_H_

#include "InputParameters.h"

/**
 * Every object that can be build by the factory should be derived from this class.
 */
class Object
{
public:
  Object(const std::string & name, InputParameters parameters);

  const std::string & name() { return _name; }

  InputParameters & parameters() { return _pars; }

  template <typename T>
  const T & getParam(const std::string & name) { return _pars.get<T>(name); }

protected:
  std::string _name;
  InputParameters _pars;
};

#endif /* OBJECT_H_ */
