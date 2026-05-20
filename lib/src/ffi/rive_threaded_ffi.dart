import 'dart:ffi';
import 'dart:isolate';

import 'package:ffi/ffi.dart';
import 'package:rive_native/src/ffi/dynamic_library_helper.dart';

final DynamicLibrary _lib = DynamicLibraryHelper.nativeLib;

// ---------------------------------------------------------------------------
// Native function typedefs
// ---------------------------------------------------------------------------

// Lifecycle
typedef _CreateNative =
    Pointer<Void> Function(
      Pointer<Void> metalTextureRenderer,
      Pointer<Void> artboard,
      Pointer<Void> stateMachine,
      Pointer<Void> viewModelInstance,
      Int32 width,
      Int32 height,
      Float devicePixelRatio,
      Int32 fit,
      Float alignmentX,
      Float alignmentY,
      // Target FPS for the bg worker's self-paced render loop. 0 = legacy
      // postElapsedTime-driven mode (worker waits up to 100 ms on the wake
      // CV and uses externally-posted dt). > 0 = worker self-paces at this
      // rate using steady_clock dt, decoupled from UI ticker rate.
      Float targetFps,
    );
typedef _CreateDart =
    Pointer<Void> Function(
      Pointer<Void> metalTextureRenderer,
      Pointer<Void> artboard,
      Pointer<Void> stateMachine,
      Pointer<Void> viewModelInstance,
      int width,
      int height,
      double devicePixelRatio,
      int fit,
      double alignmentX,
      double alignmentY,
      double targetFps,
    );

typedef _DestroyNative = Void Function(Pointer<Void> binding);
typedef _DestroyDart = void Function(Pointer<Void> binding);

typedef _ReleaseWrapperNative = Void Function(Pointer<Void> wrapper);
typedef _ReleaseWrapperDart = void Function(Pointer<Void> wrapper);

// Per-frame
typedef _PostTimeNative = Void Function(Pointer<Void> binding, Float dt);
typedef _PostTimeDart = void Function(Pointer<Void> binding, double dt);

typedef _PollEventsNative =
    Int32 Function(
      Pointer<Void> binding,
      Pointer<Pointer<Utf8>> outNames,
      Pointer<Float> outDelays,
      Int32 maxEvents,
    );
typedef _PollEventsDart =
    int Function(
      Pointer<Void> binding,
      Pointer<Pointer<Utf8>> outNames,
      Pointer<Float> outDelays,
      int maxEvents,
    );

// ViewModel inputs
typedef _SetVmStringNative =
    Void Function(
      Pointer<Void> binding,
      Pointer<Utf8> name,
      Pointer<Utf8> value,
    );
typedef _SetVmStringDart =
    void Function(
      Pointer<Void> binding,
      Pointer<Utf8> name,
      Pointer<Utf8> value,
    );

typedef _SetVmNumberNative =
    Void Function(Pointer<Void> binding, Pointer<Utf8> name, Float value);
typedef _SetVmNumberDart =
    void Function(Pointer<Void> binding, Pointer<Utf8> name, double value);

typedef _SetVmBoolNative =
    Void Function(Pointer<Void> binding, Pointer<Utf8> name, Bool value);
typedef _SetVmBoolDart =
    void Function(Pointer<Void> binding, Pointer<Utf8> name, bool value);

typedef _VmNameOnlyNative =
    Void Function(Pointer<Void> binding, Pointer<Utf8> name);
typedef _VmNameOnlyDart =
    void Function(Pointer<Void> binding, Pointer<Utf8> name);

// Snapshot
typedef _AcquireSnapshotNative =
    Int32 Function(
      Pointer<Void> binding,
      Pointer<Pointer<Utf8>> outNames,
      Pointer<Pointer<Utf8>> outValues,
      Pointer<Int32> outTypes,
      Int32 maxProperties,
    );
typedef _AcquireSnapshotDart =
    int Function(
      Pointer<Void> binding,
      Pointer<Pointer<Utf8>> outNames,
      Pointer<Pointer<Utf8>> outValues,
      Pointer<Int32> outTypes,
      int maxProperties,
    );

