/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LINEARVISCOELASTICITYMANAGER_H
#define LINEARVISCOELASTICITYMANAGER_H

#include "ElementUserObject.h"
#include "LinearViscoelasticityBase.h"
#include "RankTwoTensor.h"

class LinearViscoelasticityManager;

template <>
InputParameters validParams<LinearViscoelasticityManager>();

class LinearViscoelasticityManager : public ElementUserObject
{
public:
  LinearViscoelasticityManager(const InputParameters & parameters);

protected:
  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & /*uo*/) override{};
  virtual void finalize() override{};

  std::string _stress_name;
  const MaterialProperty<RankTwoTensor> & _stress;

  std::string _strain_name;
  const MaterialProperty<RankTwoTensor> & _strain;

  std::string _viscoelastic_model_name;
  std::shared_ptr<LinearViscoelasticityBase> _viscoelastic_model;
};

#endif // STRESSTIMESTEPSETUP_H
