#include "Value.h"

// IntValue arithmetic operations
ValuePtr IntValue::add(const Value& other) const {
    if (other.isInt()) {
        return makeInt(value + dynamic_cast<const IntValue&>(other).getValue());
    } else if (other.isFloat()) {
        return makeFloat(value.get_d() + dynamic_cast<const FloatValue&>(other).getValue());
    } else if (other.isString()) {
        return makeString(toString() + other.toString());
    }
    throw std::runtime_error("Cannot add int to " + other.toString());
}

ValuePtr IntValue::subtract(const Value& other) const {
    if (other.isInt()) {
        return makeInt(value - dynamic_cast<const IntValue&>(other).getValue());
    } else if (other.isFloat()) {
        return makeFloat(value.get_d() - dynamic_cast<const FloatValue&>(other).getValue());
    }
    throw std::runtime_error("Cannot subtract " + other.toString() + " from int");
}

ValuePtr IntValue::multiply(const Value& other) const {
    if (other.isInt()) {
        return makeInt(value * dynamic_cast<const IntValue&>(other).getValue());
    } else if (other.isFloat()) {
        return makeFloat(value.get_d() * dynamic_cast<const FloatValue&>(other).getValue());
    } else if (other.isString()) {
        // String repetition
        int count = value.get_si();
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
    if (other.isInt()) {
        const auto& otherInt = dynamic_cast<const IntValue&>(other).getValue();
        if (otherInt == 0) {
            throw std::runtime_error("division by zero");
        }
        return makeFloat(value.get_d() / otherInt.get_d());
    } else if (other.isFloat()) {
        const auto& otherFloat = dynamic_cast<const FloatValue&>(other).getValue();
        if (otherFloat == 0.0) {
            throw std::runtime_error("division by zero");
        }
        return makeFloat(value.get_d() / otherFloat);
    }
    throw std::runtime_error("Cannot divide int by " + other.toString());
}

ValuePtr IntValue::floorDivide(const Value& other) const {
    if (other.isInt()) {
        const auto& otherInt = dynamic_cast<const IntValue&>(other).getValue();
        if (otherInt == 0) {
            throw std::runtime_error("division by zero");
        }
        mpz_class quotient = value / otherInt;
        // Handle negative division (Python semantics)
        mpz_class remainder = value % otherInt;
        if (remainder != 0 && (value < 0) != (otherInt < 0)) {
            quotient -= 1;
        }
        return makeInt(quotient);
    }
    throw std::runtime_error("Cannot floor divide int by " + other.toString());
}

ValuePtr IntValue::modulo(const Value& other) const {
    if (other.isInt()) {
        const auto& otherInt = dynamic_cast<const IntValue&>(other).getValue();
        if (otherInt == 0) {
            throw std::runtime_error("modulo by zero");
        }
        // Python modulo semantics
        mpz_class remainder = value % otherInt;
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
        return makeBool(value.get_d() == dynamic_cast<const FloatValue&>(other).getValue());
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
        return makeBool(value.get_d() < dynamic_cast<const FloatValue&>(other).getValue());
    }
    throw std::runtime_error("Cannot compare int with " + other.toString());
}

ValuePtr IntValue::lessEqual(const Value& other) const {
    if (other.isInt()) {
        return makeBool(value <= dynamic_cast<const IntValue&>(other).getValue());
    } else if (other.isFloat()) {
        return makeBool(value.get_d() <= dynamic_cast<const FloatValue&>(other).getValue());
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