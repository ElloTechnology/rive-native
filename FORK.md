# Ello `rive_native` fork

This is Ello's fork of `rive_native` 0.1.6, adding a background-advance path for
Android. The Flutter client (`ElloTechnology/learn`, ticket COR-3538) uses it
to advance and render the character rig on a dedicated C++ thread, dropping the
per-frame UI-thread cost on tier-0/1 Android devices.

## Branch layout

| Branch | Purpose |
|---|---|
| `rive_native-0.1.6-baseline` | Untouched snapshot of the published `rive_native 0.1.6` source. PR base; never gets force-pushed. |
| `ello-2026-05-review` | Active fork tip. All Ello-specific changes land here. **Client pubspec pins this branch.** |
| (other historical branches under `ello-*` are spike forks; ignore them) |

Open PR: [ElloTechnology/rive-native#2](https://github.com/ElloTechnology/rive-native/pull/2) (`ello-2026-05-review` → `rive_native-0.1.6-baseline`).

## What this fork adds on top of `0.1.6`

In rough commit order on `ello-2026-05-review`:

| Area | Commit | Purpose |
|---|---|---|
| Vendored runtime | `8b2f5b8` | Drop in `ThreadedScene` source from `rive-runtime@ello/perf` (later replaced by `rive-runtime@ello/main`). |
| Android binding | `49822ff` | New `native/src/threaded_scene_binding_android.cpp` — Android GLES counterpart of the (since-dropped) iOS Metal binding. |
| Android build wiring | `5da9141`, `c5d3460`, `80f2afa` | Hook the Android binding into the Android build; skip Metal shader build when targeting Android. |
| EGL probe | `e0ac91b` | Shared-EGL-context probe used during the Phase 2 spike to validate cross-thread context sharing. |
| Public API | `d3853d4` | Add `ThreadedScene` Dart-side public entry points. |
| iOS Metal drop | `2b3e1c4` | Remove the iOS Metal binding (Ello's bg-advance is Android-only; iOS keeps the sync path). |
| Fatal-error surface | `93ffd5c` | `riveThreadedHasFatalError` FFI — render callback marks a one-way flag on EGL/GL failure. |
| Pause/resume | `0c9098c` | `riveThreadedSetPaused` FFI — used by `WidgetsBindingObserver` to halt the worker on app backgrounding. |
| GL state save/restore | `e3017bd` | Save/restore viewport, scissor, program, framebuffer, etc. around the bg flush so Impeller doesn't inherit Rive's GL state on its next `eglMakeCurrent`. |
| Null guards | `a30d79a` | `draw()` and `advanceAndApply()` null-guarded after `claimNativeOwnership` nulls the Dart wrappers. |
| First-frame sync flip | `5c7f82e`, `cae3dc3` | `runFirstFrameSync=false` on Android to avoid gfxstream emulator SIGSEGV; rebuild Android `.so` accordingly. |
| Review hygiene | `8135a19` | Drop committed native/out build artifacts. |
| Combined acquire + fatal-OR | `ef5b70e` | Vendor sync from `rive-runtime ello/main` tip; new `riveThreadedAcquireFrame` export (snapshot + events under one mutex); `ThreadedSceneBinding::hasFatalError` ORs binding-level + scene-level flags. |
| FORK.md | _this commit_ | Documentation. |

Current tip: `git log -1 origin/ello-2026-05-review`. The client pubspec pin
should match.

## Relationship to `rive-runtime` and `rive-flutter` forks

`rive_native` vendors a snapshot of `rive-runtime` under `runtime/`. The two
`ThreadedScene` source files in this fork are byte-for-byte mirrors of the
canonical source in the Ello `rive-runtime` fork. When `rive-runtime` changes,
re-sync:

```bash
cp ~/ello/other/rive-runtime/include/rive/threaded_scene.hpp \
   ~/ello/other/rive-native-spike/runtime/include/rive/threaded_scene.hpp
cp ~/ello/other/rive-runtime/src/threaded_scene.cpp \
   ~/ello/other/rive-native-spike/runtime/src/threaded_scene.cpp
```

| Fork | Branch | Open PR |
|---|---|---|
| [`ElloTechnology/rive-runtime`](https://github.com/ElloTechnology/rive-runtime) | `ello/main` | [#1](https://github.com/ElloTechnology/rive-runtime/pull/1) (`ello/main` → `main`) |
| [`ElloTechnology/rive-native`](https://github.com/ElloTechnology/rive-native) | `ello-2026-05-review` | [#2](https://github.com/ElloTechnology/rive-native/pull/2) (`ello-2026-05-review` → `rive_native-0.1.6-baseline`) |
| [`ElloTechnology/rive-flutter`](https://github.com/ElloTechnology/rive-flutter) | `ello-2026-05` | [#2](https://github.com/ElloTechnology/rive-flutter/pull/2) (`ello-2026-05` → `ello`) |

Pinned SHAs change frequently; check each repo's HEAD on the branch above.

`rive-flutter` is the higher-level Dart package — it depends on `rive_native`
and adds `BackgroundRiveWidgetController` + `BackgroundRiveView` on top of the
FFI. The client pulls `rive` (which is the `rive-flutter` package); `rive-flutter`
in turn pulls `rive_native` via `dependency_overrides:` pointing at this fork.

The full dependency chain seen by the client:

```
learn-1 pubspec
  ├── rive (git: ElloTechnology/rive-flutter @ ello-2026-05 → 103095c)
  │     └── dependency_overrides: rive_native (git: ElloTechnology/rive-native @ ef5b70e)
  └── dependency_overrides:
      └── rive_native (git: ElloTechnology/rive-native @ ef5b70e)  # same — kept for SHA pinning
```

Both overrides point at the same `rive-native` SHA. The client's own override is
the source of truth — `rive-flutter`'s override is fallback for when `rive-flutter`
is consumed without an outer override.

## Rebuilding the Android `.so`

The compiled `librive_native.so` for Android lives at
`android/src/main/jniLibs/{arm64-v8a,armeabi-v7a,x86,x86_64}/`. It is **committed
into the fork** and must be rebuilt whenever the C++ source or any FFI export
changes — otherwise the new symbols won't resolve at runtime (Dart's
`lookupFunction` is lazy, so analyze/tests pass even when the .so is stale).

Build script:

```bash
cd ~/ello/other/rive-native-spike
./native/build.sh   # writes to android/src/main/jniLibs/<abi>/
git add android/src/main/jniLibs/
git commit -m "build: rebuild Android .so for <reason>"
git push origin ello-2026-05-review
```

History shows previous rebuilds at `cae3dc3` (last published build) — that one
predates `ef5b70e`, so an Android device build today will hit
`ArgumentError: Failed to lookup symbol 'riveThreadedAcquireFrame'` until the
.so is regenerated.

## Pull-request etiquette

- Don't rebase `ello-2026-05-review` — the open PR (#2) tracks it. Add new
  commits on top.
- Don't touch `rive_native-0.1.6-baseline`. It exists so the PR diff shows
  exactly the Ello delta.
- Keep upstream-mergeable changes (anything in `runtime/`) in lockstep with
  `rive-runtime ello/main`. The vendored copy here is downstream of that fork.

## Spec context

The spec that drove this work is at
`learn-1/specs/2026-05-11-COR-3538-revive-background-rive/` in the client repo.
Start with `summary.md` then `implementation-notes.md` for the phase-by-phase
landing log (Phase 6.5 covers the latest threading hardening pass).
