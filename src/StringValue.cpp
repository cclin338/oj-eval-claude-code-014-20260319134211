#include "Value.h"

// StringValue operations
ValuePtr StringValue::add(const Value& other) const {
    return makeString(value + other.toString());
}

ValuePtr StringValue::equal(const Value& other) const {
    if (other.isString()) {
        return makeBool(value == dynamic_cast<const StringValue&>(other).getValue());
    }
    return makeBool(false);
}

ValuePtr StringValue::notEqual(const Value& other) const {
    auto eq = equal(other);
    return makeBool(!eq->asBool());
}

ValuePtr StringValue::less(const Value& other) const {
    if (other.isString()) {
        return makeBool(value < dynamic_cast<const StringValue&>(other).getValue());
    }
    throw std::runtime_error("Cannot compare string with " + other.toString());
}

ValuePtr StringValue::lessEqual(const Value& other) const {
    if (other.isString()) {
        return makeBool(value <= dynamic_cast<const StringValue&>(other).getValue());
    }
    throw std::runtime_error("Cannot compare string with " + other.toString());
}

ValuePtr StringValue::greater(const Value& other) const {
    auto lessEqCmp = lessEqual(other);
    return makeBool(!lessEqCmp->asBool());
}

ValuePtr StringValue::greaterEqual(const Value& other) const {
    auto lessCmp = less(other);
    return makeBool(!lessCmp->asBool());
}

// StringValue logical operations
ValuePtr StringValue::logicalAnd(const Value& other) const {
    return makeBool(asBool() && other.asBool());
}

ValuePtr StringValue::logicalOr(const Value& other) const {
    return makeBool(asBool() || other.asBool());
}

ValuePtr StringValue::logicalNot() const {
    return makeBool(!asBool());
}