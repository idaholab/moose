//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralVectorPostprocessor.h"
#include "SamplerBase.h"
#include "BlockRestrictable.h"
#include "Assembly.h"
#include "MooseMesh.h"
#include "SwapBackSentinel.h"
#include "FEProblem.h"

// libMesh includes
#include "libmesh/quadrature.h"

// Forward Declarations
class MooseMesh;

/**
 * This is a base class for sampling material properties for the
 * integration points in all elements in a block of a 1-D mesh.
 * The positions of those points are output in x, y, z coordinates.
 * Derived classes can be created to sample arbitrary types of
 * material properties.
 */
template <typename T>
class Sampler1DBase : public GeneralVectorPostprocessor,
                      public SamplerBase,
                      public BlockRestrictable
{
public:
  /**
   * Class constructor
   * Sets up variables for output based on the properties to be output
   * @param parameters The input parameters
   */
  Sampler1DBase(const InputParameters & parameters);

  /**
   * Initialize
   * Calls through to base class's initialize()
   */
  virtual void initialize() override;

  /**
   * Loops through all elements in a block and samples their material properties.
   */
  virtual void execute() override;

  /**
   * Finalize
   * Calls through to base class's finalize()
   */
  virtual void finalize() override;

  /**
   * Reduce the material property to a scalar for output
   * @param property The material property
   * @param curr_point The point corresponding to this material property
   * @return A scalar value from this material property to be output
   */
  virtual Real getScalarFromProperty(const T & property, const Point & curr_point) = 0;

protected:
  /// The material properties to be output
  std::vector<const MaterialProperty<T> *> _material_properties;

  /// The mesh
  MooseMesh & _mesh;

  /// The quadrature rule
  const QBase * const & _qrule;

  /// The quadrature points
  const MooseArray<Point> & _q_point;

public:
  static InputParameters validParams();
};

template <typename T>
Sampler1DBase<T>::Sampler1DBase(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SamplerBase(parameters, this, _communicator),
    BlockRestrictable(this),
    _mesh(_subproblem.mesh()),
    _qrule(_assembly.qRule()),
    _q_point(_assembly.qPoints())
{
  std::vector<std::string> material_property_names = getParam<std::vector<std::string>>("property");
  for (unsigned int i = 0; i < material_property_names.size(); ++i)
  {
    if (!hasMaterialProperty<T>(material_property_names[i]))
      mooseError("In Sampler1DBase material property: " + material_property_names[i] +
                 " does not exist.");
    _material_properties.push_back(&getMaterialProperty<T>(material_property_names[i]));
  }

  SamplerBase::setupVariables(material_property_names);
}

template <typename T>
void
Sampler1DBase<T>::initialize()
{
  SamplerBase::initialize();
}

template <typename T>
void
Sampler1DBase<T>::execute()
{
  std::vector<Real> values(_material_properties.size());

  std::set<unsigned int> needed_mat_props;
  const std::set<unsigned int> & mp_deps = getMatPropDependencies();
  needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);

  ConstElemRange & elem_range = *(_mesh.getActiveLocalElementRange());
  for (typename ConstElemRange::const_iterator el = elem_range.begin(); el != elem_range.end();
       ++el)
  {
    const Elem * elem = *el;

    if (elem->processor_id() != processor_id())
      continue;

    if (!hasBlocks(elem->subdomain_id()))
      continue;

    _subproblem.setCurrentSubdomainID(elem, _tid);
    _subproblem.prepare(elem, _tid);
    _subproblem.reinitElem(elem, _tid);

    // Set up Sentinel class so that, even if reinitMaterials() throws, we
    // still remember to swap back during stack unwinding.
    SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterials, _tid);
    _fe_problem.reinitMaterials(elem->subdomain_id(), _tid);

    for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    {
      for (unsigned int j = 0; j < _material_properties.size(); ++j)
        values[j] = getScalarFromProperty((*_material_properties[j])[qp], _q_point[qp]);

      // use the "x" coordinate as the "id"; at this time, it is not used for anything
      addSample(_q_point[qp], _q_point[qp](0), values);
    }
  }
  _fe_problem.clearActiveMaterialProperties(_tid);
}

template <typename T>
void
Sampler1DBase<T>::finalize()
{
  SamplerBase::finalize();
}
