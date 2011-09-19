#include "Kernel.h"
#include "Material.h"

//Forward Declarations
class Advection0;

template<>
InputParameters validParams<Advection0>();

class Advection0 : public Kernel
{
public:

  Advection0(const std::string & name, InputParameters parameters);

protected:

  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:

 /**
  *   Parameters for spatially linearly varying velocity.
  */
  Real _Au,_Bu,_Cu,_Av,_Bv,_Cv;

};