// Pointer events
typedef _PointerEventNative =
    Void Function(Pointer<Void> binding, Float x, Float y, Int32 pointerId);
typedef _PointerEventDart =
    void Function(Pointer<Void> binding, double x, double y, int pointerId);

// Queries
typedef _IsRunningNative = Bool Function(Pointer<Void> binding);
typedef _IsRunningDart = bool Function(Pointer<Void> binding);

typedef _HasFatalErrorNative = Bool Function(Pointer<Void> binding);
typedef _HasFatalErrorDart = bool Function(Pointer<Void> binding);

typedef _Uint64QueryNative = Uint64 Function(Pointer<Void> binding);
typedef _Uint64QueryDart = int Function(Pointer<Void> binding);

// Push-not-poll
typedef _SubscribePortNative =
    Void Function(Pointer<Void> binding, Int64 port);
typedef _SubscribePortDart = void Function(Pointer<Void> binding, int port);

typedef _UnsubscribePortNative = Void Function(Pointer<Void> binding);
typedef _UnsubscribePortDart = void Function(Pointer<Void> binding);

typedef _InitApiDLNative = IntPtr Function(Pointer<Void> data);
typedef _InitApiDLDart = int Function(Pointer<Void> data);

// Combined snapshot + events (single mutex acquisition)
typedef _AcquireFrameNative =
    Int32 Function(
      Pointer<Void> binding,
      Pointer<Pointer<Utf8>> outPropNames,
      Pointer<Pointer<Utf8>> outPropValues,
      Pointer<Int32> outPropTypes,
      Int32 maxProperties,
      Pointer<Pointer<Utf8>> outEventNames,
      Pointer<Float> outEventDelays,
      Int32 maxEvents,
      Pointer<Int32> outEventCount,
    );
typedef _AcquireFrameDart =
    int Function(
      Pointer<Void> binding,
      Pointer<Pointer<Utf8>> outPropNames,
      Pointer<Pointer<Utf8>> outPropValues,
      Pointer<Int32> outPropTypes,
      int maxProperties,
      Pointer<Pointer<Utf8>> outEventNames,
      Pointer<Float> outEventDelays,
      int maxEvents,
      Pointer<Int32> outEventCount,
    );

// ---------------------------------------------------------------------------
// Resolved function pointers
// ---------------------------------------------------------------------------

final _CreateDart _create = _lib.lookupFunction<_CreateNative, _CreateDart>(
  'riveThreadedCreate',
);

final _DestroyDart _destroy = _lib.lookupFunction<_DestroyNative, _DestroyDart>(
  'riveThreadedDestroy',
);

final _ReleaseWrapperDart _releaseArtboardWrapper = _lib
    .lookupFunction<_ReleaseWrapperNative, _ReleaseWrapperDart>(
      'riveThreadedReleaseArtboardWrapper',
    );

final _ReleaseWrapperDart _releaseStateMachineWrapper = _lib
    .lookupFunction<_ReleaseWrapperNative, _ReleaseWrapperDart>(
      'riveThreadedReleaseStateMachineWrapper',
    );

final _PostTimeDart _postTime = _lib
    .lookupFunction<_PostTimeNative, _PostTimeDart>('riveThreadedPostTime');

final _PollEventsDart _pollEvents = _lib
    .lookupFunction<_PollEventsNative, _PollEventsDart>(
      'riveThreadedPollEvents',
    );

final _SetVmStringDart _setVmEnum = _lib
    .lookupFunction<_SetVmStringNative, _SetVmStringDart>(
      'riveThreadedSetVmEnum',
    );

final _SetVmNumberDart _setVmNumber = _lib
    .lookupFunction<_SetVmNumberNative, _SetVmNumberDart>(
      'riveThreadedSetVmNumber',
    );

final _SetVmBoolDart _setVmBool = _lib
    .lookupFunction<_SetVmBoolNative, _SetVmBoolDart>('riveThreadedSetVmBool');

