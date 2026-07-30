//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "MaterialData.h"
#include "PorousFlowDictator.h"

/**
 * PorousFlowMaterial is the base class for all PorousFlow Materials
 * It allows users to specify that the Material should be a "nodal"
 * Material, in which Material Properties will be evaluated at
 * nodes (using the Variable's nodal values rather than their quadpoint
 * values).  In a derived class's computeQpProperties,
 * _qp must be recognized as a label for a quadpoint (for
 * ordinary Materials) or a node (for nodal Materials).
 *
 * For the nodal Material case, the Material Properties are sized
 * to max(number of nodes, number of quadpoints).  Only nodalDofCount() of
 * these will ever be computed and used - that is, the number of nodes
 * carrying a degree of freedom of the variables that are read at the nodes,
 * which is fewer than the number of nodes for a first-order variable on a
 * second-order mesh.  The remaining ones (if any) exist just to make sure
 * that the vectors are correctly sized in MOOSE's copying operations (etc).
 *
 * If number of quadpoints < number of nodes (eg for boundary elements)
 * care should be taken to store the required nodal information in
 * the first number_of_quadpoint elements in the std::vector!
 */
class PorousFlowMaterial : public Material
{
public:
  static InputParameters validParams();

  PorousFlowMaterial(const InputParameters & parameters);
  virtual void initialSetup() override;

protected:
  /**
   * Error if this is a nodal Material but a variable supplied to one of the named coupled-variable
   * parameters is not nodal (Lagrange).  Call this from the constructor of a derived class, naming
   * exactly those parameters the class reads with coupledGenericDofValue when at_nodes = true.
   * Parameters read behind an _is_*_nodal check must not be named, since those fall back to
   * quadpoint values and so accept non-nodal variables.
   * @param coupled_var_params names of the coupled-variable parameters that must be nodal
   */
  void checkNodalVariables(const std::vector<std::string> & coupled_var_params) const;

  /// Correctly sizes nodal materials, then initialises using Material::initStatefulProperties
  virtual void initStatefulProperties(unsigned int n_points) override;

  /// Correctly sizes nodal materials, then computes using Material::computeProperties
  virtual void computeProperties() override;

  /**
   * Compute the material properties at each node, and if the number of nodes is less
   * than the number of quadpoints, fill the remaining empty values with the last value.
   */
  void computeNodalProperties();

  /**
   * Resizes properties to be equal to max(number of nodes, number of quadpoints)
   * in the current element
   */
  void sizeNodalProperties();

  /**
   * Find the nearest quadpoint to the node labelled by nodenum
   * in the current element
   * @param nodenum the node number in the current element
   * @return the nearest quadpoint
   */
  unsigned nearestQP(unsigned nodenum) const;

  /**
   * The values of a coupled variable for this Material to read: its
   * degree-of-freedom values if this is a nodal Material and the variable is
   * nodal, and its quadpoint values otherwise.  An elemental variable (or an
   * uncoupled default) has no nodal degrees of freedom, so reading it by degree
   * of freedom would run off the end of a much shorter array.
   * @param var_name the name of the coupled variable
   * @param comp the component of the coupled variable
   */
  const VariableValue & nodalOrQpValue(const std::string & var_name, unsigned int comp = 0);

  /**
   * The number of nodes of the current element that carry a degree of freedom of
   * the variables that nodal Materials read at the nodes.  This, not
   * _current_elem->n_nodes(), is the number of nodal values that may safely be
   * computed and consumed.
   *
   * libMesh numbers element nodes vertices-first and numbers a LAGRANGE
   * variable's degrees of freedom to match, so looping 0 .. nodalDofCount() - 1
   * visits exactly the nodes that carry a degree of freedom, in order.
   *
   * This equals _current_elem->n_nodes() for a consistent LAGRANGE
   * discretisation, and is smaller for a first-order variable on a second-order
   * mesh: 4 versus 10 on TET10, 8 versus 27 on HEX27.
   */
  unsigned int nodalDofCount() const;

  /// Whether the derived class holds nodal values
  const bool _nodal_material;

  /// The variable names UserObject for the PorousFlow variables
  const PorousFlowDictator & _dictator;

  /// Names of variables used to declare/get derivatives in the
  /// DerivativeMaterialInterface to ensure consistency
  const VariableName _pressure_variable_name;
  const VariableName _saturation_variable_name;
  const VariableName _temperature_variable_name;
  const VariableName _mass_fraction_variable_name;

  /// stateful material property ids that this material supplies
  std::vector<unsigned int> _supplied_old_prop_ids;
};
