/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSPPENALTY
#define RICHARDSPPENALTY

#include "Kernel.h"

// Forward Declarations
class RichardsPPenalty;

template<>
InputParameters validParams<RichardsPPenalty>();

class RichardsPPenalty : public Kernel
{
public:

  RichardsPPenalty(const std::string & name,
                        InputParameters parameters);


protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  Real _a;
  VariableValue & _lower;
  unsigned int _lower_var_num;

};

#endif //RICHARDSPPENALTY
