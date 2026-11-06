
struct VSInput {
    float3 pos;
    float3 normal;
    float2 inTexCoord;
};
struct UniformBuffer {
    float4x4 view;
    float4x4 projection;
    uint32_t numLights;
};
struct EntityData {
    float4x4 model;
    uint textureIndex;
    uint padding0;
    uint padding1;
    uint padding2;
};
enum LightType {
    LT_DIRECTIONAL,
    LT_POINT
};
struct LightData {
    float4 vec;
    float radius;
    float illum;
    uint32_t type;
    float pad;
};
struct VSOutput {
    float4 pos : SV_Position;
    float3 wpos;
    float3 normal;
    float2 fragTexCoord;
    uint textureIndex;
};

ConstantBuffer<UniformBuffer> ubo;

[[vk::binding(1, 0)]]
Sampler2D texture[12];

[[vk::binding(2, 0)]]
StructuredBuffer<EntityData> entityBuffer;

[[vk::binding(3, 0)]]
StructuredBuffer<LightData> lightBuffer;

[shader("vertex")]
VSOutput vertMain(VSInput vertIn, uint baseInstance: SV_StartInstanceLocation) {
    EntityData entity = entityBuffer[baseInstance];
    VSOutput output;
    output.pos          = mul(ubo.projection, mul(ubo.view, mul(entity.model, float4(vertIn.pos, 1.0))));
    output.wpos         = mul(ubo.view, mul(entity.model, float4(vertIn.pos, 1.0))).xyz;
    output.normal       = mul(ubo.view, mul(entity.model, float4(vertIn.normal, 0.0))).xyz;
    output.fragTexCoord = vertIn.inTexCoord;
    output.textureIndex = entity.textureIndex;
    return output;
}
static float ambient    = 0.01f;
static float glossiness = 64.f;
float getPointIllum(float3 normal, float3 point, LightData light) {
    float3 lightPos = mul(ubo.view, light.vec).xyz;
    float3 N        = normalize(normal);
    float3 V        = normalize(-point);
    float3 L        = normalize(lightPos - point);
    float radius    = light.radius;
    float intensity = light.illum;
    float distance  = length(lightPos - point);
    float diffuse   = 0.f;
    if (distance <= radius) {
        float nDotL = dot(N, L);
        diffuse     = clamp(nDotL, 0.f, 1.f);
    }
#if 1
    // Blinn-Phong
    float3 H       = normalize(L + V);
    float nDotH    = max(dot(N, H), 0.0);
    float specular = clamp(pow(nDotH, glossiness), 0.f, 1.f);
#else
    // Phong
    float specular = 0.f;
    float3 R       = normalize(reflect(L, N));
    float vDotR    = max(dot(-V, R), 0.f);
    specular       = clamp(pow(vDotR, glossiness), 0.f, 1.f);
#endif

    float attinuation = clamp(1 - distance / radius, 0.f, 1.f);
    float illum       = (specular + diffuse) * intensity * attinuation;
    illum             = clamp(illum, 0.f, 1.f);
    return illum;
}
float getDirIllum(float3 normal, LightData light) {
    float4 direction       = float4(light.vec.xyz, 0.0);
    float3 N               = normalize(normal);
    float3 lightDir        = normalize(mul(ubo.view, -direction).xyz);
    float nDotL            = max(dot(N, lightDir), 0.f);
    float directionDiffuse = nDotL * light.illum;
    return directionDiffuse;
}
float getLight(float3 normal, float3 point) {
    float illum = 0.f;
    for (int i = 0; i < ubo.numLights; i++) {
        LightData light = lightBuffer[i];
        if (light.type == LightType::LT_POINT) {
            illum += getPointIllum(normal, point, light);
        } else if (light.type == LightType::LT_DIRECTIONAL) {
            illum += getDirIllum(normal, light);
        }
    }
    illum += ambient;
    return illum;
}

[shader("fragment")]
float4 fragMain(VSOutput fragIn) : SV_TARGET {
    float light = getLight(fragIn.normal, fragIn.wpos);
    return float4(texture[fragIn.textureIndex].Sample(fragIn.fragTexCoord).xyz * light, 1.f);
}
