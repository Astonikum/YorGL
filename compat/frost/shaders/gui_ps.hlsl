Texture2D tex : register(t0);
SamplerState samp : register(s0);

struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
    float4 col : COLOR;
};

float4 PSMain(PSInput input) : SV_TARGET {
    float4 texColor = tex.Sample(samp, input.uv);
    return texColor * input.col;
}
