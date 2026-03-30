# FM26XStoreUnlocker

DLC unlocker for Football Manager 26 (Xbox/XGP Version). Works by using a VEH hardware breakpoint hook on IL2CPP GameAssembly to bypass Denuvo and XStore API validations.

Forked from the original XStoreUnlocker project by Zephkek: [Zephkek/XStoreUnlocker](https://github.com/Zephkek/XStoreUnlocker).

## Install

You need three files in your **Football Manager 26** game folder:

| File | What it is |
|------|------------|
| `XGameRuntime.dll` | Our proxy DLL (from the release zip) |
| `XGameRuntime_o.dll` | The real Xbox runtime (you provide this) |
| `xstore_unlocker.ini` | Config file (from the release zip, or auto-created on first launch) |

Steps:

1. Copy `XGameRuntime.dll` from `C:\Windows\System32` into your FM26 game folder
2. Rename it to `XGameRuntime_o.dll`
3. Drop our `XGameRuntime.dll` and `xstore_unlocker.ini` from the release zip into the same folder
4. Launch the game

Your game folder should look like this:

```
Football Manager 26/
  fm26.exe
  GameAssembly.dll
  XGameRuntime.dll          <- ours (proxy)
  XGameRuntime_o.dll        <- the original from System32
  xstore_unlocker.ini       <- config
```

Check `xstore_unlocker.log` in the game folder to confirm the VEH hook was placed successfully on `GameAssembly.dll`.

## Configuration

The INI file is created automatically on first launch if missing.

### [Settings]

| Key | Default | Description |
|-----|---------|-------------|
| `log_enabled` | 1 | Writes to `xstore_unlocker.log` and OutputDebugString. Set to 0 for silent operation. |

## Tested Games

| Game | Status | Notes |
|------|--------|-------|
| Football Manager 26 (XGP) | Working | Bypasses Denuvo validation via GameAssembly VEH Hooks |

## Building From Source

Requires Visual Studio 2022+ with C++ desktop workload and CMake.

```bash
git clone git@github.com:Rain-31/FM26XStoreUnlocker.git
cd FM26XStoreUnlocker
cmake -A x64 -B build
cmake --build build --config Release
```

Output: `build/Release/XGameRuntime.dll`

## Legal

This software is provided for educational and research purposes only. Use it at your own risk. The author is not responsible for any consequences of using this tool. Do not use this tool to obtain content you do not have the right to access. Support the developers by purchasing games and DLCs.
