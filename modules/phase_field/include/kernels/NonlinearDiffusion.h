#pragma once

#include "ADKernel.h"

/**
 * NonlinearDiffusion is the diffusion kernel
 * strong form:
 *      nabla (D * nabla c)
 * weak form:
 *      (D * nabla c, nabla test)
 * D is the diffusivity
 */

class NonlinearDiffusion : public ADKernel
{
public:
  static InputParameters validParams();
  NonlinearDiffusion(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;
  const std::string & _diffusivity_name;
  const ADMaterialProperty<Real> & _diffusivity;
};