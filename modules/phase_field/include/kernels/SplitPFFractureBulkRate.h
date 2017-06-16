/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPLITPFFRACTUREBULKRATE_H
#define SPLITPFFRACTUREBULKRATE_H

#include "KernelValue.h"
#include "RankTwoTensor.h"

// Forward Declarations
class SplitPFFractureBulkRate;

template <>
InputParameters validParams<SplitPFFractureBulkRate>();

/**
 * Phase field based fracture model
 * This kernel computes the residual and Jacobian for bulk free energy contribution to c
 * Refer to Formulation: Miehe et. al., Int. J. Num. Methods Engg., 2010, 83. 1273-1311 Equation 63
 */
class SplitPFFractureBulkRate : public KernelValue
{
public:
  SplitPFFractureBulkRate(const InputParameters & parameters);

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Critical energy release rate for fracture
  const MaterialProperty<Real> & _gc_prop;

  /// Contribution of umdamaged strain energy to damage evolution
  const MaterialProperty<Real> & _G0_pos;

  /// Variation of undamaged strain energy driving damage evolution with strain
  const MaterialProperty<RankTwoTensor> * _dG0_pos_dstrain;

  /// Auxiliary variable: beta = Laplacian of c
  const VariableValue & _beta;
  const unsigned int _beta_var;

  /// Coupled displacement variables
  const unsigned int _ndisp;
  std::vector<unsigned int> _disp_var;
  std::string _base_name;

  /// Diffuse crack width, controls damage zone thickness
  const Real _width;

  /// Viscosity parameter ( visco -> 0, rate independent )
  const Real _viscosity;
};

#endif // SPLITPFFRACTUREBULKRATE_H
