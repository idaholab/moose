#ifndef NAVIERSTOKESMATERIAL_H
#define NAVIERSTOKESMATERIAL_H

#include "Material.h"


//Forward Declarations
class NavierStokesMaterial;

template<>
InputParameters validParams<NavierStokesMaterial>();

/**
 * This is the base class all materials should use if you are trying to use the Navier-Stokes Kernels.
 *
 * Note that the derived class just needs to compute dynamic_viscocity then call this class's
 * computeProperties() function.
 *
 * Also make sure that the derived class's validParams function just adds to this class's validParams.
 *
 * Finally, note that this Material _isn't_ registered with the MaterialFactory.  The reason is that by
 * itself this material doesn't work!  You _must_ derive from this material and compute dynamic_viscocity!
 */
class NavierStokesMaterial : public Material
{
public:
  NavierStokesMaterial(const std::string & name, InputParameters parameters);
  
protected:

  /**
   * Must be called _after_ the child class computes dynamic_viscocity.
   */
  virtual void computeProperties();

  bool _has_u;
  VariableValue  & _u;
  VariableGradient & _grad_u;

  bool _has_v;
  VariableValue  & _v;
  VariableGradient & _grad_v;

  bool _has_w;
  VariableValue  & _w;
  VariableGradient & _grad_w;

  bool _has_pe;
  VariableValue  & _pe;
  VariableGradient & _grad_pe;

  MaterialProperty<RealTensorValue> & _viscous_stress_tensor;
  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _pressure;

  MaterialProperty<Real> & _gamma;
  MaterialProperty<Real> & _c_v;
  MaterialProperty<Real> & _c_p;
  MaterialProperty<Real> & _R;
  MaterialProperty<Real> & _Pr;

  MaterialProperty<Real> & _dynamic_viscocity;
  
  Real _R_param;
  Real _gamma_param;
  Real _Pr_param;

  std::vector<VariableGradient *> _vel_grads;
};

#endif //NAVIERSTOKESMATERIAL_H
