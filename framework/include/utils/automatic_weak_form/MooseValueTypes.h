#pragma once

#include "libmesh/libmesh_common.h"
#include "libmesh/type_vector.h"
#include "libmesh/type_tensor.h"
#include "RankTwoTensor.h"
#include "RankThreeTensor.h"
#include "RankFourTensor.h"

#include <memory>
#include <variant>
#include <array>

namespace moose
{
namespace automatic_weak_form
{

using Real = libMesh::Real;
using RealVectorValue = libMesh::RealVectorValue;
using RealTensorValue = libMesh::RealTensorValue;

template <unsigned int DIM>
struct Dimension
{
  static constexpr unsigned int value = DIM;
};

using Dim1D = Dimension<1>;
using Dim2D = Dimension<2>;
using Dim3D = Dimension<3>;

struct ScalarShape {};
struct VectorShape { unsigned int dim; };
struct TensorShape { unsigned int dim; };
struct RankThreeShape { unsigned int dim; };
struct RankFourShape { unsigned int dim1, dim2; };

using Shape = std::variant<ScalarShape, VectorShape, TensorShape, RankThreeShape, RankFourShape>;

struct MooseValue
{
  Shape shape;
  
  std::variant<
    Real,
    RealVectorValue,
    RankTwoTensor,
    RankThreeTensor,
    RankFourTensor
  > data;
  
  MooseValue() : shape(ScalarShape{}), data(Real(0)) {}
  
  explicit MooseValue(Real val) : shape(ScalarShape{}), data(val) {}
  explicit MooseValue(const RealVectorValue & val, unsigned int dim) 
    : shape(VectorShape{dim}), data(val) {}
  explicit MooseValue(const RankTwoTensor & val, unsigned int dim) 
    : shape(TensorShape{dim}), data(val) {}
  explicit MooseValue(const RankThreeTensor & val, unsigned int dim) 
    : shape(RankThreeShape{dim}), data(val) {}
  explicit MooseValue(const RankFourTensor & val, unsigned int dim1, unsigned int dim2) 
    : shape(RankFourShape{dim1, dim2}), data(val) {}
  
  Real asScalar() const 
  { 
    return std::get<Real>(data); 
  }
  
  const RealVectorValue & asVector() const 
  { 
    return std::get<RealVectorValue>(data); 
  }
  
  const RankTwoTensor & asTensor() const 
  { 
    return std::get<RankTwoTensor>(data); 
  }
  
  const RankThreeTensor & asRankThree() const 
  { 
    return std::get<RankThreeTensor>(data); 
  }
  
  const RankFourTensor & asRankFour() const 
  { 
    return std::get<RankFourTensor>(data); 
  }
  
  bool isScalar() const { return std::holds_alternative<ScalarShape>(shape); }
  bool isVector() const { return std::holds_alternative<VectorShape>(shape); }
  bool isTensor() const { return std::holds_alternative<TensorShape>(shape); }
  bool isRankThree() const { return std::holds_alternative<RankThreeShape>(shape); }
  bool isRankFour() const { return std::holds_alternative<RankFourShape>(shape); }
  
