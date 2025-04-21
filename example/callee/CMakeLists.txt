# set(SRC_LIST userservice.cc ../user.pb.cc)
# set(SRC_LIST friendservice.cc ../friend.pb.cc)
set(SRC_LIST allservice.cc ../all.pb.cc)

add_executable(providerall ${SRC_LIST})
target_link_libraries(providerall mprpc protobuf::libprotobuf)