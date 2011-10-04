struct VS_STRUCT
{
    float4 Position	: POSITION;   // vertex position 
    float2 Uv		: TEXCOORD0;   // vertex diffuse color (note that COLOR0 is clamped from 0..1)
};

VS_STRUCT main(VS_STRUCT In)
{
	VS_STRUCT Output;
	Output.Uv	= In.Uv;
	Output.Position = In.Position;
	return Output;    
}
