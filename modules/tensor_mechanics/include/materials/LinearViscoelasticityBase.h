/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LINEARVISCOELASTICITYBASE_H
#define LINEARVISCOELASTICITYBASE_H

#include "Material.h"
#include "RankFourTensor.h"
#include "RankTwoTensor.h"

class LinearViscoelasticityBase;

template <>
InputParameters validParams<LinearViscoelasticityBase>();

/**
 * Represents an arbitrary assembly of springs and dashpots in series or in parallel.
 * The values of the springs and dashpots may vary in time. Generally, each dashpot is associated
 * with a single spring. However, there can be one stand-alone dashpot in the model.
 *
 * The time- and history-dependent problem is transformed into an equivalent elastic problem.
 * This necessitates to compute an apparent elastic modulus (the elastic modulus corrected by
 * the internal stress relaxation over the current time step), as well as an apparent creep
 * strain (which corresponds to the progress of the stress relaxation over the current time
 * step). This transformation depends on the nature of the assembly (in serial or in parallel)
 * rather than the actual material properties of the springs and dashpots. Therefore, methods
 * related to that transformation should not be overriden (unless the creep model uses a specific
 * arrangement). GeneralizedKelvinVoigtBase and GeneralizedMaxwellBase provide the necessary
 * methods for the generalized Kelvin-Voigt and generalized Maxwell models.
 *
 * The time-integration uses a 1-step Newmark finite difference scheme. This scheme is controlled
 * by a parameter theta (between 0 and 1, default-value 1). Theta can be automatically calibrated
 * depending on the value of the dashpot viscosities in order to reproduce the exact integral
 * of exponential series (integration_rule = "zienkiewicz").
 *
 * The actual elasticity tensor of the material can be calculated through another object
 * (for example, for time- or temperature-dependent elasticity). In such case, the apparent
 * elasticity tensor and creep strains will be corrected in order to accout for that value.
 */
class LinearViscoelasticityBase : public Material
{
public:
  LinearViscoelasticityBase(const InputParameters & parameters);

  inline std::string getApparentElasticityTensorName() const
  {
    return _base_name + "apparent_elasticity_tensor";
  }
  inline std::string getInstantaneousElasticityTensorName() const
  {
    return _base_name + "instantaneous_elasticity_tensor";
  }

  // transforms the viscous strains into an apparent creep strain (depends on the spring-dashpot
  // assembly)
  virtual void
  accumulateQpViscousStrain(unsigned int qp,
                            RankTwoTensor & accumulated_viscous_strain,
                            const std::vector<RankTwoTensor> & viscous_strains,
                            bool has_driving_eigenstrain = false,
                            const RankTwoTensor & driving_eigenstrain = RankTwoTensor()) const = 0;

  // updates the viscous strains at the end of each time step (depends on the spring-dashpot
  // assembly)
  virtual void
  updateQpViscousStrain(unsigned int qp,
                        std::vector<RankTwoTensor> & viscous_strains,
                        const std::vector<RankTwoTensor> & viscous_strains_old,
                        const RankTwoTensor & effective_strain,
                        const RankTwoTensor & effective_stress,
                        bool has_driving_eigenstrain = false,
                        const RankTwoTensor & driving_eigenstrain = RankTwoTensor()) const = 0;

  virtual unsigned int components(unsigned qp) const;
  virtual bool hasLongtermDashpot(unsigned int qp) const;

protected:
  virtual void computeQpProperties();
  // updates the value of the springs and dashpots at each quadrature point
  virtual void computeQpViscoelasticProperties() = 0;
  // transforms the springs and dashpots into an apparent and instantaneous elasticity tensors
  // (depends on the spring-dashpot assembly)
  virtual void computeQpApparentElasticityTensors() = 0;

  // utility function to fill a RankFourTensor with a Young's modulus and Poisson's ratio
  void fillIsotropicElasticityTensor(RankFourTensor & tensor,
                                     Real young_modulus,
                                     Real poisson_ratio) const;
  Real computeTheta(Real dt, Real viscosity) const;

  // name of the spring-dashpot model
  std::string _base_name;

  // parameters for the time intefration
  MooseEnum _integration_rule;
  Real _theta;

  // apparent elasticity tensor
  MaterialProperty<RankFourTensor> & _apparent_elasticity_tensor;
  // instantaneous elasticity tensor
  MaterialProperty<RankFourTensor> & _instantaneous_elasticity_tensor;

  // value of the first spring in the spring-dashpot assembly
  MaterialProperty<RankFourTensor> & _first_elasticity_tensor;
  // list of values of all subsequent springs
  MaterialProperty<std::vector<RankFourTensor>> & _springs_elasticity_tensors;
  // list of values of all associated dashpots
  MaterialProperty<std::vector<Real>> & _dashpot_viscosities;

  // elasticity tensor used in stress calculation
  std::string _elasticity_tensor_name;
  MaterialProperty<RankFourTensor> & _elasticity_tensor;

  // current value of the elasticity tensor (if calculated through other means)
  bool _has_current_elasticity_tensor;
  std::string _current_elasticity_tensor_name;
  const MaterialProperty<RankFourTensor> * _current_elasticity_tensor;
};

#endif // LINEARVISCOELASTICITYBASE_H
