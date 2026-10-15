# AUv2 uses the pinned Steinberg adapter around the same tested instrument/UI.
if(NOT APPLE OR NOT AUDIOUNIT_SDK_ROOT)
 message(FATAL_ERROR "AU requires macOS and AUDIOUNIT_SDK_ROOT")
endif()
file(GLOB APPLE_AU_SOURCES CONFIGURE_DEPENDS "${AUDIOUNIT_SDK_ROOT}/src/AudioUnitSDK/*.cpp")
add_library(tinyfreaky-apple-au STATIC ${APPLE_AU_SOURCES})
target_include_directories(tinyfreaky-apple-au PUBLIC "${AUDIOUNIT_SDK_ROOT}/include")
target_compile_features(tinyfreaky-apple-au PUBLIC cxx_std_17)
set(WRAPPER "${VST3_SDK_ROOT}/public.sdk/source/vst/auwrapper")
add_library(TinyfreakyAU MODULE "${WRAPPER}/auwrapper.mm" "${WRAPPER}/aucocoaview.mm" "${WRAPPER}/NSDataIBStream.mm")
target_compile_definitions(TinyfreakyAU PRIVATE SMTG_AUWRAPPER_USES_AUSDK CA_USE_AUDIO_PLUGIN_ONLY=0 SMTG_AUCocoaUIBase_CLASS_NAME=TinyfreakyAUCocoaView)
target_include_directories(TinyfreakyAU PRIVATE "${VST3_SDK_ROOT}")
target_link_libraries(TinyfreakyAU PRIVATE tinyfreaky-apple-au sdk_hosting "-framework AudioUnit" "-framework CoreMIDI" "-framework AudioToolbox" "-framework CoreFoundation" "-framework Carbon" "-framework Cocoa" "-framework CoreAudio")
set_target_properties(TinyfreakyAU PROPERTIES BUNDLE TRUE BUNDLE_EXTENSION component PREFIX "" OUTPUT_NAME Tinyfreaky LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/AU" MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in")
add_dependencies(TinyfreakyAU Tinyfreaky)
add_custom_command(TARGET TinyfreakyAU POST_BUILD
 COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_BUNDLE_DIR:TinyfreakyAU>/Contents/Resources"
 COMMAND "${CMAKE_COMMAND}" -E copy_directory "$<TARGET_BUNDLE_DIR:Tinyfreaky>" "$<TARGET_BUNDLE_DIR:TinyfreakyAU>/Contents/Resources/plugin.vst3"
 COMMAND /usr/bin/codesign --force --deep --sign - "$<TARGET_BUNDLE_DIR:TinyfreakyAU>"
 COMMAND /usr/bin/codesign --verify --deep --strict "$<TARGET_BUNDLE_DIR:TinyfreakyAU>")
