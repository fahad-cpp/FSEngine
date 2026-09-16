struct VSInput {
    float3 pos;
    float3 normal;
    float2 inTexCoord;
};
struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};
ConstantBuffer<UniformBuffer> ubo;
struct VSOutput {
    float4 pos : SV_Position;
    float3 normal;
    float2 fragTexCoord;
};

[shader("vertex")]
VSOutput vertMain(VSInput vertIn) {
    VSOutput output;
    output.pos          = mul(ubo.projection, mul(ubo.view, mul(ubo.model, float4(vertIn.pos, 1.0))));
    output.normal       = vertIn.normal;
    output.fragTexCoord = vertIn.inTexCoord;
    return output;
}
Sampler2D texture;
struct Light {
    float3 direction;
    float  intensity;
};
static Light directionalLight = {
    { 2.f, -2.f, -2.f },
    0.6f
};
static float ambient = 0.2f;
[shader("fragment")]
float4 fragMain(VSOutput fragIn) : SV_TARGET {
    float3 lightDir = directionalLight.direction / abs(length(directionalLight.direction));
    float2 texCoord = fragIn.fragTexCoord;
    float  nDotL    = dot(fragIn.normal, normalize(-1.f * lightDir)) * directionalLight.intensity;
    float  light    = nDotL + ambient;
    return float4(texture.Sample(texCoord).xyz * light, 1.f);
}
