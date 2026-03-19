#include "Value.h"
#include <cmath>

// FloatValue arithmetic operations
ValuePtr FloatValue::add(const Value& other) const {
    if (other.isInt()) {
        return makeFloat(value + static_cast<double>(dynamic_cast<const IntValue&>(other).getValue()));
    } else if (other.isFloat()) {
        return makeFloat(value + dynamic_cast<const FloatValue&>(other).getValue());
    }
    throw std::runtime_error("Cannot add float to " + other.toString());
}

ValuePtr FloatValue::subtract(const Value& other) const {
    if (other.isInt()) {
        return makeFloat(value - static_cast<double>(dynamic_cast<const IntValue&>(other).getValue()));
    } else if (other.isFloat()) {
        return makeFloat(value - dynamic_cast<const FloatValue&>(other).getValue());
    }
    throw std::runtime_error("Cannot subtract " + other.toString() + " from float");
}

ValuePtr FloatValue::multiply(const Value& other) const {
    if (other.isInt()) {
        return makeFloat(value * static_cast<double>(dynamic_cast<const IntValue&>(other).getValue()));
    } else if (other.isFloat()) {
        return makeFloat(value * dynamic_cast<const FloatValue&>(other).getValue());
    }
    throw std::runtime_error("Cannot multiply float by " + other.toString());
}

ValuePtr FloatValue::divide(const Value& other) const {
    if (other.isInt()) {
        const auto& otherInt = dynamic_cast<const IntValue&>(other).getValue();
        if (otherInt == 0) {
            throw std::runtime_error("division by zero");
        }
        return makeFloat(value / static_cast<double>(otherInt));
    } else if (other.isFloat()) {
        const auto& otherFloat = dynamic_cast<const FloatValue&>(other).getValue();
        if (otherFloat == 0.0) {
            throw std::runtime_error("division by zero");
        }
        return makeFloat(value / otherFloat);
    }
    throw std::runtime_error("Cannot divide float by " + other.toString());
}

ValuePtr FloatValue::floorDivide(const Value& other) const {
    if (other.isInt()) {
        const auto& otherInt = dynamic_cast<const IntValue&>(other).getValue();
        if (otherInt == 0) {
            throw std::runtime_error("division by zero");
        }
        double result = value / static_cast<double>(otherInt);
        return makeFloat(std::floor(result));
    } else if (other.isFloat()) {
        const auto& otherFloat = dynamic_cast<const FloatValue&>(other).getValue();
        if (otherFloat == 0.0) {
            throw std::runtime_error("division by zero");
        }
        double result = value / otherFloat;
        return makeFloat(std::floor(result));
    }
    throw std::runtime_error("Cannot floor divide float by " + other.toString());
}

ValuePtr FloatValue::modulo(const Value& other) const {
    if (other.isInt()) {
        const auto& otherInt = dynamic_cast<const IntValue&>(other).getValue();
        if (otherInt == 0) {
            throw std::runtime_error("modulo by zero");
        }
        double remainder = std::fmod(value, static_cast<double>(otherInt));
        return makeFloat(remainder);
    } else if (other.isFloat()) {
        const auto& otherFloat = dynamic_cast<const FloatValue&>(other).getValue();
        if (otherFloat == 0.0) {
            throw std::runtime_error("modulo by zero");
        }
        double remainder = std::fmod(value, otherFloat);
        return makeFloat(remainder);
    }
    throw std::runtime_error("Cannot compute float modulo " + other.toString());
}

// FloatValue comparison operations
ValuePtr FloatValue::equal(const Value& other) const {
    if (other.isInt()) {
        return makeBool(value == static_cast<double>(dynamic_cast<const IntValue&>(other).getValue()));
    } else if (other.isFloat()) {
        return makeBool(value == dynamic_cast<const FloatValue&>(other).getValue());
    }
    return makeBool(false);
}

ValuePtr FloatValue::notEqual(const Value& other) const {
    auto eq = equal(other);
    return makeBool(!eq->asBool());
}

ValuePtr FloatValue::less(const Value& other) const {
    if (other.isInt()) {
        return makeBool(value < static_cast<double>(dynamic_cast<const IntValue&>(other).getValue()));
    } else if (other.isFloat()) {
        return makeBool(value < dynamic_cast<const FloatValue&>(other).getValue());
    }
    throw std::runtime_error("Cannot compare float with " + other.toString());
}

ValuePtr FloatValue::lessEqual(const Value& other) const {
    if (other.isInt()) {
        return makeBool(value <= static_cast<double>(dynamic_cast<const IntValue&>(other).getValue()));
    } else if (other.isFloat()) {
        return makeBool(value <= dynamic_cast<const FloatValue&>(other).getValue());
    }
    throw std::runtime_error("Cannot compare float with " + other.toString());
}

ValuePtr FloatValue::greater(const Value& other) const {
    auto lessEqCmp = lessEqual(other);
    return makeBool(!lessEqCmp->asBool());
}

ValuePtr FloatValue::greaterEqual(const Value& other) const {
    auto lessCmp = less(other);
    return makeBool(!lessCmp->asBool());
}

// FloatValue logical operations
ValuePtr FloatValue::logicalAnd(const Value& other) const {
    return makeBool(asBool() && other.asBool());
}

ValuePtr FloatValue::logicalOr(const Value& other) const {
    return makeBool(asBool() || other.asBool());
}

ValuePtr FloatValue::logicalNot() const {
    return makeBool(value == 0.0);
}

// FloatValue unary operations
ValuePtr FloatValue::unaryPlus() const {
    return makeFloat(value);
}

ValuePtr FloatValue::unaryMinus() const {
    return makeFloat(-value);
}