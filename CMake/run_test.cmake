# some argument checking:
# test_cmd is the command to run with all its arguments

set(OUT ${test_dir}/.test.out)
set(EXP ${input}.exp)

if( NOT test_cmd )
   message( FATAL_ERROR "Variable test_cmd not defined" )
endif (NOT test_cmd)

# EXP contains the name of the "expected" output file
if( NOT EXP )
   message( FATAL_ERROR "Variable EXP not defined" )
endif( NOT EXP )

# OUT contains the name of the output file the test_cmd will produce
if( NOT OUT )
   message( FATAL_ERROR "Variable OUT not defined" )
endif( NOT OUT )

execute_process(
   COMMAND ${test_cmd} ${input}
   OUTPUT_FILE ${OUT}
   )

execute_process(
   COMMAND ${CMAKE_COMMAND} -E compare_files ${EXP} ${OUT}
   RESULT_VARIABLE test_not_successful
   OUTPUT_QUIET
   ERROR_QUIET
   )

if( test_not_successful )
   message( SEND_ERROR "${OUT} does not match ${EXP}!" )
endif( test_not_successful )
