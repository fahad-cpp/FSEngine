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
    output.normal       = mul(ubo.model, float4(vertIn.normal, 0.0)).xyz;
    output.fragTexCoord = vertIn.inTexCoord;
    return output;
}
Sampler2D texture;
struct Light {
    float3 direction;
    float  intensity;
};
static Light directionalLight = {
    { -1.f, -1.f, 1.f },
    0.6f
};
static float ambient = 0.3f;
[shader("fragment")]
float4 fragMain(VSOutput fragIn) : SV_TARGET {
    float3 N       = normalize(fragIn.normal);
    float3 L       = normalize(-directionalLight.direction);
    float  nDotL   = dot(N, L);
    float  diffuse = nDotL * directionalLight.intensity;
    float  light   = diffuse + ambient;

    return float4(texture.Sample(fragIn.fragTexCoord).xyz * light, 1.f);
}
