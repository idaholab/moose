#ifndef MTBC_H
#define MTBC_H

#include "IntegratedBC.h"
#include "MaterialProperty.h"

//Forward Declarations
class MTBC;

template<>
InputParameters validParams<MTBC>();

/**
 * Implements a simple Neumann BC with material where grad(u)=value on the boundary.
 */
class MTBC : public IntegratedBC
{
public:
  MTBC(const std::string & name, InputParameters parameters);
  
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
