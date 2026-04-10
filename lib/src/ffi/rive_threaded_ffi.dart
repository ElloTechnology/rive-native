import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:rive_native/src/ffi/dynamic_library_helper.dart';

final DynamicLibrary _lib = DynamicLibraryHelper.nativeLib;

// ---------------------------------------------------------------------------
// Native function typedefs
// ---------------------------------------------------------------------------

// Lifecycle
typedef _CreateNative = Pointer<Void> Function(
  Pointer<Void> metalTextureRenderer,
  Pointer<Void> artboard,
  Pointer<Void> stateMachine,
  Pointer<Void> viewModelInstance,
  Int32 width,
  Int32 height,
  Float devicePixelRatio,
);
typedef _CreateDart = Pointer<Void> Function(
  Pointer<Void> metalTextureRenderer,
  Pointer<Void> artboard,
  Pointer<Void> stateMachine,
  Pointer<Void> viewModelInstance,
  int width,
  int height,
  double devicePixelRatio,
);

typedef _DestroyNative = Void Function(Pointer<Void> binding);
typedef _DestroyDart = void Function(Pointer<Void> binding);

// Per-frame
typedef _PostTimeNative = Void Function(Pointer<Void> binding, Float dt);
typedef _PostTimeDart = void Function(Pointer<Void> binding, double dt);

typedef _PollEventsNative = Int32 Function(
  Pointer<Void> binding,
  Pointer<Pointer<Utf8>> outNames,
  Pointer<Float> outDelays,
  Int32 maxEvents,
);
typedef _PollEventsDart = int Function(
  Pointer<Void> binding,
  Pointer<Pointer<Utf8>> outNames,
  Pointer<Float> outDelays,
  int maxEvents,
);

// ViewModel inputs
typedef _SetVmStringNative = Void Function(
  Pointer<Void> binding,
  Pointer<Utf8> name,
  Pointer<Utf8> value,
);
typedef _SetVmStringDart = void Function(
  Pointer<Void> binding,
  Pointer<Utf8> name,
  Pointer<Utf8> value,
);

typedef _SetVmNumberNative = Void Function(
  Pointer<Void> binding,
  Pointer<Utf8> name,
  Float value,
);
typedef _SetVmNumberDart = void Function(
  Pointer<Void> binding,
  Pointer<Utf8> name,
  double value,
);

typedef _SetVmBoolNative = Void Function(
  Pointer<Void> binding,
  Pointer<Utf8> name,
  Bool value,
);
typedef _SetVmBoolDart = void Function(
  Pointer<Void> binding,
  Pointer<Utf8> name,
  bool value,
);

typedef _VmNameOnlyNative = Void Function(
  Pointer<Void> binding,
  Pointer<Utf8> name,
);
typedef _VmNameOnlyDart = void Function(
  Pointer<Void> binding,
  Pointer<Utf8> name,
);

// Snapshot
typedef _AcquireSnapshotNative = Int32 Function(
  Pointer<Void> binding,
  Pointer<Pointer<Utf8>> outNames,
  Pointer<Pointer<Utf8>> outValues,
  Pointer<Int32> outTypes,
  Int32 maxProperties,
);
typedef _AcquireSnapshotDart = int Function(
  Pointer<Void> binding,
  Pointer<Pointer<Utf8>> outNames,
  Pointer<Pointer<Utf8>> outValues,
  Pointer<Int32> outTypes,
  int maxProperties,
);

// Pointer events
typedef _PointerEventNative = Void Function(
  Pointer<Void> binding,
  Float x,
  Float y,
  Int32 pointerId,
);
typedef _PointerEventDart = void Function(
  Pointer<Void> binding,
  double x,
  double y,
  int pointerId,
);

// Queries
typedef _IsRunningNative = Bool Function(Pointer<Void> binding);
typedef _IsRunningDart = bool Function(Pointer<Void> binding);

// ---------------------------------------------------------------------------
// Resolved function pointers
// ---------------------------------------------------------------------------

final _CreateDart _create =
    _lib.lookupFunction<_CreateNative, _CreateDart>('riveThreadedCreate');

final _DestroyDart _destroy =
    _lib.lookupFunction<_DestroyNative, _DestroyDart>('riveThreadedDestroy');

final _PostTimeDart _postTime =
    _lib.lookupFunction<_PostTimeNative, _PostTimeDart>('riveThreadedPostTime');

