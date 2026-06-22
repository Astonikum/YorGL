cbuffer PerFrame : register(b0) {
    float4x4 projection;
};

struct VSInput {
    float2 pos : POSITION;
    float2 uv  : TEXCOORD;
    float4 col : COLOR;
};

struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
    float4 col : COLOR;
};

PSInput VSMain(VSInput input) {
    PSInput output;
    output.pos = mul(projection, float4(input.pos, 0.0, 1.0));
    output.uv = input.uv;
    output.col = input.col;
    return output;
}
