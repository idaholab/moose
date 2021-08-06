//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedFunctionTest.h"
#include "MathFVUtils.h"

#include "libmesh/fe_map.h"
#include "libmesh/quadrature_gauss.h"

TEST_F(ParsedFunctionTest, basicConstructor)
{
  InputParameters params = _factory->getValidParams("ParsedFunction");
  // test constructor with no additional variables
  params.set<FEProblem *>("_fe_problem") = _fe_problem.get();
  params.set<FEProblemBase *>("_fe_problem_base") = _fe_problem.get();
  params.set<SubProblem *>("_subproblem") = _fe_problem.get();
  params.set<std::string>("value") = std::string("x + 1.5*y + 2 * z + t/4");
  params.set<std::string>("_object_name") = "test";
  params.set<std::string>("_type") = "MooseParsedFunction";
  MooseParsedFunction f(params);
  Moose::Functor<Real> f_wrapped(f);
  f.initialSetup();
  EXPECT_EQ(f.value(4, Point(1, 2, 3)), 11);

  //
  // Test the functor interfaces
  //

  const auto & lm_mesh = _mesh->getMesh();

  Real f_traditional(0);
  Real f_functor(0);
  RealVectorValue gradient_traditional;
  RealVectorValue gradient_functor;
  Real dot_traditional(0);
  Real dot_functor(0);

  auto test_eq = [&f_traditional,
                  &f_functor,
                  &gradient_traditional,
                  &gradient_functor,
                  &dot_traditional,
                  &dot_functor]()
  {
    EXPECT_EQ(f_traditional, f_functor);
    for (const auto i : make_range(unsigned(LIBMESH_DIM)))
      EXPECT_EQ(gradient_traditional(i), gradient_functor(i));
    EXPECT_EQ(dot_traditional, dot_functor);
  };

  // Test elem overloads
  const Elem * const elem = lm_mesh.elem_ptr(0);
  const auto elem_arg = Moose::ElemArg{elem, false, false};
  const Point vtx_average = elem->vertex_average();
  f_traditional = f.value(0, vtx_average);
  f_functor = f_wrapped(elem_arg, 0);
  gradient_traditional = f.gradient(0, vtx_average);
  gradient_functor = f_wrapped.gradient(elem_arg, 0);
  dot_traditional = f.timeDerivative(0, vtx_average);
  dot_functor = f_wrapped.dot(elem_arg, 0);
  test_eq();

  const Elem * neighbor = nullptr;
  unsigned int side = libMesh::invalid_uint;
  for (const auto s : elem->side_index_range())
    if (elem->neighbor_ptr(s))
    {
      neighbor = elem->neighbor_ptr(s);
      side = s;
      break;
    }

  // Test elem_from_face overloads
  const FaceInfo * const fi = _mesh->faceInfo(elem, side);
  const auto elem_from_face = Moose::ElemFromFaceArg{elem, fi, false, false, elem->subdomain_id()};
  f_functor = f_wrapped(elem_from_face, 0);
  gradient_functor = f_wrapped.gradient(elem_from_face, 0);
  dot_functor = f_wrapped.dot(elem_from_face, 0);
  test_eq();

  // Test face overloads
  auto face =
      Moose::FV::makeCDFace(*fi, std::make_pair(elem->subdomain_id(), neighbor->subdomain_id()));
  f_traditional = f.value(0, fi->faceCentroid());
  f_functor = f_wrapped(face, 0);
  gradient_traditional = f.gradient(0, fi->faceCentroid());
  gradient_functor = f_wrapped.gradient(face, 0);
  dot_traditional = f.timeDerivative(0, fi->faceCentroid());
  dot_functor = f_wrapped.dot(face, 0);
  test_eq();

  // Test ElemQp overloads
  const FEFamily mapping_family = FEMap::map_fe_type(*elem);
  const FEType fe_type(elem->default_order(), mapping_family);
  std::unique_ptr<FEBase> fe(FEBase::build(elem->dim(), fe_type));
  const auto & xyz = fe->get_xyz();
  QGauss qrule(elem->dim(), fe_type.default_quadrature_order());
  fe->attach_quadrature_rule(&qrule);
  fe->reinit(elem);
  auto elem_qp = std::make_tuple(elem, 0, &qrule);
  f_traditional = f.value(0, xyz[0]);
  f_functor = f_wrapped(elem_qp, 0);
  gradient_traditional = f.gradient(0, xyz[0]);
  gradient_functor = f_wrapped.gradient(elem_qp, 0);
  dot_traditional = f.timeDerivative(0, xyz[0]);
  dot_functor = f_wrapped.dot(elem_qp, 0);
  test_eq();

  // Test ElemSideQp overloads
  QGauss qrule_face(elem->dim() - 1, fe_type.default_quadrature_order());
  fe->attach_quadrature_rule(&qrule_face);
  fe->reinit(elem, side);
  auto elem_side_qp = std::make_tuple(elem, side, 0, &qrule_face);
  f_traditional = f.value(0, xyz[0]);
  f_functor = f_wrapped(elem_side_qp, 0);
  gradient_traditional = f.gradient(0, xyz[0]);
  gradient_functor = f_wrapped.gradient(elem_side_qp, 0);
  dot_traditional = f.timeDerivative(0, xyz[0]);
  dot_functor = f_wrapped.dot(elem_side_qp, 0);
  test_eq();
}

