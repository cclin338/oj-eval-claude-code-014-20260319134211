#pragma once
#ifndef PYTHON_INTERPRETER_SCOPE_H
#define PYTHON_INTERPRETER_SCOPE_H

#include "Value.h"
#include <unordered_map>
#include <string>
#include <memory>

class Scope {
private:
    std::unordered_map<std::string, ValuePtr> variables;
    std::shared_ptr<Scope> parent;
    std::string name;

public:
    Scope(const std::string& scopeName = "global", std::shared_ptr<Scope> parentScope = nullptr)
        : name(scopeName), parent(parentScope) {}

    // Variable operations
    void setVariable(const std::string& name, ValuePtr value) {
        variables[name] = value;
    }

    ValuePtr getVariable(const std::string& name) const {
        auto it = variables.find(name);
        if (it != variables.end()) {
            return it->second;
        }
        // Check parent scope (for global variables)
        if (parent) {
            return parent->getVariable(name);
        }
        throw std::runtime_error("name '" + name + "' is not defined");
    }

    bool hasVariable(const std::string& name) const {
        auto it = variables.find(name);
        if (it != variables.end()) {
            return true;
        }
        if (parent) {
            return parent->hasVariable(name);
        }
        return false;
    }

    void deleteVariable(const std::string& name) {
        variables.erase(name);
    }

    // Get all variables in this scope
    std::unordered_map<std::string, ValuePtr> getAllVariables() const {
        return variables;
    }

    // Get parent scope
    std::shared_ptr<Scope> getParent() const {
        return parent;
    }

    // Get scope name
    std::string getName() const {
        return name;
    }

    // Clear all variables in this scope
    void clear() {
        variables.clear();
    }

    // Check if this is the global scope
    bool isGlobal() const {
        return parent == nullptr;
    }
};

using ScopePtr = std::shared_ptr<Scope>;

#endif//PYTHON_INTERPRETER_SCOPE_H