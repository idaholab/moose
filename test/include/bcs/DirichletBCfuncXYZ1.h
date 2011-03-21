#ifndef DIRICHLETBCFUNCXYZ1_H
#define DIRICHLETBCFUNCXYZ1_H

#include "NodalBC.h"
#include "UsrFunc.h"


//Forward Declarations
class DirichletBCfuncXYZ1;

template<>
InputParameters validParams<DirichletBCfuncXYZ1>();

/**
 * Implements space-dependent Dirichlet BC.
 */
class DirichletBCfuncXYZ1 : public NodalBC
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  DirichletBCfuncXYZ1(const std::string & name, InputParameters parameters);
  
  virtual ~DirichletBCfuncXYZ1(){}

protected:
  
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();
  
private:
  
 /**
  *   Parameters for the manufactured solution used.
  */
  Real _A0,_B0,_C0,_omega0;
  
};

#endif //DIRICHLETBCFUNCXYZ1_H
