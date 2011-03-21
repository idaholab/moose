#ifndef AdvDiffReaction1_H
#define AdvDiffReaction1_H

#include "Kernel.h"
#include "UsrFunc.h"


//Forward Declarations
class AdvDiffReaction1;

template<>
InputParameters validParams<AdvDiffReaction1>();

class AdvDiffReaction1 : public Kernel
{
public:

  AdvDiffReaction1(const std::string & name, InputParameters parameters);
  
protected:
   
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();
  
private:

 /**
  *   Parameters for the manufactured solution used.
  */
  Real _A0,_B0,_C0,_Au,_Bu,_Cu,_Av,_Bv,_Cv,_Ak,_Bk,_Ck,_omega0;
  
};
 
#endif //AdvDiffReaction1_H