TEST_F(ParsedFunctionTest, advancedConstructor)
{
  // test the constructor with one variable
  std::vector<std::string> one_var(1);
  one_var[0] = "q";

  InputParameters params = _factory->getValidParams("ParsedFunction");

  params.set<FEProblem *>("_fe_problem") = _fe_problem.get();
  params.set<FEProblemBase *>("_fe_problem_base") = _fe_problem.get();
  params.set<SubProblem *>("_subproblem") = _fe_problem.get();
  params.set<std::string>("value") = "x + y + q";
  params.set<std::vector<std::string>>("vars") = one_var;
  params.set<std::vector<std::string>>("vals") =
      std::vector<std::string>(1, "-1"); // Dummy value, will be overwritten in test below
  params.set<std::string>("_object_name") = "test1";
  params.set<std::string>("_type") = "MooseParsedFunction";

  MooseParsedFunction f(params);
  f.initialSetup();
  // Access address via pointer to MooseParsedFunctionWrapper that contains pointer to
  // libMesh::ParsedFunction
  fptr(f)->getVarAddress("q") = 4;
  EXPECT_EQ(f.value(0, Point(1, 2)), 7);

  // test the constructor with three variables
  std::vector<std::string> three_vars(3);
  three_vars[0] = "q";
  three_vars[1] = "w";
  three_vars[2] = "r";

  InputParameters params2 = _factory->getValidParams("ParsedFunction");
  params2.set<FEProblem *>("_fe_problem") = _fe_problem.get();
  params2.set<FEProblemBase *>("_fe_problem_base") = _fe_problem.get();
  params2.set<SubProblem *>("_subproblem") = _fe_problem.get();
  params2.set<std::string>("value") = "r*x + y/w + q";
  params2.set<std::vector<std::string>>("vars") = three_vars;
  params2.set<std::vector<std::string>>("vals") =
      std::vector<std::string>(3, "-1"); // Dummy values, will be overwritten in test below
  params2.set<std::string>("_object_name") = "test2";
  params2.set<std::string>("_type") = "MooseParsedFunction";

  MooseParsedFunction f2(params2);
  f2.initialSetup();
  fptr(f2)->getVarAddress("q") = 4;
  fptr(f2)->getVarAddress("w") = 2;
  fptr(f2)->getVarAddress("r") = 1.5;
  EXPECT_EQ(f2.value(0, Point(2, 4)), 9);

  // test the constructor with one variable that's set
  std::vector<std::string> one_val(1);
  one_val[0] = "2.5";

  InputParameters params3 = _factory->getValidParams("ParsedFunction");
  params3.set<FEProblem *>("_fe_problem") = _fe_problem.get();
  params3.set<FEProblemBase *>("_fe_problem_base") = _fe_problem.get();
  params3.set<SubProblem *>("_subproblem") = _fe_problem.get();
  params3.set<std::string>("value") = "q*x";
  params3.set<std::vector<std::string>>("vars") = one_var;
  params3.set<std::vector<std::string>>("vals") = one_val;
  params3.set<std::string>("_object_name") = "test3";
  params3.set<std::string>("_type") = "MooseParsedFunction";

  MooseParsedFunction f3(params3);
  f3.initialSetup();
  EXPECT_EQ(f3.value(0, 2), 5);

  // test the constructor with three variables
  std::vector<std::string> three_vals(3);
  three_vals[0] = "1.5";
  three_vals[1] = "1";
  three_vals[2] = "0";

  InputParameters params4 = _factory->getValidParams("ParsedFunction");
  params4.set<FEProblem *>("_fe_problem") = _fe_problem.get();
  params4.set<FEProblemBase *>("_fe_problem_base") = _fe_problem.get();
  params4.set<SubProblem *>("_subproblem") = _fe_problem.get();
  params4.set<std::string>("value") = "q*x + y/r + w";
  params4.set<std::vector<std::string>>("vars") = three_vars;
  params4.set<std::vector<std::string>>("vals") = three_vals;
  params4.set<std::string>("_object_name") = "test4";
  params4.set<std::string>("_type") = "MooseParsedFunction";

  MooseParsedFunction f4(params4);
  f4.initialSetup();
  fptr(f4)->getVarAddress("r") = 2;
  EXPECT_EQ(f4.value(0, Point(2, 4)), 6);
  fptr(f4)->getVarAddress("r") = 4;
  EXPECT_EQ(f4.value(0, Point(2, 4)), 5);
}

