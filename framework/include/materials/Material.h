//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIAL_H
#define MATERIAL_H

// MOOOSE includes
#include "MaterialProperty.h"
#include "MooseObject.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "SetupInterface.h"
#include "Coupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "ScalarCoupleable.h"
#include "FunctionInterface.h"
#include "DistributionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "MaterialPropertyInterface.h"
#include "PostprocessorInterface.h"
#include "VectorPostprocessorInterface.h"
#include "DependencyResolverInterface.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"
#include "OutputInterface.h"
#include "RandomInterface.h"

// forward declarations
class Material;
class MooseMesh;
class MaterialData;
class SubProblem;

template <>
InputParameters validParams<Material>();

/**
 * Materials compute MaterialProperties.
 */
class Material : public MooseObject,
                 public BlockRestrictable,
                 public BoundaryRestrictable,
                 public SetupInterface,
                 public Coupleable,
                 public MooseVariableDependencyInterface,
                 public ScalarCoupleable,
                 public FunctionInterface,
                 public DistributionInterface,
                 public UserObjectInterface,
                 public TransientInterface,
                 public MaterialPropertyInterface,
                 public PostprocessorInterface,
                 public VectorPostprocessorInterface,
                 public DependencyResolverInterface,
                 public Restartable,
                 public MeshChangedInterface,
                 public OutputInterface,
                 public RandomInterface
{
public:
  Material(const InputParameters & parameters);

  /**
   * Initialize stateful properties (if material has some)
   */
  virtual void initStatefulProperties(unsigned int n_points);

  /**
   * Performs the quadrature point loop, calling computeQpProperties
   */
  virtual void computeProperties();

  /**
   * Resets the properties at each quadrature point (see resetQpProperties), only called if 'compute
   * = false'.
   *
   * This method is called internally by MOOSE, you probably don't want to mess with this.
   */
  virtual void resetProperties();

  /**
   * A method for (re)computing the properties of a Material.
   *
   * This is intended to be called from other objects, by first calling
   * MaterialPropertyInterface::getMaterial and then calling this method on the Material object
   * returned.
   */
  virtual void computePropertiesAtQp(unsigned int qp);

  ///@{
  /**
   * Retrieve the property through a given input parameter key with a fallback
   * to getting it by name
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialProperty(const std::string & name);
  template <typename T>
  const ADMaterialPropertyObject<T> & getADMaterialProperty(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);
  ///@}

  ///@{
  /**
   * Retrieve the property named "name"
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyByName(const std::string & prop_name);
  template <typename T>
  const ADMaterialPropertyObject<T> & getADMaterialPropertyByName(const std::string & prop_name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOldByName(const std::string & prop_name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlderByName(const std::string & prop_name);
  ///@}

  ///@{
  /**
   * Declare the property named "name"
   */
  template <typename T>
  MaterialProperty<T> & declareProperty(const std::string & prop_name);
  template <typename T>
  MaterialProperty<T> & declarePropertyOld(const std::string & prop_name);
  template <typename T>
  MaterialProperty<T> & declarePropertyOlder(const std::string & prop_name);
  ///@}

  /**
   * Return a material property that is initialized to zero by default and does
   * not need to (but can) be declared by another material.
   */
  template <typename T>
  const MaterialProperty<T> & getZeroMaterialProperty(const std::string & prop_name);

  /**
   * Return a set of properties accessed with getMaterialProperty
   * @return A reference to the set of properties with calls to getMaterialProperty
   */
  virtual const std::set<std::string> & getRequestedItems() override { return _requested_props; }

  /**
   * Return a set of properties accessed with declareProperty
   * @return A reference to the set of properties with calls to declareProperty
   */
  virtual const std::set<std::string> & getSuppliedItems() override { return _supplied_props; }

  void checkStatefulSanity() const;

  /**
   * Get the list of output objects that this class is restricted
   * @return A vector of OutputNames
   */
  std::set<OutputName> getOutputs();

  /**
   * Returns true of the MaterialData type is not associated with volume data
   */
  bool isBoundaryMaterial() const { return _bnd; }

  /**
   * Subdomain setup evaluating material properties when required
   */
  virtual void subdomainSetup() override;

protected:
  /**
   * Evaluate material properties on subdomain
   */
  virtual void computeSubdomainProperties();

  /**
   * Users must override this method.
   */
  virtual void computeQpProperties();

  /**
   * Resets the properties prior to calculation of traditional materials (only if 'compute =
   * false').
   *
   * This method must be overridden in your class. This is called just prior to the re-calculation
   * of
   * traditional material properties to ensure that the properties are in a proper state for
   * calculation.
   */
  virtual void resetQpProperties();

  /**
   * Initialize stateful properties at quadrature points.  Note when using this function you only
   * need to address
   * the "current" material properties not the old ones directly, i.e. if you have a property named
   * "_diffusivity"
   * and an older property named "_diffusivity_old".  You only need to initialize diffusivity.
   * MOOSE will use
   * copy that initial value to the old and older values as necessary.
   */
  virtual void initQpStatefulProperties();

  /**
   * Copies dual number values from ADMaterials into Real property values for Material<->ADMaterial
   * interoperability.
   */
  void copyDualNumbersToValues();

  SubProblem & _subproblem;

  FEProblemBase & _fe_problem;
  THREAD_ID _tid;
  Assembly & _assembly;

  bool _bnd;
  bool _neighbor;

  unsigned int _qp;

  QBase *& _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
  const MooseArray<Point> & _q_point;
  /// normals at quadrature points (valid only in boundary materials)
  const MooseArray<Point> & _normals;

  const Elem *& _current_elem;

  const SubdomainID & _current_subdomain_id;

  /// current side of the current element
  unsigned int & _current_side;

  MooseMesh & _mesh;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  /// Set of properties accessed via get method
  std::set<std::string> _requested_props;

  /// Set of properties declared
  std::set<std::string> _supplied_props;

  /// The ids of the supplied properties, i.e. the indices where they
  /// are stored in the _material_data->props().  Note: these ids ARE
  /// NOT IN THE SAME ORDER AS THE _supplied_props set, which is
  /// ordered alphabetically by name.  The intention of this container
  /// is to allow rapid copying of MaterialProperty values in
  /// Material::computeProperties() without looking up the ids from
  /// the name strings each time.
  std::set<unsigned int> _supplied_prop_ids;

  /// The set of supplied regular property ids
  std::set<unsigned int> _supplied_regular_prop_ids;

  /// The set of supplied automatic differentiation property ids
  std::set<unsigned int> _supplied_ad_prop_ids;

  /// If False MOOSE does not compute this property
  const bool _compute;

  enum ConstantTypeEnum
  {
    NONE,
    ELEMENT,
    SUBDOMAIN
  };

  /// Options of the constantness level of the material
  const ConstantTypeEnum _constant_option;

  enum QP_Data_Type
  {
    CURR,
    PREV
  };

  enum Prop_State
  {
    CURRENT = 0x1,
    OLD = 0x2,
    OLDER = 0x4
  };
  std::map<std::string, int> _props_to_flags;

  /// Small helper function to call store{Subdomain,Boundary}MatPropName
  void registerPropName(std::string prop_name,
                        bool is_get,
                        Prop_State state,
                        bool is_declared_ad = false);

  /// Displacement ids
  std::vector<unsigned int> _displacements;

private:
  /// Check and throw an error if the execution has progerssed past the construction stage
  void checkExecutionStage();

  bool _has_stateful_property;

  bool _overrides_init_stateful_props = true;
};

