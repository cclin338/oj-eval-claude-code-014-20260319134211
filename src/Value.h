#pragma once
#ifndef PYTHON_INTERPRETER_VALUE_H
#define PYTHON_INTERPRETER_VALUE_H

#include <any>
#include <string>
#include <memory>
#include <vector>
#include <variant>
#include <gmpxx.h>

class Value;
using ValuePtr = std::shared_ptr<Value>;
using ValueList = std::vector<ValuePtr>;

// Forward declarations for all value types
class IntValue;
class FloatValue;
class BoolValue;
class StringValue;
class NoneValue;
class TupleValue;
class FunctionValue;

enum class ValueType {
    INT,
    FLOAT,
    BOOL,
    STRING,
    NONE,
    TUPLE,
    FUNCTION
};

// Base value class
class Value {
public:
    virtual ~Value() = default;
    virtual ValueType getType() const = 0;
    virtual std::string toString() const = 0;
    virtual ValuePtr copy() const = 0;

    // Type checking helpers
    bool isInt() const { return getType() == ValueType::INT; }
    bool isFloat() const { return getType() == ValueType::FLOAT; }
    bool isBool() const { return getType() == ValueType::BOOL; }
    bool isString() const { return getType() == ValueType::STRING; }
    bool isNone() const { return getType() == ValueType::NONE; }
    bool isTuple() const { return getType() == ValueType::TUPLE; }
    bool isFunction() const { return getType() == ValueType::FUNCTION; }

    // Conversion helpers
    virtual mpz_class asInt() const;
    virtual double asFloat() const;
    virtual bool asBool() const;

    // Arithmetic operations
    virtual ValuePtr add(const Value& other) const;
    virtual ValuePtr subtract(const Value& other) const;
    virtual ValuePtr multiply(const Value& other) const;
    virtual ValuePtr divide(const Value& other) const;
    virtual ValuePtr floorDivide(const Value& other) const;
    virtual ValuePtr modulo(const Value& other) const;

    // Comparison operations
    virtual ValuePtr equal(const Value& other) const;
    virtual ValuePtr notEqual(const Value& other) const;
    virtual ValuePtr less(const Value& other) const;
    virtual ValuePtr lessEqual(const Value& other) const;
    virtual ValuePtr greater(const Value& other) const;
    virtual ValuePtr greaterEqual(const Value& other) const;

    // Logical operations
    virtual ValuePtr logicalAnd(const Value& other) const;
    virtual ValuePtr logicalOr(const Value& other) const;
    virtual ValuePtr logicalNot() const;

    // Unary operations
    virtual ValuePtr unaryPlus() const;
    virtual ValuePtr unaryMinus() const;
};

// Integer value (arbitrary precision)
class IntValue : public Value {
private:
    mpz_class value;

public:
    IntValue() : value(static_cast<long>(0)) {}
    IntValue(const mpz_class& v) : value(v) {}
    IntValue(long long v) : value(static_cast<long>(v)) {}
    IntValue(const std::string& s) : value(s) {}

    ValueType getType() const override { return ValueType::INT; }
    std::string toString() const override { return value.get_str(); }
    ValuePtr copy() const override { return std::make_shared<IntValue>(value); }

    mpz_class getValue() const { return value; }

    mpz_class asInt() const override { return value; }
    double asFloat() const override { return value.get_d(); }
    bool asBool() const override { return value != 0; }

    ValuePtr add(const Value& other) const override;
    ValuePtr subtract(const Value& other) const override;
    ValuePtr multiply(const Value& other) const override;
    ValuePtr divide(const Value& other) const override;
    ValuePtr floorDivide(const Value& other) const override;
    ValuePtr modulo(const Value& other) const override;

    ValuePtr equal(const Value& other) const override;
    ValuePtr notEqual(const Value& other) const override;
    ValuePtr less(const Value& other) const override;
    ValuePtr lessEqual(const Value& other) const override;
    ValuePtr greater(const Value& other) const override;
    ValuePtr greaterEqual(const Value& other) const override;

    ValuePtr logicalAnd(const Value& other) const override;
    ValuePtr logicalOr(const Value& other) const override;
    ValuePtr logicalNot() const override;

    ValuePtr unaryPlus() const override;
    ValuePtr unaryMinus() const override;
};

// Float value
class FloatValue : public Value {
private:
    double value;

public:
    FloatValue() : value(0.0) {}
    FloatValue(double v) : value(v) {}

    ValueType getType() const override { return ValueType::FLOAT; }
    std::string toString() const override;
    ValuePtr copy() const override { return std::make_shared<FloatValue>(value); }

    double getValue() const { return value; }

    mpz_class asInt() const override { return mpz_class(static_cast<long>(value)); }
    double asFloat() const override { return value; }
    bool asBool() const override { return value != 0.0; }

    ValuePtr add(const Value& other) const override;
    ValuePtr subtract(const Value& other) const override;
    ValuePtr multiply(const Value& other) const override;
    ValuePtr divide(const Value& other) const override;
    ValuePtr floorDivide(const Value& other) const override;
    ValuePtr modulo(const Value& other) const override;