final _SetVmStringDart _setVmString = _lib
    .lookupFunction<_SetVmStringNative, _SetVmStringDart>(
      'riveThreadedSetVmString',
    );

final _VmNameOnlyDart _fireVmTrigger = _lib
    .lookupFunction<_VmNameOnlyNative, _VmNameOnlyDart>(
      'riveThreadedFireVmTrigger',
    );

final _VmNameOnlyDart _watchProperty = _lib
    .lookupFunction<_VmNameOnlyNative, _VmNameOnlyDart>(
      'riveThreadedWatchProperty',
    );

final _VmNameOnlyDart _unwatchProperty = _lib
    .lookupFunction<_VmNameOnlyNative, _VmNameOnlyDart>(
      'riveThreadedUnwatchProperty',
    );

final _AcquireSnapshotDart _acquireSnapshot = _lib
    .lookupFunction<_AcquireSnapshotNative, _AcquireSnapshotDart>(
      'riveThreadedAcquireSnapshot',
    );

final _PointerEventDart _pointerDown = _lib
    .lookupFunction<_PointerEventNative, _PointerEventDart>(
      'riveThreadedPointerDown',
    );

final _PointerEventDart _pointerMove = _lib
    .lookupFunction<_PointerEventNative, _PointerEventDart>(
      'riveThreadedPointerMove',
    );

final _PointerEventDart _pointerUp = _lib
    .lookupFunction<_PointerEventNative, _PointerEventDart>(
      'riveThreadedPointerUp',
    );

final _PointerEventDart _pointerExit = _lib
    .lookupFunction<_PointerEventNative, _PointerEventDart>(
      'riveThreadedPointerExit',
    );

final _IsRunningDart _isRunning = _lib
    .lookupFunction<_IsRunningNative, _IsRunningDart>('riveThreadedIsRunning');

final _HasFatalErrorDart _hasFatalError = _lib
    .lookupFunction<_HasFatalErrorNative, _HasFatalErrorDart>(
      'riveThreadedHasFatalError',
    );

final _Uint64QueryDart _advanceCount = _lib
    .lookupFunction<_Uint64QueryNative, _Uint64QueryDart>(
      'riveThreadedAdvanceCount',
    );

final _Uint64QueryDart _renderedCount = _lib
    .lookupFunction<_Uint64QueryNative, _Uint64QueryDart>(
      'riveThreadedRenderedCount',
    );

final _Uint64QueryDart _gpuRenderCount = _lib
    .lookupFunction<_Uint64QueryNative, _Uint64QueryDart>(
      'riveThreadedGpuRenderCount',
    );

final _Uint64QueryDart _consecutiveRenderFailures = _lib
    .lookupFunction<_Uint64QueryNative, _Uint64QueryDart>(
      'riveThreadedConsecutiveRenderFailures',
    );

final _HasFatalErrorDart _renderStalled = _lib
    .lookupFunction<_HasFatalErrorNative, _HasFatalErrorDart>(
      'riveThreadedRenderStalled',
    );

final _AcquireFrameDart _acquireFrame = _lib
    .lookupFunction<_AcquireFrameNative, _AcquireFrameDart>(
      'riveThreadedAcquireFrame',
    );

final _SubscribePortDart _subscribePendingPort = _lib
    .lookupFunction<_SubscribePortNative, _SubscribePortDart>(
      'riveThreadedSubscribePendingPort',
    );

final _UnsubscribePortDart _unsubscribePendingPort = _lib
    .lookupFunction<_UnsubscribePortNative, _UnsubscribePortDart>(
      'riveThreadedUnsubscribePendingPort',
    );

final _InitApiDLDart _initDartApiDL = _lib
    .lookupFunction<_InitApiDLNative, _InitApiDLDart>(
      'riveThreadedInitDartApiDL',
    );

/// One-shot Dart Native API DL initialization. The first reference forces
/// the field initializer to run; `Dart_InitializeApiDL` returns 0 on
/// success, -1 on version mismatch.
final bool _dartApiDLInitialized = (() {
  return _initDartApiDL(NativeApi.initializeApiDLData) == 0;
})();

