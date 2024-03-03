# ANDIRPG

Very basic RPG written when I have nothing better to do using only my Android
device for coding. The whole experience is funny!

## DEPENDENCIES

In order to compile you need to have the following libraries installed:
- `libcunit` for the **tests**
- `ncurses` for the UI
- `libinih` for the configuration
- `msgpack-c` for the serialization/deserialization of the game state

## COMPILE AND TEST FROM SOURCE
As stated before, the game has been developed only on Android so in order
to compile it you need to have a working `Termux` environment with the following
packages installed:
- `clang`
- `xmake`

Plus, all the dependencies listed above (you can install them using `pkg`).

Then, you can setup the compilation target using `xmake`, currently two targets
exist: `debug` and `release`. To switch from one target to the other you **must** use
the `xmake f -m <target>` command.

To compile, simply run `xmake` with no arguments, the compilation result will be
in `./build/linux/arm64-v8a/<target>`. To launch the tests you use the command
`xmake run -w . test`.

## CONTRIBUTIONS
Contributions are welcome, simply open an issue on GitHub or directly a PR!

