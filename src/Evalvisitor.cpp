#include "Evalvisitor.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <gmpxx.h>

EvalVisitor::EvalVisitor() {
    globalScope = std::make_shared<Scope>("global");
    currentScope = globalScope;
    hasReturnValue = false;
    hasBreak = false;
    hasContinue = false;

    // Add built-in functions to global scope
    // We create placeholder function values for built-ins
    globalScope->setVariable("print", makeFunction("print", std::vector<std::string>{"*args"}, false));
    globalScope->setVariable("int", makeFunction("int", std::vector<std::string>{"x"}, false));
    globalScope->setVariable("float", makeFunction("float", std::vector<std::string>{"x"}, false));
    globalScope->setVariable("str", makeFunction("str", std::vector<std::string>{"x"}, false));
    globalScope->setVariable("bool", makeFunction("bool", std::vector<std::string>{"x"}, false));
}

ValuePtr EvalVisitor::getValue(const std::any& anyValue) {
    if (!anyValue.has_value()) {
        return makeNone();
    }
    try {
        return std::any_cast<ValuePtr>(anyValue);
    } catch (const std::bad_any_cast& e) {
        throw std::runtime_error("Value conversion error");
    }
}

void EvalVisitor::setValue(const std::any& anyValue, ValuePtr value) {
    // This is used for return values
    const_cast<std::any&>(anyValue) = value;
}

ValuePtr EvalVisitor::visitContext(antlr4::tree::ParseTree* ctx) {
    if (!ctx) return makeNone();
    auto result = visit(ctx);
    return getValue(result);
}

void EvalVisitor::resetControlFlowFlags() {
    hasBreak = false;
    hasContinue = false;
    hasReturnValue = false;
    returnValue = nullptr;
}

// File input - top level
std::any EvalVisitor::visitFile_input(Python3Parser::File_inputContext *ctx) {
    resetControlFlowFlags();
    for (auto child : ctx->children) {
        if (child != ctx->EOF()) {
            visit(child);
            if (shouldBreak() || shouldContinue() || hasReturned()) {
                break;
            }
        }
    }
    return std::any();
}

// Statement handling
std::any EvalVisitor::visitStmt(Python3Parser::StmtContext *ctx) {
    if (hasReturned() || shouldBreak() || shouldContinue()) {
        return std::any();
    }
    return visitChildren(ctx);
}

std::any EvalVisitor::visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) {
    if (hasReturned() || shouldBreak() || shouldContinue()) {
        return std::any();
    }
    return visitChildren(ctx);
}

std::any EvalVisitor::visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) {
    if (hasReturned() || shouldBreak() || shouldContinue()) {
        return std::any();
    }
    return visitChildren(ctx);
}

// Expression statement - handles assignment and augmented assignment
std::any EvalVisitor::visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) {
    if (ctx->augassign()) {
        // Augmented assignment (+=, -=, *=, /=, //=, %=)
        // For now, not implemented - need proper lvalue handling
        throw std::runtime_error("Augmented assignment not fully implemented");
    } else if (ctx->getText().find('=') != std::string::npos && ctx->testlist().size() >= 2) {
        // Regular assignment or chained assignment
        auto right = visitContext(ctx->testlist().back());

        // Handle single assignment for now
        if (ctx->testlist().size() == 2) {
            // For simple assignment, we need to parse the variable name
            // This is a hack for simple cases like "a = 5"
            auto leftText = ctx->testlist(0)->getText();
            // Check if it's a simple name (no operators)
            if (leftText.find_first_of("+-*/%<>=!()[]{},") == std::string::npos) {
                currentScope->setVariable(leftText, right);
                return right;
            }
        }

        return right;
    } else {
        // Just an expression
        auto result = visitContext(ctx->testlist(0));
        return result;
    }
}

// Flow control statements
std::any EvalVisitor::visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) {
    hasBreak = true;
    return std::any();
}

std::any EvalVisitor::visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) {
    hasContinue = true;
    return std::any();
}

std::any EvalVisitor::visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) {
    if (ctx->testlist()) {
        returnValue = visitContext(ctx->testlist());
    } else {
        returnValue = makeNone();
    }
    hasReturnValue = true;
    return std::any();
}

// Compound statements
std::any EvalVisitor::visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) {
    if (hasReturned() || shouldBreak() || shouldContinue()) {
        return std::any();
    }
    return visitChildren(ctx);
}

