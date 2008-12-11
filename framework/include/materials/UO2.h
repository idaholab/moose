#include "Material.h"

#ifndef UO2_H
#define UO2_H

/**
 * Simple Uranium Oxide material.
 */
class UO2 : public Material
{
public:
  UO2(std::string name,
      Parameters parameters,
      unsigned int block_id,
      std::vector<std::string> coupled_to,
      std::vector<std::string> coupled_as)
    :Material(name,parameters,block_id,coupled_to,coupled_as),
    _has_temp(isCoupled("temp")),
    _temp(_has_temp ? coupledVal("temp") : _zero),
    _grad_temp(_has_temp ? coupledGrad("temp") : _grad_zero),
    _has_oxygen(isCoupled("oxygen")),
    _oxygen(_has_oxygen ? coupledVal("oxygen") : _zero),
    _grad_oxygen(_has_oxygen ? coupledGrad("oxygen") : _grad_zero),
    _density(declareRealProperty("density")),  
    _thermal_conductivity(declareRealProperty("thermal_conductivity")),
    _specific_heat(declareRealProperty("specific_heat")),
    _thermal_expansion(declareRealProperty("thermal_expansion")),
    _youngs_modulus(declareRealProperty("youngs_modulus")),
    _poissons_ratio(declareRealProperty("poissons_ratio"))
  {}

protected:
  virtual void computeProperties();

private:
  bool _has_temp;
  std::vector<Real> & _temp;
  std::vector<RealGradient> & _grad_temp;

  bool _has_oxygen;  
  std::vector<Real> & _oxygen;
  std::vector<RealGradient> & _grad_oxygen;

  std::vector<Real> & _density;
  std::vector<Real> & _thermal_conductivity;
  std::vector<Real> & _specific_heat;
  std::vector<Real> & _thermal_expansion;
  std::vector<Real> & _youngs_modulus;
  std::vector<Real> & _poissons_ratio;
};

#endif //UO2_H
