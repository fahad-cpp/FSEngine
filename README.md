# FSEngine
Vulkan based game engine made in C++

# current status

- full vulkan graphics pipeline
- using stb_image for texture loading
- using my own maths utilities
- using my own windowing library (windows + linux (x11) only)
- WASD camera system
- hardcoded lights
- BLOB Entity
- Uniform buffers to send entity orientation
- Vulkan is tightly coupled with the engine end the entity

# todo

- Add MSAA
- support glTF and better lighting 
- decouple the engine from the rendering API 
- and maybe have software + vulkan + other apis as one click/line change
- also decouple the engine from the game
- implement my own ECS
- have some ui maybe