// If statement
std::any EvalVisitor::visitIf_stmt(Python3Parser::If_stmtContext *ctx) {
    // Evaluate the if condition
    auto condition = visitContext(ctx->test(0));
    if (condition->asBool()) {
        return visit(ctx->suite(0));
    }

    // Check elif conditions
    for (size_t i = 0; i < ctx->ELIF().size(); ++i) {
        auto elifCondition = visitContext(ctx->test(i + 1));
        if (elifCondition->asBool()) {
            return visit(ctx->suite(i + 1));
        }
    }

    // Check else clause
    if (ctx->ELSE()) {
        size_t elseSuiteIndex = ctx->ELIF().size() + 1;
        return visit(ctx->suite(elseSuiteIndex));
    }

    return std::any();
}

// While loop
std::any EvalVisitor::visitWhile_stmt(Python3Parser::While_stmtContext *ctx) {
    while (true) {
        auto condition = visitContext(ctx->test());
        if (!condition->asBool()) {
            break;
        }

        resetControlFlowFlags();
        visit(ctx->suite());

        if (shouldBreak()) {
            hasBreak = false;
            break;
        }
        if (shouldContinue()) {
            hasContinue = false;
            continue;
        }
        if (hasReturned()) {
            break;
        }
    }
    return std::any();
}

std::any EvalVisitor::visitSuite(Python3Parser::SuiteContext *ctx) {
    if (hasReturned() || shouldBreak() || shouldContinue()) {
        return std::any();
    }
    return visitChildren(ctx);
}

// Test (top-level boolean expression)
std::any EvalVisitor::visitTest(Python3Parser::TestContext *ctx) {
    return visit(ctx->or_test());
}

std::any EvalVisitor::visitOr_test(Python3Parser::Or_testContext *ctx) {
    if (ctx->and_test().size() == 1) {
        return visit(ctx->and_test(0));
    }

    // Evaluate OR expressions left to right
    auto result = visitContext(ctx->and_test(0));
    for (size_t i = 1; i < ctx->and_test().size(); ++i) {
        if (result->asBool()) {
            // Short-circuit
            return result;
        }
        auto rhs = visitContext(ctx->and_test(i));
        result = result->logicalOr(*rhs);
    }
    return result;
}

std::any EvalVisitor::visitAnd_test(Python3Parser::And_testContext *ctx) {
    if (ctx->not_test().size() == 1) {
        return visit(ctx->not_test(0));
    }

    // Evaluate AND expressions left to right
    auto result = visitContext(ctx->not_test(0));
    for (size_t i = 1; i < ctx->not_test().size(); ++i) {
        if (!result->asBool()) {
            // Short-circuit
            return result;
        }
        auto rhs = visitContext(ctx->not_test(i));
        result = result->logicalAnd(*rhs);
    }
    return result;
}

std::any EvalVisitor::visitNot_test(Python3Parser::Not_testContext *ctx) {
    if (ctx->NOT()) {
        auto operand = visitContext(ctx->not_test());
        return operand->logicalNot();
    }
    return visit(ctx->comparison());
}

// Comparison operations
std::any EvalVisitor::visitComparison(Python3Parser::ComparisonContext *ctx) {
    if (ctx->comp_op().empty()) {
        // No comparison operator, just arithmetic expression
        return visit(ctx->arith_expr(0));
    }

    auto left = visitContext(ctx->arith_expr(0));
    auto right = visitContext(ctx->arith_expr(1));

    std::string op = ctx->comp_op(0)->getText();
    if (op == "==") {
        return left->equal(*right);
    } else if (op == "!=") {
        return left->notEqual(*right);
    } else if (op == "<") {
        return left->less(*right);
    } else if (op == "<=") {
        return left->lessEqual(*right);
    } else if (op == ">") {
        return left->greater(*right);
    } else if (op == ">=") {
        return left->greaterEqual(*right);
    } else {
        throw std::runtime_error("Unknown comparison operator: " + op);
    }
}

// Arithmetic expressions
std::any EvalVisitor::visitArith_expr(Python3Parser::Arith_exprContext *ctx) {
    if (ctx->addorsub_op().empty()) {
        return visit(ctx->term(0));
    }

    auto result = visitContext(ctx->term(0));
    for (size_t i = 0; i < ctx->addorsub_op().size(); ++i) {
        auto rhs = visitContext(ctx->term(i + 1));
        std::string op = ctx->addorsub_op(i)->getText();
        if (op == "+") {
            result = result->add(*rhs);
        } else if (op == "-") {
            result = result->subtract(*rhs);
        }
    }
    return result;
}

