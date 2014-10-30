message(STATUS "downloading...
     src='https://googletest.googlecode.com/files/gtest-1.7.0.zip'
     dst='/home/jwright/art/cse531/build/googletest-prefix/src/gtest-1.7.0.zip'
     timeout='10 seconds'")




file(DOWNLOAD
  "https://googletest.googlecode.com/files/gtest-1.7.0.zip"
  "/home/jwright/art/cse531/build/googletest-prefix/src/gtest-1.7.0.zip"
  SHOW_PROGRESS
  # no EXPECTED_HASH
  TIMEOUT;10
  STATUS status
  LOG log)

list(GET status 0 status_code)
list(GET status 1 status_string)

if(NOT status_code EQUAL 0)
  message(FATAL_ERROR "error: downloading 'https://googletest.googlecode.com/files/gtest-1.7.0.zip' failed
  status_code: ${status_code}
  status_string: ${status_string}
  log: ${log}
")
endif()

message(STATUS "downloading... done")
