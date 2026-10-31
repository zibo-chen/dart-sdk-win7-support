// Copyright (c) 2023, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#include "platform/unwinding_records.h"

#include "platform/assert.h"
#include "platform/globals.h"

namespace dart {

#if defined(NEED_WINDOWS_UNWINDING_RECORDS)

#if defined(TARGET_ARCH_X64)
const intptr_t kReservedUnwindingRecordsSizeBytes = 64;
#elif defined(TARGET_ARCH_ARM64)
const intptr_t kReservedUnwindingRecordsSizeBytes = 4 * KB;
#else
#error Unhandled Windows architecture.
#endif

intptr_t UnwindingRecordsPlatform::SizeInBytes() {
  return kReservedUnwindingRecordsSizeBytes;
}

#endif  // defined(NEED_WINDOWS_UNWINDING_RECORDS)

// Only use these definitions when the ELF loader may be used on 64-bit Windows,
// as it is the only client of these methods (e.g., _not_ in gen_snapshot).
#if defined(UNWINDING_RECORDS_WINDOWS_HOST)

namespace {

using AddGrowableTable = decltype(&RtlAddGrowableFunctionTable);
using DeleteGrowableTable = decltype(&RtlDeleteGrowableFunctionTable);

const AddGrowableTable add_growable_table = reinterpret_cast<AddGrowableTable>(
    GetProcAddress(GetModuleHandleW(L"ntdll.dll"),
                   "RtlAddGrowableFunctionTable"));
const DeleteGrowableTable delete_growable_table =
    reinterpret_cast<DeleteGrowableTable>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"),
                       "RtlDeleteGrowableFunctionTable"));

struct RegisteredFunctionTable {
  void* growable = nullptr;
  PRUNTIME_FUNCTION fixed = nullptr;
};

}  // namespace

void UnwindingRecordsPlatform::RegisterExecutableMemory(
    void* start,
    intptr_t size,
    void** pp_dynamic_table) {
  intptr_t unwinding_record_offset = size - kReservedUnwindingRecordsSizeBytes;
  uint8_t* record_ptr = static_cast<uint8_t*>(start) + unwinding_record_offset;
  RegisterExecutableMemory(start, size, record_ptr, pp_dynamic_table);
}

void UnwindingRecordsPlatform::RegisterExecutableMemory(
    void* start,
    intptr_t size,
    void* records_start,
    void** pp_dynamic_table) {
  CodeRangeUnwindingRecord* record =
      reinterpret_cast<CodeRangeUnwindingRecord*>(records_start);
  RELEASE_ASSERT(record->magic == kUnwindingRecordMagic);
  uword start_num = reinterpret_cast<intptr_t>(start);
  uword end_num = start_num + size;
  auto* table = new RegisteredFunctionTable();
  auto* functions =
      reinterpret_cast<PRUNTIME_FUNCTION>(record->runtime_function);
  if (add_growable_table != nullptr && delete_growable_table != nullptr) {
    DWORD status = add_growable_table(
        &table->growable, functions, record->runtime_function_count,
        record->runtime_function_count, start_num, end_num);
    if (status != 0) {
      FATAL("Failed to add growable function table: 0x%x\n", status);
    }
  } else {
    // The complete entry array is known at registration time. Windows 7's
    // fixed-size API can describe the same range without a growable table.
    table->fixed = functions;
    if (!RtlAddFunctionTable(functions, record->runtime_function_count,
                             start_num)) {
      FATAL("Failed to add function table: 0x%x\n", GetLastError());
    }
  }
  *pp_dynamic_table = table;
}

void UnwindingRecordsPlatform::UnregisterDynamicTable(void* p_dynamic_table) {
  auto* table = static_cast<RegisteredFunctionTable*>(p_dynamic_table);
  if (table->fixed != nullptr) {
    RELEASE_ASSERT(RtlDeleteFunctionTable(table->fixed));
  } else {
    delete_growable_table(table->growable);
  }
  delete table;
}

#endif  // defined(UNWINDING_RECORDS_WINDOWS_HOST)

}  // namespace dart
