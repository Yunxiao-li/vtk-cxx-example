project (${WIKI}HyperTreeGrid)

if(NOT VTK_BINARY_DIR)
  set(VTK_LIBRARIES "")
  find_package(VTK COMPONENTS
    CommonColor
    FiltersGeneral
    FiltersHyperTree
    FiltersSources
    InteractionStyle
    RenderingCore
    RenderingFreeType
    RenderingOpenGL2
    OPTIONAL_COMPONENTS
    TestingRendering
    QUIET
    )
endif()

set(KIT_LIBS ${VTK_LIBRARIES})

#
# Build all .cxx files in the directory
file(GLOB ALL_FILES *.cxx)

# include(${WikiExamples_SOURCE_DIR}/CMake/RequiresVersion.cmake)
# set(VERSION_MIN "7.0")
# Requires_Version(ParametricKuenDemo ${VERSION_MIN} ALL_FILES)
# Requires_Version(ParametricObjectsDemo2 ${VERSION_MIN} ALL_FILES)

foreach(SOURCE_FILE ${ALL_FILES})
  string(REPLACE ".cxx" "" TMP ${SOURCE_FILE})
  string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/ "" EXAMPLE ${TMP})
  add_executable(${WIKI}${EXAMPLE} ${EXECUTABLE_FLAG} ${EXAMPLE}.cxx)
  target_link_libraries(${WIKI}${EXAMPLE} ${KIT_LIBS})
  vtk_module_autoinit(
    TARGETS ${WIKI}${EXAMPLE}
    MODULES ${VTK_LIBRARIES}
    )
endforeach()

if (BUILD_TESTING)
  # Testing
  set(KIT HyperTreeGrid)
  set(NEEDS_ARGS
    )
  include(${WikiExamples_SOURCE_DIR}/CMake/ExamplesTesting.cmake)
endif()
