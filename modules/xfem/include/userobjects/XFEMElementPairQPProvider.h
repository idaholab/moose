/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#pragma once

#include "GeneralUserObject.h"
#include "ElementPairQPProvider.h"
#include "ElementPairLocator.h"

class XFEMElementPairQPProvider;

/**
 * Provides test QPs for testing XFEMMaterialManager
 */
class XFEMElementPairQPProvider : public GeneralUserObject, public ElementPairQPProvider
{
public:
  static InputParameters validParams();

  XFEMElementPairQPProvider(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void timestepSetup() override;

  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};

  virtual const std::map<unique_id_type, std::vector<Point>> & getExtraQPMap() const override
  {
    return _extra_qp_map;
  };

  virtual const std::map<unique_id_type, std::pair<const Elem *, const Elem *>> &
  getElementPairMap() const override
  {
    return _elem_pair_map;
  };

protected:
  std::map<unique_id_type, std::vector<Point>> _extra_qp_map;
  std::map<unique_id_type, std::pair<const Elem *, const Elem *>> _elem_pair_map;
  std::map<unsigned int, std::shared_ptr<ElementPairLocator>> * _element_pair_locators;
};
