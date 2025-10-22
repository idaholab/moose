//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralOptimization.h"
#include "ParameterMesh.h"

/**
 * Mesh-based parameter optimization
 */
class ParameterMeshOptimization : public GeneralOptimization
{

public:
  static InputParameters validParams();
  ParameterMeshOptimization(const InputParameters & parameters);

  virtual Real computeObjective() override;
  virtual void computeGradient(libMesh::PetscVector<Number> & gradient) const override;

protected:
  virtual void setICsandBounds() override;

private:
  /**
   * Read initialization data off of parameter mesh and error check.
   * @return values read from mesh
   */
  std::vector<Real> parseExodusData(const FEType fetype,
                                    const FileName mesh_file_name,
                                    const std::vector<unsigned int> & exodus_timestep,
                                    const std::string & mesh_var_name) const;

  /// Store parameter meshes for regularization computation
  std::vector<std::unique_ptr<ParameterMesh>> _parameter_meshes;

  /// Vector of regularization coefficients corresponding to each type
  const std::vector<Real> _regularization_coeffs;

  /// Regularization types to apply
  const std::vector<ParameterMesh::RegularizationType> _regularization_types;
};
