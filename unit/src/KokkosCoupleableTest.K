//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "AppFactory.h"
#include "Coupleable.h"
#include "FEProblem.h"
#include "KokkosDatum.h"
#include "MooseMain.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MooseVariableFieldBase.h"
#include "MooseVariableScalar.h"
#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"
#include "ScalarCoupleable.h"

class KokkosCoupleableTestObject : public MooseObject, public Coupleable, public ScalarCoupleable
{
public:
  using ScalarCoupleable::kokkosCoupledScalarDot;
  using ScalarCoupleable::kokkosCoupledScalarDotDu;
  using ScalarCoupleable::kokkosCoupledScalarValue;
  using ScalarCoupleable::kokkosCoupledScalarValueOld;
  using ScalarCoupleable::kokkosCoupledScalarValueOlder;
  using ScalarCoupleable::kokkosCoupledVectorTagScalarValue;
  using ScalarCoupleable::kokkosCoupledVectorTagScalarValueByName;

  static InputParameters validParams()
  {
    InputParameters params = MooseObject::validParams();
    params.registerBase("UserObject");
    params.addCoupledVar("field", "A coupled field variable");
    params.addCoupledVar("fields", "Multiple coupled field variables");
    params.addCoupledVar("vector", "A coupled vector field variable");
    params.addCoupledVar("vectors", "Multiple coupled vector field variables");
    params.addCoupledVar("scalar", "A coupled scalar variable");
    params.addCoupledVar("default_field", std::vector<Real>{1.25, -2.5}, "Default field values");
    params.addCoupledVar("default_vector", std::vector<Real>{1, 2, 3}, "Default vector value");
    params.addCoupledVar("default_scalar", 4.5, "Default scalar value");
    params.addParam<TagName>("tag", Moose::OLD_SOLUTION_TAG, "Vector tag used by the tests");
    params.addParam<bool>("implicit", true, "Whether the object uses an implicit scheme");
    params.addParam<bool>("nodal", false, "Whether coupledValue requests nodal values");
    return params;
  }

  KokkosCoupleableTestObject(const InputParameters & params)
    : MooseObject(params), Coupleable(this, params.get<bool>("nodal")), ScalarCoupleable(this)
  {
  }
};

registerMooseObject("MooseUnitApp", KokkosCoupleableTestObject);

