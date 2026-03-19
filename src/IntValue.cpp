#include "Value.h"
#include <limits>
#include <cstdlib>
#include <sstream>
#include <iostream>

IntValue::IntValue(const std::string& s) : isOverflow(false) {
    // Check if the number is too large for long long
    try {
        long long parsed = std::stoll(s);
        value = parsed;
    } catch (const std::out_of_range&) {
        // Number is too large, store as overflow
        isOverflow = true;
        overflowValue = s;
        value = 0;  // Default to 0 for overflow values (they won't be used)
    }
}

std::string IntValue::toString() const {
    if (isOverflow) {
        return overflowValue;
    }
    return std::to_string(value);
}

std::string IntValue::asBigIntString() const {
    return toString();
}

ValuePtr IntValue::copy() const {
    if (isOverflow) {
        auto copy = std::make_shared<IntValue>(overflowValue);
        return copy;
    }
    return std::make_shared<IntValue>(value);
}

// IntValue arithmetic operations
ValuePtr IntValue::add(const Value& other) const {
    if (isOverflow) {
        throw std::runtime_error("Overflow integer arithmetic not supported");
    }

    if (other.isInt()) {
        return makeInt(value + dynamic_cast<const IntValue&>(other).getValue());
    } else if (other.isFloat()) {
        return makeFloat(value + dynamic_cast<const FloatValue&>(other).getValue());
    } else if (other.isString()) {
        return makeString(toString() + other.toString());
    }
    throw std::runtime_error("Cannot add int to " + other.toString());
}

ValuePtr IntValue::subtract(const Value& other) const {
    if (isOverflow) {
        throw std::runtime_error("Overflow integer arithmetic not supported");
    }

    if (other.isInt()) {
        return makeInt(value - dynamic_cast<const IntValue&>(other).getValue());
    } else if (other.isFloat()) {
        return makeFloat(value - dynamic_cast<const FloatValue&>(other).getValue());
    }
    throw std::runtime_error("Cannot subtract " + other.toString() + " from int");
}

ValuePtr IntValue::multiply(const Value& other) const {
    if (isOverflow) {
        throw std::runtime_error("Overflow integer arithmetic not supported");
    }

    if (other.isInt()) {
        return makeInt(value * dynamic_cast<const IntValue&>(other).getValue());
    } else if (other.isFloat()) {
        return makeFloat(value * dynamic_cast<const FloatValue&>(other).getValue());
    } else if (other.isString()) {
        // String repetition
        int count = static_cast<int>(value);
        if (count < 0) {
            throw std::runtime_error("can't multiply sequence by negative int");
        }
        const auto& str = dynamic_cast<const StringValue&>(other).getValue();
        std::string result;
        for (int i = 0; i < count; ++i) {
            result += str;
        }
        return makeString(result);
    }
    throw std::runtime_error("Cannot multiply int by " + other.toString());
}

ValuePtr IntValue::divide(const Value& other) const {
    if (isOverflow) {
        throw std::runtime_error("Overflow integer arithmetic not supported");
    }

    if (other.isInt()) {
        const auto& otherInt = dynamic_cast<const IntValue&>(other).getValue();
        if (otherInt == 0) {
            throw std::runtime_error("division by zero");
        }
        return makeFloat(static_cast<double>(value) / otherInt);
    } else if (other.isFloat()) {
        const auto& otherFloat = dynamic_cast<const FloatValue&>(other).getValue();
        if (otherFloat == 0.0) {
            throw std::runtime_error("division by zero");
        }
        return makeFloat(value / otherFloat);
    }
    throw std::runtime_error("Cannot divide int by " + other.toString());
}

ValuePtr IntValue::floorDivide(const Value& other) const {
    if (isOverflow) {
        throw std::runtime_error("Overflow integer arithmetic not supported");
    }

    if (other.isInt()) {
        const auto& otherInt = dynamic_cast<const IntValue&>(other).getValue();
        if (otherInt == 0) {
            throw std::runtime_error("division by zero");
        }
        // Python floor division semantics
        long long quotient = value / otherInt;
        long long remainder = value % otherInt;
        if (remainder != 0 && (value < 0) != (otherInt < 0)) {
            quotient -= 1;
        }
        return makeInt(quotient);
    }
    throw std::runtime_error("Cannot floor divide int by " + other.toString());
}

ValuePtr IntValue::modulo(const Value& other) const {
    if (isOverflow) {
        throw std::runtime_error("Overflow integer arithmetic not supported");
    }

    if (other.isInt()) {
        const auto& otherInt = dynamic_cast<const IntValue&>(other).getValue();
        if (otherInt == 0) {
            throw std::runtime_error("modulo by zero");
        }
        // Python modulo semantics
        long long remainder = value % otherInt;
        if (remainder != 0 && (value < 0) != (otherInt < 0)) {
            remainder += otherInt;
        }
        return makeInt(remainder);
    }
    throw std::runtime_error("Cannot compute int modulo " + other.toString());
}

// IntValue comparison operations
ValuePtr IntValue::equal(const Value& other) const {
    if (other.isInt()) {
        return makeBool(value == dynamic_cast<const IntValue&>(other).getValue());
    } else if (other.isFloat()) {
        return makeBool(value == dynamic_cast<const FloatValue&>(other).getValue());
    } else if (other.isBool()) {
        return makeBool((value != 0) == dynamic_cast<const BoolValue&>(other).getValue());
    }
    return makeBool(false);
}

ValuePtr IntValue::notEqual(const Value& other) const {
    auto eq = equal(other);
    return makeBool(!eq->asBool());
}

ValuePtr IntValue::less(const Value& other) const {
    if (other.isInt()) {
        return makeBool(value < dynamic_cast<const IntValue&>(other).getValue());
    } else if (other.isFloat()) {
        return makeBool(value < dynamic_cast<const FloatValue&>(other).getValue());
    }
    throw std::runtime_error("Cannot compare int with " + other.toString());
}

ValuePtr IntValue::lessEqual(const Value& other) const {
    if (other.isInt()) {
        return makeBool(value <= dynamic_cast<const IntValue&>(other).getValue());
    } else if (other.isFloat()) {
        return makeBool(value <= dynamic_cast<const FloatValue&>(other).getValue());
    }
    throw std::runtime_error("Cannot compare int with " + other.toString());
}

ValuePtr IntValue::greater(const Value& other) const {
    auto lessEqCmp = lessEqual(other);
    return makeBool(!lessEqCmp->asBool());
}

ValuePtr IntValue::greaterEqual(const Value& other) const {
    auto lessCmp = less(other);
    return makeBool(!lessCmp->asBool());
}

// IntValue logical operations
ValuePtr IntValue::logicalAnd(const Value& other) const {
    return makeBool(asBool() && other.asBool());
}

ValuePtr IntValue::logicalOr(const Value& other) const {
    return makeBool(asBool() || other.asBool());
}

ValuePtr IntValue::logicalNot() const {
    return makeBool(value == 0);
}

// IntValue unary operations
ValuePtr IntValue::unaryPlus() const {
    return makeInt(value);
}

ValuePtr IntValue::unaryMinus() const {
    return makeInt(-value);
}