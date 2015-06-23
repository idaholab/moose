/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MATERIAL_H
#define MATERIAL_H

#include "MooseObject.h"
#include "SetupInterface.h"
#include "Coupleable.h"
#include "ScalarCoupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "MaterialProperty.h"
#include "MaterialPropertyInterface.h"
#include "MaterialData.h"
#include "ParallelUniqueId.h"
#include "Problem.h"
#include "SubProblem.h"
#include "DependencyResolverInterface.h"
#include "Function.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "Restartable.h"
#include "ZeroInterface.h"
#include "MeshChangedInterface.h"
#include "OutputInterface.h"

// libMesh includes
#include "libmesh/quadrature_gauss.h"
#include "libmesh/elem.h"

// forward declarations
class Material;
class MooseMesh;

/**
 * Holds a data structure used to compute material properties at a Quadrature point
 */
struct QpData
{
  virtual ~QpData(){}

  inline virtual QpData& operator=(const QpData &) { return *this; }
};


template<>
InputParameters validParams<Material>();

/**
 * Holds material properties that are assigned to blocks.
 */
class Material :
  public MooseObject,
  public BlockRestrictable,
  public BoundaryRestrictable,
  public SetupInterface,
  public Coupleable,
  public MooseVariableDependencyInterface,
  public ScalarCoupleable,
  public FunctionInterface,
  public UserObjectInterface,
  public TransientInterface,
  public MaterialPropertyInterface,
  public PostprocessorInterface,
  public DependencyResolverInterface,
  public Restartable,
  public ZeroInterface,
  public MeshChangedInterface,
  public OutputInterface
{
public:
  Material(const std::string & name, InputParameters parameters);

  virtual ~Material();

  /**
   * All materials must override this virtual.
   * This is where they fill up the vectors with values.
   */
  virtual void computeProperties();

  /**
   * Initialize stateful properties (if material has some)
   */
  virtual void initStatefulProperties(unsigned int n_points);

  ///@{
  /**
   * Retrieve the property throgh a given input parameter key with a fallback
   * to getting it by name
   */
  template<typename T>
  const MaterialProperty<T> & getMaterialProperty(const std::string & name);
  template<typename T>
  const MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);
  template<typename T>
  const MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);
  ///@}

  ///@{
  /**
   * Retrieve the property named "name"
   */
  template<typename T>
  const MaterialProperty<T> & getMaterialPropertyByName(const std::string & prop_name);
  template<typename T>
  const MaterialProperty<T> & getMaterialPropertyOldByName(const std::string & prop_name);
  template<typename T>
  const MaterialProperty<T> & getMaterialPropertyOlderByName(const std::string & prop_name);
  ///@}

  ///@{
  /**
   * Declare the property named "name"
   */
  template<typename T>
  MaterialProperty<T> & declareProperty(const std::string & prop_name);
  template<typename T>
  MaterialProperty<T> & declarePropertyOld(const std::string & prop_name);
  template<typename T>
  MaterialProperty<T> & declarePropertyOlder(const std::string & prop_name);
  ///@}

  /**
   * Return a set of properties accessed with getMaterialProperty
   * @return A reference to the set of properties with calls to getMaterialProperty
   */
  virtual
  const std::set<std::string> &
  getRequestedItems() { return _requested_props; }

  /**
   * Return a set of properties accessed with declareProperty
   * @return A reference to the set of properties with calls to declareProperty
   */
  virtual
  const std::set<std::string> &
  getSuppliedItems() { return _supplied_props; }

  /**
   * Return a set of propreties that have been accessed with materialProperty
   * @return A reference to the properties with calls to materialProperty
   */
  virtual
  const std::set<std::string> &
  getIterativeItems() { return _iterative_props; }

  /**
   * Return the unique property id for the supplied property name
   */
  template<typename T>
  unsigned int
  materialProperty(const std::string & prop_name);

  void checkStatefulSanity() const;

  /**
   * Check if a material property is valid for all blocks of this Material
   *
   * This method returns true if the supplied property name has been declared
   * in a Material object on the block ids for this object.
   *
   * @param prop_name the name of the property to query
   * @return true if the property exists for all block ids of the object, otherwise false
   *
   * @see BlockRestrictable::hasBlockMaterialPropertyHelper
   */
  virtual bool hasBlockMaterialPropertyHelper(const std::string & prop_name);

  /**
   * Get the list of output objects that this class is restricted
   * @return A vector of OutputNames
   */
  std::set<OutputName> getOutputs();

