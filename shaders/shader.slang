struct VSInput {
    float2 pos;
    float3 color;
};
struct VSOutput {
    float3 color;
    float4 sv_position : SV_Position;
};

[shader("vertex")]
VSOutput vertMain(VSInput vertIn) {
    VSOutput output;
    output.sv_position = float4(vertIn.pos, 0.0, 1.0);
    output.color = vertIn.color;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput inVert) {
    float3 color = inVert.color;
    return float4(color, 1.0);
}