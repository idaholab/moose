#ifndef BODYFORCERZ_H
#define BODYFORCERZ_H

#include "BodyForce.h"

//Forward Declarations
class BodyForceRZ;

template<>
InputParameters validParams<BodyForceRZ>();

class BodyForceRZ : public BodyForce
{
public:

BodyForceRZ(const std::string &name,InputParameters p)
  :BodyForce(name,p)

{}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};
#endif //BODYFORCERZ_H
