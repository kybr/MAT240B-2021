
Run the `get-and-build-audio-transport` first. Then build `main.cpp` using allolib_playground's run.sh script. Then fix the executable as shown below.


This example probably only works on macOS.


Initially, this example will build, but crash when run:

    ./run.sh MAT240B-2021/audio-transport/main.cpp
    ... snip ...
    dyld: Library not loaded: @rpath/libaudio_transport.dylib
      Referenced from: /Users/?????????/allolib_playground/MAT240B-2021/audio-transport/bin/./main
      Reason: image not found

We have to fix the executable with this command:

    cd xxxx/audio-transport
    install_name_tool -add_rpath @executable_path/.. bin/main