class KokkosCoupledValueEvaluator : public Moose::Kokkos::AssemblyHolder,
                                    public Moose::Kokkos::FESystemHolder
{
public:
  KokkosCoupledValueEvaluator(const Moose::Kokkos::Assembly & assembly,
                              Moose::Kokkos::Array<Moose::Kokkos::FESystem> & systems,
                              const Moose::Kokkos::VariableValue & elemental_value,
                              const Moose::Kokkos::VariableGradient & elemental_gradient,
                              const Moose::Kokkos::VariableValue & elemental_dof_value,
                              const Moose::Kokkos::VariableValue & nodal_value,
                              const Moose::Kokkos::VariableValue & nodal_dof_value,
                              const Moose::Kokkos::VariableValue & alpha,
                              const Moose::Kokkos::VariableValue & beta,
                              const Moose::Kokkos::VectorVariableValue & vector_value,
                              const Moose::Kokkos::VectorVariableValue & vector_dof_value,
                              const Moose::Kokkos::VectorVariableValue & vector_nodal_value,
                              const Moose::Kokkos::ADVariableValue & elemental_ad_value,
                              const Moose::Kokkos::ADVariableValue & elemental_ad_dof_value,
                              const Moose::Kokkos::ADVariableValue & nodal_ad_value,
                              const Moose::Kokkos::ADVariableValue & nodal_ad_dof_value,
                              Moose::Kokkos::Array<Real> & results)
    : AssemblyHolder(assembly),
      FESystemHolder(systems),
      _elemental_value(elemental_value),
      _elemental_gradient(elemental_gradient),
      _elemental_dof_value(elemental_dof_value),
      _nodal_value(nodal_value),
      _nodal_dof_value(nodal_dof_value),
      _alpha(alpha),
      _beta(beta),
      _vector_value(vector_value),
      _vector_dof_value(vector_dof_value),
      _vector_nodal_value(vector_nodal_value),
      _elemental_ad_value(elemental_ad_value),
      _elemental_ad_dof_value(elemental_ad_dof_value),
      _nodal_ad_value(nodal_ad_value),
      _nodal_ad_dof_value(nodal_ad_dof_value),
      _results(results)
  {
  }

  KOKKOS_FUNCTION void operator()(const int) const
  {
    const auto & elemental_primary = _elemental_value.variable();
    const auto & nodal_primary = _nodal_value.variable();
    AssemblyDatum elem_datum(0,
                             libMesh::invalid_uint,
                             kokkosAssembly(),
                             kokkosSystems(),
                             elemental_primary,
                             elemental_primary.var());
    AssemblyDatum node_datum(
        0, kokkosAssembly(), kokkosSystems(), nodal_primary, nodal_primary.var());

    _results(0) = _elemental_value(elem_datum, 0);
    const auto gradient = _elemental_gradient(elem_datum, 0);
    for (unsigned int i = 0; i < 3; ++i)
      _results(1 + i) = gradient(i);
    _results(4) = _elemental_dof_value(elem_datum, 0);
    _results(5) = _nodal_value(node_datum, 0);
    _results(6) = _nodal_dof_value(node_datum, 0);
    _results(7) = _alpha(elem_datum, 0);
    _results(8) = _beta(node_datum, 0);

    const auto vector_value = _vector_value(elem_datum, 0);
    const auto vector_dof_value = _vector_dof_value(elem_datum, 0);
    const auto vector_nodal_value = _vector_nodal_value(node_datum, 0);
    for (unsigned int i = 0; i < 3; ++i)
    {
      _results(9 + i) = vector_value(i);
      _results(12 + i) = vector_dof_value(i);
      _results(15 + i) = vector_nodal_value(i);
    }

    const auto elemental_ad_value = _elemental_ad_value(elem_datum, 0);
    const auto elemental_ad_dof_value = _elemental_ad_dof_value(elem_datum, 0);
    const auto elemental_dof =
        elem_datum.system(elemental_primary.sys())
            .getElemGlobalDofIndex(elem_datum.elem().id, 0, elemental_primary.var());
    _results(18) = elemental_ad_value.value();
    _results(19) = elemental_ad_value.derivatives()[elemental_dof];
    _results(20) = elemental_ad_dof_value.value();
    _results(21) = elemental_ad_dof_value.derivatives()[elemental_dof];

    const auto nodal_ad_value = _nodal_ad_value(node_datum, 0);
    const auto nodal_ad_dof_value = _nodal_ad_dof_value(node_datum, 0);
    const auto nodal_dof = node_datum.system(nodal_primary.sys())
                               .getNodeGlobalDofIndex(node_datum.node(), 0, nodal_primary.var());
    _results(22) = nodal_ad_value.value();
    _results(23) = nodal_ad_value.derivatives()[nodal_dof];
    _results(24) = nodal_ad_dof_value.value();
    _results(25) = nodal_ad_dof_value.derivatives()[nodal_dof];
  }

private:
  const Moose::Kokkos::VariableValue _elemental_value;
  const Moose::Kokkos::VariableGradient _elemental_gradient;
  const Moose::Kokkos::VariableValue _elemental_dof_value;
  const Moose::Kokkos::VariableValue _nodal_value;
  const Moose::Kokkos::VariableValue _nodal_dof_value;
  const Moose::Kokkos::VariableValue _alpha;
  const Moose::Kokkos::VariableValue _beta;
  const Moose::Kokkos::VectorVariableValue _vector_value;
  const Moose::Kokkos::VectorVariableValue _vector_dof_value;
  const Moose::Kokkos::VectorVariableValue _vector_nodal_value;
  const Moose::Kokkos::ADVariableValue _elemental_ad_value;
  const Moose::Kokkos::ADVariableValue _elemental_ad_dof_value;
  const Moose::Kokkos::ADVariableValue _nodal_ad_value;
  const Moose::Kokkos::ADVariableValue _nodal_ad_dof_value;
  Moose::Kokkos::Array<Real> _results;
};

class KokkosCoupleableTest : public ::testing::Test
{
public:
  KokkosCoupleableTest()
    : _app(Moose::createMooseApp("MooseUnitApp", 0, nullptr)), _factory(_app->getFactory())
  {
    InputParameters mesh_params = _factory.getValidParams("GeneratedMesh");
    mesh_params.set<MooseEnum>("dim") = "2";
    mesh_params.set<unsigned int>("nx") = 1;
    mesh_params.set<unsigned int>("ny") = 1;
    _mesh = _factory.createUnique<MooseMesh>("GeneratedMesh", "moose_mesh", mesh_params);
    _mesh->setMeshBase(_mesh->buildMeshBaseObject());
    _mesh->buildMesh();
    _mesh->prepare();

    InputParameters problem_params = _factory.getValidParams("FEProblem");
    problem_params.set<MooseMesh *>("mesh") = _mesh.get();
    problem_params.set<std::string>(MooseBase::name_param) = "problem";
    _problem = _factory.create<FEProblem>("FEProblem", "problem", problem_params);
    _problem->createQRules(libMesh::QGAUSS, libMesh::FIRST, libMesh::FIRST, libMesh::FIRST);
    _problem->addVectorTag(Moose::SOLUTION_DOT_TAG, Moose::VECTOR_TAG_SOLUTION);
    _app->actionWarehouse().problemBase() = _problem;
  }

protected:
  MooseVariableFieldBase & addFieldVariable(const std::string & name)
  {
    InputParameters params = _factory.getValidParams("MooseVariable");
    _problem->addVariable("MooseVariable", name, params);
    return _problem->getVariable(0, name);
  }

  MooseVariableFieldBase & addVectorVariable(const std::string & name)
  {
    InputParameters params = _factory.getValidParams("VectorMooseVariable");
    params.set<MooseEnum>("family") = "LAGRANGE_VEC";
    _problem->addVariable("VectorMooseVariable", name, params);
    return _problem->getVariable(0, name);
  }

  MooseVariableFieldBase & addElementalVariable(const std::string & name)
  {
    InputParameters params = _factory.getValidParams("MooseVariable");
    params.set<MooseEnum>("family") = "MONOMIAL";
    params.set<MooseEnum>("order") = "CONSTANT";
    _problem->addVariable("MooseVariable", name, params);
    return _problem->getVariable(0, name);
  }

