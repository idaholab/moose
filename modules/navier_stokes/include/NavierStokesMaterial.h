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
NavierStokesMaterial(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:

  /**
   * Must be called _after_ the child class computes dynamic_viscocity.
   */
  virtual void computeProperties();

  bool _has_u;
  std::vector<Real> & _u;
  std::vector<RealGradient> & _grad_u;

  bool _has_v;
  std::vector<Real> & _v;
  std::vector<RealGradient> & _grad_v;

  bool _has_w;
  std::vector<Real> & _w;
  std::vector<RealGradient> & _grad_w;

  bool _has_pe;
  std::vector<Real> & _pe;
  std::vector<RealGradient> & _grad_pe;

  MooseArray<RealTensorValue> & _viscous_stress_tensor;
  MooseArray<Real> & _thermal_conductivity;
  MooseArray<Real> & _pressure;

  Real & _gamma;
  Real & _c_v;
  Real & _c_p;
  Real & _R;
  Real & _Pr;

  MooseArray<Real> & _dynamic_viscocity;
  
  Real _R_param;
  Real _gamma_param;
  Real _Pr_param;

  std::vector<std::vector<RealGradient> *> _vel_grads;
};

#endif //NAVIERSTOKESMATERIAL_H