  unsigned int getDimension() const
  {
    if (auto * v = std::get_if<VectorShape>(&shape))
      return v->dim;
    if (auto * t = std::get_if<TensorShape>(&shape))
      return t->dim;
    if (auto * r3 = std::get_if<RankThreeShape>(&shape))
      return r3->dim;
    if (auto * r4 = std::get_if<RankFourShape>(&shape))
      return r4->dim1;
    return 0;
  }
};

inline MooseValue operator+(const MooseValue & a, const MooseValue & b)
{
  if (a.isScalar() && b.isScalar())
    return MooseValue(a.asScalar() + b.asScalar());
  if (a.isVector() && b.isVector())
    return MooseValue(a.asVector() + b.asVector(), a.getDimension());
  if (a.isTensor() && b.isTensor())
  {
    RankTwoTensor result = a.asTensor();
    result += b.asTensor();
    return MooseValue(result, a.getDimension());
  }
  mooseError("Type mismatch in addition");
}

inline MooseValue operator-(const MooseValue & a, const MooseValue & b)
{
  if (a.isScalar() && b.isScalar())
    return MooseValue(a.asScalar() - b.asScalar());
  if (a.isVector() && b.isVector())
    return MooseValue(a.asVector() - b.asVector(), a.getDimension());
  if (a.isTensor() && b.isTensor())
  {
    RankTwoTensor result = a.asTensor();
    result -= b.asTensor();
    return MooseValue(result, a.getDimension());
  }
  mooseError("Type mismatch in subtraction");
}

inline MooseValue operator*(const MooseValue & a, const MooseValue & b)
{
  if (a.isScalar() && b.isScalar())
    return MooseValue(a.asScalar() * b.asScalar());
  
  if (a.isScalar() && b.isVector())
    return MooseValue(a.asScalar() * b.asVector(), b.getDimension());
  
  if (a.isVector() && b.isScalar())
    return MooseValue(b.asScalar() * a.asVector(), a.getDimension());
  
  if (a.isScalar() && b.isTensor())
  {
    RankTwoTensor result = b.asTensor();
    result *= a.asScalar();
    return MooseValue(result, b.getDimension());
  }
  
  if (a.isTensor() && b.isScalar())
  {
    RankTwoTensor result = a.asTensor();
    result *= b.asScalar();
    return MooseValue(result, a.getDimension());
  }
  
  if (a.isTensor() && b.isTensor())
    return MooseValue(a.asTensor() * b.asTensor(), a.getDimension());
  
  if (a.isVector() && b.isVector())
    return MooseValue(a.asVector() * b.asVector());
  
  mooseError("Unsupported multiplication types");
}

inline MooseValue dot(const MooseValue & a, const MooseValue & b)
{
  if (a.isVector() && b.isVector())
    return MooseValue(a.asVector() * b.asVector());
  
  if (a.isTensor() && b.isVector())
    return MooseValue(a.asTensor() * b.asVector(), a.getDimension());
  
  if (a.isVector() && b.isTensor())
    return MooseValue(b.asTensor().transpose() * a.asVector(), a.getDimension());
  
  mooseError("dot product requires vectors or tensor-vector");
}

inline MooseValue contract(const MooseValue & a, const MooseValue & b)
{
  if (a.isTensor() && b.isTensor())
    return MooseValue(a.asTensor().doubleContraction(b.asTensor()));
  
  mooseError("contract requires two tensors");
}

inline MooseValue trace(const MooseValue & a)
{
  if (a.isTensor())
    return MooseValue(a.asTensor().trace());
  
  mooseError("trace requires a tensor");
}

inline MooseValue det(const MooseValue & a)
{
  if (a.isTensor())
    return MooseValue(a.asTensor().det());
  
  mooseError("determinant requires a tensor");
}

inline MooseValue transpose(const MooseValue & a)
{
  if (a.isTensor())
    return MooseValue(a.asTensor().transpose(), a.getDimension());
  
  mooseError("transpose requires a tensor");
}

inline MooseValue inverse(const MooseValue & a)
{
  if (a.isTensor())
    return MooseValue(a.asTensor().inverse(), a.getDimension());
  
  mooseError("inverse requires a tensor");
}

inline MooseValue norm(const MooseValue & a)
{
  if (a.isVector())
    return MooseValue(a.asVector().norm());
  
  if (a.isTensor())
    return MooseValue(a.asTensor().L2norm());
  
  mooseError("norm requires vector or tensor");
}

inline MooseValue outer(const MooseValue & a, const MooseValue & b)
{
  if (a.isVector() && b.isVector())
  {
    RankTwoTensor result;
    const auto & va = a.asVector();
    const auto & vb = b.asVector();
    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
        result(i, j) = va(i) * vb(j);
    return MooseValue(result, a.getDimension());
  }
  
  mooseError("outer product requires two vectors");
}

inline MooseValue sym(const MooseValue & a)
{
  if (a.isTensor())
  {
    RankTwoTensor result = a.asTensor();
    result = (result + result.transpose()) * 0.5;
    return MooseValue(result, a.getDimension());
  }
  
  mooseError("sym requires a tensor");
}

inline MooseValue skew(const MooseValue & a)
{
  if (a.isTensor())
  {
    RankTwoTensor result = a.asTensor();
    result = (result - result.transpose()) * 0.5;
    return MooseValue(result, a.getDimension());
  }
  
  mooseError("skew requires a tensor");
}

inline MooseValue dev(const MooseValue & a)
{
  if (a.isTensor())
    return MooseValue(a.asTensor().deviatoric(), a.getDimension());
  
  mooseError("dev requires a tensor");
}

struct GradientResult
{
  Shape shape;
  unsigned int spatial_dim;
  
  bool isVectorGradient() const 
  { 
    return std::holds_alternative<VectorShape>(shape); 
  }
  
  bool isTensorGradient() const 
  { 
    return std::holds_alternative<TensorShape>(shape); 
  }
};

inline GradientResult gradientShape(const Shape & input_shape, unsigned int spatial_dim)
{
  if (std::holds_alternative<ScalarShape>(input_shape))
    return {VectorShape{spatial_dim}, spatial_dim};
  
  if (std::holds_alternative<VectorShape>(input_shape))
    return {TensorShape{spatial_dim}, spatial_dim};
  
  if (std::holds_alternative<TensorShape>(input_shape))
    return {RankThreeShape{spatial_dim}, spatial_dim};
  
  mooseError("Cannot take gradient of rank > 2");
}

inline Shape divergenceShape(const Shape & input_shape)
{
  if (std::holds_alternative<VectorShape>(input_shape))
    return ScalarShape{};
  
  if (std::holds_alternative<TensorShape>(input_shape))
  {
    auto & ts = std::get<TensorShape>(input_shape);
    return VectorShape{ts.dim};
  }
  
  if (std::holds_alternative<RankThreeShape>(input_shape))
  {
    auto & r3s = std::get<RankThreeShape>(input_shape);
    return TensorShape{r3s.dim};
  }
  
  mooseError("Cannot take divergence of scalar or rank > 3");
}

}
}