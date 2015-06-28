/**
 * This file is part of the "FnordMetric" project
 *   Copyright (c) 2014 Paul Asmuth, Google Inc.
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <algorithm>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include "symboltable.h"
#include <fnord/exception.h>

namespace csql {

void SymbolTable::registerFunction(
    const String& symbol,
    void (*fn)(int, SValue*, SValue*)) {
  SFunction sym;
  sym.type = FN_PURE;
  sym.vtable.t_pure.call = fn;
  registerFunction(symbol, sym);
}

void SymbolTable::registerFunction(
    const String& symbol,
    AggregateFunction fn) {
  SFunction sym;
  sym.type = FN_AGGREGATE;
  sym.vtable.t_aggregate.scratch_size = fn.scratch_size;
  sym.vtable.t_aggregate.accumulate = fn.accumulate;
  sym.vtable.t_aggregate.get = fn.get;
  sym.vtable.t_aggregate.reset = fn.reset;
  sym.vtable.t_aggregate.init = fn.init;
  sym.vtable.t_aggregate.free = fn.free;
  registerFunction(symbol, sym);
}

void SymbolTable::registerFunction(const String& symbol, SFunction fn) {
  syms_.emplace(symbol, fn);
}

void SymbolTable::registerSymbol(
    const std::string& symbol,
    void (*method)(void*, int, SValue*, SValue*)) {
  std::string symbol_downcase = symbol;
  std::transform(
      symbol_downcase.begin(),
      symbol_downcase.end(),
      symbol_downcase.begin(),
      ::tolower);

  symbols_.emplace(
      std::make_pair(
          symbol_downcase,
          SymbolTableEntry(symbol_downcase, method)));
}

void SymbolTable::registerSymbol(
    const std::string& symbol,
    void (*method)(void*, int, SValue*, SValue*),
    size_t scratchpad_size,
    void (*free_method)(void*)) {
  std::string symbol_downcase = symbol;
  std::transform(
      symbol_downcase.begin(),
      symbol_downcase.end(),
      symbol_downcase.begin(),
      ::tolower);

  symbols_.emplace(
      std::make_pair(
          symbol_downcase,
          SymbolTableEntry(
              symbol_downcase,
              method,
              scratchpad_size,
              free_method)));
}

SymbolTableEntry const* SymbolTable::lookupSymbol(const std::string& symbol)
    const {
  std::string symbol_downcase = symbol;
  std::transform(
      symbol_downcase.begin(),
      symbol_downcase.end(),
      symbol_downcase.begin(),
      ::tolower);

  auto iter = symbols_.find(symbol_downcase);

  if (iter == symbols_.end()) {
    RAISE(kRuntimeError, "symbol not found: %s", symbol.c_str());
    return nullptr;
  } else {
    return &iter->second;
  }
}

SFunction SymbolTable::lookup(const String& symbol) const {
  auto iter = syms_.find(symbol);

  if (iter == syms_.end()) {
    RAISEF(kRuntimeError, "symbol not found: $0", symbol);
  }

  return iter->second;
}

bool SymbolTable::isAggregateFunction(const String& symbol) const {
  auto sf = lookup(symbol);

  switch (sf.type) {
    case FN_AGGREGATE:
      return true;
    case FN_PURE:
      return false;
  }
}

SymbolTableEntry::SymbolTableEntry(
    const std::string& symbol,
    void (*method)(void*, int, SValue*, SValue*),
    size_t scratchpad_size,
    void (*free_method)(void*)) :
    call_(method),
    scratchpad_size_(scratchpad_size) {}

SymbolTableEntry::SymbolTableEntry(
    const std::string& symbol,
    void (*method)(void*, int, SValue*, SValue*)) :
    SymbolTableEntry(symbol, method, 0, nullptr) {}

bool SymbolTableEntry::isAggregate() const {
  return scratchpad_size_ > 0;
}

void (*SymbolTableEntry::getFnPtr() const)(void*, int, SValue*, SValue*) {
  return call_;
}

size_t SymbolTableEntry::getScratchpadSize() const {
  return scratchpad_size_;
}

}
