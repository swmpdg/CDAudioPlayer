###################################################
#                                                 #
#                                                 #
#   CDAudioPlayer CMake build file                #
#                                                 #
#                                                 #
###################################################
cmake_minimum_required( VERSION 3.6 )

# function to collect all the sources from sub-directories
# into a single list
function(add_sources)
  get_property(is_defined GLOBAL PROPERTY SRCS_LIST DEFINED)
  if(NOT is_defined)
    define_property(GLOBAL PROPERTY SRCS_LIST
      BRIEF_DOCS "List of source files"
      FULL_DOCS "List of source files to be compiled in one library")
  endif()
  # make absolute paths
  set(SRCS)
  foreach(s IN LISTS ARGN)
    if(NOT IS_ABSOLUTE "${s}")
      get_filename_component(s "${s}" ABSOLUTE)
    endif()
    list(APPEND SRCS "${s}")
  endforeach()
  # append to global list
  set_property(GLOBAL APPEND PROPERTY SRCS_LIST "${SRCS}")
endfunction(add_sources)

# preprocess sources
function( preprocess_sources )
	set(PREP_SRCS PARENT_SCOPE)
	get_property(SRCS GLOBAL PROPERTY SRCS_LIST)
	foreach(s IN LISTS SRCS)
	  file(RELATIVE_PATH rs "${CMAKE_CURRENT_SOURCE_DIR}" "${s}")
	  string(REGEX REPLACE "r$" "" o "${CMAKE_CURRENT_BINARY_DIR}/${rs}")
	  add_custom_command(
		OUTPUT "${o}"
		COMMAND ${CMAKE_COMMAND} -E copy "${s}" "${o}"
		DEPENDS "${s}"
		COMMENT "Creating ${o}"
		VERBATIM
		)
	  list(APPEND PREP_SRCS "${s}")
	endforeach()
	set(PREP_SRCS ${PREP_SRCS} PARENT_SCOPE)
endfunction( preprocess_sources )

function( create_source_groups _src_root_path )
	get_property(SRCS GLOBAL PROPERTY SRCS_LIST)
	foreach(_source IN ITEMS ${SRCS})
		get_filename_component(_source_path "${_source}" PATH)
		file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
		string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
		source_group("${_group_path}" FILES "${_source}")
	endforeach()
endfunction( create_source_groups )

#Function to clear the sources list.
function( clear_sources )
	set_property( GLOBAL PROPERTY SRCS_LIST "" )
	set( PREP_SRCS PARENT_SCOPE )
endfunction( clear_sources )

# Function to add include files.
function( add_includes )
  get_property(is_defined GLOBAL PROPERTY INCLUDES_LIST DEFINED)
  if(NOT is_defined)
    define_property(GLOBAL PROPERTY INCLUDES_LIST
      BRIEF_DOCS "List of include files"
      FULL_DOCS "List of include files to be compiled in one library")
  endif()
  # make absolute paths
  set(INCLUDES)
  foreach(s IN LISTS ARGN)
    if(NOT IS_ABSOLUTE "${s}")
      get_filename_component(s "${s}" ABSOLUTE)
    endif()
    list(APPEND INCLUDES "${s}")
  endforeach()
  # append to global list
  set_property(GLOBAL APPEND PROPERTY INCLUDES_LIST "${INCLUDES}")
endfunction( add_includes )

# Function to install includes.
function( install_includes _include_root_path )
	get_property(INCLUDES GLOBAL PROPERTY INCLUDES_LIST)
	
	foreach(_include IN ITEMS ${INCLUDES})
		get_filename_component(_include_path "${_include}" PATH)
		file(RELATIVE_PATH _include_path_rel "${_include_root_path}" "${_include_path}")
		string(REPLACE "/" "\\" _group_path "${_include_path_rel}")
		install( FILES "${_include}" DESTINATION "include/${_group_path}" )
	endforeach()
	
	set_property( GLOBAL PROPERTY INCLUDES_LIST "" )
