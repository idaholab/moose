#include "Kernel.h"
#include "Material.h"

//Forward Declarations
class Diffusion0;

template<>
InputParameters validParams<Diffusion0>();

class Diffusion0 : public Kernel
{
public:

  Diffusion0(const std::string & name, InputParameters parameters);

protected:

  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:

 /**
  *   Parameters for spatially linearly varying diffusivity.
  */
  Real _Ak,_Bk,_Ck;

};
