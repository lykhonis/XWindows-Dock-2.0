float delta : register(C0);
sampler2D implicitInputSampler : register(S0);
float4 main(float2 uv : TEXCOORD) : COLOR
{
   float4 c = tex2D(implicitInputSampler, uv);
   c.r *= delta;
   c.b *= delta;
   c.g *= delta;
   return c;
}