find_package(PkgConfig)
pkg_check_modules( finalcut
  finalcut
  )

link_directories(
  ${finalcut_LIBRARY_DIRS}
  )

add_executable(hello
  hello.cpp
  )
add_compile_options(-g)
add_compile_options(-std=c++17)
add_compile_options(${finalcut_INCLUDE_DIRS})

target_link_libraries(hello
  ${finalcut_LIBRARIES}
  )

add_executable(labelsizebug
  labelsizebug.cpp
  )

target_link_libraries(labelsizebug
  ${finalcut_LIBRARIES}
  )

add_executable(tetris
  tetris.cpp
  )

target_link_libraries(tetris
  ${finalcut_LIBRARIES}
  )
