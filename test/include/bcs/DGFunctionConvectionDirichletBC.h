#ifndef DGFUNCTIONCONVECTIONDIRICHLETBC_H
#define DGFUNCTIONCONVECTIONDIRICHLETBC_H

#include "IntegratedBC.h"

class DGFunctionConvectionDirichletBC;
class Function;

template<>
InputParameters validParams<DGFunctionConvectionDirichletBC>();

/**
 * Implements a simple BC for DG
 *
 * BC derived from convection problem that can handle:
 * velocity * n_e * u_up * [v]
 *
 * [a] = [ a_1 - a_2 ]
 * u_up = u|E_e_1 if velocity.n_e >= 0
 *        u|E_e_2 if velocity.n_e < 0
 *        with n_e pointing from E_e_1 to E_e_2
 *
 */

class DGFunctionConvectionDirichletBC : public IntegratedBC
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same constructor.
   */
  DGFunctionConvectionDirichletBC( const std::string & name, InputParameters parameters);

  virtual ~DGFunctionConvectionDirichletBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  
private:
  Function & _func;


  RealVectorValue _velocity;

  Real _x;
  Real _y;
  Real _z;
};

#endif
