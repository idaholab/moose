//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "ComputeIncrementalStrainBase.h"
#include "Material.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "RotationTensor.h"
#include "Assembly.h"
#include "CrackFrontDefinition.h"
#include "EnrichmentFunctionCalculation.h"

/**
 * ComputeIncrementalStrain defines a strain increment and rotation increment (=1), for small
 * strains.
 */
class ComputeCrackTipEnrichmentIncrementalStrain : public ComputeIncrementalStrainBase,
                                                   public EnrichmentFunctionCalculation
{
public:
  static InputParameters validParams();

  ComputeCrackTipEnrichmentIncrementalStrain(const InputParameters & parameters);
  virtual ~ComputeCrackTipEnrichmentIncrementalStrain() {}
  virtual void computeProperties() override;

protected:
  /// enrichment displacement
  std::vector<Real> _enrich_disp;

  /// gradient of enrichment displacement
  std::vector<RealVectorValue> _grad_enrich_disp;
  std::vector<RealVectorValue> _grad_enrich_disp_old;

  /// enrichment displacement variables
  std::vector<std::vector<MooseVariableFEBase *>> _enrich_variable;

  /// the current shape functions
  const VariablePhiValue & _phi;

  /// gradient of the shape function
  const VariablePhiGradient & _grad_phi;

  /// gradient of the enriched solution
  MaterialProperty<RankTwoTensor> & _grad_enrich_disp_tensor;
  const MaterialProperty<RankTwoTensor> & _grad_enrich_disp_tensor_old;

private:
  /// enrichment function value
  std::vector<Real> _B;
  /// derivatives of enrichment function respect to global cooridnate
  std::vector<RealVectorValue> _dBX;
  /// derivatives of enrichment function respect to crack front cooridnate
  std::vector<RealVectorValue> _dBx;
  /// enrichment function at node I
  std::vector<std::vector<Real>> _BI;
  /// shape function
  const std::vector<std::vector<Real>> * _fe_phi;
  /// gradient of shape function
  const std::vector<std::vector<RealGradient>> * _fe_dphi;
  NonlinearSystem * _nl;
  const NumericVector<Number> * _sln;
};
