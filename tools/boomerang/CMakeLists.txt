# Build a stand-alone executable
if(PHASAR_IN_TREE)
  add_phasar_executable(boomerang
    boomerang.cpp
  )
else()
  add_executable(boomerang
    boomerang.cpp
  )
endif()

target_link_libraries(boomerang
  LINK_PUBLIC
  phasar_config
  phasar_controller
  phasar_db
  phasar_controlflow
  phasar_ifdside
  phasar_mono
  phasar_wpds
  phasar_syncpds
  ${PHASAR_PLUGINS_LIB}
  wali
  phasar_passes
  phasar_pointer
  phasar_typehierarchy
  phasar_phasarllvm_utils
  phasar_utils
  boost_program_options
  boost_filesystem
  boost_graph
  boost_system
  boost_log
  ${BOOST_THREAD}
  ${Boost_LIBRARIES}
  ${CMAKE_DL_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  ${llvm_libs}
)

if(NOT PHASAR_IN_TREE)
target_link_libraries(boomerang
  LINK_PUBLIC
  ${llvm_libs}
)
endif()

set(LLVM_LINK_COMPONENTS
)