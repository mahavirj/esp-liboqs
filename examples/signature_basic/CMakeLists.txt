# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)

# Set the components to include the liboqs component from the parent directory
set(EXTRA_COMPONENT_DIRS "../..")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(signature_basic)
