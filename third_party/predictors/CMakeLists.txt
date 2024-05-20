if (ENABLE_PREDICT)
  if (USE_TORCH)
    add_subdirectory(torchscript)
  else ()
    add_subdirectory(onnx)
  endif ()
endif()

