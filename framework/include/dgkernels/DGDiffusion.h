#ifndef DGDIFFUSION_H
#define DGDIFFUSION_H

#include "DGKernel.h"

//Forward Declarations
class DGDiffusion;

template<>
InputParameters validParams<DGDiffusion>();

/**
 * DG kernel for diffusion
 *
 * General DG kernel that this class can handle is:
 * { \grad u * n_e} [v] + epsilon { \grad v * n_e } [u] + (sigma / |e| * [u][v])
 *
 *  [a] = [ a_1 - a_2 ]
 *  {a} = 0.5 * (a_1 + a_2)
 *
 */
class DGDiffusion : public DGKernel
{
public:
  DGDiffusion(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual(DGResidualType type);
  virtual Real computeQpJacobian(DGJacobianType type);

  Real _epsilon;
  Real _sigma;
};
 
#endif
