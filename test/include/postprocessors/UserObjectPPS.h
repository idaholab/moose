#ifndef USEROBJECTPPS_H
#define USEROBJECTPPS_H

#include "GeneralPostprocessor.h"
#include "MTUserObject.h"

class UserObjectPPS;

template <>
InputParameters validParams<UserObjectPPS>();

class UserObjectPPS : public GeneralPostprocessor
{
public:
  UserObjectPPS(const std::string & name, InputParameters params);
  virtual ~UserObjectPPS();

  virtual void initialize();
  virtual void execute();
  virtual PostprocessorValue getValue();

protected:
  const MTUserObject & _mt_ud;
};

#endif /* USEROBJECTPPS_H */
