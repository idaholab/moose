#ifndef COEFDIFFUSION_H
#define COEFDIFFUSION_H

#include "Kernel.h"

//Forward Declarations
class CoefDiffusion;

template<>
Parameters valid_params<CoefDiffusion>();

class CoefDiffusion : public Kernel
{
public:

  CoefDiffusion(std::string name,
            Parameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to=std::vector<std::string>(0),
            std::vector<std::string> coupled_as=std::vector<std::string>(0));

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  Real _coef;
};
#endif //COEFDIFFUSION_H
