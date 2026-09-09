struct VSInput {
    float3 pos;
    float3 color;
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
    float3 color;
    float2 fragTexCoord;
};

[shader("vertex")]
VSOutput vertMain(VSInput vertIn) {
    VSOutput output;
    output.pos = mul(ubo.projection, mul(ubo.view, mul(ubo.model, float4(vertIn.pos, 1.0))));
    output.color = vertIn.color;
    output.fragTexCoord = vertIn.inTexCoord;
    return output;
}
Sampler2D texture;
[shader("fragment")]
float4 fragMain(VSOutput inVert) : SV_TARGET{
    float3 color = inVert.color;
    float2 texCoord = inVert.fragTexCoord;
    return texture.Sample(texCoord);
}