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


/** 

  Random number generator (hash function)

  Sylvain Lefebvre 2004-07-03

*/


#define M_PI 3.14159265358979323846

#define fmodp(x,n) ((n)*frac((x)/(n)))

//-----------------------------------------------------------------------------

float2 old_rand(float2 ij,float2 n)
{
  float2 xy0 = floor((ij+n)*2000.0)/256.0;
  float2 xym = frac(xy0/257.0)*257.0 + 1.0;
  float2 xym2= frac(xym*xym);
  float2 pxy = xym2.xy * xym.yx;
  float2 xy1 = xy0 + pxy.xy + pxy.yx;
  return (xy1);
}

  
//-----------------------------------------------------------------------------


float2 our_rand_cpu(float2 ij)
{
  float2 xy0 = ij/M_PI;
  float2 xym = fmodp(xy0,257.0) + 1.0;
  float2 xym2= frac(xym*xym);
  float2 pxy = xym2.xy * xym.yx;
  float2 xy1 = xy0 + pxy.xy + pxy.xx;
  return (frac(xy1));
}


//-----------------------------------------------------------------------------


float2 our_rand(float2 ij)
{
  const float4 a=float4(M_PI * M_PI * M_PI * M_PI, exp(4.0),  1.0, 0.0);
  const float4 b=float4(pow(13.0, M_PI / 2.0), sqrt(1997.0),  0.0, 1.0);

  float2 xy0    = ij/M_PI;
  float2 xym    = fmodp(xy0.xy,257.0)+1.0;
  float2 xym2   = frac(xym*xym);
  float4 pxy    = float4(xym.yx * xym2 , frac(xy0));
  float2 xy1    = float2(dot(pxy,a) , dot(pxy,b));
  float2 result = frac(xy1);
  
  return (result);
}


//-----------------------------------------------------------------------------


float2 mccool_rand(float2 ij)
{
  const float4 a=float4(M_PI * M_PI * M_PI * M_PI, exp(4.0), pow(13.0, M_PI / 2.0), sqrt(1997.0));
  float4 result =float4(ij,ij);

  for(int i = 0; i < 3; i++) 
  {
      result.x = frac(dot(result, a));
      result.y = frac(dot(result, a));
      result.z = frac(dot(result, a));
      result.w = frac(dot(result, a));
  }

  return (result.xy*256.0);
}




//-----------------------------------------------------------------------------

// select a rand function

float2 rand(float2 ij,float2 n)
{
//  return (our_rand_cpu(ij));
  return (our_rand(ij));
//  return (permut_rand(ij));
}


//-----------------------------------------------------------------------------


float4 PostProcessPS( float2 Tex : TEXCOORD0 ) : COLOR
{ 
	
	Tex.y = Tex.y + ftime;

    float4 Color = tex2D( g_scrSampler, Tex.xy);  

    Color.a = 1.0f;


Color.r = rand(0,0); 
Color.g = rand(Tex.xy+ftime, ftime);
Color.b = rand(Tex.xy, 0);


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
