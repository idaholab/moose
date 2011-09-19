#ifndef FORCING_FUNCTION_XYZ0_H
#define FORCING_FUNCTION_XYZ0_H

#include "Kernel.h"
#include "UsrFunc.h"


//Forward Declarations
class ForcingFunctionXYZ0;

template<>
InputParameters validParams<ForcingFunctionXYZ0>();

class ForcingFunctionXYZ0 : public Kernel
{
public:

  ForcingFunctionXYZ0(const std::string & name, InputParameters parameters);

protected:

  virtual Real computeQpResidual();


private:

 /**
  *   Parameters for the manufactured solution used.
  */
  Real _A0,_B0,_C0,_Au,_Bu,_Cu,_Av,_Bv,_Cv,_Ak,_Bk,_Ck,_omega0;

};

#endif //FORCING_FUNCTION_XYZ0H
