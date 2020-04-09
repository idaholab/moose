#include "NumberBase.h"

NumberBase *
NumberBase::getNumber(Real real_num)
{
  return new RealNumber(real_num);
}

NumberBase *
NumberBase::getNumber(int int_num)
{
  return new IntNumber(int_num);
}

NumberBase *
NumberBase::getNumber(bool bool_num)
{
  return new IntNumber(bool_num);
}

NumberBase *
NumberBase::getNumber(Fraction frac_num)
{
  return new FractionNumber(frac_num);
}

NumberBase *
NumberBase::getNumber(ExpressionBuilderToo::EBTermNode * node)
{
  ExpressionBuilderToo::EBNumberNode<Real> * real_node =
      dynamic_cast<ExpressionBuilderToo::EBNumberNode<Real> *>(node);
  ExpressionBuilderToo::EBNumberNode<int> * int_node =
      dynamic_cast<ExpressionBuilderToo::EBNumberNode<int> *>(node);
  ExpressionBuilderToo::EBNumberNode<bool> * bool_node =
      dynamic_cast<ExpressionBuilderToo::EBNumberNode<bool> *>(node);
  if (real_node != NULL)
    return getNumber(real_node->getTValue());
  else if (int_node != NULL)
    return getNumber(int_node->getTValue());
  else if (bool_node != NULL)
    return getNumber(bool_node->getTValue());
  else
    mooseError("Unknown number node type");
}

NumberBase *
NumberBase::getNumber(RealNumber & real_num)
{
  return new RealNumber(real_num.getRealNumber());
}

NumberBase *
NumberBase::getNumber(IntNumber & int_num)
{
  return new IntNumber(int_num.getRealNumber());
}

NumberBase &
NumberBase::operator+(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs + *this;
  Real self_num = getRealNumber();
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(self_num + rhs_num);
  return *new_num;
}

NumberBase &
NumberBase::operator-(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return (-rhs) + *this;
  Real self_num = getRealNumber();
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(self_num - rhs_num);
  return *new_num;
}

NumberBase & NumberBase::operator*(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs * *this;
  Real self_num = getRealNumber();
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(self_num * rhs_num);
  return *new_num;
}

NumberBase &
NumberBase::operator/(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return (*getNumber(1.0) / rhs) * *this;
  Real self_num = getRealNumber();
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(self_num / rhs_num);
  return *new_num;
}

NumberBase & NumberBase::operator%(NumberBase & rhs) // Note: Need some work
{
  Real self_num = getRealNumber();
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(fmod(self_num, rhs_num));
  return *new_num;
}

NumberBase &
NumberBase::operator<(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs >= *this;
  Real self_num = getRealNumber();
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(self_num < rhs_num);
  return *new_num;
}

NumberBase &
NumberBase::operator>(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs <= *this;
  Real self_num = getRealNumber();
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(self_num > rhs_num);
  return *new_num;
}

NumberBase &
NumberBase::operator<=(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs > *this;
  Real self_num = getRealNumber();
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(self_num <= rhs_num);
  return *new_num;
}

NumberBase &
NumberBase::operator>=(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs < *this;
  Real self_num = getRealNumber();
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(self_num >= rhs_num);
  return *new_num;
}

NumberBase &
NumberBase::operator==(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs == *this;
  Real self_num = getRealNumber();
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(self_num == rhs_num);
  return *new_num;
}

NumberBase &
NumberBase::operator!=(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs != *this;
  Real self_num = getRealNumber();
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(self_num != rhs_num);
  return *new_num;
}

NumberBase &
NumberBase::operator-()
{
  Real self_num = getRealNumber();
  NumberBase * new_num = getNumber(-self_num);
  return *new_num;
}

NumberBase & NumberBase::operator!()
{
  Real self_num = getRealNumber();
  NumberBase * new_num = getNumber(!self_num);
  return *new_num;
}

NumberBase &
NumberBase::sin(NumberBase & rhs)
{
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(std::sin(rhs_num));
  return *new_num;
}

NumberBase &
NumberBase::cos(NumberBase & rhs)
{
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(std::cos(rhs_num));
  return *new_num;
}

NumberBase &
NumberBase::tan(NumberBase & rhs)
{
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(std::tan(rhs_num));
  return *new_num;
}

NumberBase &
NumberBase::abs(NumberBase & rhs)
{
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(std::abs(rhs_num));
  return *new_num;
}

