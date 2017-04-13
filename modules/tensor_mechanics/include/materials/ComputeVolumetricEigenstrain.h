/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEVOLUMETRICEIGENSTRAIN_H
#define COMPUTEVOLUMETRICEIGENSTRAIN_H

#include "ComputeEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeVolumetricEigenstrain computes an eigenstrain that is defined by a set of scalar
 * material properties that summed together define the volumetric change.  This also
 * computes the derivatives of that eigenstrain with respect to a supplied set of variable
 * dependencies.");
 */
class ComputeVolumetricEigenstrain : public DerivativeMaterialInterface<ComputeEigenstrainBase>
{
public:
  ComputeVolumetricEigenstrain(const InputParameters & parameters);

protected:
  virtual void initialSetup();
  virtual void computeQpEigenstrain();

  /// number of variables the material depends on
  const unsigned int _num_args;

  /// Names of the material properties that define volumetric change
  const std::vector<MaterialPropertyName> _volumetric_material_names;
  /// The material properties that define volumetric change
  std::vector<const MaterialProperty<Real> *> _volumetric_materials;

  /// first derivatives of the volumetric materials with respect to the args
  std::vector<std::vector<const MaterialProperty<Real> *>> _dvolumetric_materials;
  /// second derivatives of the volumetric materials with respect to the args
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _d2volumetric_materials;

  /// first derivatives of the elastic strain with respect to the args
  std::vector<MaterialProperty<RankTwoTensor> *> _delastic_strain;
  /// second derivatives of the elastic strain with respect to the args
  std::vector<std::vector<MaterialProperty<RankTwoTensor> *>> _d2elastic_strain;
};

#endif // COMPUTE_VOLUMETRIC_EIGENSTRAIN_H
