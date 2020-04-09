#pragma once

#include "MooseError.h"
#include "MooseUtils.h"
#include "Fraction.h"
#include "libmesh/libmesh_common.h"
#include "ExpressionBuilderToo.h"

class RealNumber;
class IntNumber;

class NumberBase
{
public:
  static NumberBase * getNumber(Real real_num);
  static NumberBase * getNumber(int int_num);
  static NumberBase * getNumber(bool bool_num);
  static NumberBase * getNumber(Fraction frac_num);
  static NumberBase * getNumber(ExpressionBuilderToo::EBTermNode * node);
  static NumberBase * getNumber(RealNumber & real_num);
  static NumberBase * getNumber(IntNumber & int_num);

  virtual NumberBase & operator+(NumberBase & rhs);
  virtual NumberBase & operator-(NumberBase & rhs);
  virtual NumberBase & operator*(NumberBase & rhs);
  virtual NumberBase & operator/(NumberBase & rhs);
  virtual NumberBase & operator%(NumberBase & rhs);
  virtual NumberBase & operator<(NumberBase & rhs);
  virtual NumberBase & operator>(NumberBase & rhs);
  virtual NumberBase & operator<=(NumberBase & rhs);
  virtual NumberBase & operator>=(NumberBase & rhs);
  virtual NumberBase & operator==(NumberBase & rhs);
  virtual NumberBase & operator!=(NumberBase & rhs);
  virtual NumberBase & operator-();
  virtual NumberBase & operator!();

  static NumberBase & sin(NumberBase & rhs);
  static NumberBase & cos(NumberBase & rhs);
  static NumberBase & tan(NumberBase & rhs);
  static NumberBase & abs(NumberBase & rhs);
  static NumberBase & log(NumberBase & rhs);
  static NumberBase & log2(NumberBase & rhs);
  static NumberBase & log10(NumberBase & rhs);
  static NumberBase & exp(NumberBase & rhs);
  static NumberBase & sinh(NumberBase & rhs);
  static NumberBase & cosh(NumberBase & rhs);

  static NumberBase & min(NumberBase & lhs, NumberBase & rhs);
  static NumberBase & max(NumberBase & lhs, NumberBase & rhs);
  static NumberBase & pow(NumberBase & lhs, NumberBase & rhs);

  virtual operator ExpressionBuilderToo::EBTermNode *() = 0;
  virtual bool conditional() = 0;
  virtual Real getRealNumber() = 0;
  virtual int getIntNumber() = 0;
  virtual Fraction getFractionNumber() = 0;
  virtual int precedence() = 0;
};

class RealNumber : public NumberBase
{
public:
  RealNumber(Real number) : _number(number){};

  virtual operator ExpressionBuilderToo::EBTermNode *()
  {
    return new ExpressionBuilderToo::EBNumberNode<Real>(_number);
  };
  virtual bool conditional();
  virtual Real getRealNumber() { return _number; };
  virtual int getIntNumber() { return _number; };
  virtual Fraction getFractionNumber() { mooseError("Cannot coerce Real to a Fraction"); }
  virtual int precedence() { return 2; }

private:
  Real _number;
};

class IntNumber : public NumberBase
{
public:
  IntNumber(int number) : _number(number){};

  virtual NumberBase & operator+(NumberBase & rhs);
  virtual NumberBase & operator-(NumberBase & rhs);
  virtual NumberBase & operator*(NumberBase & rhs);
  virtual NumberBase & operator/(NumberBase & rhs);
  virtual NumberBase & operator%(NumberBase & rhs);
  virtual NumberBase & operator<(NumberBase & rhs);
  virtual NumberBase & operator>(NumberBase & rhs);
  virtual NumberBase & operator<=(NumberBase & rhs);
  virtual NumberBase & operator>=(NumberBase & rhs);
  virtual NumberBase & operator==(NumberBase & rhs);
  virtual NumberBase & operator!=(NumberBase & rhs);
  virtual NumberBase & operator-();
  virtual NumberBase & operator!();

  virtual operator ExpressionBuilderToo::EBTermNode *()
  {
    return new ExpressionBuilderToo::EBNumberNode<int>(_number);
  };
  virtual bool conditional();
  virtual Real getRealNumber() { return _number; };
  virtual int getIntNumber() { return _number; };
  virtual Fraction getFractionNumber() { return Fraction(_number); }
  virtual int precedence() { return 0; }

private:
  int _number;
};

class FractionNumber : public NumberBase
{
public:
  FractionNumber(Fraction number) : _number(number){};

  virtual NumberBase & operator+(NumberBase & rhs);
  virtual NumberBase & operator-(NumberBase & rhs);
  virtual NumberBase & operator*(NumberBase & rhs);
  virtual NumberBase & operator/(NumberBase & rhs);
  virtual NumberBase & operator%(NumberBase & rhs);
  virtual NumberBase & operator<(NumberBase & rhs);
  virtual NumberBase & operator>(NumberBase & rhs);
  virtual NumberBase & operator<=(NumberBase & rhs);
  virtual NumberBase & operator>=(NumberBase & rhs);
  virtual NumberBase & operator==(NumberBase & rhs);
  virtual NumberBase & operator!=(NumberBase & rhs);
  virtual NumberBase & operator-();
  virtual NumberBase & operator!();

  virtual operator ExpressionBuilderToo::EBTermNode *()
  {
    return new ExpressionBuilderToo::EBNumberNode<Fraction>(_number);
  };
  virtual bool conditional();
  virtual Real getRealNumber() { return _number * 1.0; };
  virtual int getIntNumber() { return _number * 1.0; };
  virtual Fraction getFractionNumber() { return _number; }
  virtual int precedence() { return 1; }

private:
  Fraction _number;
};
