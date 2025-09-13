Each real time kernel port consists of three files that contain the core kernel
components and are common to every port, and one or more files that are 
specific to a particular microcontroller and or compiler.

+ The FreeRTOS/Source directory contains the three files that are common to 
every port.  The kernel is contained within these three files.

+ The FreeRTOS/Source/Portable directory contains the files that are specific to 
a particular microcontroller and or compiler.

+ The FreeRTOS/Source/include directory contains the real time kernel header 
files.

See the readme file in the FreeRTOS/Source/Portable directory for more 
information.

每个实时内核移植都包含三个文件，这些文件包含核心内核组件，并且每个移植版本都通用；此外，
还有一个或多个文件，这些文件特定于特定的微控制器和/或编译器。

+ FreeRTOS/Source 目录包含每个移植版本都通用的三个文件。内核就包含在这三个文件中。

+ FreeRTOS/Source/Portable 目录包含特定于特定的微控制器和/或编译器的文件。

+ FreeRTOS/Source/include 目录包含实时内核头文件。

更多信息，请参阅 FreeRTOS/Source/Portable 目录中的 readme 文件。