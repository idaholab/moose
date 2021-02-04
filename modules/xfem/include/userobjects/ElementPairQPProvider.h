/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#pragma once

class ElementPairQPProvider;

template <>
InputParameters validParams<ElementPairQPProvider>();

/**
 * Provide a list of extra QPs to be evaluated using the XFEMElemPairMaterialManager
 * The key of the map is the min(elem1->unique_id(),elem2->unique_id())
 */
class ElementPairQPProvider
{
public:
  virtual const std::map<unique_id_type, std::vector<Point>> & getExtraQPMap() const = 0;
  virtual const std::map<unique_id_type, std::pair<const Elem *, const Elem *>> &
  getElementPairMap() const = 0;
};
