#include "Value.h"
#include <iomanip>
#include <sstream>
#include <stdexcept>

// Factory functions
ValuePtr makeInt(long long value);
ValuePtr makeInt(const std::string& value);
ValuePtr makeFloat(double value);
ValuePtr makeFloat(const std::string& value);
ValuePtr makeBool(bool value);
ValuePtr makeString(const std::string& value);
ValuePtr makeNone();
ValuePtr makeTuple(const ValueList& elements);
ValuePtr makeFunction(const std::string& name, const std::vector<std::string>& params, bool hasDefaults);
#include <cmath>
#include <algorithm>

// Base Value class default implementations
long long Value::asInt() const {
    throw std::runtime_error("Cannot convert " + toString() + " to int");
}

double Value::asFloat() const {
    throw std::runtime_error("Cannot convert " + toString() + " to float");
}

std::string Value::asBigIntString() const {
    return toString();
}

bool Value::asBool() const {
    return true;  // Most values are truthy by default
}

// Default arithmetic operations (throw errors)
ValuePtr Value::add(const Value& other) const {
    throw std::runtime_error("Cannot perform addition on " + toString());
}

ValuePtr Value::subtract(const Value& other) const {
    throw std::runtime_error("Cannot perform subtraction on " + toString());
}

ValuePtr Value::multiply(const Value& other) const {
    throw std::runtime_error("Cannot perform multiplication on " + toString());
}

ValuePtr Value::divide(const Value& other) const {
    throw std::runtime_error("Cannot perform division on " + toString());
}

ValuePtr Value::floorDivide(const Value& other) const {
    throw std::runtime_error("Cannot perform floor division on " + toString());
}

ValuePtr Value::modulo(const Value& other) const {
    throw std::runtime_error("Cannot perform modulo on " + toString());
}

// Default comparison operations
ValuePtr Value::equal(const Value& other) const {
    return makeBool(false);  // Different types are not equal
}

ValuePtr Value::notEqual(const Value& other) const {
    auto eq = equal(other);
    return makeBool(!eq->asBool());
}

ValuePtr Value::less(const Value& other) const {
    throw std::runtime_error("Cannot compare " + toString() + " with " + other.toString());
}

ValuePtr Value::lessEqual(const Value& other) const {
    throw std::runtime_error("Cannot compare " + toString() + " with " + other.toString());
}

ValuePtr Value::greater(const Value& other) const {
    throw std::runtime_error("Cannot compare " + toString() + " with " + other.toString());
}

ValuePtr Value::greaterEqual(const Value& other) const {
    throw std::runtime_error("Cannot compare " + toString() + " with " + other.toString());
}

// Default logical operations
ValuePtr Value::logicalAnd(const Value& other) const {
    return makeBool(asBool() && other.asBool());
}

ValuePtr Value::logicalOr(const Value& other) const {
    return makeBool(asBool() || other.asBool());
}

ValuePtr Value::logicalNot() const {
    return makeBool(!asBool());
}

// Default unary operations
ValuePtr Value::unaryPlus() const {
    throw std::runtime_error("Cannot apply unary plus to " + toString());
}

ValuePtr Value::unaryMinus() const {
    throw std::runtime_error("Cannot apply unary minus to " + toString());
}

// FloatValue toString
std::string FloatValue::toString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << value;
    std::string result = oss.str();

    // Remove trailing zeros
    result.erase(result.find_last_not_of('0') + 1, std::string::npos);

    // If the decimal point is now the last character, remove it
    if (result.back() == '.') {
        result.pop_back();
    }

    return result;
}

// TupleValue toString
std::string TupleValue::toString() const {
    std::string result = "(";
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) result += ", ";
        result += elements[i]->toString();
    }
    if (elements.size() == 1) {
        result += ",";
    }
    result += ")";
    return result;
}

ValuePtr TupleValue::copy() const {
    ValueList copiedElements;
    for (const auto& elem : elements) {
        copiedElements.push_back(elem->copy());
    }
    return std::make_shared<TupleValue>(copiedElements);
}

ValuePtr TupleValue::at(size_t index) const {
    if (index >= elements.size()) {
        throw std::runtime_error("tuple index out of range");
    }
    return elements[index];
}

// Tuple comparison operations
ValuePtr TupleValue::equal(const Value& other) const {
    if (other.getType() != ValueType::TUPLE) {
        return makeBool(false);
    }
    const auto& otherTuple = dynamic_cast<const TupleValue&>(other);
    if (size() != otherTuple.size()) {
        return makeBool(false);
    }
    for (size_t i = 0; i < size(); ++i) {
        auto eq = elements[i]->equal(*otherTuple.at(i));
        if (!eq->asBool()) {
            return makeBool(false);
        }
    }
    return makeBool(true);
}

ValuePtr TupleValue::notEqual(const Value& other) const {
    auto eq = equal(other);
    return makeBool(!eq->asBool());
}

ValuePtr TupleValue::less(const Value& other) const {
    if (other.getType() != ValueType::TUPLE) {
        throw std::runtime_error("cannot compare tuple with non-tuple");
    }
    const auto& otherTuple = dynamic_cast<const TupleValue&>(other);
    size_t minSize = std::min(size(), otherTuple.size());
    for (size_t i = 0; i < minSize; ++i) {
        auto lessCmp = elements[i]->less(*otherTuple.at(i));
        auto equalCmp = elements[i]->equal(*otherTuple.at(i));
        if (lessCmp->asBool()) {
            return makeBool(true);
        }
        if (!equalCmp->asBool()) {
            return makeBool(false);
        }
    }
    return makeBool(size() < otherTuple.size());
}

ValuePtr TupleValue::lessEqual(const Value& other) const {
    auto lessCmp = less(other);
    auto equalCmp = equal(other);
    return makeBool(lessCmp->asBool() || equalCmp->asBool());
}

ValuePtr TupleValue::greater(const Value& other) const {
    auto lessEqCmp = lessEqual(other);
    return makeBool(!lessEqCmp->asBool());
}

ValuePtr TupleValue::greaterEqual(const Value& other) const {
    auto lessCmp = less(other);
    return makeBool(!lessCmp->asBool());
}

// NoneValue operations
ValuePtr NoneValue::equal(const Value& other) const {
    return makeBool(other.isNone());
}

ValuePtr NoneValue::notEqual(const Value& other) const {
    return makeBool(!other.isNone());
}

// Factory functions
ValuePtr makeInt(long long value) {
    return std::make_shared<IntValue>(value);
}

ValuePtr makeInt(const std::string& value) {
    return std::make_shared<IntValue>(value);
}

ValuePtr makeFloat(double value) {
    return std::make_shared<FloatValue>(value);
}

ValuePtr makeFloat(const std::string& value) {
    return std::make_shared<FloatValue>(std::stod(value));
}

ValuePtr makeBool(bool value) {
    return std::make_shared<BoolValue>(value);
}

ValuePtr makeString(const std::string& value) {
    return std::make_shared<StringValue>(value);
}

ValuePtr makeNone() {
    return std::make_shared<NoneValue>();
}

ValuePtr makeTuple(const ValueList& elements) {
    return std::make_shared<TupleValue>(elements);
}

ValuePtr makeFunction(const std::string& name, const std::vector<std::string>& params, bool hasDefaults) {
    return std::make_shared<FunctionValue>(name, params, hasDefaults);
}