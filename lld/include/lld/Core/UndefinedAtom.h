//===- Core/UndefinedAtom.h - An Undefined Atom ---------------------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLD_CORE_UNDEFINED_ATOM_H
#define LLD_CORE_UNDEFINED_ATOM_H

#include "lld/Core/Atom.h"

namespace lld {

/// An UndefinedAtom has no content.
/// It exists as a place holder for a future atom.
class UndefinedAtom : public Atom {
public:
  /// Whether this undefined symbol needs to be resolved,
  /// or whether it can just evaluate to nullptr.
  /// This concept is often called "weak", but that term
  /// is overloaded to mean other things too.
  enum CanBeNull {
    /// Normal symbols must be resolved at build time
    canBeNullNever,

    /// This symbol can be missing at runtime and will evalute to nullptr.
    /// That is, the static linker still must find a definition (usually
    /// is some shared library), but at runtime, the dynamic loader
    /// will allow the symbol to be missing and resolved to nullptr.
    ///
    /// On Darwin this is generated using a function prototype with
    /// __attribute__((weak_import)).
    /// On linux this is generated using a function prototype with
    ///  __attribute__((weak)).
    /// On Windows this feature is not supported.
    canBeNullAtRuntime,

    /// This symbol can be missing at build time.
    /// That is, the static linker will not error if a definition for
    /// this symbol is not found at build time. Instead, the linker
    /// will build an executable that lets the dynamic loader find the
    /// symbol at runtime.
    /// This feature is not supported on Darwin nor Windows.
    /// On linux this is generated using a function prototype with
    ///  __attribute__((weak)).
    canBeNullAtBuildtime
  };

  virtual CanBeNull canBeNull() const = 0;

  static inline bool classof(const Atom *a) {
    return a->definition() == definitionUndefined;
  }
  static inline bool classof(const UndefinedAtom *) { return true; }

  /// Returns an undefined atom if this undefined symbol has a synonym.  This is
  /// mainly used in COFF. In COFF, an unresolved external symbol can have up to
  /// one optional name (sym2) in addition to its regular name (sym1). If a
  /// definition of sym1 exists, sym1 is resolved normally. Otherwise, all
  /// references to sym1 refer to sym2 instead. In that case sym2 must be
  /// resolved, or link will fail.
  virtual const UndefinedAtom *fallback() const { return nullptr; }

protected:
  UndefinedAtom() : Atom(definitionUndefined) {}
  virtual ~UndefinedAtom() {}
};

} // namespace lld

#endif // LLD_CORE_UNDEFINED_ATOM_H
