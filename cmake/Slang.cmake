# Compile one entry point into one SPIR-V module next to the executable.
# DEPFILE tracks Slang imports/includes, not just the top-level source file.
function(baller_add_slang_shader source entry stage output_name)
    set(shader_source "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
    set(shader_dir "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/shaders")
    set(shader_output "${shader_dir}/${output_name}.spv")
    set(shader_depfile "${shader_dir}/${output_name}.d")

    add_custom_command(
        OUTPUT "${shader_output}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${shader_dir}"
        COMMAND "${BALLER_SLANGC_EXECUTABLE}" "${shader_source}"
            -entry "${entry}" -stage "${stage}"
            -target spirv -profile spirv_1_6
            -emit-spirv-directly -fvk-use-entrypoint-name
            -matrix-layout-column-major
            "$<IF:$<CONFIG:Debug>,-O0,-O2>" "$<$<CONFIG:Debug>:-g>"
            -I "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
            -depfile "${shader_depfile}"
            -o "${shader_output}"
        DEPENDS "${shader_source}" "${BALLER_SLANGC_EXECUTABLE}"
        DEPFILE "${shader_depfile}"
        COMMENT "Compiling Slang ${stage} shader: ${output_name} (${entry})"
        COMMAND_EXPAND_LISTS
        VERBATIM
    )

    set(BALLER_SHADER_OUTPUTS ${BALLER_SHADER_OUTPUTS} "${shader_output}" PARENT_SCOPE)
endfunction()