NumberBase &
NumberBase::log(NumberBase & rhs)
{
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(std::log(rhs_num));
  return *new_num;
}

NumberBase &
NumberBase::log2(NumberBase & rhs)
{
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(std::log2(rhs_num));
  return *new_num;
}

NumberBase &
NumberBase::log10(NumberBase & rhs)
{
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(std::log10(rhs_num));
  return *new_num;
}

NumberBase &
NumberBase::exp(NumberBase & rhs)
{
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(std::exp(rhs_num));
  return *new_num;
}

NumberBase &
NumberBase::sinh(NumberBase & rhs)
{
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(std::sinh(rhs_num));
  return *new_num;
}

NumberBase &
NumberBase::cosh(NumberBase & rhs)
{
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(std::cosh(rhs_num));
  return *new_num;
}

NumberBase &
NumberBase::sqrt(NumberBase & rhs)
{
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(std::sqrt(rhs_num));
  return *new_num;
}

NumberBase &
NumberBase::min(NumberBase & lhs, NumberBase & rhs)
{
  Real rhs_num = rhs.getRealNumber();
  Real lhs_num = lhs.getRealNumber();
  if (rhs_num < lhs_num)
    return rhs;
  return lhs;
}

NumberBase &
NumberBase::max(NumberBase & lhs, NumberBase & rhs)
{
  Real rhs_num = rhs.getRealNumber();
  Real lhs_num = lhs.getRealNumber();
  if (rhs_num > lhs_num)
    return rhs;
  return lhs;
}

NumberBase &
NumberBase::pow(NumberBase & lhs, NumberBase & rhs)
{
  Real lhs_num = lhs.getRealNumber();
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(std::pow(lhs_num, rhs_num));
  return *new_num;
}

bool
RealNumber::conditional()
{
  if (getRealNumber())
    return true;
  return false;
}

NumberBase &
IntNumber::operator+(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs + *this;
  int self_num = getIntNumber();
  int rhs_num = rhs.getIntNumber();
  NumberBase * new_num = getNumber(self_num + rhs_num);
  return *new_num;
}

NumberBase &
IntNumber::operator-(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return (-rhs) + *this;
  int self_num = getIntNumber();
  int rhs_num = rhs.getIntNumber();
  NumberBase * new_num = getNumber(self_num - rhs_num);
  return *new_num;
}

NumberBase & IntNumber::operator*(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs * *this;
  int self_num = getIntNumber();
  int rhs_num = rhs.getIntNumber();
  NumberBase * new_num = getNumber(self_num * rhs_num);
  return *new_num;
}

NumberBase &
IntNumber::operator/(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return (*getNumber(Fraction(1)) / rhs) * *this;
  int self_num = getIntNumber();
  int rhs_num = rhs.getIntNumber();
  NumberBase * new_num = getNumber(self_num / rhs_num);
  return *new_num;
}

NumberBase &
IntNumber::operator%(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
  {
    Real self_num = getRealNumber();
    Real rhs_num = rhs.getRealNumber();
    NumberBase * new_num = getNumber(fmod(self_num, rhs_num));
    return *new_num;
  }
  int self_num = getIntNumber();
  int rhs_num = rhs.getIntNumber();
  NumberBase * new_num = getNumber(self_num % rhs_num);
  return *new_num;
}

NumberBase &
IntNumber::operator<(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs >= *this;
  int self_num = getIntNumber();
  int rhs_num = rhs.getIntNumber();
  NumberBase * new_num = getNumber(self_num < rhs_num);
  return *new_num;
}

NumberBase &
IntNumber::operator>(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs <= *this;
  int self_num = getIntNumber();
  int rhs_num = rhs.getIntNumber();
  NumberBase * new_num = getNumber(self_num > rhs_num);
  return *new_num;
}

NumberBase &
IntNumber::operator<=(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs > *this;
  int self_num = getIntNumber();
  int rhs_num = rhs.getIntNumber();
  NumberBase * new_num = getNumber(self_num <= rhs_num);
  return *new_num;
}

NumberBase &
IntNumber::operator>=(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs < *this;
  int self_num = getIntNumber();
  int rhs_num = rhs.getIntNumber();
  NumberBase * new_num = getNumber(self_num >= rhs_num);
  return *new_num;
}

