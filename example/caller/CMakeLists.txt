# set(SRC_LIST calluserservice.cc ../user.pb.cc)
set(SRC_FRIEND callfriendservice.cc ../friend.pb.cc )
set(SRC_USER calluserservice.cc ../user.pb.cc)

add_executable(consumer1 ${SRC_FRIEND})
add_executable(consumer2 ${SRC_USER})
target_link_libraries(consumer1 mprpc protobuf::libprotobuf)
target_link_libraries(consumer2 mprpc protobuf::libprotobuf)
