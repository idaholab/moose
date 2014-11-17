#include "Kernel.h"
#include "Material.h"

//Forward Declarations
class WrongJacobianDiffusion;

template<>
InputParameters validParams<WrongJacobianDiffusion>();

class WrongJacobianDiffusion : public Kernel
{
public:
  WrongJacobianDiffusion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  Real _rfactor;
  Real _jfactor;
};
