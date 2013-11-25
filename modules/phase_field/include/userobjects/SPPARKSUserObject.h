#ifndef SPPARKSUSEROBJECT_H
#define SPPARKSUSEROBJECT_H

#include "GeneralUserObject.h"

class SPPARKSUserObject : public GeneralUserObject
{
public:

  SPPARKSUserObject(const std::string & name, InputParameters parameters);

  virtual ~SPPARKSUserObject();

  virtual void initialSetup() {}

  virtual void residualSetup() {}

  virtual void timestepSetup() {}

  virtual void initialize();
  virtual void execute();
  virtual void finalize() {}

  Real getValue(const std::string & quantity) const;

protected:

  void * _spparks;

  const std::string & _file;
};

template<>
InputParameters validParams<SPPARKSUserObject>();

#endif
