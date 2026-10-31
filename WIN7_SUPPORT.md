# Experimental Windows 7 runtime support

Base: `fc3da898ea1664b8a8633f6ec03d2c2290bb3d01`, the Dart revision pinned by
Flutter 3.44.1. This branch is a companion to
`zibo-chen/flutter-win7-support`, branch `medivh/win7-3.44.1`.

Status: source changes prepared; Windows compilation and Windows 7 runtime
acceptance are pending. This is not a supported Dart SDK distribution.

Changes retain the modern Windows behavior when its API is available:

- Resolve `GetHostNameW` dynamically; fall back to `GetComputerNameExW` for
  Unicode hostnames on Win7.
- Resolve `GetCurrentThreadStackLimits` dynamically; use the older Dart
  TIB/VirtualQuery implementation on Win7. The reservation boundary, rather
  than only the committed stack boundary, is required.
- Resolve growable unwind tables dynamically; use fixed-size
  `RtlAddFunctionTable`/`RtlDeleteFunctionTable` on Win7. Both JIT pages and
  loaded AOT code use the same registration lifecycle.
- Resolve PathCch APIs dynamically from the system directory. On Win7 use
  GetFullPathNameW and extended path prefixes, including relative directory
  symlinks and UNC paths.

The initial target is **Windows 7 SP1 x64 release Flutter applications**. It
does not imply all SDK developer tools, x86, or arbitrary native packages work.

Regression probe:

```
dart tests/standalone/io/windows_7_compatibility_test.dart
```

Run it with the matching source-built SDK on Windows 2022 CI and Win7. Creating
symlinks requires the appropriate Windows privilege. The Flutter companion
also exercises native FFI, rendering, sockets, isolates and Unicode file I/O.
Actual Win7 results and subsequent limitations belong in the companion's
acceptance record. Native camera/inference/printer acceptance is separate.
