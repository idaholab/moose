/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSHALFGAUSSIANSINK
#define RICHARDSHALFGAUSSIANSINK

#include "IntegratedBC.h"

// Forward Declarations
class RichardsHalfGaussianSink;

template<>
InputParameters validParams<RichardsHalfGaussianSink>();

class RichardsHalfGaussianSink : public IntegratedBC
{
public:

  RichardsHalfGaussianSink(const std::string & name,
                        InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  Real _maximum;
  Real _sd;
  Real _centre;
  Function * const _m_func;
};

#endif //RICHARDSHALFGAUSSIANSINK
