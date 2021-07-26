if (CLAD_ENABLE_DOXYGEN)
  find_package(Doxygen REQUIRED)

  set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/doxygen.cfg.in)
  set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/doxygen.cfg)

  set(docs_srcdir ${CMAKE_CURRENT_SOURCE_DIR})
  set(docs_builddir ${CMAKE_CURRENT_BINARY_DIR})
  set(clad_srcdir ${CMAKE_SOURCE_DIR})
  file(READ ${CMAKE_SOURCE_DIR}/VERSION PACKAGE_VERSION)

  configure_file(${DOXYFILE_IN} ${DOXYFILE} @ONLY)

  add_custom_target(doxygen-clad
                    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Generate clad documentation with Doxygen"
                    VERBATIM)
else()
message(WARNING "No documentation tool enabled. Enable any documentation tool "
                "to generate documentation build targets")
endif()

find_program(SPHINX_EXECUTABLE
             NAMES sphinx-build
             DOC "Path to sphinx-build executable")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sphinx
                                  "Failed to find sphinx-build executable"
                                  SPHINX_EXECUTABLE)

find_package(Sphinx REQUIRED)
set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/sphinx/)
set(SPHINX_BUILD ${CMAKE_BINARY_DIR}/docs/sphinx)

add_custom_target(Sphinx
                  COMMAND
                  ${SPHINX_EXECUTABLE} -b html
                  -Dbreathe_projects.Clad=${docs_builddir}/doxygen/xml
                  ${SPHINX_SOURCE} ${SPHINX_BUILD}
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                  COMMENT "Generating documentation with Sphinx")
