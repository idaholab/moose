#ifndef DIRICHLETBCFUNCXYZ0_H
#define DIRICHLETBCFUNCXYZ0_H

#include "NodalBC.h"
#include "UsrFunc.h"


//Forward Declarations
class DirichletBCfuncXYZ0;

template<>
InputParameters validParams<DirichletBCfuncXYZ0>();

/**
 * Implements space-dependent Dirichlet BC.
 */
class DirichletBCfuncXYZ0 : public NodalBC
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  DirichletBCfuncXYZ0(const std::string & name, InputParameters parameters);

  virtual ~DirichletBCfuncXYZ0(){}

protected:

  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:

 /**
  *   Parameters for the manufactured solution used.
  */
  Real _A0,_B0,_C0,_omega0;

};

#endif //DIRICHLETBCFUNCXYZ0_H