endfunction( install_includes )

macro(configure_msvc_runtime)
  if(MSVC)
    # Default to statically-linked runtime.
    if("${MSVC_RUNTIME}" STREQUAL "")
      set(MSVC_RUNTIME "static")
    endif()
    # Set compiler options.
    set(variables
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
    if(${MSVC_RUNTIME} STREQUAL "static")
      message(STATUS
        "MSVC -> forcing use of statically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endif()
      endforeach()
    else()
      message(STATUS
        "MSVC -> forcing use of dynamically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MT")
          string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
        endif()
      endforeach()
    endif()
  endif()
endmacro()

project( CDAudioPlayer )

#Needs static CRT to avoid alloc issues.
set( MSVC_RUNTIME "static" )

configure_msvc_runtime()

set(variables
  CMAKE_C_FLAGS_DEBUG
  CMAKE_C_FLAGS_MINSIZEREL
  CMAKE_C_FLAGS_RELEASE
  CMAKE_C_FLAGS_RELWITHDEBINFO
  CMAKE_CXX_FLAGS_DEBUG
  CMAKE_CXX_FLAGS_MINSIZEREL
  CMAKE_CXX_FLAGS_RELEASE
  CMAKE_CXX_FLAGS_RELWITHDEBINFO
)
message(STATUS "Initial build flags:")
foreach(variable ${variables})
  message(STATUS "  '${variable}': ${${variable}}")
endforeach()
message(STATUS "")

#Shared preprocessor definitions
set( SHARED_DEFS
)

if( MSVC )
	set( SHARED_DEFS
		${SHARED_DEFS}
		_CRT_SECURE_NO_WARNINGS
		_SCL_SECURE_NO_WARNINGS
	)
	
	set( LINUX_32BIT_FLAG "" )
elseif( UNIX )

	set( LINUX_32BIT_FLAG "-m32" )
endif()

#C++14 support
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y" )

set( EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external )

#
#CDAudioPlayer begin
#

#External dependencies not included in this repository
set( FMOD_LOWLEVEL_PATH CACHE PATH "Path to FMOD Studio Low Level API root directory. This is the directory that includes the inc and lib directories" )

if( FMOD_LOWLEVEL_PATH STREQUAL "" )
	MESSAGE( FATAL_ERROR "FMOD Studio Low Level API is required to use this program" )
endif()

if( MSVC )
	find_library( FMOD_LOWLEVEL_ARCHIVE fmodL_vc${CMAKE_STATIC_LIBRARY_SUFFIX} PATHS "${FMOD_LOWLEVEL_PATH}/lib" NO_DEFAULT_PATH )
else()
	find_library( FMOD_LOWLEVEL_ARCHIVE fmod${CMAKE_STATIC_LIBRARY_SUFFIX} PATHS "${FMOD_LOWLEVEL_PATH}/lib" NO_DEFAULT_PATH )
endif()

add_subdirectory( src )

preprocess_sources()

add_executable( CDAudioPlayer ${PREP_SRCS} )

target_include_directories( CDAudioPlayer PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${FMOD_LOWLEVEL_PATH}/inc
)

target_compile_definitions( CDAudioPlayer PRIVATE
	${SHARED_DEFS}
)

target_link_libraries( CDAudioPlayer
	${FMOD_LOWLEVEL_ARCHIVE}
)

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}" )

clear_sources()

#Copy all libs over to the binary dir so we can use them locally.
#Because there might be an fmod.dll in SysWOW64 we must do this so it uses the local copy.
foreach( config_type IN LISTS CMAKE_CONFIGURATION_TYPES )
	file( COPY "${FMOD_LOWLEVEL_PATH}/lib/" DESTINATION ${CMAKE_BINARY_DIR}/${config_type} )
endforeach()

#
#CDAudioPlayer end
#