  MooseVariableScalar & addScalarVariable(const std::string & name)
  {
    InputParameters params = _factory.getValidParams("MooseVariableScalar");
    params.set<MooseEnum>("order") = "FIRST";
    params.set<MooseEnum>("family") = "SCALAR";
    _problem->addVariable("MooseVariableScalar", name, params);
    return _problem->getScalarVariable(0, name);
  }

  void initSystems()
  {
    for (const auto system : make_range(_problem->numNonlinearSystems()))
      _problem->getNonlinearSystemBase(system).system().init();
    _problem->getAuxiliarySystem().system().init();
  }

  void setFieldValue(MooseVariableFieldBase & variable, Real value)
  {
    auto & system = variable.sys();
    auto & solution = system.solution();
    std::vector<dof_id_type> dof_indices;

    for (const auto * elem : _mesh->getMesh().active_local_element_ptr_range())
    {
      system.dofMap().dof_indices(elem, dof_indices, variable.number());
      for (const auto dof : dof_indices)
        solution.set(dof, value);
    }
  }

  void setScalarValue(MooseVariableScalar & variable, Real value)
  {
    auto & system = variable.sys();
    auto & solution = system.solution();
    std::vector<dof_id_type> dof_indices;
    system.dofMap().SCALAR_dof_indices(dof_indices, variable.number());
    for (const auto dof : dof_indices)
      solution.set(dof, value);
  }

  void setVectorFieldValue(MooseVariableFieldBase & variable, const RealVectorValue & value)
  {
    auto & system = variable.sys();
    auto & solution = system.solution();
    const auto system_number = system.number();
    const auto variable_number = variable.number();

    for (const auto * node : _mesh->getMesh().local_node_ptr_range())
      for (const auto component : make_range(node->n_comp(system_number, variable_number)))
        solution.set(node->dof_number(system_number, variable_number, component), value(component));
  }

  std::shared_ptr<KokkosCoupleableTestObject>
  createObject(const std::string & name,
               const std::vector<VariableName> & field = {},
               const std::vector<VariableName> & fields = {},
               const std::vector<VariableName> & vector = {},
               const std::vector<VariableName> & vectors = {},
               const std::vector<VariableName> & scalar = {},
               bool implicit = true,
               bool nodal = false)
  {
    InputParameters params = _factory.getValidParams("KokkosCoupleableTestObject");
    params.set<FEProblemBase *>("_fe_problem_base") = _problem.get();
    params.set<SubProblem *>("_subproblem") = _problem.get();
    params.set<bool>("implicit") = implicit;
    params.set<bool>("nodal") = nodal;
    if (!field.empty())
      params.set<std::vector<VariableName>>("field") = field;
    if (!fields.empty())
      params.set<std::vector<VariableName>>("fields") = fields;
    if (!vector.empty())
      params.set<std::vector<VariableName>>("vector") = vector;
    if (!vectors.empty())
      params.set<std::vector<VariableName>>("vectors") = vectors;
    if (!scalar.empty())
      params.set<std::vector<VariableName>>("scalar") = scalar;
    return _factory.create<KokkosCoupleableTestObject>("KokkosCoupleableTestObject", name, params);
  }

  template <typename Wrapper>
  void expectVariable(const Wrapper & wrapper,
                      const MooseVariableBase & moose_variable,
                      const TagName & tag,
                      unsigned int components = 1)
  {
    const auto & variable = wrapper.variable();
    EXPECT_TRUE(variable.initialized());
    EXPECT_TRUE(variable.coupled());
    EXPECT_EQ(variable.components(), components);
    EXPECT_EQ(variable.var(), moose_variable.number());
    EXPECT_EQ(variable.sys(), moose_variable.sys().number());
    EXPECT_EQ(variable.tag(), _problem->getVectorTagID(tag));
  }

  template <typename Wrapper>
  void expectVariables(const Wrapper & wrapper,
                       const MooseVariableBase & first,
                       const MooseVariableBase & second,
                       const TagName & tag)
  {
    const auto & variable = wrapper.variable();
    EXPECT_TRUE(variable.initialized());
    EXPECT_TRUE(variable.coupled());
    EXPECT_EQ(variable.components(), 2);
    EXPECT_EQ(variable.var(0), first.number());
    EXPECT_EQ(variable.var(1), second.number());
    EXPECT_EQ(variable.sys(0), first.sys().number());
    EXPECT_EQ(variable.sys(1), second.sys().number());
    EXPECT_EQ(variable.tag(), _problem->getVectorTagID(tag));
  }

  std::unique_ptr<MooseMesh> _mesh;
  std::shared_ptr<MooseApp> _app;
  Factory & _factory;
  std::shared_ptr<FEProblem> _problem;
};

