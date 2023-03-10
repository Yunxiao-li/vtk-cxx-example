project (${WIKI}Shaders)

if(NOT VTK_BINARY_DIR)
  set(VTK_LIBRARIES "")
  find_package(VTK COMPONENTS
    CommonColor
    CommonCore
    CommonDataModel
    CommonTransforms
    FiltersCore
    FiltersGeneral
    FiltersSources
    IOGeometry
    IOImage
    IOLegacy
    IOPLY
    IOXML
    ImagingCore
    InteractionStyle
    InteractionWidgets
    RenderingCore
    RenderingFreeType
    RenderingOpenGL2
    RenderingGL2PSOpenGL2
    OPTIONAL_COMPONENTS
    TestingRendering
    QUIET
    )
  if (VTK_VERSION VERSION_LESS "8.90.0")
    include(${VTK_USE_FILE})
  endif()
endif()

set(KIT_LIBS ${VTK_LIBRARIES})
#
# Build all .cxx files in the directory
file(GLOB ALL_FILES *.cxx)
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
  set(KIT Shaders)
  set(NEEDS_ARGS
    BozoShader
    BozoShaderDemo
    ColorByNormal
    CubeMap
    MarbleShader
    MarbleShaderDemo
    SpatterShader
    SphereMap
    )

  set(DATA ${WikiExamples_SOURCE_DIR}/src/Testing/Data)

  add_test(${KIT}-BozoShader ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KIT}CxxTests
    TestBozoShader ${DATA}/Shaders/PerlinNoise.glsl ${DATA}/horse.ply)

  add_test(${KIT}-BozoShaderDemo ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KIT}CxxTests
    TestBozoShaderDemo ${DATA}/Shaders/PerlinNoise.glsl
    ${DATA}/horse.ply
    ${DATA}/teapot.g
    ${DATA}/trumpet.obj
    ${DATA}/shark.ply
    ${DATA}/Torso.vtp
    ${DATA}/cow.g
    ${DATA}/Armadillo.ply
    ${DATA}/42400-IDGH.stl
    ${DATA}/doorman/doorman.obj )

  add_test(${KIT}-ColorByNormal ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KIT}CxxTests
    TestColorByNormal ${DATA}/Armadillo.ply)

  add_test(${KIT}-CubeMap ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KIT}CxxTests
    TestCubeMap ${DATA}/horse.ply ${DATA}/Skyboxes/skybox1/)

  add_test(${KIT}-MarbleShader ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KIT}CxxTests
    TestMarbleShader ${DATA}/Shaders/PerlinNoise.glsl ${DATA}/horse.ply)

  add_test(${KIT}-MarbleShaderDemo ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KIT}CxxTests
    TestMarbleShaderDemo ${DATA}/Shaders/PerlinNoise.glsl ${DATA}/teapot.g)

  add_test(${KIT}-SpatterShader ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KIT}CxxTests
    TestSpatterShader ${DATA}/Shaders/PerlinNoise.glsl ${DATA}/horse.ply)

  add_test(${KIT}-SphereMap ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KIT}CxxTests
    TestSphereMap ${DATA}/Bunny.vtp ${DATA}/Skyboxes/wintersun.jpg)


  include(${WikiExamples_SOURCE_DIR}/CMake/ExamplesTesting.cmake)
endif()
