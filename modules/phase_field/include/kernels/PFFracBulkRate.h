/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
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

  PFFracBulkRate(const std::string & name, InputParameters parameters);

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
  MaterialProperty<Real> & _gc_prop;
  ///Contribution from positive component of strain to energy
  MaterialProperty<Real> & _G0_pos;
  ///Variation of positive component of energy with strain
  MaterialProperty<RankTwoTensor> & _dG0_pos_dstrain;
  ///Auxiliary variable: beta = Laplacian of c
  VariableValue & _betaval;
  unsigned int _beta_var;

  const bool _xdisp_coupled;
  const bool _ydisp_coupled;
  const bool _zdisp_coupled;

  const unsigned int _xdisp_var;
  const unsigned int _ydisp_var;
  const unsigned int _zdisp_var;

  ///Characteristic length, controls damage zone thickness
  Real _l;
  ///Viscosity parameter ( visco -> 0, rate independent )
  Real _visco;

 private:

};
#endif //PFFRACBULKRATE_H
