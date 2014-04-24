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

/**
 * Applies a fluid sink to the boundary.
 * The sink has strength
 * _maximum*exp(-(0.5*(p - c)/_sd)^2)*_m_func for p<c
 * _maximum*_m_func for p>=c
 * This is typically used for modelling evapotranspiration
 * from the top of a groundwater model
 */
class RichardsHalfGaussianSink : public IntegratedBC
{
public:

  RichardsHalfGaussianSink(const std::string & name,
                        InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  /// maximum of the Gaussian sink
  Real _maximum;

  /// standard deviation of the Gaussian sink
  Real _sd;

  /// centre of the Gaussian sink
  Real _centre;

  /// multiplying function: all fluxes will be multiplied by this
  Function & _m_func;
};

#endif //RICHARDSHALFGAUSSIANSINK