// ---------------------------------------------------------------------------
// Reported event from the state machine
// ---------------------------------------------------------------------------

class RiveThreadedEvent {
  final String name;
  final double secondsDelay;
  const RiveThreadedEvent(this.name, this.secondsDelay);
}

/// Combined result of a single [RiveThreadedBindings.acquireFrame] call.
class ThreadedFrame {
  final List<SnapshotEntry> properties;
  final List<RiveThreadedEvent> events;
  const ThreadedFrame({required this.properties, required this.events});
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

  // Per-controller acquireFrame buffer cache. Allocated lazily on first call,
  // grown if a later call requests a larger cap, freed in [dispose]. acquireFrame
  // is single-threaded per controller (called from the Flutter UI thread only),
  // so no synchronization is needed around the cache. The eventCount slot is
  // allocated at first use and lives until dispose.
  Pointer<Pointer<Utf8>>? _bufPropNames;
  Pointer<Pointer<Utf8>>? _bufPropValues;
  Pointer<Int32>? _bufPropTypes;
  int _bufPropCap = 0;

  Pointer<Pointer<Utf8>>? _bufEventNames;
  Pointer<Float>? _bufEventDelays;
  int _bufEventCap = 0;

  Pointer<Int32>? _bufEventCount;

  RiveThreadedBindings._(this._ptr);

  /// [fit] is the index of the Rive `Fit` enum (matches the C++
  /// `rive::Fit` ordering: 0=fill, 1=contain, 2=cover, 3=fitWidth,
  /// 4=fitHeight, 5=none, 6=scaleDown, 7=layout).
  ///
  /// [alignmentX] / [alignmentY] are the x/y components of a Rive `Alignment`
  /// (each in [-1, 1]; center = (0, 0), topLeft = (-1, -1)).
  static RiveThreadedBindings? create({
    required Pointer<Void> metalTextureRenderer,
    required Pointer<Void> artboard,
    required Pointer<Void> stateMachine,
    required Pointer<Void> viewModelInstance,
    required int width,
    required int height,
    required double devicePixelRatio,
    int fit = 1, // Fit.contain
    double alignmentX = 0.0,
    double alignmentY = 0.0,
    // 0 = legacy: bg worker waits on postElapsedTime from the UI ticker.
    // > 0 = bg worker self-paces at this rate using steady_clock dt, so the
    // render-success SurfaceProducer.scheduleFrame wake can pump the
    // compositor at the configured FPS even when the UI ticker is idle.
    double targetFps = 0.0,
  }) {
    final ptr = _create(
      metalTextureRenderer,
      artboard,
      stateMachine,
      viewModelInstance,
      width,
      height,
      devicePixelRatio,
      fit,
      alignmentX,
      alignmentY,
      targetFps,
    );
    if (ptr == nullptr || ptr.address == 0) return null;
    return RiveThreadedBindings._(ptr);
  }

  static void releaseClaimedArtboard(Pointer<Void> artboard) {
    if (artboard == nullptr || artboard.address == 0) return;
    _releaseArtboardWrapper(artboard);
  }

  static void releaseClaimedStateMachine(Pointer<Void> stateMachine) {
    if (stateMachine == nullptr || stateMachine.address == 0) return;
    _releaseStateMachineWrapper(stateMachine);
  }

  bool get isDisposed => _ptr == null;
  bool get isRunning => _ptr != null && _isRunning(_ptr!);

  /// True after the native render callback marked a fatal EGL/GL error (e.g.
  /// EGL_CONTEXT_LOST, clear/makeRenderer/flush returning false). One-way flag;
  /// once set, the bg worker is permanently halted and a sync remount is the
  /// only recovery.
  bool get hasFatalError => _ptr != null && _hasFatalError(_ptr!);