template <typename T>
const MaterialProperty<T> &
Material::getMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid imput parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const ADMaterialPropertyObject<T> &
Material::getADMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid imput parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const ADMaterialPropertyObject<T> * default_property = defaultADMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getADMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyOld(const std::string & name)
{
  // Check if the supplied parameter is a valid imput parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyOldByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyOlder(const std::string & name)
{
  // Check if the supplied parameter is a valid imput parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyOlderByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyByName(const std::string & prop_name)
{
  checkExecutionStage();
  // The property may not exist yet, so declare it (declare/getMaterialProperty are referencing the
  // same memory)
  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, Material::CURRENT);
  return MaterialPropertyInterface::getMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const ADMaterialPropertyObject<T> &
Material::getADMaterialPropertyByName(const std::string & prop_name)
{
  checkExecutionStage();
  // The property may not exist yet, so declare it (declare/getADMaterialProperty are referencing
  // the same memory)
  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, Material::CURRENT);
  return MaterialPropertyInterface::getADMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyOldByName(const std::string & prop_name)
{
  registerPropName(prop_name, true, Material::OLD);
  return MaterialPropertyInterface::getMaterialPropertyOldByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyOlderByName(const std::string & prop_name)
{
  registerPropName(prop_name, true, Material::OLDER);
  return MaterialPropertyInterface::getMaterialPropertyOlderByName<T>(prop_name);
}

template <typename T>
MaterialProperty<T> &
Material::declareProperty(const std::string & prop_name)
{
  registerPropName(prop_name, false, Material::CURRENT);
  return _material_data->declareProperty<T>(prop_name);
}

template <typename T>
MaterialProperty<T> &
Material::declarePropertyOld(const std::string & prop_name)
{
  mooseDoOnce(
      mooseDeprecated("declarePropertyOld is deprecated and not needed anymore.\nUse "
                      "getMaterialPropertyOld (only) if a reference is required in this class."));
  registerPropName(prop_name, false, Material::OLD);
  return _material_data->declarePropertyOld<T>(prop_name);
}

template <typename T>
MaterialProperty<T> &
Material::declarePropertyOlder(const std::string & prop_name)
{
  mooseDoOnce(
      mooseDeprecated("declarePropertyOlder is deprecated and not needed anymore.  Use "
                      "getMaterialPropertyOlder (only) if a reference is required in this class."));
  registerPropName(prop_name, false, Material::OLDER);
  return _material_data->declarePropertyOlder<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
Material::getZeroMaterialProperty(const std::string & prop_name)
{
  checkExecutionStage();
  MaterialProperty<T> & preload_with_zero = _material_data->getProperty<T>(prop_name);

  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, Material::CURRENT);
  _fe_problem.markMatPropRequested(prop_name);

  // Register this material on these blocks and boundaries as a zero property with relaxed
  // consistency checking
  for (std::set<SubdomainID>::const_iterator it = blockIDs().begin(); it != blockIDs().end(); ++it)
    _fe_problem.storeSubdomainZeroMatProp(*it, prop_name);
  for (std::set<BoundaryID>::const_iterator it = boundaryIDs().begin(); it != boundaryIDs().end();
       ++it)
    _fe_problem.storeBoundaryZeroMatProp(*it, prop_name);

  // set values for all qpoints to zero
  // (in multiapp scenarios getMaxQps can return different values in each app; we need the max)
  unsigned int nqp = _mi_feproblem.getMaxQps();
  if (nqp > preload_with_zero.size())
    preload_with_zero.resize(nqp);
  for (unsigned int qp = 0; qp < nqp; ++qp)
    mooseSetToZero<T>(preload_with_zero[qp]);

  return preload_with_zero;
}

#endif // MATERIAL_H
