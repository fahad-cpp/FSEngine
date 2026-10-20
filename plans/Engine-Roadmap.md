# Engine Roadmap

## PBR and rendering

- Metallic-roughness PBR with direct lighting

- Image-based lighting (IBL)

- Normal mapping and tangent-space handling

- HDR rendering and tone mapping

- Correct sRGB and linear color-space handling

- Shadow mapping

- Alpha masking and blending where supported

## glTF and asset pipeline

- glTF 2.0 meshes, transforms and materials

- Texture loading, mipmaps and sampler settings

- Asset validation and meaningful error messages

- GPU resource creation and cleanup

- Hot reload without leaking resources or invalidating in-flight GPU work

## GPU-driven rendering

- Compute-based frustum culling

- GPU-generated indirect draw commands

- Multi-draw indirect (MDI) - [Done]

- Instanced rendering

- Correct compute-to-draw synchronization

- Indirect draw limits and fallback handling

## Engineering and proof

- Clean C++ ownership and RAII

- Vulkan validation layers with no known errors in the tested paths

- GPU and CPU profiling

- Large-scene stress test

- Reproducible builds, README and demo video