final _PollEventsDart _pollEvents = _lib
    .lookupFunction<_PollEventsNative, _PollEventsDart>(
        'riveThreadedPollEvents');

final _SetVmStringDart _setVmEnum = _lib
    .lookupFunction<_SetVmStringNative, _SetVmStringDart>(
        'riveThreadedSetVmEnum');

final _SetVmNumberDart _setVmNumber = _lib
    .lookupFunction<_SetVmNumberNative, _SetVmNumberDart>(
        'riveThreadedSetVmNumber');

final _SetVmBoolDart _setVmBool =
    _lib.lookupFunction<_SetVmBoolNative, _SetVmBoolDart>(
        'riveThreadedSetVmBool');

final _SetVmStringDart _setVmString = _lib
    .lookupFunction<_SetVmStringNative, _SetVmStringDart>(
        'riveThreadedSetVmString');

final _VmNameOnlyDart _fireVmTrigger = _lib
    .lookupFunction<_VmNameOnlyNative, _VmNameOnlyDart>(
        'riveThreadedFireVmTrigger');

final _VmNameOnlyDart _watchProperty = _lib
    .lookupFunction<_VmNameOnlyNative, _VmNameOnlyDart>(
        'riveThreadedWatchProperty');

final _VmNameOnlyDart _unwatchProperty = _lib
    .lookupFunction<_VmNameOnlyNative, _VmNameOnlyDart>(
        'riveThreadedUnwatchProperty');

final _AcquireSnapshotDart _acquireSnapshot = _lib
    .lookupFunction<_AcquireSnapshotNative, _AcquireSnapshotDart>(
        'riveThreadedAcquireSnapshot');

final _PointerEventDart _pointerDown = _lib
    .lookupFunction<_PointerEventNative, _PointerEventDart>(
        'riveThreadedPointerDown');

final _PointerEventDart _pointerMove = _lib
    .lookupFunction<_PointerEventNative, _PointerEventDart>(
        'riveThreadedPointerMove');

final _PointerEventDart _pointerUp =
    _lib.lookupFunction<_PointerEventNative, _PointerEventDart>(
        'riveThreadedPointerUp');

final _PointerEventDart _pointerExit = _lib
    .lookupFunction<_PointerEventNative, _PointerEventDart>(
        'riveThreadedPointerExit');

final _IsRunningDart _isRunning =
    _lib.lookupFunction<_IsRunningNative, _IsRunningDart>(
        'riveThreadedIsRunning');

// ---------------------------------------------------------------------------
// Reported event from the state machine
// ---------------------------------------------------------------------------

class RiveThreadedEvent {
  final String name;
  final double secondsDelay;
  const RiveThreadedEvent(this.name, this.secondsDelay);
}

// ---------------------------------------------------------------------------
// Snapshot value types
// ---------------------------------------------------------------------------

enum SnapshotValueType { monostate, boolean, number, string }

class SnapshotEntry {
  final String name;
  final SnapshotValueType type;
  final String rawValue;

  const SnapshotEntry({
    required this.name,
    required this.type,
    required this.rawValue,
  });

  bool? get boolValue =>
      type == SnapshotValueType.boolean ? rawValue == 'true' : null;

  double? get numberValue =>
      type == SnapshotValueType.number ? double.tryParse(rawValue) : null;

  String? get stringValue =>
      type == SnapshotValueType.string || type == SnapshotValueType.boolean
          ? rawValue
          : null;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

class RiveThreadedBindings {
  Pointer<Void>? _ptr;

  RiveThreadedBindings._(this._ptr);

  static RiveThreadedBindings? create({
    required Pointer<Void> metalTextureRenderer,
    required Pointer<Void> artboard,
    required Pointer<Void> stateMachine,
    required Pointer<Void> viewModelInstance,
    required int width,
    required int height,
    required double devicePixelRatio,
  }) {
    final ptr = _create(
      metalTextureRenderer,
      artboard,
      stateMachine,
      viewModelInstance,
      width,
      height,
      devicePixelRatio,
    );
    if (ptr == nullptr || ptr.address == 0) return null;
    return RiveThreadedBindings._(ptr);
  }

  bool get isDisposed => _ptr == null;
  bool get isRunning => _ptr != null && _isRunning(_ptr!);

