#ifndef MTBC_H
#define MTBC_H

#include "BoundaryCondition.h"


//Forward Declarations
class MTBC;

template<>
InputParameters validParams<MTBC>();

/**
 * Implements a simple Neumann BC with material where grad(u)=value on the boundary.
 */
class MTBC : public BoundaryCondition
{
public:
  MTBC(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual ~MTBC() {}

protected:
  virtual Real computeQpResidual();
  
private:
  /**
   * Value of grad(u) on the boundary.
   */
  Real _value;
  std::string _prop_name;
  MaterialProperty<Real> & _mat;
};

#endif //NEUMANNBC_H
