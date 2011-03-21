#ifndef SIDEFLUXINTEGRAL_H_
#define SIDEFLUXINTEGRAL_H_

#include "SideIntegral.h"
#include "MaterialPropertyInterface.h"

//Forward Declarations
class SideFluxIntegral;

template<>
InputParameters validParams<SideFluxIntegral>();

/**
 * This postprocessor computes a side integral of the mass flux.
 */
class SideFluxIntegral :
  public SideIntegral,
  public Moose::MaterialPropertyInterface
{
public:
  SideFluxIntegral(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpIntegral();

  std::string _diffusivity;
  MaterialProperty<Real> & _diffusion_coef;
};
 
#endif // SIDEFLUXINTEGRAL_H_
