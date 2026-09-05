struct VSInput {
    float3 pos;
    float3 color;
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
};

[shader("vertex")]
VSOutput vertMain(VSInput vertIn) {
    VSOutput output;
    output.pos = mul(ubo.projection, mul(ubo.view, mul(ubo.model, float4(vertIn.pos, 1.0))));
    output.color = vertIn.color;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput inVert) : SV_TARGET{
    float3 color = inVert.color;
    return float4(color, 1.0);
}