NumberBase &
IntNumber::operator==(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs == *this;
  int self_num = getIntNumber();
  int rhs_num = rhs.getIntNumber();
  NumberBase * new_num = getNumber(self_num == rhs_num);
  return *new_num;
}

NumberBase &
IntNumber::operator!=(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs != *this;
  int self_num = getIntNumber();
  int rhs_num = rhs.getIntNumber();
  NumberBase * new_num = getNumber(self_num != rhs_num);
  return *new_num;
}

NumberBase &
IntNumber::operator-()
{
  int self_num = getIntNumber();
  NumberBase * new_num = getNumber(-self_num);
  return *new_num;
}

NumberBase & IntNumber::operator!()
{
  int self_num = getIntNumber();
  NumberBase * new_num = getNumber(!self_num);
  return *new_num;
}

bool
IntNumber::conditional()
{
  if (getIntNumber())
    return true;
  return false;
}

NumberBase &
FractionNumber::operator+(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs + *this;
  Fraction self_num = getFractionNumber();
  Fraction rhs_num = rhs.getFractionNumber();
  NumberBase * new_num = getNumber(self_num + rhs_num);
  return *new_num;
}

NumberBase &
FractionNumber::operator-(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return (-rhs) + *this;
  Fraction self_num = getFractionNumber();
  Fraction rhs_num = rhs.getFractionNumber();
  NumberBase * new_num = getNumber(self_num - rhs_num);
  return *new_num;
}

NumberBase & FractionNumber::operator*(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs * *this;
  Fraction self_num = getFractionNumber();
  Fraction rhs_num = rhs.getFractionNumber();
  NumberBase * new_num = getNumber(self_num * rhs_num);
  return *new_num;
}

NumberBase &
FractionNumber::operator/(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return (*getNumber(1.0) / rhs) * *this;
  Fraction self_num = getFractionNumber();
  Fraction rhs_num = rhs.getFractionNumber();
  NumberBase * new_num = getNumber(self_num / rhs_num);
  return *new_num;
}

NumberBase &
FractionNumber::operator%(NumberBase & rhs)
{
  Real self_num = getRealNumber();
  Real rhs_num = rhs.getRealNumber();
  NumberBase * new_num = getNumber(fmod(self_num, rhs_num));
  return *new_num;
}

NumberBase &
FractionNumber::operator<(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs >= *this;
  Fraction self_num = getFractionNumber();
  Fraction rhs_num = rhs.getFractionNumber();
  NumberBase * new_num = getNumber(self_num < rhs_num);
  return *new_num;
}

NumberBase &
FractionNumber::operator>(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs <= *this;
  Fraction self_num = getFractionNumber();
  Fraction rhs_num = rhs.getFractionNumber();
  NumberBase * new_num = getNumber(self_num > rhs_num);
  return *new_num;
}

NumberBase &
FractionNumber::operator<=(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs > *this;
  Fraction self_num = getFractionNumber();
  Fraction rhs_num = rhs.getFractionNumber();
  NumberBase * new_num = getNumber(self_num <= rhs_num);
  return *new_num;
}

NumberBase &
FractionNumber::operator>=(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs < *this;
  Fraction self_num = getFractionNumber();
  Fraction rhs_num = rhs.getFractionNumber();
  NumberBase * new_num = getNumber(self_num >= rhs_num);
  return *new_num;
}

NumberBase &
FractionNumber::operator==(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs == *this;
  Fraction self_num = getFractionNumber();
  Fraction rhs_num = rhs.getFractionNumber();
  NumberBase * new_num = getNumber(self_num == rhs_num);
  return *new_num;
}

NumberBase &
FractionNumber::operator!=(NumberBase & rhs)
{
  if (precedence() < rhs.precedence())
    return rhs != *this;
  Fraction self_num = getFractionNumber();
  Fraction rhs_num = rhs.getFractionNumber();
  NumberBase * new_num = getNumber(self_num != rhs_num);
  return *new_num;
}

NumberBase &
FractionNumber::operator-()
{
  Fraction self_num = getFractionNumber();
  NumberBase * new_num = getNumber(-self_num);
  return *new_num;
}

NumberBase & FractionNumber::operator!()
{
  Fraction self_num = getFractionNumber();
  NumberBase * new_num = getNumber(!self_num);
  return *new_num;
}

bool
FractionNumber::conditional()
{
  if (getFractionNumber())
    return true;
  return false;
}
