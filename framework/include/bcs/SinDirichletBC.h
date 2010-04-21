#ifndef SINDIRICHLETBC_H
#define SINDIRICHLETBC_H

#include "libmesh_common.h"
#include "BoundaryCondition.h"


//Forward Declarations
class SinDirichletBC;

template<>
InputParameters validParams<SinDirichletBC>();

class SinDirichletBC : public BoundaryCondition
{
public:

  SinDirichletBC(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();

private:
  Real _initial;
  Real _final;
  Real _duration;
};
 
#endif
