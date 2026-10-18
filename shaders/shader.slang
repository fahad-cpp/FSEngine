
struct VSInput {
    float3 pos;
    float3 normal;
    float2 inTexCoord;
};
struct UniformBuffer {
    float4x4 view;
    float4x4 projection;
};
ConstantBuffer<UniformBuffer> ubo;

struct EntityData {
    float4x4 model;
    uint textureIndex;
    uint padding0;
    uint padding1;
    uint padding2;
};
Sampler2D texture[];
StructuredBuffer<EntityData> entityBuffer;
struct VSOutput {
    float4 pos : SV_Position;
    float3 wpos;
    float3 normal;
    float2 fragTexCoord;
    uint textureIndex;
};

[shader("vertex")]
VSOutput vertMain(VSInput vertIn,uint instanceID : SV_InstanceID) {
    EntityData entity = entityBuffer[instanceID];
    VSOutput output;
    output.pos          = mul(ubo.projection, mul(ubo.view, mul(entity.model, float4(vertIn.pos, 1.0))));
    output.wpos         = mul(ubo.view, mul(entity.model, float4(vertIn.pos, 1.0))).xyz;
    output.normal       = mul(ubo.view, mul(entity.model, float4(vertIn.normal, 0.0))).xyz;
    output.fragTexCoord = vertIn.inTexCoord;
    output.textureIndex = entity.textureIndex;
    return output;
}
struct DirectionalLight {
    float3 direction;
    float  intensity;
};
struct PointLight {
    float3 position;
    float  radius;
    float  intensity;
};
static DirectionalLight directionalLight = {
    { -1.f, -1.f, 1.f },
    0.3f
};

static PointLight pointLight = {
    { 0.f, 1.f, 0.f },
    40.f,
    0.4f
};
static float ambient    = 0.01f;
static float glossiness = 64.f;
float        getPointIllum(float3 normal, float3 point, PointLight pLight) {
    float3 lightPos = mul(ubo.view, float4(pLight.position, 1.0)).xyz;
    float3 N        = normalize(normal);
    float3 V        = normalize(-point);
    float3 L        = normalize(lightPos - point);
    float  distance = length(lightPos - point);
    float  diffuse  = 0.f;
    if (distance <= pLight.radius) {
        float nDotL = dot(N, L);
        diffuse     = clamp(nDotL, 0.f, 1.f);
    }
#if 1
    // Blinn-Phong
    float3 H        = normalize(L + V);
    float  nDotH    = max(dot(N, H), 0.0);
    float  specular = clamp(pow(nDotH, glossiness), 0.f, 1.f);
#else
    // Phong
    float  specular = 0.f;
    float3 R        = normalize(reflect(L, N));
    float  vDotR    = max(dot(-V, R), 0.f);
    specular        = clamp(pow(vDotR, glossiness), 0.f, 1.f);
#endif

    float attinuation = clamp(1 - distance / pLight.radius, 0.f, 1.f);
    float illum       = (specular + diffuse) * pLight.intensity * attinuation;
    illum             = clamp(illum, 0.f, 1.f);
    return illum;
}
float getLight(float3 normal, float3 point, DirectionalLight dLight, PointLight pLight) {

    float  pointIllum       = getPointIllum(normal, point, pLight);
    float  dirIllum         = 0.f;
    float3 N                = normalize(normal);
    float3 lightDir         = normalize(mul(ubo.view, float4(-dLight.direction,0.0)).xyz);
    float  nDotL            = max(dot(N, lightDir), 0.f);
    float  directionDiffuse = nDotL * dLight.intensity;
    dirIllum                = directionDiffuse;

    return (pointIllum + dirIllum + ambient);
}
[shader("fragment")]
float4 fragMain(VSOutput fragIn) : SV_TARGET {
    float light = getLight(fragIn.normal, fragIn.wpos, directionalLight, pointLight);

    return float4(texture[fragIn.textureIndex].Sample(fragIn.fragTexCoord).xyz * light, 1.f);
    // return float4(fragIn.wpos, 1.f);
}
