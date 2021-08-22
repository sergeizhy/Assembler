
- Files to be assembled are located in ./files/ under .as extention
- Assembled project path is located in ./out/bin/asm
- Source code of the project is located in ./src/

Brief explanation

- Project was built to assemble .as files for an invented 16bit machine with a custom set of commands given
- File handler creates a struct with the whole file in it
  - while creating the file 'File_descriptor' errors will be checked an unnecessary delimiters will be trimmed 
- First pass does all the heavy lifting maintaining a hashtable with all the required labels to be later processed by the second pass
  - first pass is divided to two main sections before first argument and after first argumnet 
    - before first argument labels and instructions will be processed and attributes will be extracted from the relevent hashtable
    - after first argument data will be processed according an attribute which was found before first argument
- Second pass creates the relevent files and checks for undefined labels found in the first pass