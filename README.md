# FSEngine
Vulkan based game engine made in C++

## Screenshots
![Specular Lighting](showcase/sponza.png)

- 97,336 non-instanced cubes running at ~90+ FPS while being updated every frame (Ryzen 5 7600 RTX 3060)
![Cubes](showcase/97kcubes.png)

## Building Requirements:
Vulkan : 1.4<br>
Slangc : 2025.0+<br>
Platform : Windows or Linux<br>
CMake : 3.10+<br>

## Controls
WASD - move<br>
Arrow keys/Mouse - rotate<br>
G - lock-unlock mouse<br>

## current status

- full vulkan graphics pipeline
- specular and diffuse lighting (blinn-phong)
- using stb_image for texture loading
- using my own maths utilities
- using my own windowing library (windows + linux (x11) only)
- WASD camera system
- SSBO to send entity orientation
- MultiDrawIndirect to draw
- ImGui Scene Management

## todo

- support glTF and better lighting 
- decouple the engine from vulkan
- decouple the engine from the game
- implement ECS
- have software + vulkan + other apis as one click/line change
- Add MSAA