  void dispose() {
    if (_ptr != null) {
      _destroy(_ptr!);
      _ptr = null;
    }
  }

  // --- Per-frame ---

  void postElapsedTime(double dt) {
    if (_ptr == null) return;
    _postTime(_ptr!, dt);
  }

  List<RiveThreadedEvent> pollEvents({int maxEvents = 32}) {
    if (_ptr == null) return const [];
    final namesPtr = calloc<Pointer<Utf8>>(maxEvents);
    final delaysPtr = calloc<Float>(maxEvents);

    final count = _pollEvents(_ptr!, namesPtr, delaysPtr, maxEvents);

    final events = <RiveThreadedEvent>[];
    for (var i = 0; i < count; i++) {
      events.add(RiveThreadedEvent(
        namesPtr[i].toDartString(),
        delaysPtr[i],
      ));
    }

    calloc.free(namesPtr);
    calloc.free(delaysPtr);
    return events;
  }

  // --- ViewModel inputs ---

  void setEnumProperty(String name, String value) {
    if (_ptr == null) return;
    final namePtr = name.toNativeUtf8();
    final valuePtr = value.toNativeUtf8();
    _setVmEnum(_ptr!, namePtr, valuePtr);
    calloc.free(namePtr);
    calloc.free(valuePtr);
  }

  void setNumberProperty(String name, double value) {
    if (_ptr == null) return;
    final namePtr = name.toNativeUtf8();
    _setVmNumber(_ptr!, namePtr, value);
    calloc.free(namePtr);
  }

  void setBoolProperty(String name, bool value) {
    if (_ptr == null) return;
    final namePtr = name.toNativeUtf8();
    _setVmBool(_ptr!, namePtr, value);
    calloc.free(namePtr);
  }

  void setStringProperty(String name, String value) {
    if (_ptr == null) return;
    final namePtr = name.toNativeUtf8();
    final valuePtr = value.toNativeUtf8();
    _setVmString(_ptr!, namePtr, valuePtr);
    calloc.free(namePtr);
    calloc.free(valuePtr);
  }

  void fireTrigger(String name) {
    if (_ptr == null) return;
    final namePtr = name.toNativeUtf8();
    _fireVmTrigger(_ptr!, namePtr);
    calloc.free(namePtr);
  }

  // --- ViewModel snapshot ---

  void watchProperty(String name) {
    if (_ptr == null) return;
    final namePtr = name.toNativeUtf8();
    _watchProperty(_ptr!, namePtr);
    calloc.free(namePtr);
  }

  void unwatchProperty(String name) {
    if (_ptr == null) return;
    final namePtr = name.toNativeUtf8();
    _unwatchProperty(_ptr!, namePtr);
    calloc.free(namePtr);
  }

  List<SnapshotEntry> acquireSnapshot({int maxProperties = 32}) {
    if (_ptr == null) return const [];
    final namesPtr = calloc<Pointer<Utf8>>(maxProperties);
    final valuesPtr = calloc<Pointer<Utf8>>(maxProperties);
    final typesPtr = calloc<Int32>(maxProperties);

    final count =
        _acquireSnapshot(_ptr!, namesPtr, valuesPtr, typesPtr, maxProperties);

    final entries = <SnapshotEntry>[];
    for (var i = 0; i < count; i++) {
      final typeInt = typesPtr[i];
      entries.add(SnapshotEntry(
        name: namesPtr[i].toDartString(),
        type: SnapshotValueType.values[typeInt.clamp(0, 3)],
        rawValue: valuesPtr[i].toDartString(),
      ));
    }

    calloc.free(namesPtr);
    calloc.free(valuesPtr);
    calloc.free(typesPtr);
    return entries;
  }

  // --- Pointer events ---

  void pointerDown(double x, double y, {int pointerId = 0}) {
    if (_ptr == null) return;
    _pointerDown(_ptr!, x, y, pointerId);
  }

  void pointerMove(double x, double y, {int pointerId = 0}) {
    if (_ptr == null) return;
    _pointerMove(_ptr!, x, y, pointerId);
  }

  void pointerUp(double x, double y, {int pointerId = 0}) {
    if (_ptr == null) return;
    _pointerUp(_ptr!, x, y, pointerId);
  }

  void pointerExit(double x, double y, {int pointerId = 0}) {
    if (_ptr == null) return;
    _pointerExit(_ptr!, x, y, pointerId);
  }
}