    ValuePtr equal(const Value& other) const override;
    ValuePtr notEqual(const Value& other) const override;
    ValuePtr less(const Value& other) const override;
    ValuePtr lessEqual(const Value& other) const override;
    ValuePtr greater(const Value& other) const override;
    ValuePtr greaterEqual(const Value& other) const override;

    ValuePtr logicalAnd(const Value& other) const override;
    ValuePtr logicalOr(const Value& other) const override;
    ValuePtr logicalNot() const override;

    ValuePtr unaryPlus() const override;
    ValuePtr unaryMinus() const override;
};

// Boolean value
class BoolValue : public Value {
private:
    bool value;

public:
    BoolValue() : value(false) {}
    BoolValue(bool v) : value(v) {}

    ValueType getType() const override { return ValueType::BOOL; }
    std::string toString() const override { return value ? "True" : "False"; }
    ValuePtr copy() const override { return std::make_shared<BoolValue>(value); }

    bool getValue() const { return value; }

    mpz_class asInt() const override { return value ? 1 : 0; }
    double asFloat() const override { return value ? 1.0 : 0.0; }
    bool asBool() const override { return value; }

    ValuePtr equal(const Value& other) const override;
    ValuePtr notEqual(const Value& other) const override;
    ValuePtr less(const Value& other) const override;
    ValuePtr lessEqual(const Value& other) const override;
    ValuePtr greater(const Value& other) const override;
    ValuePtr greaterEqual(const Value& other) const override;

    ValuePtr logicalAnd(const Value& other) const override;
    ValuePtr logicalOr(const Value& other) const override;
    ValuePtr logicalNot() const override;

    ValuePtr unaryPlus() const override;
    ValuePtr unaryMinus() const override;
};

// String value
class StringValue : public Value {
private:
    std::string value;

public:
    StringValue() : value("") {}
    StringValue(const std::string& v) : value(v) {}

    ValueType getType() const override { return ValueType::STRING; }
    std::string toString() const override { return value; }
    ValuePtr copy() const override { return std::make_shared<StringValue>(value); }

    std::string getValue() const { return value; }

    bool asBool() const override { return !value.empty(); }

    ValuePtr add(const Value& other) const override;

    ValuePtr equal(const Value& other) const override;
    ValuePtr notEqual(const Value& other) const override;
    ValuePtr less(const Value& other) const override;
    ValuePtr lessEqual(const Value& other) const override;
    ValuePtr greater(const Value& other) const override;
    ValuePtr greaterEqual(const Value& other) const override;

    ValuePtr logicalAnd(const Value& other) const override;
    ValuePtr logicalOr(const Value& other) const override;
    ValuePtr logicalNot() const override;
};

// None value
class NoneValue : public Value {
public:
    ValueType getType() const override { return ValueType::NONE; }
    std::string toString() const override { return "None"; }
    ValuePtr copy() const override { return std::make_shared<NoneValue>(); }

    bool asBool() const override { return false; }

    ValuePtr equal(const Value& other) const override;
    ValuePtr notEqual(const Value& other) const override;
};

// Tuple value
class TupleValue : public Value {
private:
    ValueList elements;

public:
    TupleValue() = default;
    TupleValue(const ValueList& elems) : elements(elems) {}

    ValueType getType() const override { return ValueType::TUPLE; }
    std::string toString() const override;
    ValuePtr copy() const override;

    ValueList getElements() const { return elements; }
    size_t size() const { return elements.size(); }
    ValuePtr at(size_t index) const;

    bool asBool() const override { return !elements.empty(); }

    ValuePtr equal(const Value& other) const override;
    ValuePtr notEqual(const Value& other) const override;
    ValuePtr less(const Value& other) const override;
    ValuePtr lessEqual(const Value& other) const override;
    ValuePtr greater(const Value& other) const override;
    ValuePtr greaterEqual(const Value& other) const override;
};

// Function value (for storing user-defined functions)
class FunctionValue : public Value {
private:
    // Function context will be defined later
    std::string name;
    std::vector<std::string> params;
    bool hasDefaults;

public:
    FunctionValue(const std::string& n, const std::vector<std::string>& p, bool h)
        : name(n), params(p), hasDefaults(h) {}

    ValueType getType() const override { return ValueType::FUNCTION; }
    std::string toString() const override { return "<function " + name + ">"; }
    ValuePtr copy() const override { return std::make_shared<FunctionValue>(name, params, hasDefaults); }

    std::string getName() const { return name; }
    std::vector<std::string> getParams() const { return params; }
    bool hasDefaultParams() const { return hasDefaults; }

    bool asBool() const override { return true; }
};

// Factory functions
ValuePtr makeInt(const mpz_class& value);
ValuePtr makeInt(long long value);
ValuePtr makeInt(const std::string& value);
ValuePtr makeFloat(double value);
ValuePtr makeFloat(const std::string& value);
ValuePtr makeBool(bool value);
ValuePtr makeString(const std::string& value);
ValuePtr makeNone();
ValuePtr makeTuple(const ValueList& elements);
ValuePtr makeFunction(const std::string& name, const std::vector<std::string>& params, bool hasDefaults);

#endif//PYTHON_INTERPRETER_VALUE_H