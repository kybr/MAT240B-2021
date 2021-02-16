message("binary dir: ${CMAKE_CURRENT_BINARY_DIR}")
message("file full path: ${file_full_path}")
message("app path: ${app_path}")
message("app name: ${app_name}")
message("al path: ${al_path}")


#find_library(NFD_LIBRARY nfd HINTS nativefiledialog/build/lib/Release/x64)

# You can set verbose cmake messages:
#set(AL_VERBOSE_OUTPUT 1)

# other directories to include. You can use relative paths to the
# source file being built.
#set(app_include_dirs nativefiledialog/src/include)

# other libraries to link
set(app_link_libs nfd)

# definitions. Prepend -D to any defines
#set(app_definitions -DUSE_COLOR)

# compile flags
# This flag ignores unused variable warning. You probably don't want to do this...
#set(app_compile_flags -Wno-unused-variable)

# linker flags, with `-` in the beginning
set(app_linker_flags -L${app_path})
