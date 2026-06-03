cmake_minimum_required(VERSION 3.15) 
project(ATM_Management)              
set(CMAKE_CXX_STANDARD 17)  
add_definitions(-DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0A00 -D_WIN32_IE=0x0A00)       

# 1. List all your .cpp source files from the src folder
set(SOURCES
    src/main.cpp
    src/Database.cpp
    src/ATM.cpp
    src/ATMGui.cpp
)

# 2. Tell the compiler where your .h header files are
include_directories(include)         

# 3. Create the executable (WIN32 hides the background console window)
add_executable(ATM_Management WIN32 ${SOURCES})

# 4. Link the required Windows and SQL Server (ODBC) libraries
target_link_libraries(ATM_Management
    odbc32      
    odbccp32    
    comctl32    
    user32      
    gdi32       
    kernel32    
)