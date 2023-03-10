project (${WIKI}InfoVis)

if(NOT VTK_BINARY_DIR)
  set(VTK_LIBRARIES "")
  find_package(VTK COMPONENTS
    CommonColor
    CommonComputationalGeometry
    CommonCore
    CommonDataModel
    FiltersGeneral
    FiltersSources
    FiltersStatistics
    ImagingCore
    ImagingSources
    IOCore
    IOImage
    IOInfovis
    IOXML
    InfovisCore
    InfovisLayout
    InteractionImage
    InteractionStyle
    RenderingCore
    RenderingFreeType
    RenderingGL2PSOpenGL2
    RenderingOpenGL2
    RenderingContextOpenGL2
    ViewsCore
    ViewsInfovis
    vtksys
    OPTIONAL_COMPONENTS
    TestingRendering
    QUIET
    )
endif()
set(KIT_LIBS ${VTK_LIBRARIES})

#
# Build all .cxx files in the directory
file(GLOB ALL_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cxx)

set(BOOST_SRCS)
if(NOT VTK_USE_BOOST)
  list(APPEND BOOST_SRCS
    MutableGraphHelper.cxx
    )
endif()

if (BOOST_SRCS)
  list(REMOVE_ITEM ALL_FILES ${BOOST_SRCS})
endif()

if(VTK_USE_BOOST)
  Requires_Module(MutableGraphHelper InfovisBoostGraphAlgorithms)
endif()

Requires_Class(WordCloudDemo vtkWordCloud)

if(NOT VTK_USE_PARALLEL)
  set(SKIP PKMeansClustering)
  message(STATUS "VTKWikiExamples: ${SKIP} requires VTK_PARALLEL and will not be built")
  string(REGEX REPLACE "[^;]*${SKIP}.cxx"
    "" ALL_FILES "${ALL_FILES}")
endif()

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
  set(KIT InfoVis)

  set(DATA ${WikiExamples_SOURCE_DIR}/src/Testing/Data)

  set(NEEDS_ARGS
    DelimitedTextReader
    DelimitedTextWriter
    WordCloud
    XGMLReader
    )

  if ("${ALL_FILES}" MATCHES ".*TreeMapView.*")
    set(NEEDS_ARGS "${NEEDS_ARGS};TreeMapView")
    add_test(${KIT}-TreeMapView ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KIT}CxxTests
      TestTreeMapView ${DATA}/Infovis-XML-vtkclasses.xml ${DATA}/Infovis-XML-vtklibrary.xml)
  endif()

  if ("${ALL_FILES}" MATCHES ".*WordCloudDemo.*")
    set(NEEDS_ARGS "${NEEDS_ARGS};WordCloudDemo")
  add_test(${KIT}-WordCloudDemo ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KIT}CxxTests
    TestWordCloudDemo ${DATA}/Gettysburg.txt  --title "GettysBurg Address" --size 1280 1024 --orientationDistribution 0 0 --offsetDistribution 0 0 --replacementPairs consecrated consecrate dedicate dedicated --dpi 400 --fontFileName ${DATA}/Canterbury.ttf)
  endif()

  set(TEMP ${WikiExamples_BINARY_DIR}/Testing/Temporary)

  add_test(${KIT}-DelimitedTextReader ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KIT}CxxTests
    TestDelimitedTextReader ${DATA}/DelimitedData.txt)

  add_test(${KIT}-DelimitedTextWriter ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KIT}CxxTests
    TestDelimitedTextWriter ${TEMP}/foo.txt)

  add_test(${KIT}-WordCloud ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KIT}CxxTests
    TestWordCloud ${DATA}/Gettysburg.txt --dpi 150 --fontFile ${DATA}/Canterbury.ttf)

  add_test(${KIT}-XGMLReader ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KIT}CxxTests
    TestXGMLReader ${DATA}/fsm.gml)

  include(${WikiExamples_SOURCE_DIR}/CMake/ExamplesTesting.cmake)

endif()
