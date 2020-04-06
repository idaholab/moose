//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ALEKernel.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "CrackFrontDefinition.h"
#include "EnrichmentFunctionCalculation.h"

// Forward Declarations

/**
 * CrackTipEnrichmentStressDivergenceTensors implements the residual and jacobian for enrichement
 * displacement variables.
 *
 */
class CrackTipEnrichmentStressDivergenceTensors : public ALEKernel,
                                                  public EnrichmentFunctionCalculation
{
public:
  static InputParameters validParams();

  CrackTipEnrichmentStressDivergenceTensors(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const std::string _base_name;

  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankFourTensor> & _Jacobian_mult;

  const MaterialProperty<RankTwoTensor> * _deformation_gradient;
  const MaterialProperty<RankTwoTensor> * _deformation_gradient_old;
  const MaterialProperty<RankTwoTensor> * _rotation_increment;

  /// displacement components
  const unsigned int _component;
  /// enrichment function components
  const unsigned int _enrichment_component;

  /// Coupled enrichment displacement variables
  unsigned int _nenrich_disp;
  std::vector<unsigned int> _enrich_disp_var;

  /// Coupled displacement variables
  unsigned int _ndisp;
  std::vector<unsigned int> _disp_var;

private:
  /// enrichment function value
  std::vector<Real> _B;
  /// derivatives of enrichment function respect to global cooridnate
  std::vector<RealVectorValue> _dBX;
  /// derivatives of enrichment function respect to crack front cooridnate
  std::vector<RealVectorValue> _dBx;
  /// enrichment function at node I
  std::vector<Real> _BI;
  /// enrichment function at node J
  std::vector<Real> _BJ;
};
