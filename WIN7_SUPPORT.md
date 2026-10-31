# Experimental Windows 7 runtime support

Base: `fc3da898ea1664b8a8633f6ec03d2c2290bb3d01`, the Dart revision pinned by
Flutter 3.44.1. This branch is a companion to
`zibo-chen/flutter-win7-support`, branch `medivh/win7-3.44.1`.

Status: source-built Dart 3.12.1 and the Flutter engine compile and link on
Windows 2022. The complete standalone runtime probe passes there in companion
run `41186401142`. A Flutter release app using Dart commit
`a6ed413da4b5690ec3e05f347af1400170d2a2f3` passes first-frame rendering,
Unicode file I/O, hostname, loopback HTTP, isolate exception stacks and FFI on
Windows 7 SP1 x64. Keyboard/button input and normal exit are visually recorded
by the companion's [acceptance evidence](https://github.com/zibo-chen/flutter-win7-support/blob/medivh/win7-3.44.1/win7/evidence/2026-10-31-win7/acceptance.json).

This is not a supported standalone Dart SDK distribution. The full standalone
probe's long-path/symlink cases were run on Windows 2022, not on Win7; the Win7
result above is the Flutter release application. Business plugins, IME, other
hardware and long-running stability remain separate acceptance work.

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
