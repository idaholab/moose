#ifndef FUNCTIONPENALTYDIRICHLETBC_H
#define FUNCTIONPENALTYDIRICHLETBC_H

#include "IntegratedBC.h"

class FunctionPenaltyDirichletBC;
class Function;

template<>
InputParameters validParams<FunctionPenaltyDirichletBC>();

/**
 * A different approach to applying Dirichlet BCs
 *
 * uses \int(p u \cdot \phi)=\int(p f \cdot \phi) on d\omega
 *
 */

class FunctionPenaltyDirichletBC : public IntegratedBC
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same constructor.
   */
  FunctionPenaltyDirichletBC(const std::string & name, InputParameters parameters);

  virtual ~FunctionPenaltyDirichletBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  Function & _func;

  Real _p;
};

#endif