// Term (multiplication/division)
std::any EvalVisitor::visitTerm(Python3Parser::TermContext *ctx) {
    if (ctx->muldivmod_op().empty()) {
        return visit(ctx->factor(0));
    }

    auto result = visitContext(ctx->factor(0));
    for (size_t i = 0; i < ctx->muldivmod_op().size(); ++i) {
        auto rhs = visitContext(ctx->factor(i + 1));
        std::string op = ctx->muldivmod_op(i)->getText();
        if (op == "*") {
            result = result->multiply(*rhs);
        } else if (op == "/") {
            result = result->divide(*rhs);
        } else if (op == "//") {
            result = result->floorDivide(*rhs);
        } else if (op == "%") {
            result = result->modulo(*rhs);
        }
    }
    return result;
}

// Factor (unary operations)
std::any EvalVisitor::visitFactor(Python3Parser::FactorContext *ctx) {
    if (ctx->atom_expr()) {
        return visit(ctx->atom_expr());
    }

    // Unary operation
    auto operand = visitContext(ctx->factor());
    std::string op = ctx->children[0]->getText();
    if (op == "+") {
        return operand->unaryPlus();
    } else if (op == "-") {
        return operand->unaryMinus();
    } else {
        throw std::runtime_error("Unknown unary operator: " + op);
    }
}

// Trailer (function call handling)
std::any EvalVisitor::visitTrailer(Python3Parser::TrailerContext *ctx) {
    // For now, just return the function arguments as a tuple
    // In a full implementation, we'd need to handle the actual function call
    if (ctx->arglist()) {
        return visit(ctx->arglist());
    }
    // Return empty tuple for no arguments
    return makeTuple(ValueList{});
}

// Atom expression (function calls)
std::any EvalVisitor::visitAtom_expr(Python3Parser::Atom_exprContext *ctx) {
    auto atom = visitContext(ctx->atom());

    if (ctx->trailer()) {
        // Handle function calls
        // Check if the preceding atom is a function
        if (atom->isFunction()) {
            // Get the function name
            const auto& funcName = dynamic_cast<const FunctionValue&>(*atom).getName();

            // Get arguments from the trailer
            ValuePtr args;
            if (ctx->trailer()->arglist()) {
                args = visitContext(ctx->trailer()->arglist());
            } else {
                // No arguments: empty tuple
                args = makeTuple(ValueList{});
            }

            // Handle built-in functions
            if (funcName == "print") {
                // Handle print function
                if (args->isTuple()) {
                    const auto& tuple = dynamic_cast<const TupleValue&>(*args);
                    bool first = true;
                    for (const auto& elem : tuple.getElements()) {
                        if (!first) std::cout << " ";
                        std::cout << elem->toString();
                        first = false;
                    }
                } else {
                    std::cout << args->toString();
                }
                std::cout << std::endl;
                std::fflush(stdout);  // Ensure output is flushed
                return makeNone();
            } else if (funcName == "int") {
                // Handle int() function
                if (args->isTuple()) {
                    const auto& tuple = dynamic_cast<const TupleValue&>(*args);
                    if (tuple.size() == 0) {
                        return makeInt(0);
                    } else if (tuple.size() == 1) {
                        const auto& arg = tuple.at(0);
                        if (arg->isInt()) return arg;
                        if (arg->isFloat()) return makeInt(arg->asInt());
                        if (arg->isBool()) return makeInt(arg->asInt());
                        if (arg->isString()) {
                            try {
                                return makeInt(dynamic_cast<const StringValue&>(*arg).getValue());
                            } catch (const std::exception& e) {
                                throw std::runtime_error("invalid literal for int()");
                            }
                        }
                    }
                }
                return makeInt(0);
            } else if (funcName == "float") {
                // Handle float() function
                if (args->isTuple()) {
                    const auto& tuple = dynamic_cast<const TupleValue&>(*args);
                    if (tuple.size() == 0) {
                        return makeFloat(0.0);
                    } else if (tuple.size() == 1) {
                        const auto& arg = tuple.at(0);
                        if (arg->isFloat()) return arg;
                        if (arg->isInt()) return makeFloat(arg->asFloat());
                        if (arg->isBool()) return makeFloat(arg->asFloat());
                        if (arg->isString()) {
                            try {
                                return makeFloat(std::stod(dynamic_cast<const StringValue&>(*arg).getValue()));
                            } catch (const std::exception& e) {
                                throw std::runtime_error("could not convert string to float");
                            }
                        }
                    }
                }
                return makeFloat(0.0);
            } else if (funcName == "str") {
                // Handle str() function
                if (args->isTuple()) {
                    const auto& tuple = dynamic_cast<const TupleValue&>(*args);
                    if (tuple.size() == 0) {
                        return makeString("");
                    } else if (tuple.size() == 1) {
                        return makeString(tuple.at(0)->toString());
                    }
                }
                return makeString("");
            } else if (funcName == "bool") {
                // Handle bool() function
                if (args->isTuple()) {
                    const auto& tuple = dynamic_cast<const TupleValue&>(*args);
                    if (tuple.size() == 0) {
                        return makeBool(false);
                    } else if (tuple.size() == 1) {
                        return makeBool(tuple.at(0)->asBool());
                    }
                }
                return makeBool(false);
            }
        }
        // For user-defined functions, we would need to implement proper function execution
        return makeNone();
    }

    return atom;
}

