//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RelationshipManager.h"

// MOOSE Forward Declares
class MaterialData;
class MaterialPropertyStorage;

/**
 * RedistributeProperties is used for its \p redistribute() callback,
 * which ensures that any stateful properties are restributed to the
 * same properties to which elements they are associated with are
 * redistributed.
 */
class RedistributeProperties : public RelationshipManager
{
public:
  static InputParameters validParams();

  RedistributeProperties(const InputParameters & parameters);

  RedistributeProperties(const RedistributeProperties & other) = default;

  /**
   * This function doesn't actually add anything to \p
   * coupled_elements - we solely use the \p redistribute()
   * override to keep internal data structures up to date.
   */
  virtual void operator()(const MeshBase::const_element_iterator & range_begin,
                          const MeshBase::const_element_iterator & range_end,
                          processor_id_type p,
                          map_type & coupled_elements) override;

  /**
   * Abstract GhostingFunctor code relies on clone(), which for us is
   * just a copy construction into a new object.
   */
  virtual std::unique_ptr<GhostingFunctor> clone() const override;

  virtual std::string getInfo() const override;

  virtual bool operator>=(const RelationshipManager & rhs) const override;

  virtual void redistribute() override;

  /**
   * Pushes the given pair ( \p mat_data , \p mat_props ) onto our
   * list of \p _materials data to redistribute each time our
   * underlying mesh is redistributed.
   */
  void addMaterialPropertyStorage(std::vector<std::shared_ptr<MaterialData>> & mat_data,
                                  MaterialPropertyStorage & mat_props);

private:
  std::vector<std::pair<std::vector<std::shared_ptr<MaterialData>> *, MaterialPropertyStorage *>>
      _materials;
};
