#include "Value.h"

// BoolValue comparison operations
ValuePtr BoolValue::equal(const Value& other) const {
    if (other.isBool()) {
        return makeBool(value == dynamic_cast<const BoolValue&>(other).getValue());
    } else if (other.isInt()) {
        return makeBool((value ? 1 : 0) == dynamic_cast<const IntValue&>(other).getValue());
    } else if (other.isFloat()) {
        return makeBool((value ? 1.0 : 0.0) == dynamic_cast<const FloatValue&>(other).getValue());
    }
    return makeBool(false);
}

ValuePtr BoolValue::notEqual(const Value& other) const {
    auto eq = equal(other);
    return makeBool(!eq->asBool());
}

ValuePtr BoolValue::less(const Value& other) const {
    if (other.isBool()) {
        return makeBool((value ? 1 : 0) < (dynamic_cast<const BoolValue&>(other).getValue() ? 1 : 0));
    } else if (other.isInt()) {
        return makeBool((value ? 1 : 0) < dynamic_cast<const IntValue&>(other).getValue());
    } else if (other.isFloat()) {
        return makeBool((value ? 1.0 : 0.0) < dynamic_cast<const FloatValue&>(other).getValue());
    }
    throw std::runtime_error("Cannot compare bool with " + other.toString());
}

ValuePtr BoolValue::lessEqual(const Value& other) const {
    if (other.isBool()) {
        return makeBool((value ? 1 : 0) <= (dynamic_cast<const BoolValue&>(other).getValue() ? 1 : 0));
    } else if (other.isInt()) {
        return makeBool((value ? 1 : 0) <= dynamic_cast<const IntValue&>(other).getValue());
    } else if (other.isFloat()) {
        return makeBool((value ? 1.0 : 0.0) <= dynamic_cast<const FloatValue&>(other).getValue());
    }
    throw std::runtime_error("Cannot compare bool with " + other.toString());
}

ValuePtr BoolValue::greater(const Value& other) const {
    auto lessEqCmp = lessEqual(other);
    return makeBool(!lessEqCmp->asBool());
}

ValuePtr BoolValue::greaterEqual(const Value& other) const {
    auto lessCmp = less(other);
    return makeBool(!lessCmp->asBool());
}

// BoolValue logical operations
ValuePtr BoolValue::logicalAnd(const Value& other) const {
    return makeBool(value && other.asBool());
}

ValuePtr BoolValue::logicalOr(const Value& other) const {
    return makeBool(value || other.asBool());
}

ValuePtr BoolValue::logicalNot() const {
    return makeBool(!value);
}

// BoolValue unary operations
ValuePtr BoolValue::unaryPlus() const {
    return makeInt(value ? 1 : 0);
}

ValuePtr BoolValue::unaryMinus() const {
    return makeInt(value ? -1 : 0);
}