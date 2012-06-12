#ifndef DENSITY_H
#define DENSITY_H

#include "Material.h"


/**
 * Compute density, which may changed based on a deforming mesh.
 */
class Density : public Material
{
public:
  Density( const std::string & name,
           InputParameters params );

protected:
  virtual void computeProperties();

  const bool _is_coupled;
  const bool _is_RZ;
  const VariableGradient & _grad_disp_x;
  const VariableGradient & _grad_disp_y;
  const VariableGradient & _grad_disp_z;
  const VariableValue & _disp_r;

  const Real _orig_density;
  MaterialProperty<Real> & _density;
};

template<>
InputParameters validParams<Density>();

#endif // DENSITY_H
