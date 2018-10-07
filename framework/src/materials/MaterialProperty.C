template <>
void
MaterialProperty<Real>::scale(Real a)
{
  auto nqp = _value.size();
  for (decltype(nqp) qp = 0; qp < nqp; ++qp)
    _value[qp] *= a;
}

template <>
void
MaterialProperty<Real>::add(const PropertyValue * x, Real a)
{
  mooseAssert(x != NULL, "Adding NULL?");
  auto nqp = _value.size();
  for (decltype(nqp) qp = 0; qp < nqp; ++qp)
    _value[qp] += cast_ptr<const MaterialProperty<Real> *>(x)->_value[qp] * a;
}

template <>
void
MaterialProperty<std::vector<Real>>::scale(Real a)
{
  auto nqp = _value.size();
  for (decltype(nqp) qp = 0; qp < nqp; ++qp)
    for (auto & v : _value[qp])
      v *= a;
}

template <>
void
MaterialProperty<std::vector<Real>>::add(const PropertyValue * x, Real a)
{
  mooseAssert(x != NULL, "Adding NULL?");
  auto nqp = _value.size();
  for (decltype(nqp) qp = 0; qp < nqp; ++qp)
  {
    auto nv = _value[qp].size();
    for (decltype(nv) i = 0; i < nv; ++i)
      _value[qp][i] += cast_ptr<const MaterialProperty<std::vector<Real>> *>(x)->_value[qp][i] * a;
  }
}
