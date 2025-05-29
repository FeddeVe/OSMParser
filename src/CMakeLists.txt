target_sources(${CMAKE_PROJECT_NAME} PRIVATE   
   ${CMAKE_CURRENT_SOURCE_DIR}/OSMParser.cpp    
   ${CMAKE_CURRENT_SOURCE_DIR}/OSMDispatcher.cpp    
   ${CMAKE_CURRENT_SOURCE_DIR}/OSMWorker.cpp    
   ${CMAKE_CURRENT_SOURCE_DIR}/Database.cpp  
   ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPool.cpp  
)   

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Proto")    