//===- lib/ReaderWriter/MachO/MachOLinkingContext.cpp ---------------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lld/ReaderWriter/MachOLinkingContext.h"
#include "GOTPass.hpp"
#include "StubsPass.hpp"
#include "ReferenceKinds.h"

#include "lld/Core/PassManager.h"
#include "lld/ReaderWriter/Reader.h"
#include "lld/ReaderWriter/Writer.h"
#include "lld/Passes/LayoutPass.h"
#include "lld/Passes/RoundTripNativePass.h"
#include "lld/Passes/RoundTripYAMLPass.h"

#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/MachO.h"

using lld::mach_o::KindHandler;
using namespace llvm::MachO;

namespace lld {

bool MachOLinkingContext::parsePackedVersion(StringRef str, uint32_t &result) {
  result = 0;

  if (str.empty())
    return false;

  SmallVector<StringRef, 3> parts;
  llvm::SplitString(str, parts, ".");

  unsigned long long num;
  if (llvm::getAsUnsignedInteger(parts[0], 10, num))
    return true;
  if (num > 65535)
    return true;
  result = num << 16;

  if (parts.size() > 1) {
    if (llvm::getAsUnsignedInteger(parts[1], 10, num))
      return true;
    if (num > 255)
      return true;
    result |= (num << 8);
  }

  if (parts.size() > 2) {
    if (llvm::getAsUnsignedInteger(parts[2], 10, num))
      return true;
    if (num > 255)
      return true;
    result |= num;
  }

  return false;
}


MachOLinkingContext::ArchInfo MachOLinkingContext::_s_archInfos[] = {
  { "x86_64", arch_x86_64, true,  CPU_TYPE_X86_64,  CPU_SUBTYPE_X86_64_ALL },
  { "i386",   arch_x86,    true,  CPU_TYPE_I386,    CPU_SUBTYPE_X86_ALL },
  { "ppc",    arch_ppc,    false, CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_ALL },
  { "armv6",  arch_armv6,  true,  CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V6 },
  { "armv7",  arch_armv7,  true,  CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V7 },
  { "armv7s", arch_armv7s, true,  CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V7S },
  { "",       arch_unknown,false, 0,                0 }
};

MachOLinkingContext::Arch
MachOLinkingContext::archFromCpuType(uint32_t cputype, uint32_t cpusubtype) {
  for (ArchInfo *info = _s_archInfos; !info->archName.empty(); ++info) {
    if ((info->cputype == cputype) && (info->cpusubtype == cpusubtype))
      return info->arch;
  }
  return arch_unknown;
}

MachOLinkingContext::Arch
MachOLinkingContext::archFromName(StringRef archName) {
  for (ArchInfo *info = _s_archInfos; !info->archName.empty(); ++info) {
    if (info->archName.equals(archName))
      return info->arch;
  }
  return arch_unknown;
}

uint32_t MachOLinkingContext::cpuTypeFromArch(Arch arch) {
  assert(arch != arch_unknown);
  for (ArchInfo *info = _s_archInfos; !info->archName.empty(); ++info) {
    if (info->arch == arch)
      return info->cputype;
  }
  llvm_unreachable("Unknown arch type");
}

uint32_t MachOLinkingContext::cpuSubtypeFromArch(Arch arch) {
  assert(arch != arch_unknown);
  for (ArchInfo *info = _s_archInfos; !info->archName.empty(); ++info) {
    if (info->arch == arch)
      return info->cpusubtype;
  }
  llvm_unreachable("Unknown arch type");
}

MachOLinkingContext::MachOLinkingContext()
    : _outputFileType(MH_EXECUTE), _outputFileTypeStatic(false),
      _doNothing(false), _arch(arch_unknown), _os(OS::macOSX), _osMinVersion(0),
      _pageZeroSize(unspecifiedPageZeroSize), 
      _pageSize(4096), 
      _compatibilityVersion(0), _currentVersion(0),
      _deadStrippableDylib(false), _kindHandler(nullptr) {}

MachOLinkingContext::~MachOLinkingContext() {}

uint32_t MachOLinkingContext::getCPUType() const {
  return cpuTypeFromArch(_arch);
}

uint32_t MachOLinkingContext::getCPUSubType() const {
  return cpuSubtypeFromArch(_arch);
}

bool MachOLinkingContext::is64Bit(Arch arch) {
  for (ArchInfo *info = _s_archInfos; !info->archName.empty(); ++info) {
    if (info->arch == arch) {
      return (info->cputype & CPU_ARCH_ABI64);
    }
  }
  // unknown archs are not 64-bit.
  return false;
}

bool MachOLinkingContext::isHostEndian(Arch arch) {
  assert(arch != arch_unknown);
  for (ArchInfo *info = _s_archInfos; !info->archName.empty(); ++info) {
    if (info->arch == arch) {
      return (info->littleEndian == llvm::sys::IsLittleEndianHost);
    }
  }
  llvm_unreachable("Unknown arch type");
}

bool MachOLinkingContext::isBigEndian(Arch arch) {
  assert(arch != arch_unknown);
  for (ArchInfo *info = _s_archInfos; !info->archName.empty(); ++info) {
    if (info->arch == arch) {
      return ! info->littleEndian;
    }
  }
  llvm_unreachable("Unknown arch type");
}



bool MachOLinkingContext::is64Bit() const {
  return is64Bit(_arch);
}

bool MachOLinkingContext::outputTypeHasEntry() const {
  switch (_outputFileType) {
  case MH_EXECUTE:
  case MH_DYLINKER:
  case MH_PRELOAD:
    return true;
  default:
    return false;
  }
}

bool MachOLinkingContext::minOS(StringRef mac, StringRef iOS) const {
  uint32_t parsedVersion;
  switch (_os) {
  case OS::macOSX:
    if (parsePackedVersion(mac, parsedVersion))
      return false;
    return _osMinVersion >= parsedVersion;
  case OS::iOS:
  case OS::iOS_simulator:
    if (parsePackedVersion(iOS, parsedVersion))
      return false;
    return _osMinVersion >= parsedVersion;
  case OS::unknown:
    break;
  }
  llvm_unreachable("target not configured for iOS or MacOSX");
}

bool MachOLinkingContext::addEntryPointLoadCommand() const {
  if ((_outputFileType == MH_EXECUTE) && !_outputFileTypeStatic) {
    return minOS("10.8", "6.0");
  }
  return false;
}

bool MachOLinkingContext::addUnixThreadLoadCommand() const {
  switch (_outputFileType) {
  case MH_EXECUTE:
    if (_outputFileTypeStatic)
      return true;
    else
      return !minOS("10.8", "6.0");
    break;
  case MH_DYLINKER:
  case MH_PRELOAD:
    return true;
  default:
    return false;
  }
}

bool MachOLinkingContext::validateImpl(raw_ostream &diagnostics) {
  if ((_outputFileType == MH_EXECUTE) && _entrySymbolName.empty()){
    if (_outputFileTypeStatic) {
      _entrySymbolName = "start";
    } else if (addUnixThreadLoadCommand()) {
      // If targeting older OS, use start (in crt1.o)
      _entrySymbolName = "start";
    } else if (addEntryPointLoadCommand()) {
      // If targeting newer OS, use _main
      _entrySymbolName = "_main";
    }
  }

  // TODO: if -arch not specified, look at arch of first .o file.

  // Set default __PAGEZERO for main executables
  if ((_outputFileType == MH_EXECUTE) && !_outputFileTypeStatic
                                && (_pageZeroSize == unspecifiedPageZeroSize)) {
    if (is64Bit(_arch))
      _pageZeroSize = 0x100000000;
    else
      _pageZeroSize = 0x00010000;
  }

  if (_currentVersion && _outputFileType != MH_DYLIB) {
    diagnostics << "error: -current_version can only be used with dylibs\n";
    return false;
  }

  if (_compatibilityVersion && _outputFileType != MH_DYLIB) {
    diagnostics
        << "error: -compatibility_version can only be used with dylibs\n";
    return false;
  }

  if (_deadStrippableDylib && _outputFileType != MH_DYLIB) {
    diagnostics
        << "error: -mark_dead_strippable_dylib can only be used with dylibs.\n";
    return false;
  }

  if (!_bundleLoader.empty() && outputFileType() != MH_BUNDLE) {
    diagnostics
        << "error: -bundle_loader can only be used with Mach-O bundles\n";
    return false;
  }

  return true;
}

bool MachOLinkingContext::setOS(OS os, StringRef minOSVersion) {
  _os = os;
  return parsePackedVersion(minOSVersion, _osMinVersion);
}

void MachOLinkingContext::addPasses(PassManager &pm) {
  if (outputFileType() != MH_OBJECT) {
    pm.add(std::unique_ptr<Pass>(new mach_o::GOTPass));
    pm.add(std::unique_ptr<Pass>(new mach_o::StubsPass(*this)));
  }
  pm.add(std::unique_ptr<Pass>(new LayoutPass()));
}

Writer &MachOLinkingContext::writer() const {
  if (!_writer) {
    _writer = createWriterMachO(*this);
  }
  return *_writer;
}

KindHandler &MachOLinkingContext::kindHandler() const {
  if (!_kindHandler)
    _kindHandler = KindHandler::create(_arch);
  return *_kindHandler;
}

ErrorOr<Reference::Kind>
MachOLinkingContext::relocKindFromString(StringRef str) const {
  return kindHandler().stringToKind(str);
}

ErrorOr<std::string>
MachOLinkingContext::stringFromRelocKind(Reference::Kind kind) const {
  return std::string(kindHandler().kindToString(kind));
}

} // end namespace lld
