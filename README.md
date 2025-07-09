# Simple OpenGL Application with Moving Camera

This is a simple OpenGL application that demonstrates basic graphics functionality, including a moving camera. The application allows you to explore 3D environments and offers the flexibility to build using either Visual Studio or Premake.

## Features
- **Moving Camera**: A basic camera system that you can control, allowing you to explore the scene.
- **OpenGL Rendering**: Basic rendering of 3D objects with shaders.
- **Support for Visual Studio**: If you’re using Visual Studio, the solution file (`.sln`) is included.
- **Premake Support**: If you're not using Visual Studio, you can easily set up the project with Premake. A `run_premake5.bat` file is included to automatically create the necessary files.

## Requirements

Before running the application, make sure you have the following installed:

- **C++ Compiler**: GCC, Clang, or MSVC (if using Visual Studio).
- **OpenGL**: OpenGL libraries and drivers installed.
- **GLFW**: Used for window and input handling.
- **GLEW**: For managing OpenGL extensions.
- **Premake** (Optional): If you want to build with Premake instead of Visual Studio.

You will also need to install these dependencies:
- **GLFW**: For creating windows and handling input.
- **GLEW**: For OpenGL extension loading.

## Setup Instructions

### Option 1: Using Visual Studio

1. **Open the Solution in Visual Studio**:
   - Open the provided solution file `OpenGL_SimpleApp.sln` in Visual Studio.
   - Ensure that you have the necessary dependencies (GLFW, GLEW) set up properly in the Visual Studio project.

2. **Build the Solution**:
   - Simply build the project in Visual Studio (use `Ctrl + Shift + B`).
   - The project will compile and generate an executable you can run.

3. **Run the Application**:
   - Press `Ctrl + F5` to run the application.
   - You should see a simple OpenGL window with a moving camera.

### Option 2: Using Premake (for non-VS users)

1. **Setup Premake**:
   - If you don’t have Visual Studio or prefer to use Premake, navigate to the `scripts` folder of the repository.
   - Double-click on the `run_premake5.bat` file to automatically generate project files for your system.
   
   The script will:
   - Generate Makefiles (for Unix-based systems) or Visual Studio project files (for Windows, if you don’t use `run_premake5.bat`).

2. **Building the Project**:
   - Once the project files are generated, you can either:
     - Use `make` (on Unix-based systems).
     - Open the generated Visual Studio project (if you chose to use VS).
   - Compile the project using your chosen method.

3. **Run the Application**:
   - Once compiled, run the application and you should see the window with the moving camera.

## Moving the Camera

The camera can be controlled using basic input keys:

- **W, A, S, D**: Move the camera forward, left, backward, and right.
- **Mouse Movement**: Use the mouse to look around.
- **Esc**: Close the window and exit the application.
- These are just the basics for camera movement, you can add more with the Window::AddKeyComb. Please explore all options.

You can modify the camera speed and other parameters in the code to customize the movement.

## Current Development Status

This project is currently **open-source** and under development. Here’s what’s planned:

- **Physics Features**: We’re working on adding physics capabilities to the engine, such as collision detection and rigid body dynamics.
- **Engine Class**: A new `Engine` class is being developed to improve code structure and provide a modular framework for future additions.

Feel free to contribute by forking the repository and submitting pull requests!

## License

This project is open-source and released under the MIT License.

## Contributions

We welcome contributions! If you’d like to help improve the project or add new features, please:

1. Fork the repository.
2. Create a feature branch (`git checkout -b feature-name`).
3. Make your changes and commit them (`git commit -am 'Add new feature'`).
4. Push the branch (`git push origin feature-name`).
5. Submit a pull request.

## Acknowledgments

This project relies on the following libraries:

- **GLFW**: [https://www.glfw.org/](https://www.glfw.org/)
- **GLEW**: [http://glew.sourceforge.net/](http://glew.sourceforge.net/)
- **OpenGL**: The core rendering engine for all graphics.

## Contact

If you have any questions or feedback, feel free to open an issue on the GitHub repository or contact me at [akmalmukhamadiev2020@gmail.com].

