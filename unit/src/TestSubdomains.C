#include "coefficients.h"
#include "mfem.hpp"
#include "gtest/gtest.h"

double
scalar_f(const mfem::Vector & x)
{
  return x.Sum();
}

void
vector_f(const mfem::Vector & x, mfem::Vector & f)
{
  for (int i = 0; i < x.Size(); ++i)
  {
    f[i] = sin(x[i]);
  }
}

TEST(CheckSetup, PWCoefficient)
{
  mfem::Vector one_vec(3);
  one_vec = 1.0;

  platypus::Subdomain air("air", 1);
  air._scalar_coefficients.Register("electrical_conductivity",
                                    std::make_shared<mfem::ConstantCoefficient>(1e-4));
  air._scalar_coefficients.Register("magnetic_permeability",
                                    std::make_shared<mfem::ConstantCoefficient>(M_PI * 4.0e-7));
  air._vector_coefficients.Register("vector_function",
                                    std::make_shared<mfem::VectorConstantCoefficient>(one_vec));

  platypus::Subdomain plate("plate", 2);
  plate._scalar_coefficients.Register("electrical_conductivity",
                                      std::make_shared<mfem::FunctionCoefficient>(scalar_f));
  plate._scalar_coefficients.Register("magnetic_permeability",
                                      std::make_shared<mfem::ConstantCoefficient>(M_PI * 4.0e-7));
  plate._vector_coefficients.Register(
      "vector_function", std::make_shared<mfem::VectorFunctionCoefficient>(3, vector_f));

  platypus::Subdomain coil1("coil1", 3);
  coil1._scalar_coefficients.Register("electrical_conductivity",
                                      std::make_shared<mfem::ConstantCoefficient>(1e4));
  coil1._scalar_coefficients.Register("magnetic_permeability",
                                      std::make_shared<mfem::ConstantCoefficient>(M_PI * 4.0e-7));
  coil1._vector_coefficients.Register(
      "vector_function", std::make_shared<mfem::VectorFunctionCoefficient>(3, vector_f));

  platypus::Subdomain coil2("coil2", 4);
  coil2._scalar_coefficients.Register("electrical_conductivity",
                                      std::make_shared<mfem::ConstantCoefficient>(1e4));
  coil2._scalar_coefficients.Register("magnetic_permeability",
                                      std::make_shared<mfem::ConstantCoefficient>(M_PI * 4.0e-7));
  coil2._vector_coefficients.Register(
      "vector_function", std::make_shared<mfem::VectorFunctionCoefficient>(3, vector_f));

  platypus::Subdomain coil3("coil3", 5);
  coil3._scalar_coefficients.Register("electrical_conductivity",
                                      std::make_shared<mfem::ConstantCoefficient>(1e4));
  coil3._scalar_coefficients.Register("magnetic_permeability",
                                      std::make_shared<mfem::ConstantCoefficient>(M_PI * 4.0e-7));
  coil3._vector_coefficients.Register(
      "vector_function", std::make_shared<mfem::VectorFunctionCoefficient>(3, vector_f));

  platypus::Subdomain coil4("coil4", 6);
  coil4._scalar_coefficients.Register("electrical_conductivity",
                                      std::make_shared<mfem::ConstantCoefficient>(1e4));
  coil4._scalar_coefficients.Register("magnetic_permeability",
                                      std::make_shared<mfem::ConstantCoefficient>(M_PI * 4.0e-7));
  coil4._vector_coefficients.Register(
      "vector_function", std::make_shared<mfem::VectorFunctionCoefficient>(3, vector_f));

  platypus::Coefficients coefficients(
      std::vector<platypus::Subdomain>({air, plate, coil1, coil2, coil3, coil4}));

  coefficients._scalars.Register("I", std::make_shared<mfem::ConstantCoefficient>(1.0));
  coefficients._vectors.Register("const_vector",
                                 std::make_shared<mfem::VectorConstantCoefficient>(one_vec));

  EXPECT_EQ(coefficients._subdomains.size(), 6);

  EXPECT_TRUE(coefficients._scalars.Has("electrical_conductivity"));
  EXPECT_TRUE(coefficients._scalars.Has("magnetic_permeability"));
  EXPECT_TRUE(coefficients._scalars.Has("I"));

  EXPECT_TRUE(coefficients._vectors.Has("vector_function"));
  EXPECT_TRUE(coefficients._vectors.Has("const_vector"));
}

TEST(CheckData, Coefficients)
{

  platypus::Subdomain wire("wire", 1);
  wire._scalar_coefficients.Register("property_one",
                                     std::make_shared<mfem::ConstantCoefficient>(1.0));
  wire._scalar_coefficients.Register("property_two",
                                     std::make_shared<mfem::ConstantCoefficient>(150.0));

  platypus::Subdomain air("air", 2);
  air._scalar_coefficients.Register("property_one",
                                    std::make_shared<mfem::ConstantCoefficient>(26.0));
  air._scalar_coefficients.Register("property_two",
                                    std::make_shared<mfem::ConstantCoefficient>(152.0));

  platypus::Coefficients coefficients(std::vector<platypus::Subdomain>({wire, air}));

  // Verify predefined values
  mfem::IsoparametricTransformation t;
  mfem::IntegrationPoint ip;

  mfem::Coefficient * pw = coefficients._scalars.Get("property_one");
  t.Attribute = 1;
  EXPECT_DOUBLE_EQ(pw->Eval(t, ip), 1.0);
  t.Attribute = 2;
  EXPECT_DOUBLE_EQ(pw->Eval(t, ip), 26.0);

  pw = coefficients._scalars.Get("property_two");
  t.Attribute = 1;
  EXPECT_DOUBLE_EQ(pw->Eval(t, ip), 150.0);
  t.Attribute = 2;
  EXPECT_DOUBLE_EQ(pw->Eval(t, ip), 152.0);
}