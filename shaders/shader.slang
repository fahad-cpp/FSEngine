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
    output.pos = mul(ubo.projection, mul(ubo.view, mul(ubo.model, float4(vertIn.pos, 1.0))));
    output.normal = vertIn.normal;
    output.fragTexCoord = vertIn.inTexCoord;
    return output;
}
Sampler2D texture;
[shader("fragment")]
float4 fragMain(VSOutput fragIn) : SV_TARGET{
    float2 texCoord = fragIn.fragTexCoord;
    return float4(fragIn.normal,1.0);
}