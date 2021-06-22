//**************************************************************//
//  Effect File exported by RenderMonkey 1.6
//
//  - Although many improvements were made to RenderMonkey FX  
//    file export, there are still situations that may cause   
//    compilation problems once the file is exported, such as  
//    occasional naming conflicts for methods, since FX format 
//    does not support any notions of name spaces. You need to 
//    try to create workspaces in such a way as to minimize    
//    potential naming conflicts on export.                    
//    
//  - Note that to minimize resulting name collisions in the FX 
//    file, RenderMonkey will mangle names for passes, shaders  
//    and function names as necessary to reduce name conflicts. 
//**************************************************************//

//--------------------------------------------------------------//
// EdgeDetection
//--------------------------------------------------------------//
//--------------------------------------------------------------//
// Emboss
//--------------------------------------------------------------//
string EdgeDetection_Emboss_ScreenAlignedQuad : ModelData = "..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Models\\ScreenAlignedQuad.3ds";

float4x4 matViewProjection;

struct VS_INPUT 
{
   float4 mPosition : POSITION0;
   float2 mUV : TEXCOORD0;
};

struct VS_OUTPUT 
{
   float4 mPosition : POSITION0;
   float2 mUV : TEXCOORD0;
};

VS_OUTPUT EdgeDetection_Emboss_Vertex_Shader_vs_main( VS_INPUT Input )
{
   VS_OUTPUT Output;

   Output.mPosition = Input.mPosition;
   Output.mUV = Input.mUV;
   
   return( Output );
   
}




struct PS_INPUT
{
   float2 mUV : TEXCOORD0;
};

texture SceneTexture_Tex
<
   string ResourceName = ".\\";
>;
sampler2D SceneSampler = sampler_state
{
   Texture = (SceneTexture_Tex);
};

float3x3 K = 
{
-2,-1, 0,
-1, 0, 1,
 0, 1, 2
};

float2 gPixelOffset : ViewportDimensionsInverse;

float4 EdgeDetection_Emboss_Pixel_Shader_ps_main(PS_INPUT Input) : COLOR0
{  
   float Res = 0;
   for(int y = -1; y <= 1; ++y)
   {
      for(int x = -1; x <= 1; ++x)
      {
         float2 offset = float2(x, y) * gPixelOffset;
         float3 tex = tex2D(SceneSampler, Input.mUV + offset).rgb;
         float luminance = dot(tex, float3(0.3, 0.59, 0.11));
   
         Res += luminance * K[y+1][x+1];
      }
   }
   
   Res += 0.5f;
   
   return float4(Res.xxx, 1);
}








//--------------------------------------------------------------//
// Technique Section for EdgeDetection
//--------------------------------------------------------------//
technique EdgeDetection
{
   pass Emboss
   {
      CULLMODE = NONE;

      VertexShader = compile vs_2_0 EdgeDetection_Emboss_Vertex_Shader_vs_main();
      PixelShader = compile ps_2_0 EdgeDetection_Emboss_Pixel_Shader_ps_main();
   }

}

