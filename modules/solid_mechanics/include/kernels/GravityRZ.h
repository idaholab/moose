#ifndef GRAVITYRZ_H
#define GRAVITYRZ_H

#include "Gravity.h"

//Forward Declarations
class GravityRZ;

template<>
InputParameters validParams<GravityRZ>();

class GravityRZ : public Gravity
{
public:

  GravityRZ(const std::string & name, InputParameters parameters);

  virtual ~GravityRZ() {}

protected:
  virtual Real computeQpResidual();

};

#endif
