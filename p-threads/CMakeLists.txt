
SET(CMAKE_C_FLAGS "-pthread")
add_executable(partha_lock_test lock_test.c )
add_executable(partha_prod_cons_m prod_cons_m.c)
add_executable(partha_prod_cons_s prod_cons_s.c)
add_executable(partha_race-c race-c.c)
add_executable(partha_race-p race-p.c)
add_executable(partha_read_write_m read_write_m.c)
add_executable(partha_read_write_s read_write_s.c)
add_executable(partha_sem_test sem_test.c)
add_executable(partha_thread_test thread_test.c)
