/***************************************************************/
/* New DGMatDiffusion DGKernel based upon the following paper: */
/*   Discontinuous Galerkin methods for convection-diffusion   */
/*       equations for varying and vanishing diffusivity       */
/*                 by: J. PROFT and B. RIVIERE                 */
/***************************************************************/

#ifndef ENHANCEDDGMATDIFFUSION_H
#define ENHANCEDDGMATDIFFUSION_H

#include "DGKernel.h"

//Forward Declarations
class EnhancedDGMatDiffusion;

template<>
InputParameters validParams<EnhancedDGMatDiffusion>();



class EnhancedDGMatDiffusion : public DGKernel
{
public:
  EnhancedDGMatDiffusion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual(DGResidualType type);
  virtual Real computeQpJacobian(DGJacobianType type);

  Real _epsilon;
  Real _sigma;
  Real _limiting_factor;
  Real _conv;
  RealVectorValue _velocity;
  Real _x;
  Real _y;
  Real _z;
  Real _adaptive;

  std::string _prop_name;                     // name of the material property
  MaterialProperty<Real> & _diff;             // diffusivity
  MaterialProperty<Real> & _diff_neighbor;    // diffusivity
};

#endif
