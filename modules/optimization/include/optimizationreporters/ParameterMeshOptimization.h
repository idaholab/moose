//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralOptimization.h"

class ParameterMesh;

/**
 * Mesh-based parameter optimization
 */
class ParameterMeshOptimization : public GeneralOptimization
{

public:
  static InputParameters validParams();
  ParameterMeshOptimization(const InputParameters & parameters);

protected:
  virtual void setICsandBounds() override;

private:
  /**
   * Read initialization data off of parameter mesh and error check.
   * @return values read from mesh
   */
  std::vector<Real> parseExodusData(const std::vector<unsigned int> & exodus_timestep,
                                    const ParameterMesh & pmesh,
                                    const std::string & mesh_var_name,
                                    unsigned int ntimes) const;
};
