/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#pragma once

#include "GeneralUserObject.h"
#include "ExtraQPProvider.h"

class MooseMesh;

/**
 * Provides test QPs for testing XFEMMaterialManager
 */
class ExtraQPTest : public GeneralUserObject, public ExtraQPProvider
{
public:
  static InputParameters validParams();

  ExtraQPTest(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};

  virtual const std::map<dof_id_type, std::vector<Point>> & getExtraQPMap() const override
  {
    return _extra_qp_map;
  };

protected:
  std::map<dof_id_type, std::vector<Point>> _extra_qp_map;
  MooseMesh & _mesh;
  std::vector<Point> _points;
};
