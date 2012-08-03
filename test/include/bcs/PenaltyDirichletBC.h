#ifndef PENALTYDIRICHLETBC_H
#define PENALTYDIRICHLETBC_H

#include "IntegratedBC.h"

class PenaltyDirichletBC;
class Function;

template<>
InputParameters validParams<PenaltyDirichletBC>();

/**
 * A different approach to applying Dirichlet BCs
 *
 * uses \int(p u \cdot \phi)=\int(p f \cdot \phi) on d\omega
 *
 */

class PenaltyDirichletBC : public IntegratedBC
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same constructor.
   */
  PenaltyDirichletBC(const std::string & name, InputParameters parameters);

  virtual ~PenaltyDirichletBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  Function & _func;

  Real _p;
  Real _v;
};

#endif
