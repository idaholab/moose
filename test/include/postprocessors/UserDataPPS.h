#ifndef USERDATAPPS_H
#define USERDATAPPS_H

#include "GeneralPostprocessor.h"
#include "MTUserData.h"

class UserDataPPS;

template <>
InputParameters validParams<UserDataPPS>();

class UserDataPPS : public GeneralPostprocessor
{
public:
  UserDataPPS(const std::string & name, InputParameters params);
  virtual ~UserDataPPS();

  virtual void initialize();
  virtual void execute();
  virtual PostprocessorValue getValue();

protected:
  const MTUserData & _mt_ud;
};

#endif /* USERDATAPPS_H */
