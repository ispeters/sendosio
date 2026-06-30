# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(ALL_EXAMPLES todo)
message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(beman.sendosio.examples.${example})
    target_sources(beman.sendosio.examples.${example} PRIVATE ${example}.cpp)
    target_link_libraries(
        beman.sendosio.examples.${example}
        PRIVATE beman::sendosio
    )
    if(BEMAN_SENDOSIO_USE_MODULES)
        set_target_properties(
            beman.sendosio.examples.${example}
            PROPERTIES CXX_MODULE_STD ON
        )
    endif()
endforeach()