TEST_F(KokkosCoupleableTest, FieldVariableAPIs)
{
  auto & u = addFieldVariable("u");
  auto & v = addFieldVariable("v");
  auto object = createObject("field_object", {"u"}, {"u", "v"});

  expectVariable(object->kokkosCoupledVectorTagValueByName("field", Moose::SOLUTION_TAG),
                 u,
                 Moose::SOLUTION_TAG);
  expectVariables(object->kokkosCoupledVectorTagValuesByName("fields", Moose::SOLUTION_TAG),
                  u,
                  v,
                  Moose::SOLUTION_TAG);
  expectVariable(object->kokkosCoupledVectorTagGradientByName("field", Moose::SOLUTION_TAG),
                 u,
                 Moose::SOLUTION_TAG);
  expectVariables(object->kokkosCoupledVectorTagGradientsByName("fields", Moose::SOLUTION_TAG),
                  u,
                  v,
                  Moose::SOLUTION_TAG);
  expectVariable(object->kokkosCoupledVectorTagNodalValueByName("field", Moose::SOLUTION_TAG),
                 u,
                 Moose::SOLUTION_TAG);
  expectVariables(object->kokkosCoupledVectorTagNodalValuesByName("fields", Moose::SOLUTION_TAG),
                  u,
                  v,
                  Moose::SOLUTION_TAG);
  expectVariable(object->kokkosCoupledVectorTagDofValueByName("field", Moose::SOLUTION_TAG),
                 u,
                 Moose::SOLUTION_TAG);
  expectVariables(object->kokkosCoupledVectorTagDofValuesByName("fields", Moose::SOLUTION_TAG),
                  u,
                  v,
                  Moose::SOLUTION_TAG);

  expectVariable(object->kokkosCoupledVectorTagValue("field", "tag"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorTagValues("fields", "tag"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(
      object->kokkosCoupledVectorTagGradient("field", "tag"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorTagGradients("fields", "tag"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(
      object->kokkosCoupledVectorTagNodalValue("field", "tag"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorTagNodalValues("fields", "tag"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(
      object->kokkosCoupledVectorTagDofValue("field", "tag"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorTagDofValues("fields", "tag"), u, v, Moose::OLD_SOLUTION_TAG);

  expectVariable(object->kokkosCoupledValue("field"), u, Moose::SOLUTION_TAG);
  expectVariables(object->kokkosCoupledValues("fields"), u, v, Moose::SOLUTION_TAG);
  expectVariable(object->kokkosCoupledGradient("field"), u, Moose::SOLUTION_TAG);
  expectVariables(object->kokkosCoupledGradients("fields"), u, v, Moose::SOLUTION_TAG);
  expectVariable(object->kokkosCoupledNodalValue("field"), u, Moose::SOLUTION_TAG);
  expectVariables(object->kokkosCoupledNodalValues("fields"), u, v, Moose::SOLUTION_TAG);
  expectVariable(object->kokkosCoupledDofValue("field"), u, Moose::SOLUTION_TAG);
  expectVariables(object->kokkosCoupledDofValues("fields"), u, v, Moose::SOLUTION_TAG);

  expectVariable(object->kokkosCoupledValueOld("field"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(object->kokkosCoupledValuesOld("fields"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(object->kokkosCoupledGradientOld("field"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(object->kokkosCoupledGradientsOld("fields"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(object->kokkosCoupledNodalValueOld("field"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(object->kokkosCoupledNodalValuesOld("fields"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(object->kokkosCoupledDofValueOld("field"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(object->kokkosCoupledDofValuesOld("fields"), u, v, Moose::OLD_SOLUTION_TAG);

  expectVariable(object->kokkosCoupledValueOlder("field"), u, Moose::OLDER_SOLUTION_TAG);
  expectVariables(object->kokkosCoupledValuesOlder("fields"), u, v, Moose::OLDER_SOLUTION_TAG);
  expectVariable(object->kokkosCoupledGradientOlder("field"), u, Moose::OLDER_SOLUTION_TAG);
  expectVariables(object->kokkosCoupledGradientsOlder("fields"), u, v, Moose::OLDER_SOLUTION_TAG);
  expectVariable(object->kokkosCoupledNodalValueOlder("field"), u, Moose::OLDER_SOLUTION_TAG);
  expectVariables(object->kokkosCoupledNodalValuesOlder("fields"), u, v, Moose::OLDER_SOLUTION_TAG);
  expectVariable(object->kokkosCoupledDofValueOlder("field"), u, Moose::OLDER_SOLUTION_TAG);
  expectVariables(object->kokkosCoupledDofValuesOlder("fields"), u, v, Moose::OLDER_SOLUTION_TAG);

  expectVariable(object->kokkosCoupledDot("field"), u, Moose::SOLUTION_DOT_TAG);
  expectVariables(object->kokkosCoupledDots("fields"), u, v, Moose::SOLUTION_DOT_TAG);
  expectVariable(object->kokkosCoupledNodalDot("field"), u, Moose::SOLUTION_DOT_TAG);
  expectVariables(object->kokkosCoupledNodalDots("fields"), u, v, Moose::SOLUTION_DOT_TAG);
  EXPECT_DOUBLE_EQ(+object->kokkosCoupledDotDu("field"), u.sys().duDotDu(u.number()));
}

TEST_F(KokkosCoupleableTest, VectorVariableAPIs)
{
  auto & u = addVectorVariable("u");
  auto & v = addVectorVariable("v");
  auto object = createObject("vector_object", {}, {}, {"u"}, {"u", "v"});

  expectVariable(object->kokkosCoupledVectorTagVectorValueByName("vector", Moose::SOLUTION_TAG),
                 u,
                 Moose::SOLUTION_TAG);
  expectVariables(object->kokkosCoupledVectorTagVectorValuesByName("vectors", Moose::SOLUTION_TAG),
                  u,
                  v,
                  Moose::SOLUTION_TAG);
  expectVariable(object->kokkosCoupledVectorTagVectorGradientByName("vector", Moose::SOLUTION_TAG),
                 u,
                 Moose::SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorTagVectorGradientsByName("vectors", Moose::SOLUTION_TAG),
      u,
      v,
      Moose::SOLUTION_TAG);
  expectVariable(
      object->kokkosCoupledVectorTagVectorNodalValueByName("vector", Moose::SOLUTION_TAG),
      u,
      Moose::SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorTagVectorNodalValuesByName("vectors", Moose::SOLUTION_TAG),
      u,
      v,
      Moose::SOLUTION_TAG);
  expectVariable(object->kokkosCoupledVectorTagVectorDofValueByName("vector", Moose::SOLUTION_TAG),
                 u,
                 Moose::SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorTagVectorDofValuesByName("vectors", Moose::SOLUTION_TAG),
      u,
      v,
      Moose::SOLUTION_TAG);

  expectVariable(
      object->kokkosCoupledVectorTagVectorValue("vector", "tag"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorTagVectorValues("vectors", "tag"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(
      object->kokkosCoupledVectorTagVectorGradient("vector", "tag"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(object->kokkosCoupledVectorTagVectorGradients("vectors", "tag"),
                  u,
                  v,
                  Moose::OLD_SOLUTION_TAG);
  expectVariable(
      object->kokkosCoupledVectorTagVectorNodalValue("vector", "tag"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(object->kokkosCoupledVectorTagVectorNodalValues("vectors", "tag"),
                  u,
                  v,
                  Moose::OLD_SOLUTION_TAG);
  expectVariable(
      object->kokkosCoupledVectorTagVectorDofValue("vector", "tag"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(object->kokkosCoupledVectorTagVectorDofValues("vectors", "tag"),
                  u,
                  v,
                  Moose::OLD_SOLUTION_TAG);

  expectVariable(object->kokkosCoupledVectorValue("vector"), u, Moose::SOLUTION_TAG);
  expectVariables(object->kokkosCoupledVectorValues("vectors"), u, v, Moose::SOLUTION_TAG);
  expectVariable(object->kokkosCoupledVectorGradient("vector"), u, Moose::SOLUTION_TAG);
  expectVariables(object->kokkosCoupledVectorGradients("vectors"), u, v, Moose::SOLUTION_TAG);
  expectVariable(object->kokkosCoupledVectorNodalValue("vector"), u, Moose::SOLUTION_TAG);
  expectVariables(object->kokkosCoupledVectorNodalValues("vectors"), u, v, Moose::SOLUTION_TAG);
  expectVariable(object->kokkosCoupledVectorDofValue("vector"), u, Moose::SOLUTION_TAG);
  expectVariables(object->kokkosCoupledVectorDofValues("vectors"), u, v, Moose::SOLUTION_TAG);

  expectVariable(object->kokkosCoupledVectorValueOld("vector"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(object->kokkosCoupledVectorValuesOld("vectors"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(object->kokkosCoupledVectorGradientOld("vector"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorGradientsOld("vectors"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(object->kokkosCoupledVectorNodalValueOld("vector"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorNodalValuesOld("vectors"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(object->kokkosCoupledVectorDofValueOld("vector"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorDofValuesOld("vectors"), u, v, Moose::OLD_SOLUTION_TAG);

  expectVariable(object->kokkosCoupledVectorValueOlder("vector"), u, Moose::OLDER_SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorValuesOlder("vectors"), u, v, Moose::OLDER_SOLUTION_TAG);
  expectVariable(object->kokkosCoupledVectorGradientOlder("vector"), u, Moose::OLDER_SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorGradientsOlder("vectors"), u, v, Moose::OLDER_SOLUTION_TAG);
  expectVariable(
      object->kokkosCoupledVectorNodalValueOlder("vector"), u, Moose::OLDER_SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorNodalValuesOlder("vectors"), u, v, Moose::OLDER_SOLUTION_TAG);
  expectVariable(object->kokkosCoupledVectorDofValueOlder("vector"), u, Moose::OLDER_SOLUTION_TAG);
  expectVariables(
      object->kokkosCoupledVectorDofValuesOlder("vectors"), u, v, Moose::OLDER_SOLUTION_TAG);

  expectVariable(object->kokkosCoupledVectorDot("vector"), u, Moose::SOLUTION_DOT_TAG);
  expectVariables(object->kokkosCoupledVectorDots("vectors"), u, v, Moose::SOLUTION_DOT_TAG);
  expectVariable(object->kokkosCoupledVectorNodalDot("vector"), u, Moose::SOLUTION_DOT_TAG);
  expectVariables(object->kokkosCoupledVectorNodalDots("vectors"), u, v, Moose::SOLUTION_DOT_TAG);
}

TEST_F(KokkosCoupleableTest, ADFieldVariableAPIs)
{
  auto & u = addFieldVariable("u");
  auto & v = addFieldVariable("v");
  auto object = createObject("ad_object", {"u"}, {"u", "v"});

  expectVariable(object->kokkosADCoupledVectorTagValueByName("field", Moose::SOLUTION_TAG),
                 u,
                 Moose::SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledVectorTagValuesByName("fields", Moose::SOLUTION_TAG),
                  u,
                  v,
                  Moose::SOLUTION_TAG);
  expectVariable(object->kokkosADCoupledVectorTagGradientByName("field", Moose::SOLUTION_TAG),
                 u,
                 Moose::SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledVectorTagGradientsByName("fields", Moose::SOLUTION_TAG),
                  u,
                  v,
                  Moose::SOLUTION_TAG);
  expectVariable(object->kokkosADCoupledVectorTagNodalValueByName("field", Moose::SOLUTION_TAG),
                 u,
                 Moose::SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledVectorTagNodalValuesByName("fields", Moose::SOLUTION_TAG),
                  u,
                  v,
                  Moose::SOLUTION_TAG);
  expectVariable(object->kokkosADCoupledVectorTagDofValueByName("field", Moose::SOLUTION_TAG),
                 u,
                 Moose::SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledVectorTagDofValuesByName("fields", Moose::SOLUTION_TAG),
                  u,
                  v,
                  Moose::SOLUTION_TAG);

  expectVariable(object->kokkosADCoupledVectorTagValue("field", "tag"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(
      object->kokkosADCoupledVectorTagValues("fields", "tag"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(
      object->kokkosADCoupledVectorTagGradient("field", "tag"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(
      object->kokkosADCoupledVectorTagGradients("fields", "tag"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(
      object->kokkosADCoupledVectorTagNodalValue("field", "tag"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(
      object->kokkosADCoupledVectorTagNodalValues("fields", "tag"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(
      object->kokkosADCoupledVectorTagDofValue("field", "tag"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(
      object->kokkosADCoupledVectorTagDofValues("fields", "tag"), u, v, Moose::OLD_SOLUTION_TAG);

  expectVariable(object->kokkosADCoupledValue("field"), u, Moose::SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledValues("fields"), u, v, Moose::SOLUTION_TAG);
  expectVariable(object->kokkosADCoupledGradient("field"), u, Moose::SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledGradients("fields"), u, v, Moose::SOLUTION_TAG);
  expectVariable(object->kokkosADCoupledNodalValue("field"), u, Moose::SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledNodalValues("fields"), u, v, Moose::SOLUTION_TAG);
  expectVariable(object->kokkosADCoupledDofValue("field"), u, Moose::SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledDofValues("fields"), u, v, Moose::SOLUTION_TAG);

  expectVariable(object->kokkosADCoupledValueOld("field"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledValuesOld("fields"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(object->kokkosADCoupledGradientOld("field"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledGradientsOld("fields"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(object->kokkosADCoupledNodalValueOld("field"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledNodalValuesOld("fields"), u, v, Moose::OLD_SOLUTION_TAG);
  expectVariable(object->kokkosADCoupledDofValueOld("field"), u, Moose::OLD_SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledDofValuesOld("fields"), u, v, Moose::OLD_SOLUTION_TAG);

  expectVariable(object->kokkosADCoupledValueOlder("field"), u, Moose::OLDER_SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledValuesOlder("fields"), u, v, Moose::OLDER_SOLUTION_TAG);
  expectVariable(object->kokkosADCoupledGradientOlder("field"), u, Moose::OLDER_SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledGradientsOlder("fields"), u, v, Moose::OLDER_SOLUTION_TAG);
  expectVariable(object->kokkosADCoupledNodalValueOlder("field"), u, Moose::OLDER_SOLUTION_TAG);
  expectVariables(
      object->kokkosADCoupledNodalValuesOlder("fields"), u, v, Moose::OLDER_SOLUTION_TAG);
  expectVariable(object->kokkosADCoupledDofValueOlder("field"), u, Moose::OLDER_SOLUTION_TAG);
  expectVariables(object->kokkosADCoupledDofValuesOlder("fields"), u, v, Moose::OLDER_SOLUTION_TAG);

  expectVariable(object->kokkosADCoupledDot("field"), u, Moose::SOLUTION_DOT_TAG);
  expectVariables(object->kokkosADCoupledDots("fields"), u, v, Moose::SOLUTION_DOT_TAG);
  expectVariable(object->kokkosADCoupledNodalDot("field"), u, Moose::SOLUTION_DOT_TAG);
  expectVariables(object->kokkosADCoupledNodalDots("fields"), u, v, Moose::SOLUTION_DOT_TAG);
}

TEST_F(KokkosCoupleableTest, CoupledValuesOnDevice)
{
  if (!_problem->mesh().getKokkosMesh())
    GTEST_SKIP() << "Kokkos mesh not available";

  auto & nodal = addFieldVariable("nodal");
  auto & elemental = addElementalVariable("elemental");
  auto & vector = addVectorVariable("vector");
  auto & alpha = addScalarVariable("alpha");
  auto & beta = addScalarVariable("beta");
  auto elemental_object =
      createObject("elemental_value_object", {"elemental"}, {}, {}, {}, {"alpha", "beta"});
  auto nodal_object =
      createObject("nodal_value_object", {"nodal"}, {}, {}, {}, {"alpha", "beta"}, true, true);
  auto vector_object = createObject("vector_value_object", {}, {}, {"vector"});
  auto vector_nodal_object =
      createObject("vector_nodal_value_object", {}, {}, {"vector"}, {}, {}, true, true);

  const auto elemental_value = elemental_object->kokkosCoupledValue("field");
  const auto elemental_gradient = elemental_object->kokkosCoupledGradient("field");
  const auto elemental_dof_value = elemental_object->kokkosCoupledDofValue("field");
  const auto nodal_value = nodal_object->kokkosCoupledValue("field");
  const auto nodal_dof_value = nodal_object->kokkosCoupledDofValue("field");
  const auto alpha_value = elemental_object->kokkosCoupledScalarValue("scalar", 0);
  const auto beta_value = nodal_object->kokkosCoupledScalarValue("scalar", 1);
  const auto vector_value = vector_object->kokkosCoupledVectorValue("vector");
  const auto vector_dof_value = vector_object->kokkosCoupledVectorDofValue("vector");
  const auto vector_nodal_value = vector_nodal_object->kokkosCoupledVectorValue("vector");
  const auto elemental_ad_value = elemental_object->kokkosADCoupledValue("field");
  const auto elemental_ad_dof_value = elemental_object->kokkosADCoupledDofValue("field");
  const auto nodal_ad_value = nodal_object->kokkosADCoupledValue("field");
  const auto nodal_ad_dof_value = nodal_object->kokkosADCoupledDofValue("field");

  EXPECT_FALSE(elemental_value.variable().nodal());
  EXPECT_TRUE(nodal_value.variable().nodal());
  EXPECT_THROW(elemental_object->kokkosCoupledNodalValue("field"), MooseRuntimeError);

  initSystems();
  setFieldValue(nodal, 2.5);
  setFieldValue(elemental, -1.75);
  const RealVectorValue expected_vector(3.5, -2.25, 0);
  setVectorFieldValue(vector, expected_vector);
  setScalarValue(alpha, 9.25);
  setScalarValue(beta, -4.5);
  nodal.sys().solution().close();
  nodal.sys().update();

  _problem->initKokkos();

  auto & systems = _problem->getKokkosFESystems();
  auto & system = systems[nodal.sys().number()];
  system.setActiveVariables({&nodal, &elemental, &vector});
  const auto solution_tag = _problem->getVectorTagID(Moose::SOLUTION_TAG);
  system.setActiveSolutionTags({solution_tag});
  system.sync(Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);
  system.reinit();
  systems.copyToDevice();

  Moose::Kokkos::Array<Real> results(26);
  KokkosCoupledValueEvaluator evaluator(_problem->kokkosAssembly(),
                                        systems,
                                        elemental_value,
                                        elemental_gradient,
                                        elemental_dof_value,
                                        nodal_value,
                                        nodal_dof_value,
                                        alpha_value,
                                        beta_value,
                                        vector_value,
                                        vector_dof_value,
                                        vector_nodal_value,
                                        elemental_ad_value,
                                        elemental_ad_dof_value,
                                        nodal_ad_value,
                                        nodal_ad_dof_value,
                                        results);
  Kokkos::parallel_for(1, evaluator);
  Kokkos::fence();

  results.copyToHost();
  EXPECT_DOUBLE_EQ(results(0), -1.75);
  for (const auto i : make_range(1u, 4u))
    EXPECT_NEAR(results(i), 0, TOLERANCE);
  EXPECT_DOUBLE_EQ(results(4), -1.75);
  EXPECT_DOUBLE_EQ(results(5), 2.5);
  EXPECT_DOUBLE_EQ(results(6), 2.5);
  EXPECT_DOUBLE_EQ(results(7), 9.25);
  EXPECT_DOUBLE_EQ(results(8), -4.5);
  for (const auto i : make_range(3))
  {
    EXPECT_DOUBLE_EQ(results(9 + i), expected_vector(i));
    EXPECT_DOUBLE_EQ(results(12 + i), expected_vector(i));
    EXPECT_DOUBLE_EQ(results(15 + i), expected_vector(i));
  }
  EXPECT_DOUBLE_EQ(results(18), -1.75);
  EXPECT_DOUBLE_EQ(results(19), 1);
  EXPECT_DOUBLE_EQ(results(20), -1.75);
  EXPECT_DOUBLE_EQ(results(21), 1);
  EXPECT_DOUBLE_EQ(results(22), 2.5);
  EXPECT_DOUBLE_EQ(results(23), 1);
  EXPECT_DOUBLE_EQ(results(24), 2.5);
  EXPECT_DOUBLE_EQ(results(25), 1);
}

TEST_F(KokkosCoupleableTest, DefaultAPIs)
{
  auto object = createObject("default_object");

  const auto default_value_wrapper = object->kokkosCoupledValues("default_field");
  const auto & default_value = default_value_wrapper.variable();
  EXPECT_FALSE(default_value.coupled());
  EXPECT_EQ(default_value.components(), 2);
  EXPECT_DOUBLE_EQ(default_value.value(0), 1.25);
  EXPECT_DOUBLE_EQ(default_value.value(1), -2.5);

  const auto default_gradient_wrapper = object->kokkosCoupledGradients("default_field");
  const auto & default_gradient = default_gradient_wrapper.variable();
  EXPECT_FALSE(default_gradient.coupled());
  EXPECT_EQ(default_gradient.components(), 2);

  const auto default_vector_wrapper = object->kokkosCoupledVectorValue("default_vector");
  const auto & default_vector = default_vector_wrapper.variable();
  EXPECT_FALSE(default_vector.coupled());
  EXPECT_TRUE(default_vector.vector());
  EXPECT_EQ(default_vector.components(), 1);
  EXPECT_DOUBLE_EQ(default_vector.vectorValue()(0), 1);
  EXPECT_DOUBLE_EQ(default_vector.vectorValue()(1), 2);
  EXPECT_DOUBLE_EQ(default_vector.vectorValue()(2), 3);

  const auto zero_value_wrapper = object->kokkosZeroValue();
  const auto & zero_value = zero_value_wrapper.variable();
  EXPECT_FALSE(zero_value.coupled());
  EXPECT_DOUBLE_EQ(zero_value.value(), 0);
  EXPECT_FALSE(object->kokkosZeroGradient().variable().coupled());
  EXPECT_FALSE(object->kokkosZeroNodalValue().variable().coupled());
  EXPECT_DOUBLE_EQ(+object->kokkosCoupledDotDu("default_field"), 0);

  const auto scalar_wrapper = object->kokkosCoupledScalarValue("default_scalar");
  const auto & scalar = scalar_wrapper.variable();
  EXPECT_FALSE(scalar.coupled());
  EXPECT_TRUE(scalar.scalar());
  EXPECT_EQ(scalar.components(), 1);
  for (const auto i : make_range(static_cast<unsigned int>(_problem->getMaxScalarOrder())))
    EXPECT_DOUBLE_EQ(scalar.value(i), 4.5);
}

TEST_F(KokkosCoupleableTest, ScalarVariableAPIs)
{
  _problem->transient(true);
  auto & alpha = addScalarVariable("alpha");
  auto & beta = addScalarVariable("beta");
  auto object = createObject("scalar_object", {}, {}, {}, {}, {"alpha", "beta"});

  expectVariable(object->kokkosCoupledVectorTagScalarValueByName("scalar", Moose::SOLUTION_TAG, 1),
                 beta,
                 Moose::SOLUTION_TAG);
  expectVariable(
      object->kokkosCoupledVectorTagScalarValue("scalar", "tag", 1), beta, Moose::OLD_SOLUTION_TAG);
  expectVariable(object->kokkosCoupledScalarValue("scalar", 1), beta, Moose::SOLUTION_TAG);
  expectVariable(object->kokkosCoupledScalarValueOld("scalar", 1), beta, Moose::OLD_SOLUTION_TAG);
  expectVariable(
      object->kokkosCoupledScalarValueOlder("scalar", 1), beta, Moose::OLDER_SOLUTION_TAG);
  expectVariable(object->kokkosCoupledScalarDot("scalar", 1), beta, Moose::SOLUTION_DOT_TAG);
  EXPECT_DOUBLE_EQ(+object->kokkosCoupledScalarDotDu("scalar", 1),
                   beta.sys().duDotDu(beta.number()));

  EXPECT_TRUE(object->kokkosCoupledScalarValue("scalar").variable().scalar());
  EXPECT_EQ(object->kokkosCoupledScalarValue("scalar").variable().var(), alpha.number());
}

TEST_F(KokkosCoupleableTest, ValidationErrors)
{
  auto & field = addFieldVariable("field_var");
  addVectorVariable("vector_var");
  addScalarVariable("scalar_var");
  auto object = createObject("error_object", {"field_var"}, {}, {"vector_var"}, {}, {"scalar_var"});

  EXPECT_THROW(object->kokkosCoupledVectorTagValue("field", "invalid_tag_parameter"),
               MooseRuntimeError);
  EXPECT_THROW(object->kokkosCoupledVectorValue("field"), MooseRuntimeError);
  EXPECT_THROW(object->kokkosCoupledValue("vector"), MooseRuntimeError);
  EXPECT_THROW(object->kokkosCoupledScalarValue("field"), MooseRuntimeError);
  EXPECT_THROW(object->kokkosCoupledValue("scalar"), MooseRuntimeError);
  EXPECT_THROW(object->kokkosCoupledScalarValueOld("scalar"), MooseRuntimeError);
  EXPECT_THROW(object->kokkosCoupledScalarValueOlder("scalar"), MooseRuntimeError);
  EXPECT_THROW(object->kokkosCoupledScalarDot("scalar"), MooseRuntimeError);
  EXPECT_THROW(object->kokkosCoupledScalarDotDu("scalar"), MooseRuntimeError);

  _problem->transient(true);
  auto explicit_object =
      createObject("explicit_object", {field.name()}, {}, {}, {}, {"scalar_var"}, false);
  EXPECT_THROW(explicit_object->kokkosCoupledScalarValueOlder("scalar"), MooseRuntimeError);
}
