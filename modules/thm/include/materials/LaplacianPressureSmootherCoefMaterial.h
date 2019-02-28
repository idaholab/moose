#ifndef LAPLACIANPRESSURESMOOTHERCOEFMATERIAL_H
#define LAPLACIANPRESSURESMOOTHERCOEFMATERIAL_H

#include "Material.h"

class LaplacianPressureSmootherCoefMaterial;

template <>
InputParameters validParams<LaplacianPressureSmootherCoefMaterial>();

/**
 * Computes the dissipation coefficient for a smoother based on the Laplacian
 * of pressure.
 *
 * The smoother is given as "Method III", Equation (18), in
 * <em>
 * P. Nithiarasu, O.C. Zienkiewicz, B.V.K. Satya Sai, K. Morgan, R. Codina and M. Vazquez: Shock
 * Capturing
 * Viscosities for the General Fluid Mechanics Algorithm, Int. J. Numer. Meth. Fluids 28: 1325 -
 * 1353 (1998)
 * </em>:
 *
 * \f[
 *   C_e h^3 \frac{|\mathbf{V}| + c}{\bar{p}}|\nabla^2 p|
 *   \int_\Omega \frac{dN}{dx_i} \frac{dN}{dx_i} \phi^n d\Omega
 * \f]
 */
class LaplacianPressureSmootherCoefMaterial : public Material
{
public:
  LaplacianPressureSmootherCoefMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  Real _Ce;
  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _c;
  const VariableValue & _p_bar;
  const VariableValue & _laplace_p;

  const bool _use_reference_pressure;
  const Real _p_ref;

  const bool _use_low_mach_fix;

  MaterialProperty<Real> & _coef;
};

#endif /* LAPLACIANPRESSURESMOOTHERCOEFMATERIAL_H */
