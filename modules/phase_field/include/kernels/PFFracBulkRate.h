/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PFFRACBULKRATE_H
#define PFFRACBULKRATE_H
/**
 * Phase field based fracture model
 * This kernel computes the residual and jacobian for bulk free energy contribution to c
 * Refer to Formulation: Miehe et. al., Int. J. Num. Methods Engg., 2010, 83. 1273-1311 Equation 63
 */
#include "KernelValue.h"
#include "RankTwoTensor.h"

//Forward Declarations
class PFFracBulkRate;

template<>
InputParameters validParams<PFFracBulkRate>();

class PFFracBulkRate : public KernelValue
{
public:

  PFFracBulkRate(const InputParameters & parameters);

protected:

  enum PFFunctionType
  {
    Jacobian,
    Residual
  };

  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  virtual Real computeDFDOP(PFFunctionType type);
  ///Critical energy release rate for fracture
  const MaterialProperty<Real> & _gc_prop;
  ///Contribution of umdamaged strain energy to damage evolution
  const MaterialProperty<Real> & _G0_pos;
  ///Variation of undamaged strain energy driving damage evolution with strain
  const MaterialProperty<RankTwoTensor> * _dG0_pos_dstrain;
  ///Auxiliary variable: beta = Laplacian of c
  const VariableValue & _betaval;
  const unsigned int _beta_var;
  // std::string _base_name;

  /// Coupled displacement variables
  unsigned int _ndisp;
  std::vector<unsigned int> _disp_var;
  std::string _base_name;

  ///Characteristic length, controls damage zone thickness
  Real _l;
  ///Viscosity parameter ( visco -> 0, rate independent )
  Real _visco;

 private:

};
#endif //PFFRACBULKRATE_H