  /// Total bg-thread cycles completed since this binding was created. Bumped
  /// once per `runOneFrame` (state-machine advance + snapshot + event
  /// collection), regardless of whether the render callback produced an
  /// image. Diverges from [renderedCount] once the render callback no-ops
  /// (zero-size surface, paused worker, damage tracking).
  int get advanceCount => _ptr == null ? 0 : _advanceCount(_ptr!);

  /// Total bg-thread cycles that produced a new RenderImage. Subset of
  /// [advanceCount].
  ///
  /// **On Android this is uniformly 0** — the bg render callback is
  /// GPU-direct (the texture is the output) and always returns
  /// `nullptr`, so the upstream counter never bumps. Use
  /// [gpuRenderCount] on Android instead.
  int get renderedCount => _ptr == null ? 0 : _renderedCount(_ptr!);

  /// Total bg-thread cycles where `clear` + `makeRenderer` + `flush` all
  /// succeeded. The authoritative "frame painted" counter on Android.
  /// Zero on platforms whose render callback returns an in-memory image
  /// (use [renderedCount] there).
  int get gpuRenderCount => _ptr == null ? 0 : _gpuRenderCount(_ptr!);

  /// Consecutive bg cycles where the render callback failed (fatal
  /// short-circuit, or any of clear/makeRenderer/flush returned false).
  /// Resets to 0 on each successful render. Paused cycles don't count
  /// as failures.
  int get consecutiveRenderFailures =>
      _ptr == null ? 0 : _consecutiveRenderFailures(_ptr!);

  /// True after 30+ consecutive failed bg cycles (~500ms at 60Hz).
  /// Distinct from [hasFatalError]: a stall doesn't halt the worker
  /// (the state machine keeps advancing) — it just signals nothing is
  /// being painted. The most common cause on Android is the device's
  /// GL driver not supporting PLS — look for `Rive AndroidRenderTexture:
  /// Renderer (PLS) NOT supported` in logcat at error level.
  bool get renderStalled => _ptr != null && _renderStalled(_ptr!);

  void dispose() {
    if (_ptr != null) {
      _destroy(_ptr!);
      _ptr = null;
    }
    if (_bufPropNames != null) {
      calloc.free(_bufPropNames!);
      calloc.free(_bufPropValues!);
      calloc.free(_bufPropTypes!);
      _bufPropNames = null;
      _bufPropValues = null;
      _bufPropTypes = null;
      _bufPropCap = 0;
    }
    if (_bufEventNames != null) {
      calloc.free(_bufEventNames!);
      calloc.free(_bufEventDelays!);
      _bufEventNames = null;
      _bufEventDelays = null;
      _bufEventCap = 0;
    }
    if (_bufEventCount != null) {
      calloc.free(_bufEventCount!);
      _bufEventCount = null;
    }
  }

  void _ensurePropBuffers(int cap) {
    if (cap <= _bufPropCap && _bufPropNames != null) return;
    if (_bufPropNames != null) {
      calloc.free(_bufPropNames!);
      calloc.free(_bufPropValues!);
      calloc.free(_bufPropTypes!);
    }
    final n = cap < 1 ? 1 : cap;
    _bufPropNames = calloc<Pointer<Utf8>>(n);
    _bufPropValues = calloc<Pointer<Utf8>>(n);
    _bufPropTypes = calloc<Int32>(n);
    _bufPropCap = n;
  }

  void _ensureEventBuffers(int cap) {
    if (cap <= _bufEventCap && _bufEventNames != null) return;
    if (_bufEventNames != null) {
      calloc.free(_bufEventNames!);
      calloc.free(_bufEventDelays!);
    }
    final n = cap < 1 ? 1 : cap;
    _bufEventNames = calloc<Pointer<Utf8>>(n);
    _bufEventDelays = calloc<Float>(n);
    _bufEventCap = n;
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
      events.add(RiveThreadedEvent(namesPtr[i].toDartString(), delaysPtr[i]));
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

    final count = _acquireSnapshot(
      _ptr!,
      namesPtr,
      valuesPtr,
      typesPtr,
      maxProperties,
    );

    final entries = <SnapshotEntry>[];
    for (var i = 0; i < count; i++) {
      final typeInt = typesPtr[i];
      entries.add(
        SnapshotEntry(
          name: namesPtr[i].toDartString(),
          type: SnapshotValueType.values[typeInt.clamp(0, 3)],
          rawValue: valuesPtr[i].toDartString(),
        ),
      );
    }

    calloc.free(namesPtr);
    calloc.free(valuesPtr);
    calloc.free(typesPtr);
    return entries;
  }

