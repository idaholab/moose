#ifndef CN2AdvDiffReaction1_H
#define CN2AdvDiffReaction1_H

#include "Kernel.h"
#include "UsrFunc.h"


//Forward Declarations
class CN2AdvDiffReaction1;

template<>
InputParameters validParams<CN2AdvDiffReaction1>();

class CN2AdvDiffReaction1 : public Kernel
{
public:

  CN2AdvDiffReaction1(const std::string & name, InputParameters parameters);

protected:

  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:

 /**
  *   Parameters for the manufactured solution used.
  */
  Real _A0,_B0,_C0,_Au,_Bu,_Cu,_Av,_Bv,_Cv,_Ak,_Bk,_Ck,_omega0;

};

#endif //CN2AdvDiffReaction1_H
