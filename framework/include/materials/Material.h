#include "Kernel.h"

#ifndef MATERIAL_H
#define MATERIAL_H

/**
 * Holds material properties that are assigned to blocks.
 */
class Material : public Kernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  Material(Parameters parameters,
           unsigned int block_id,
           std::vector<std::string> coupled_to,
           std::vector<std::string> coupled_as)
    :Kernel(parameters, Kernel::_es->get_system(0).variable_name(0), false, coupled_to, coupled_as),
    _zero(0),
    _grad_zero(0),
    _block_id(block_id),
    _has_temp(std::find(coupled_as.begin(),coupled_as.end(),"temp") != coupled_as.end()),
    _temp(_has_temp ? coupledVal("temp") : _zero),
    _grad_temp(_has_temp ? coupledGrad("temp") : _grad_zero),
    _has_oxygen(std::find(coupled_as.begin(),coupled_as.end(),"oxygen") != coupled_as.end()),
    _oxygen(_has_oxygen ? coupledVal("oxygen") : _zero),
    _grad_oxygen(_has_oxygen ? coupledGrad("oxygen") : _grad_zero),
    _has_neut(std::find(coupled_as.begin(),coupled_as.end(),"neut") != coupled_as.end()),
    _neut(_has_neut ? coupledVal("neut") : _zero),
    _thermal_conductivity(1),
    _thermal_expansion(1),
    _specific_heat(1),
    _density(1),
    _youngs_modulus(1),
    _poissons_ratio(1),
    _neutron_diffusion_coefficient(1),
    _neutron_absorption_xs(1),
    _neutron_fission_xs(1),
    _neutron_per_fission(1),
    _neutron_velocity(1),
    _neutron_per_power(1),
    _heat_xfer_coefficient(1),
    _temp0(1),
    _temp_fluid(1)
  {}

  virtual ~Material(){}

  /** 
   * Block ID the Material is active on.
   * 
   * @return The block ID.
   */
  unsigned int blockID(){ return _block_id; }

  /**
   * Causes the material to recompute all of it's values
   * at the quadrature points.  This is a helper in the base
   * class that does a bunch of common setup first then calls
   * computeProperties().
   */
  void materialReinit();

  std::vector<Real> & thermalConductivity(){ return _thermal_conductivity; }
  std::vector<Real> & thermalExpansion(){ return _thermal_expansion; }
  std::vector<Real> & specificHeat(){ return _specific_heat; }
  std::vector<Real> & density(){ return _density; }
  
  std::vector<Real> & youngsModulus(){ return _youngs_modulus; }
  std::vector<Real> & poissonsRatio(){ return _poissons_ratio; }

  std::vector<Real> & neutronDiffusionCoefficient(){ return _neutron_diffusion_coefficient; }
  std::vector<Real> & neutronAbsorptionXS(){ return _neutron_absorption_xs; }
  std::vector<Real> & neutronFissionXS(){ return _neutron_fission_xs; }
  std::vector<Real> & neutronPerFission(){ return _neutron_per_fission; }
  std::vector<Real> & neutronVelocity(){ return _neutron_velocity; }
  std::vector<Real> & neutronPerPower(){ return _neutron_per_power; }

  std::vector<Real> & heatXferCoefficient(){ return _heat_xfer_coefficient; }
  std::vector<Real> & temp0(){ return _temp0; }
  std::vector<Real> & tempFluid(){ return _temp_fluid; }
    
private:
  std::vector<Real> _zero;
  std::vector<RealGradient> _grad_zero;

protected:

  /**
   * All materials must override this virtual.
   * This is where they fill up the vectors with values.
   */
  virtual void computeProperties() = 0;

  /**
   * Block ID this material is active on.
   */
  unsigned int _block_id;

  /**
   * Doesn't do anything for materials.
   */
  virtual Real computeQpResidual(){ return 0; }

  bool _has_temp;
  std::vector<Real> & _temp;
  std::vector<RealGradient> & _grad_temp;

  bool _has_oxygen;  
  std::vector<Real> & _oxygen;
  std::vector<RealGradient> & _grad_oxygen;

  bool _has_neut;
  std::vector<Real> & _neut;
  
  std::vector<Real> _thermal_conductivity;
  std::vector<Real> _thermal_expansion;
  std::vector<Real> _specific_heat;
  std::vector<Real> _density;
  std::vector<Real> _youngs_modulus;
  std::vector<Real> _poissons_ratio;
  std::vector<Real> _neutron_diffusion_coefficient;
  std::vector<Real> _neutron_absorption_xs;
  std::vector<Real> _neutron_fission_xs;
  std::vector<Real> _neutron_per_fission;
  std::vector<Real> _neutron_velocity;
  std::vector<Real> _neutron_per_power;
  std::vector<Real> _heat_xfer_coefficient;
  std::vector<Real> _temp0;
  std::vector<Real> _temp_fluid;
  
};

#endif //MATERIAL_H