  /// Acquires the latest snapshot AND drains queued output events in a single
  /// FFI round-trip. The native side holds the cached-image mutex once for the
  /// snapshot copy and the output queue mutex once for the event drain, so the
  /// caller sees a snapshot and event batch produced by overlapping bg cycles
  /// rather than two bg cycles apart.
  ///
  /// Returns at most [maxProperties] snapshot entries (truncated; current
  /// rig usage caps at 64) and at most [maxEvents] events per call (drain in
  /// a loop if more may be queued — see kDefaultPollCap consumer).
  ThreadedFrame acquireFrame({int maxProperties = 64, int maxEvents = 128}) {
    if (_ptr == null) return const ThreadedFrame(properties: [], events: []);

    _ensurePropBuffers(maxProperties);
    _ensureEventBuffers(maxEvents);
    _bufEventCount ??= calloc<Int32>(1);

    final propNamesPtr = _bufPropNames!;
    final propValuesPtr = _bufPropValues!;
    final propTypesPtr = _bufPropTypes!;
    final eventNamesPtr = _bufEventNames!;
    final eventDelaysPtr = _bufEventDelays!;
    final eventCountPtr = _bufEventCount!;

    final propCount = _acquireFrame(
      _ptr!,
      propNamesPtr,
      propValuesPtr,
      propTypesPtr,
      maxProperties,
      eventNamesPtr,
      eventDelaysPtr,
      maxEvents,
      eventCountPtr,
    );
    final eventCount = eventCountPtr[0];

    final properties = List<SnapshotEntry>.generate(
      propCount,
      (i) {
        final typeInt = propTypesPtr[i];
        return SnapshotEntry(
          name: propNamesPtr[i].toDartString(),
          type: SnapshotValueType.values[typeInt.clamp(0, 3)],
          rawValue: propValuesPtr[i].toDartString(),
        );
      },
      growable: false,
    );
    final events = List<RiveThreadedEvent>.generate(
      eventCount,
      (i) => RiveThreadedEvent(
        eventNamesPtr[i].toDartString(),
        eventDelaysPtr[i],
      ),
      growable: false,
    );

    return ThreadedFrame(properties: properties, events: events);
  }

  // --- Push-not-poll ---

  /// Register [port] so the worker thread posts `1` after each bg cycle that
  /// produced output. Coalesced: at most one outstanding notification per
  /// binding. The notification is cleared by the next [acquireFrame] call.
  ///
  /// Idempotent on resubscribe — the latest port wins and the pending gate
  /// resets so the new port gets a fresh notification on the next produce.
  /// No-op if the binding was already disposed.
  void subscribePendingPort(SendPort port) {
    if (_ptr == null) return;
    // Force the lazy initializer to run in every build mode. assert() is
    // stripped in profile and release, so a `assert(_dartApiDLInitialized,
    // ...)` here would compile out and Dart_InitializeApiDL would never be
    // called — the worker's first Dart_PostInteger_DL would then jump
    // through a null function pointer and SIGSEGV on Thread-16 inside
    // ThreadedScene::runOneFrame.
    if (!_dartApiDLInitialized) {
      throw StateError(
        'riveThreadedInitDartApiDL returned -1 (Dart API version mismatch)',
      );
    }
    _subscribePendingPort(_ptr!, port.nativePort);
  }

  /// Stop posting to the previously-registered port. Safe to call multiple
  /// times. A post already in flight may still be delivered to the
  /// (now-closed) ReceivePort, which silently drops it.
  void unsubscribePendingPort() {
    if (_ptr == null) return;
    _unsubscribePendingPort(_ptr!);
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
