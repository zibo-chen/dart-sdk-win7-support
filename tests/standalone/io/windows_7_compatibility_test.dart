// Copyright (c) 2026, the Dart project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'dart:convert';
import 'dart:io';
import 'dart:isolate';

// Run with the matching source-built Dart SDK, including on Windows 7 SP1.
Future<void> main() async {
  if (!Platform.isWindows) return;
  if (Platform.localHostname.isEmpty) throw StateError('Empty hostname');
  final directory = await Directory.systemTemp.createTemp('dart-win7-');
  try {
    final target = await Directory('${directory.path}/中文').create();
    await File('${target.path}/probe.txt').writeAsString('文件与相对链接');
    final child = await Directory('${directory.path}/child').create();
    final link = await Link('${child.path}/relative').create('../中文');
    final result = await File('${link.path}/probe.txt').readAsString();
    if (result != '文件与相对链接') throw StateError('Relative directory link failed');
    final longDirectory = await Directory(
      '${directory.path}/${List.filled(6, 'segment-0123456789012345678901234567890123456789').join('/')}',
    ).create(recursive: true);
    final longFile = File('${longDirectory.path}/长路径.txt');
    await longFile.writeAsString('long-path-ok');
    if (await longFile.readAsString() != 'long-path-ok') {
      throw StateError('Long path failed');
    }
    final stack = await Isolate.run(() {
      try {
        throw StateError('exception-probe');
      } catch (_, stack) {
        return '$stack';
      }
    });
    if (stack.isEmpty) throw StateError('Missing isolate exception stack');
    stdout.writeln(jsonEncode({'passed': true, 'dart': Platform.version}));
  } finally {
    await directory.delete(recursive: true);
  }
}