// Atomic values
std::any EvalVisitor::visitAtom(Python3Parser::AtomContext *ctx) {
    if (ctx->NUMBER()) {
        std::string numStr = ctx->NUMBER()->getText();
        if (numStr.find('.') != std::string::npos || numStr.find('e') != std::string::npos ||
            numStr.find('E') != std::string::npos) {
            return makeFloat(std::stod(numStr));
        } else {
            return makeInt(numStr);
        }
    } else if (ctx->STRING().size() > 0) {
        std::string result;
        for (auto strToken : ctx->STRING()) {
            std::string str = strToken->getText();
            // Remove quotes
            str = str.substr(1, str.length() - 2);
            result += str;
        }
        return makeString(result);
    } else if (ctx->NAME()) {
        std::string name = ctx->NAME()->getText();
        return currentScope->getVariable(name);
    } else if (ctx->NONE()) {
        return makeNone();
    } else if (ctx->TRUE()) {
        return makeBool(true);
    } else if (ctx->FALSE()) {
        return makeBool(false);
    } else if (ctx->test()) {
        return visit(ctx->test());
    } else if (ctx->format_string()) {
        return visit(ctx->format_string());
    }

    throw std::runtime_error("Unknown atom type");
}

// Testlist (comma-separated expressions, for tuples)
std::any EvalVisitor::visitTestlist(Python3Parser::TestlistContext *ctx) {
    if (ctx->test().size() == 1 && !ctx->COMMA(0)) {
        return visit(ctx->test(0));
    }

    // Create tuple
    ValueList elements;
    for (auto test : ctx->test()) {
        elements.push_back(visitContext(test));
    }
    return makeTuple(elements);
}

// Function definition (stub for now)
std::any EvalVisitor::visitFuncdef(Python3Parser::FuncdefContext *ctx) {
    std::string funcName = ctx->NAME()->getText();
    // For now, just store a placeholder function value
    // In a full implementation, we'd store the function body and parameters
    auto func = makeFunction(funcName, std::vector<std::string>(), false);
    currentScope->setVariable(funcName, func);
    return std::any();
}

std::any EvalVisitor::visitParameters(Python3Parser::ParametersContext *ctx) {
    return std::any();
}

std::any EvalVisitor::visitTypedargslist(Python3Parser::TypedargslistContext *ctx) {
    return std::any();
}

std::any EvalVisitor::visitTfpdef(Python3Parser::TfpdefContext *ctx) {
    return std::any();
}

// Function arguments
std::any EvalVisitor::visitArglist(Python3Parser::ArglistContext *ctx) {
    // Return a tuple of argument values
    ValueList args;
    for (auto arg : ctx->argument()) {
        args.push_back(visitContext(arg));
    }
    return makeTuple(args);
}

std::any EvalVisitor::visitArgument(Python3Parser::ArgumentContext *ctx) {
    if (ctx->test().size() == 1) {
        return visit(ctx->test(0));
    } else if (ctx->test().size() == 2) {
        // Keyword argument
        return visit(ctx->test(1));  // Return the value
    }
    return std::any();
}

// Format strings (f-strings)
std::any EvalVisitor::visitFormat_string(Python3Parser::Format_stringContext *ctx) {
    std::string result;

    // Build the string from format string components
    auto fmtLiterals = ctx->FORMAT_STRING_LITERAL();
    auto testlists = ctx->testlist();

    size_t literalIndex = 0;
    size_t testIndex = 0;

    // First add the initial literal part
    if (literalIndex < fmtLiterals.size()) {
        std::string lit = fmtLiterals[literalIndex]->getText();
        lit = lit.substr(1, lit.length() - 2);  // Remove quotes
        result += lit;
        literalIndex++;
    }

    // Process expressions and their following literals
    while (testIndex < testlists.size()) {
        // Evaluate the expression
        auto value = visitContext(testlists[testIndex]);
        result += value->toString();
        testIndex++;

        // Add the following literal part
        if (literalIndex < fmtLiterals.size()) {
            std::string lit = fmtLiterals[literalIndex]->getText();
            lit = lit.substr(1, lit.length() - 2);  // Remove quotes
            result += lit;
            literalIndex++;
        }
    }

    return makeString(result);
}