TEST_F(ParsedFunctionTest, testVariables)
{
  // a lot of this functionality is tested in advancedConstructor as well
  // test one variable, make sure we can change it by the reference any time
  std::vector<std::string> one_var(1);
  one_var[0] = "q";

  InputParameters params = _factory->getValidParams("ParsedFunction");
  params.set<FEProblem *>("_fe_problem") = _fe_problem.get();
  params.set<FEProblemBase *>("_fe_problem_base") = _fe_problem.get();
  params.set<SubProblem *>("_subproblem") = _fe_problem.get();
  params.set<std::string>("value") = "x + y + q";
  params.set<std::vector<std::string>>("vars") = one_var;
  params.set<std::vector<std::string>>("vals") =
      std::vector<std::string>(1, "-1"); // Dummy value, will be overwritten in test below
  params.set<std::string>("_object_name") = "test1";
  params.set<std::string>("_type") = "MooseParsedFunction";

  MooseParsedFunction f(params);
  f.initialSetup();
  Real & q = fptr(f)->getVarAddress("q");
  q = 4;
  EXPECT_EQ(f.value(0, Point(1, 2)), 7);
  q = 2;
  EXPECT_EQ(f.value(0, Point(1, 2)), 5);
  q = -4;
  EXPECT_EQ(f.value(0, Point(1, 2)), -1);

  // test three variables, test updating them randomly
  std::vector<std::string> three_vars(3);
  three_vars[0] = "q";
  three_vars[1] = "w";
  three_vars[2] = "r";

  InputParameters params2 = _factory->getValidParams("ParsedFunction");
  params2.set<FEProblem *>("_fe_problem") = _fe_problem.get();
  params2.set<FEProblemBase *>("_fe_problem_base") = _fe_problem.get();
  params2.set<SubProblem *>("_subproblem") = _fe_problem.get();
  params2.set<std::string>("value") = "r*x + y/w + q";
  params2.set<std::vector<std::string>>("vars") = three_vars;
  params2.set<std::vector<std::string>>("vals") =
      std::vector<std::string>(3, "-1"); // Dummy values, will be overwritten in test below
  params2.set<std::string>("_object_name") = "test2";
  params2.set<std::string>("_type") = "MooseParsedFunction";

  MooseParsedFunction f2(params2);
  f2.initialSetup();
  Real & q2 = fptr(f2)->getVarAddress("q");
  Real & w2 = fptr(f2)->getVarAddress("w");
  Real & r2 = fptr(f2)->getVarAddress("r");
  q2 = 4;
  w2 = 2;
  r2 = 1.5;
  EXPECT_EQ(f2.value(0, Point(2, 4)), 9);
  q2 = 1;
  w2 = 4;
  r2 = 2.5;
  EXPECT_EQ(f2.value(0, Point(2, 4)), 7);
  q2 = 2;
  EXPECT_EQ(f2.value(0, Point(2, 4)), 8);
  w2 = 3;
  EXPECT_EQ(f2.value(0, Point(2, 6)), 9);
}

TEST_F(ParsedFunctionTest, testConstants)
{
  // this functions tests that pi and e get correctly substituted
  // it also tests built in functions of the function parser
  InputParameters params = _factory->getValidParams("ParsedFunction");
  params.set<std::string>("_object_name") = "test1";
  params.set<FEProblem *>("_fe_problem") = _fe_problem.get();
  params.set<FEProblemBase *>("_fe_problem_base") = _fe_problem.get();
  params.set<SubProblem *>("_subproblem") = _fe_problem.get();
  params.set<std::string>("value") = "log(e) + x";
  params.set<std::string>("_type") = "MooseParsedFunction";

  MooseParsedFunction f(params);
  f.initialSetup();
  EXPECT_NEAR(2, f.value(0, 1), 0.0000001);

  InputParameters params2 = _factory->getValidParams("ParsedFunction");
  params2.set<std::string>("_object_name") = "test2";
  params2.set<FEProblem *>("_fe_problem") = _fe_problem.get();
  params2.set<FEProblemBase *>("_fe_problem_base") = _fe_problem.get();
  params2.set<SubProblem *>("_subproblem") = _fe_problem.get();
  params2.set<std::string>("value") = "sin(pi*x)";
  params2.set<std::string>("_type") = "MooseParsedFunction";

  MooseParsedFunction f2(params2);
  f2.initialSetup();
  EXPECT_NEAR(0, f2.value(0, 1), 0.0000001);
  EXPECT_NEAR(1, f2.value(0, 0.5), 0.0000001);
  EXPECT_NEAR(-1, f2.value(0, 1.5), 0.0000001);
}
