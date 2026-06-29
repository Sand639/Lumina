// Lumina - Triangle shader (Phase 3, DXC / Shader Model 6.0)
// このファイルは実行時に DXC でコンパイルされ、保存するとホットリロードされる。
// 色や座標をいじって保存すると、再ビルドなしで見た目が変わる。

struct VSInput
{
    float3 pos   : POSITION;
    float4 color : COLOR;
};

struct PSInput
{
    float4 pos   : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput o;
    o.pos   = float4(input.pos, 1.0);
    o.color = input.color;
    return o;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
