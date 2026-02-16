# Debugger

A simple debugger project. This project currently provides basic debugging functionalities.

## Features
- Basic debugging capabilities (see `main.c` and related files)
- Support for loading and stepping through assembly code
- Initial support for breakpoints
- Modular structure for future enhancements

## Planned Features
- **DWARF Debugging Support:**
  - Planned implementation of DWARF parsing for advanced symbol and debug information.
- **Cross-Platform Breakpoints:**
  - Improved and more robust breakpoint handling across different platforms and architectures.
- **Additional Debugging Features:**
  - More advanced features and improvements to the debugging workflow.

## Getting Started
1. Clone the repository:
   ```sh
   git clone https://github.com/atongithub/debugger.git
   ```
2. Build the project:
   ```sh
   gcc -g -Wall -Wextra -O0 -o debugger main.c
   ```
3. Run the debugger:
   ```sh
   ./debugger <your_elf_file>
   ```

## License
This project is licensed under the MIT License. See the `LICENSE` file for details.

