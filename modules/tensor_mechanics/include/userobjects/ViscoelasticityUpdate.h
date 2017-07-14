/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef VISCOELASTICITYUPDATE_H
#define VISCOELASTICITYUPDATE_H

#include "ElementUserObject.h"
#include "ComputeCreepStrainBase.h"

class ViscoelasticityUpdate;

template <>
InputParameters validParams<ViscoelasticityUpdate>();

/**
 * This class updates creep and creep-like strains at the end of each time step.
 *
 * In viscoelastic simulations, at least one of such object must be called in order
 * to update the strains correctly.
 */
class ViscoelasticityUpdate : public ElementUserObject
{
public:
  ViscoelasticityUpdate(const InputParameters & parameters);

protected:
  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & /*uo*/) override{};
  virtual void finalize() override{};

  // defines which strain field is used to update the creep strains (default: mechanical_strain)
  std::string _strain_name;
  const MaterialProperty<RankTwoTensor> & _strain;

  // defines which stress field is used to update the creep strains (default: stress)
  std::string _stress_name;
  const MaterialProperty<RankTwoTensor> & _stress;

  // list of ComputeCreepStrainBase objects which are updated at the end of each time step
  std::vector<std::string> _creep_strain_names;
  std::vector<MooseSharedPointer<ComputeCreepStrainBase>> _creep_strains;
};

#endif // VISCOELASTICITYUPDATE_H
