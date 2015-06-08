#ifndef RELAP7OBJECT_H
#define RELAP7OBJECT_H

#include "MooseObject.h"

class RELAP7Object;

template<>
InputParameters validParams<RELAP7Object>();

/**
 *
 */
class RELAP7Object : public MooseObject
{
public:
  RELAP7Object(const std::string & name, InputParameters parameters);
  virtual ~RELAP7Object();
};

#endif /* RELAP7OBJECT_H */