protected:
  SubProblem & _subproblem;

  FEProblem & _fe_problem;
  THREAD_ID _tid;
  Assembly & _assembly;
  bool _bnd;
  bool _neighbor;
  MaterialData & _material_data;

  unsigned int _qp;

  QBase * & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
  const MooseArray<Point> & _q_point;
  /// normals at quadrature points (valid only in boundary materials)
  const MooseArray<Point> & _normals;

  const Elem * & _current_elem;

  /// current side of the current element
  unsigned int & _current_side;

  MooseMesh & _mesh;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  /// Set of properties accessed via get method
  std::set<std::string> _requested_props;

  /// Set of properties declared
  std::set<std::string> _supplied_props;

  /// Set of properties used for iteration
  std::set<std::string> _iterative_props;

  /// Reference to the MaterialWarehouse
  MaterialWarehouse & _material_warehouse;

  enum QP_Data_Type {
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

  /**
   * Initialize stateful properties at quadrature points.  Note when using this function you only need to address
   * the "current" material properties not the old ones directly, i.e. if you have a property named "_diffusivity"
   * and an older property named "_diffusivity_old".  You only need to initialize diffusivity.  MOOSE will use
   * copy that initial value to the old and older values as necessary.
   */
  virtual void initQpStatefulProperties();

  /**
   * Compute material properties in quadrature point
   * Materials has to redefine this.
   */
  virtual void computeQpProperties();

  /**
   * This function is called to create the data structure that will be associated
   * with a quadrature point
   */
  virtual QpData * createData();

  ///@{
  /**
   * Recalculate the specified material object given a property id
   * @param prop_id The property id(s) of interest that needs to be recomputed, this id should be retrieved with
   *                a call to `materialProperty`.
   * @param qp The quadrature point index at which to recompute the properties
   *
   * Note, this method calls computeQpProperties on the Material object responsible for computing the supplied property;
   * hence, this will also compute all other properties as well.
   */
  void recomputeMaterial(const unsigned int & prop_id, unsigned int qp);
  void recomputeMaterial(const std::vector<unsigned int> & prop_id, unsigned int qp);
  ///@}

  std::map<unsigned int, std::vector<QpData *> > _qp_prev;
  std::map<unsigned int, std::vector<QpData *> > _qp_curr;

private:

  ///@{
  /**
   * Helper functions to call storeMatPropName
   */
  void registerPropName(const std::string & prop_name);
  void registerSuppliedPropName(const std::string & prop_name, Prop_State state);
  ////@}

  /**
   * A helper function for recomputing the material properties at a specific quadrature point
   * @param qp The quadrature point to recompute
   */
  void recomputeProperties(unsigned int qp);

  bool _has_stateful_property;
};

template<typename T>
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

template<typename T>
unsigned int
Material::materialProperty(const std::string & prop_name)
{
  _iterative_props.insert(prop_name);
  registerPropName(prop_name);
  _fe_problem.markMatPropRequested(prop_name);
  return _material_data.getPropertyId(prop_name);
}

template<typename T>
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

template<typename T>
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

template<typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyByName(const std::string & prop_name)
{
  // The property may not exist yet, so declare it (declare/getMaterialProperty are referencing the same memory)
  _requested_props.insert(prop_name);
  registerPropName(prop_name);
  _fe_problem.markMatPropRequested(prop_name);
  return _material_data.getProperty<T>(prop_name);
}

template<typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyOldByName(const std::string & prop_name)
{
  registerPropName(prop_name);
  _fe_problem.markMatPropRequested(prop_name);
  return _material_data.getPropertyOld<T>(prop_name);
}

template<typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyOlderByName(const std::string & prop_name)
{
  registerPropName(prop_name);
  _fe_problem.markMatPropRequested(prop_name);
  return _material_data.getPropertyOlder<T>(prop_name);
}


template<typename T>
MaterialProperty<T> &
Material::declareProperty(const std::string & prop_name)
{
  _supplied_props.insert(prop_name);
  registerSuppliedPropName(prop_name, Material::CURRENT);
  return _material_data.declareProperty<T>(prop_name);
}

template<typename T>
MaterialProperty<T> &
Material::declarePropertyOld(const std::string & prop_name)
{
  registerSuppliedPropName(prop_name, Material::OLD);
  return _material_data.declarePropertyOld<T>(prop_name);
}

template<typename T>
MaterialProperty<T> &
Material::declarePropertyOlder(const std::string & prop_name)
{
  registerSuppliedPropName(prop_name, Material::OLDER);
  return _material_data.declarePropertyOlder<T>(prop_name);
}


#endif //MATERIAL_H
