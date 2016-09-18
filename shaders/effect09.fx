float4x4 worldview; 
float4x4 wv_inv_trans;
float4x4 world;

float4x4 zw_rotation;  
float4x4 yw_rotation;  
float4x4 xw_rotation;  

float3 view_eye;
float3 light_direction;
texture mesh_texture;
float ftime;



float4 diffuse = { 0.8f, 0.5f, 0.5f, 1.0f};
float4 ambient = { 0.4f, 0.8f, 0.4f, 1.0f }; 
float4 specular = { 0.4f, 0.4f, 0.8f, 1.0f }; 
    
sampler2D screen;


sampler outline;
sampler TextureSample = sampler_state
{
    Texture = <mesh_texture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};



struct VS_OUTPUT
{
    float4 pos : POSITION;
    float3 normal : TEXCOORD0;
    float2 uv : TEXCOORD1;
    float3 light : TEXCOORD2;
    float3 view : TEXCOORD3;
    float3 pos2 : TEXCOORD4;
};


VS_OUTPUT VS(
    float4 pos : POSITION, 
    float3 normal : NORMAL, 
    float2 uv : TEXCOORD0)
{
    VS_OUTPUT vout = (VS_OUTPUT)0;
    
    vout.pos = mul(pos, zw_rotation);
    vout.pos = mul(vout.pos, yw_rotation);
    vout.pos = mul(vout.pos, xw_rotation);
    
    vout.pos = mul(vout.pos, worldview);
    vout.light = light_direction;
    vout.normal = normalize(mul(normal, world));
    vout.uv = uv; 
    vout.view = view_eye - normalize(mul(pos, world));
    
    vout.pos2 = vout.pos;
    return vout;
}


float4 PS(
    float3 norm : TEXCOORD0,
    float2 uv : TEXCOORD1,
    float3 light: TEXCOORD2, 
    float3 view : TEXCOORD3,
    float3 pos2 : TEXCOORD4) : COLOR
{
    // Phong Lighting from Gamasutra
    
    float3 normy = norm;
    float4 diff = saturate(dot(light, normy));

    // R = 2 * (N.L) * N - L
    float3 reflecto = normalize(2 * dot(light, normy) * normy - light_direction);


    // R.V^n    
    float4 specular = saturate(pow(saturate(dot(reflecto, view)), 8));
    
    float4 tuv = tex2D(TextureSample, uv);
    float chya = dot(normalize(light), normy) * 0.5 + 0.5;
    float4 color = tuv * chya + specular;
    
    return color;
}



technique RenderSceneCHya
{
    pass P0
    {          
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


technique RenderSceneNoTexture
{
    pass P0
    {          
        VertexShader = null;
        PixelShader  = null;
    }
}

texture g_scrTexture;

sampler2D g_scrSampler =
sampler_state
{
    Texture = <g_scrTexture>;
    AddressU = Wrap;
    AddressV = Wrap;
    MinFilter = Point;
    MagFilter = Linear;
    MipFilter = Linear;
};




float4 PostProcessPS( float2 Tex : TEXCOORD0 ) : COLOR
{ 
	Tex.y = Tex.y + (sin(Tex.x*200)*0.01);

    float4 Color = tex2D( g_scrSampler, Tex.xy);  

    Color.a = 1.0f;

	Color.r = Color.r*sin(Tex.y*100)*2;
	Color.g = Color.g*cos(Tex.y*200)*2;
	Color.b = Color.b*sin(Tex.y*300)*2;

    return Color;
}


technique RenderPostProcess
{
    pass p0
    {
        VertexShader = null;
        PixelShader = compile ps_2_0 PostProcessPS();
        ZEnable = false;
    }
}
